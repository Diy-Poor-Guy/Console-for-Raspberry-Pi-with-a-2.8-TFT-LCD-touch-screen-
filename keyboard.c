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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <linux/input.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>
#include "wiringPi.h"
#include <unistd.h>

#include "touch.h"
#include "TftLcd.h"
#include "keyboard.h"

#define KEY_WIDTH     24
#define SPACE_WIDTH   104
#define KEY_HEIGHT    17

#define LETTER_ROW_CNT    4
#define LETTER_COL_CNT    12
#define LETTER_EXTRA_CNT  6

extern int Debug;
extern void SendToBash(char c);
extern void TogleCursor(void);

int Mode = 0;

char *KeybdLower[] = {
   "1234567890-=",
   "qwertyuiop[]",
   "asdfghjkl;'\x085",   // Alt 133 = 
   "zxcvbnm,./\\\x083",  // Alt 131 KEYBD_UP         
};
char *KeybdUpper[] = {
   "!@#$%^&*()_+",
   "QWERTYUIOP{}",
   "ASDFGHJKL:\"\x085",  // Alt 133 = 
   "ZXCVBNM<>?`\x084",   // Alt 132 KEYBD_DOWN       
};

typedef struct LetterButtons{
  char Name;
  int X;
  int Y;
  int LastTime;
}LetterButtons;
          
struct LetterButtons LetterButton[LETTER_ROW_CNT][LETTER_COL_CNT];
struct LetterButtons LetterExtras[LETTER_EXTRA_CNT];

void createButton(int x, int y, int w, int h, char *text, 
                   int backgroundColor, int foregroundColor)
{
  char *p = text;
  int length = 0;

  //get length of the string *text
  while(*(p+length))
          length++;
  if((length*8)> (w-2))
  {
    printf("####error,button too small for text####\n");
    exit(1);
  }
  //Draw button foreground 
  drawSquare(x,y,w,h,foregroundColor);
  //Place text on the button.  Try and center it in a primitive way
  put_string(x+((w-(length*8))/2), y+((h-Font->Height)/2)-1,text, WHITE);
}

void DrawKeyboard(int Upper)
{
  char KeyExtra[3];
  int x,y;
  char Name[5];
  
  for(y = 0; y < LETTER_ROW_CNT; y++)
  {
    for(x = 0; x < LETTER_COL_CNT; x++)
    {
      if(Upper)
      {
        sprintf(Name, "%c", KeybdUpper[y][x]);
        LetterButton[y][x].Name = KeybdUpper[y][x];
      }
      else
      {
        sprintf(Name, "%c", KeybdLower[y][x]);
        LetterButton[y][x].Name = KeybdLower[y][x];
      }
      LetterButton[y][x].X = 2 + y * 3 + x * 26;
      LetterButton[y][x].Y = 145 + y * (KEY_HEIGHT + 2);
      createButton(LetterButton[y][x].X, LetterButton[y][x].Y,
            KEY_WIDTH, KEY_HEIGHT,
            Name, BLACK, BLUE);
      LetterButton[y][x].LastTime  = 0;
    }
  }
  LetterExtras[0].Name = KEYBD_BACKSPACE; 
  LetterExtras[0].X = 290;
  LetterExtras[0].Y = 222; 
  KeyExtra[0] = KEYBD_BACKSPACE;
  KeyExtra[1] = 0;
  createButton( LetterExtras[0].X, LetterExtras[0].Y, 
             KEY_WIDTH, KEY_HEIGHT,
             &KeyExtra[0], BLACK, BLUE);
  LetterExtras[1].Name = KEYBD_LEFT; 
  LetterExtras[1].X = 230;
  LetterExtras[1].Y = 222; 
  KeyExtra[0] = KEYBD_LEFT;
  createButton( LetterExtras[1].X, LetterExtras[1].Y, 
             KEY_WIDTH, KEY_HEIGHT,
             &KeyExtra[0], BLACK, BLUE);
  LetterExtras[2].Name = KEYBD_RIGHT; 
  LetterExtras[2].X = 255;
  LetterExtras[2].Y = 222; 
  KeyExtra[0] = KEYBD_RIGHT;
  createButton( LetterExtras[2].X, LetterExtras[2].Y, 
             KEY_WIDTH, KEY_HEIGHT,
             &KeyExtra[0], BLACK, BLUE);
  LetterExtras[3].Name = ' ';              //   SPACE 
  LetterExtras[3].X = 100;
  LetterExtras[3].Y = 222; 
  KeyExtra[0] = ' ';
  createButton( LetterExtras[3].X, LetterExtras[3].Y, 
             SPACE_WIDTH, KEY_HEIGHT,
             &KeyExtra[0], BLACK, BLUE);
  LetterExtras[4].Name = KEYBD_CTL; 
  LetterExtras[4].X = 25;
  LetterExtras[4].Y = 222; 
  KeyExtra[0] = KEYBD_CTL;
  createButton( LetterExtras[4].X, LetterExtras[4].Y, 
             KEY_WIDTH, KEY_HEIGHT,
             &KeyExtra[0], BLACK, BLUE);
  LetterExtras[5].Name = KEYBD_ALT; 
  LetterExtras[5].X = 55;
  LetterExtras[5].Y = 222; 
  KeyExtra[0] = KEYBD_ALT;
  createButton( LetterExtras[5].X, LetterExtras[5].Y, 
             KEY_WIDTH, KEY_HEIGHT,
             &KeyExtra[0], BLACK, BLUE);
}

