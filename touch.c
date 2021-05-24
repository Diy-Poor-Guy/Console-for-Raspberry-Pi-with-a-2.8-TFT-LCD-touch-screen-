/*
    Copyright (C) 2021 The whole Universe
    
    Derived from work done in 2013 by Mark Williams

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
    MA 02111-1307, USA
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <time.h>

#include "wiringPi.h"
#include "console.h"

#define FROM_TOUCH_C
#include "touch.h"
#undef FROM_TOUCH_C

#define KWHT  "\x1B[37m"
#define KYEL  "\x1B[33m"

int InputEvFd;
char EventNameStr[22];

int openTouchScreen(char * Name, int *screenXmin,int *screenXmax,int *screenYmin,int *screenYmax)
{
  int i;

  for(i = 0; i < 10; i++)
  {
    if(DeviceNumber != -1)
      i = DeviceNumber;
    sprintf(EventNameStr, "/dev/input/event%d", i); 
    if(DeviceListMode || (Debug > 11))printf("   Open %s\n", EventNameStr);
    if((InputEvFd = open(EventNameStr, O_RDONLY)) < 0) 
    {
      continue;
    }
    if(getTouchScreenDetails(Name, screenXmin, screenXmax, screenYmin, screenYmax) == 1)
    {
      if(fcntl(InputEvFd , F_SETFL, O_NONBLOCK) < 0)
      {
        if(Debug > 11)printf("Fail on NON BLOCKING\n");
        exit(2);
      }
      return(1);
    }
    close(InputEvFd);
    if(DeviceNumber != -1)
      return(0);
  }
  return(0);
}


int getTouchScreenDetails(char * Name, int *screenXmin,int *screenXmax,int *screenYmin,int *screenYmax)
{
  unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
  char name[88] = "Unknown";
  int abs[6] = {0};

  ioctl(InputEvFd, EVIOCGNAME(sizeof(name)), name);
  if(DeviceListMode || (Debug > 11))printf("      Input device name: \"%s\"\n", name);

  memset(bit, 0, sizeof(bit));
  ioctl(InputEvFd, EVIOCGBIT(0, EV_MAX), bit[0]);
  if(Debug > 11)printf("Supported events:\n");

  int i,j,k;
  for (i = 0; i < EV_MAX; i++)
  {
    if (test_bit(i, bit[0])) 
    {
      if(Debug > 11)printf("  Event type %d (%s)\n", i, events[i] ? events[i] : "?");
      if (!i) continue;
      ioctl(InputEvFd, EVIOCGBIT(i, KEY_MAX), bit[i]);
      for (j = 0; j < KEY_MAX; j++)
      {
        if (test_bit(j, bit[i])) 
        {
          if(Debug > 11)printf("    Event code %d (%s)\n", j, names[i] ? (names[i][j] ? names[i][j] : "?") : "?");
          if (i == EV_ABS) 
          {
            ioctl(InputEvFd, EVIOCGABS(j), abs);
            for (k = 0; k < 5; k++)
              if ((k < 3) || abs[k])
              {
                if(Debug > 11)printf("     %s %6d\n", absval[k], abs[k]);
                if (j == 0)
                {
                  if (!strcmp(absval[k], "Min  ")) *screenXmin =  abs[k];
                  if (!strcmp(absval[k], "Max  ")) *screenXmax =  abs[k];
                }
                if (j == 1)
                {
                  if (!strcmp(absval[k], "Min  ")) *screenYmin =  abs[k];
                  if (!strcmp(absval[k], "Max  ")) *screenYmax =  abs[k];
                }
              }
            }

          }
        }
      }
  }
  if(DeviceListMode == 0)
  {
    if(!strcmp(Name, name))
      return(1);
  }
  return(0);
}


int getTouchSample(int *scaledX, int *scaledY, int *rawPressure)
{
  int i;
  /* how many bytes were read */
  int rb;
  /* the events (up to 64 at once) */
  struct input_event ev[64];
  int rawX, rawY;

  if(Debug > 20) printf("Reading from FD%d = %s\n", InputEvFd, EventNameStr);
  rb = read(InputEvFd,ev,sizeof(struct input_event)*64);
  if(Debug > 20) printf("Read %d\n", rb);
  if(rb < 1)
  {
    usleep(100);
    return(0);
  }
  for (i = 0;  i <  (rb / sizeof(struct input_event)); i++)
  {
    if((ev[i].type) ==  EV_SYN){
      if(Debug > 10) printf("Event type is %s%s%s = Start of New Event\n",KYEL,events[ev[i].type],KWHT);
    }
    else if (ev[i].type == EV_KEY && ev[i].code == 330 && ev[i].value == 1)
    {
      if(Debug > 10) printf("Event type is %s%s%s & Event code is %sTOUCH(330)%s & Event value is %s1%s = Touch Starting\n", KYEL,events[ev[i].type],KWHT,KYEL,KWHT,KYEL,KWHT);
    }
    else if (ev[i].type == EV_KEY && ev[i].code == 330 && ev[i].value == 0)
    {
      if(Debug > 10) printf("Event type is %s%s%s & Event code is %sTOUCH(330)%s & Event value is %s0%s = Touch Finished\n", KYEL,events[ev[i].type],KWHT,KYEL,KWHT,KYEL,KWHT);
    }
    else if (ev[i].type == EV_ABS && ev[i].code == 1 && ev[i].value > 0)
    {
      if(Debug > 11) printf("Event type is %s%s%s & Event code is %sX(0)%s & Event value is %s%d%s\n", 
                  KYEL,events[ev[i].type],KWHT,KYEL,KWHT,KYEL,ev[i].value,KWHT);
      rawX = ev[i].value;
      *scaledX = rawX/(scaleXvalue - 2.1) - 20;
      if(*scaledX < 0)
        *scaledX = 0;
      if(*scaledX >= ScreenXres)
        *scaledX = ScreenXres - 1;
    }
    else if (ev[i].type == EV_ABS  && ev[i].code == 0 && ev[i].value > 0)
    {
      if(Debug > 11) printf("Event type is %s%s%s & Event code is %sY(1)%s & Event value is %s%d%s\n", 
                   KYEL,events[ev[i].type],KWHT,KYEL,KWHT,KYEL,ev[i].value,KWHT);
      rawY = screenYmax  - ev[i].value;
      *scaledY = rawY/(scaleYvalue - 2.1) - 20;
      if(*scaledY < 0)
        *scaledY = 0;
      if(*scaledY >= ScreenYres)
        *scaledY = ScreenYres - 1;
    }
    else if (ev[i].type == EV_ABS  && ev[i].code == 24 && ev[i].value > 0)
    {
      if(Debug > 10) printf("Event type is %s%s%s & Event code is %sPressure(24)%s & Event value is %s%d%s\n", KYEL,events[ev[i].type],KWHT,KYEL,KWHT,KYEL,ev[i].value,KWHT);
      *rawPressure = ev[i].value;
    }
  }
  return(1);
}