void HiglighButton(int x, int y, int w, int h, int borderColor)
{
  //Draw button outline
  drawBox(x-2,y-2,w+1,h+1,borderColor);
}

int mymillis()
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec) * 1000 + (tv.tv_usec)/1000;
}

void KeybdEdit(void)
{
  int x,y, width;
  int rawPressure, scaledX, scaledY;
  int prevX = 0, prevY = 0, dX, dY, millis;
  int Upper = 0;
  int CursorLastTime;  

  DrawKeyboard(Upper);
  CursorLastTime  = mymillis();
  if(Debug > 1) printf("In Keyboard\n");
  while(1)
  {
    usleep(250000);
    if(getTouchSample(&scaledX, &scaledY, &rawPressure) != 0)
    {
      if(Debug > 5) printf("X = %3d Y = %3d\n", scaledX, scaledY);
      dX = prevX - scaledX; 
      dY = prevY - scaledY;
      if(!(((dX < 5) && (dX > -5)) && ((dY < 5) && (dY > -5)) && ((mymillis() - millis) < 550)))
      {
        millis = mymillis();
        prevX = scaledX;
        prevY = scaledY;
  
        //put_pixel_16bpp(scaledX, scaledY, 255,255,255);
        for(y = 0; y < LETTER_ROW_CNT; y++)
        {
          for(x = 0; x < LETTER_COL_CNT; x++)
          {
            //See if the results retuned by the touch event fall within the coordinates of the button
            if((scaledX  > LetterButton[y][x].X && scaledX < 
                          (LetterButton[y][x].X + KEY_WIDTH)) && 
                (scaledY > LetterButton[y][x].Y && scaledY < 
                          (LetterButton[y][x].Y + KEY_HEIGHT)))
            {
              HiglighButton(
                    LetterButton[y][x].X,
                    LetterButton[y][x].Y,
                    KEY_WIDTH, KEY_HEIGHT,
                    WHITE);
              LetterButton[y][x].LastTime = mymillis();
              if(LetterButton[y][x].Name == KEYBD_RETURN)
              {
                SendToBash('\n');
              }
              else if(LetterButton[y][x].Name == KEYBD_UP)   //131
              {
                if(Debug > 1)printf("Shift UP\n");
                Upper = 1;
                LetterButton[y][x].LastTime = 0;
                HiglighButton(
                    LetterButton[y][x].X,
                    LetterButton[y][x].Y,
                    KEY_WIDTH, KEY_HEIGHT,
                    BLACK);
                DrawKeyboard(Upper);
              }
              else if(LetterButton[y][x].Name == KEYBD_DOWN) // 132
              {
                if(Debug > 1)printf("Shift DOWN\n");
                Upper = 0;
                LetterButton[y][x].LastTime = 0;
                HiglighButton(
                    LetterButton[y][x].X,
                    LetterButton[y][x].Y,
                    KEY_WIDTH, KEY_HEIGHT,
                    BLACK);
                DrawKeyboard(Upper);
              }
              else
              {
                switch(Mode)
                {
                  case KEYBD_CTL:
                    if(LetterButton[y][x].Name >= 'a')
                      SendToBash(LetterButton[y][x].Name - 'a');
                    else if(LetterButton[y][x].Name >= 'A')
                      SendToBash(LetterButton[y][x].Name - 'A');
                    HiglighButton(
                      LetterExtras[4].X,
                      LetterExtras[4].Y,
                      KEY_WIDTH, KEY_HEIGHT,
                      BLACK);
                    break;

                  case KEYBD_ALT:
                    SendToBash(LetterButton[y][x].Name + 128);
                    HiglighButton(
                      LetterExtras[5].X,
                      LetterExtras[5].Y,
                      KEY_WIDTH, KEY_HEIGHT,
                      BLACK);
                    break;

                  default :
                    SendToBash(LetterButton[y][x].Name);
                    break;
                }
                Mode = 0;
              }
            }
          }
        }
        for(x = 0; x < LETTER_EXTRA_CNT; x++)
        {
          if(LetterExtras[x].Name == ' ')
            width = SPACE_WIDTH;
          else
            width = KEY_WIDTH;
          //See if the results retuned by the touch event fall within the coordinates of the button
          if((scaledX  > LetterExtras[x].X && scaledX < 
                        (LetterExtras[x].X + width)) && 
              (scaledY > LetterExtras[x].Y && scaledY < 
                        (LetterExtras[x].Y + KEY_HEIGHT)))
          {
            switch(LetterExtras[x].Name)
            {
              case KEYBD_BACKSPACE  :  // 134
                HiglighButton(
                  LetterExtras[x].X,
                  LetterExtras[x].Y,
                  KEY_WIDTH, KEY_HEIGHT,
                  WHITE);
                LetterExtras[x].LastTime = mymillis();
                SendToBash('\b');
                break;
                
              case KEYBD_LEFT       :  // 135
              case KEYBD_RIGHT      :  // 136
                HiglighButton(
                  LetterExtras[x].X,
                  LetterExtras[x].Y,
                  KEY_WIDTH, KEY_HEIGHT,
                  WHITE);
                LetterExtras[x].LastTime = mymillis();
                SendToBash(LetterExtras[x].Name);
                break;
                
              case KEYBD_ALT        :  // 129
                if(Mode == 0)
                {
                  HiglighButton(
                    LetterExtras[x].X,
                    LetterExtras[x].Y,
                    KEY_WIDTH, KEY_HEIGHT,
                    WHITE);
                  Mode = KEYBD_ALT;
                }
                else
                {
                  if(Mode == KEYBD_ALT)
                  {
                    HiglighButton(
                      LetterExtras[x].X,
                      LetterExtras[x].Y,
                      KEY_WIDTH, KEY_HEIGHT,
                      BLACK);
                    Mode = 0;
                  }
                }
                break;
                
              case KEYBD_CTL        :  // 130
                if(Mode == 0)
                {
                  HiglighButton(
                    LetterExtras[x].X,
                    LetterExtras[x].Y,
                    KEY_WIDTH, KEY_HEIGHT,
                    WHITE);
                  Mode = KEYBD_CTL;
                }
                else
                {
                  if(Mode == KEYBD_CTL)
                  {
                    HiglighButton(
                      LetterExtras[x].X,
                      LetterExtras[x].Y,
                      KEY_WIDTH, KEY_HEIGHT,
                      BLACK);
                    Mode = 0;
                  }
                }
                break;
                
              case ' ' :  
                HiglighButton(
                  LetterExtras[x].X,
                  LetterExtras[x].Y,
                  SPACE_WIDTH, KEY_HEIGHT,
                  WHITE);
                LetterExtras[x].LastTime = mymillis();
                SendToBash(' ');
                break;
            }
          }
        }
      }
    }
    for(y = 0; y < LETTER_ROW_CNT; y++)
    {
      for(x = 0; x < LETTER_COL_CNT; x++)
      {
        if(LetterButton[y][x].LastTime)
        {
          if((mymillis() - LetterButton[y][x].LastTime) > 250)
          {
            LetterButton[y][x].LastTime = 0;
              HiglighButton(
                  LetterButton[y][x].X,
                  LetterButton[y][x].Y,
                  KEY_WIDTH, KEY_HEIGHT,
                  BLACK);
          }
        }
      }
    }
    for(x = 0; x < LETTER_EXTRA_CNT; x++)
    {
      if(LetterExtras[x].LastTime)
      {
        if((mymillis() - LetterExtras[x].LastTime) > 250)
        {
          LetterExtras[x].LastTime = 0;
          if(LetterExtras[x].Name == ' ')
          {
            HiglighButton(
                LetterExtras[x].X,
                LetterExtras[x].Y,
                SPACE_WIDTH, KEY_HEIGHT,
                BLACK);
          }
          else
          {
            HiglighButton(
                LetterExtras[x].X,
                LetterExtras[x].Y,
                KEY_WIDTH, KEY_HEIGHT,
                BLACK);
          }
        }
      }
    }
    if((mymillis() - CursorLastTime) > 1000)
    {
      CursorLastTime  = mymillis();
      if(Debug > 19 ) printf("Toggle cursor\n");
      TogleCursor();
    }
  }
}

