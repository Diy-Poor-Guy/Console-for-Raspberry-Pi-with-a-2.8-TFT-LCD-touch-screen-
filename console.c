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
#include <unistd.h>
#include <signal.h>
#include <unistd.h> 
#include <errno.h> 
#include <time.h> 
#include <ctype.h> 
#include <sys/time.h>
#include <string.h> 
#include <pthread.h>
#include <semaphore.h>

#include "touch.h"
#include "TftLcd.h"
#include "keyboard.h"
#include "console.h"

#define PIPE_READ  0 
#define PIPE_WRITE 1 

#define MARGIN     5 
#define CH_WIDTH   9 

#define VERSION    "0.1" 
 
int Debug = 0;
sem_t CurMutex;
int aStdinPipe[2];
int aStdoutPipe[2]; 
int ScrRow = 0, ScrColl = 0;
int CurOn          = 0;
int TimerCount     = 0;
int DeviceNumber   = -1;
int FbDeviceNumber = -1;
int DeviceListMode = 0;
int FbDeviceListMode = 0;
int FontNo         = 0;

int ScrLines = 15;  // 15 for 8X8 fornt = 120 pixels

void ScrollUp(void)
{
  ScrollSubScr(MARGIN, MARGIN + ScrLines * (Font->Height + 1));
}

void TogleCursor(void)
{
  sem_wait(&CurMutex);
  if(Debug > 15) printf("Semphore 1\n");
  if(CurOn == 0)
  {
    CurOn = 1;
  }
  else
  {
    CurOn = 0;
  }
  invert_char(CH_WIDTH * ScrColl, (Font->Height + 1) * ScrRow + MARGIN);
  sem_post(&CurMutex);
}

void PutScreenLine(char *Line, int Col)
{
  int i, y;
  
  y = (Font->Height + 1) * ScrRow + MARGIN;
  for(i = 0; i <= strlen(Line); i++)
    erase_char(CH_WIDTH * i, y);
  i = 0;
  while(*Line)
  {
    if((*Line == '\n') || (*Line == '\n'))
      break;
    put_char(CH_WIDTH * i++, y, *Line++, Col);
  }
  erase_char(CH_WIDTH * i, y);
}

void PutScreenChar(char c, int Col)
{
  switch(c)
  {
    case '\n' :
      ScrRow++;
      if(ScrRow >= ScrLines)
      {
        ScrollUp();
        ScrRow = ScrLines - 1;
      }
      ScrColl = 0;
      break;
    case '\r' :
      ScrColl = 0;
      break;
    case '\b' :
      ScrColl--;
      if(ScrColl <= 0)
         ScrColl = 0;
      erase_char(CH_WIDTH * ScrColl - 1, (Font->Height + 1) * ScrRow + MARGIN);
      break;
    default :  
      put_char(CH_WIDTH * ScrColl++, (Font->Height + 1) * ScrRow + MARGIN, c, Col);
      if(ScrColl >= 35)
      {
        ScrColl = 0;
        ScrRow++;
        if(ScrRow >= ScrLines)
        {
          ScrollUp();
          ScrRow = ScrLines - 1;
        }
      }
  }
}

void SendToBash(char c)
{
  char store = c;
  int i;
  static char InLine[44] = "";
  static int  LineCnt = 0;
  static int  CursIdx = 0;

  sem_wait(&CurMutex);
  if(Debug > 15) printf("Semphore 2\n");
  if(CurOn == 1)
  {
    CurOn = 0;
    invert_char(CH_WIDTH * ScrColl, (Font->Height + 1) * ScrRow + MARGIN);
  }
  if(c == KEYBD_LEFT)  // 135
  {
    if(CursIdx > 0)
    {
      CursIdx--; 
      ScrColl--;
    }
    c = 'L';
  }
  else if(c == KEYBD_RIGHT)  // 136
  {
    if(CursIdx < LineCnt)
    {
      CursIdx++; 
      ScrColl++;
    }
    c = 'R';
  }
  else if((c == '\n') || (c == '\r'))
  {
    ScrColl++;
    InLine[LineCnt++] = c;
    InLine[LineCnt] = '\0';
    if(Debug) printf("Command = [%s]\n", InLine);
    for(i = 0; i < LineCnt; i++)
    {    
      write(aStdinPipe[PIPE_WRITE], &InLine[i], 1);
    }
    CursIdx = LineCnt; 
    ScrColl = LineCnt;
    PutScreenLine(InLine, LIGHT_BLUE);
    LineCnt = 0;
    CursIdx = 0; 
    c = 'N';
    if(Debug) printf("Got = [%s]\n", InLine);
    if(Debug > 14 ) printf("Send [%c][%d]\n", c, store);
    sem_post(&CurMutex);
    return;
  }
  else if(c == '\b')
  {
    if(CursIdx)
    {
      memcpy(&InLine[CursIdx - 1], &InLine[CursIdx] , LineCnt - CursIdx + 1);
      if(LineCnt != 0)
         LineCnt--;
      if(CursIdx != 0)
         CursIdx--;
      ScrColl--;
      InLine[LineCnt] = '\0';
    }
    c = 'B';
  }
  else
  {
    if(CursIdx == LineCnt)
    {
      ScrColl++;
      LineCnt++;
      InLine[CursIdx++] = c;
    }
    else
    {
      LineCnt++;
      ScrColl++;
      for(i = LineCnt + 1; i > CursIdx; i--)
        InLine[i] = InLine[i - 1];
      InLine[CursIdx++] = c;
    }
    InLine[LineCnt] = '\0';
  }
  PutScreenLine(InLine, LIGHT_BLUE);
  if(Debug) printf("Got = [%s]\n", InLine);
  if(Debug > 14 ) printf("Send [%c][%d]\n", c, store);
  sem_post(&CurMutex);
  TogleCursor();
}

void ReadTask(void)
{
  char InChar, s; 

  if(CurOn == 1)
  {
    CurOn = 0;
    invert_char(CH_WIDTH * ScrColl, (Font->Height + 1) * ScrRow + MARGIN);
  }
  while (read(aStdoutPipe[PIPE_READ], &InChar, 1) == 1)  
  { 
    if(Debug > 15) write(STDOUT_FILENO, &InChar, 1); 
    sem_wait(&CurMutex);
    if(Debug > 11) printf("Semphore 3\n");
    PutScreenChar(InChar, WHITE);
    sem_post(&CurMutex);
    s = InChar;
    if((InChar == '\n') || (InChar == '\r'))
      InChar = '~';
    if(Debug > 8 ) printf("Read 1 :[%c][%d]\n", InChar, s);
  } 
}

int createChild(const char* szCommand, char* const aArguments[],  
                char* const aEnvironment[]) 
{ 
  int nChild; 
  int nResult; 
  static pthread_t ReadThread;  
 
  if (pipe(aStdinPipe) < 0)  
  { 
    perror("allocating pipe for child input redirect"); 
    return -1; 
  } 
  if (pipe(aStdoutPipe) < 0)  
  { 
    close(aStdinPipe[PIPE_READ]); 
    close(aStdinPipe[PIPE_WRITE]); 
    perror("allocating pipe for child output redirect"); 
    return -1; 
  } 
 
  nChild = fork(); 
  if (0 == nChild)  
  { 
    // child continues here 
 
    // redirect stdin 
    if (dup2(aStdinPipe[PIPE_READ], STDIN_FILENO) == -1) { 
      exit(errno); 
    } 
 
    // redirect stdout 
    if (dup2(aStdoutPipe[PIPE_WRITE], STDOUT_FILENO) == -1) { 
      exit(errno); 
    } 
 
    // redirect stderr 
    if (dup2(aStdoutPipe[PIPE_WRITE], STDERR_FILENO) == -1) { 
      exit(errno); 
    } 
 
    // all these are for use by parent only 
    close(aStdinPipe[PIPE_READ]); 
    close(aStdinPipe[PIPE_WRITE]); 
    close(aStdoutPipe[PIPE_READ]); 
    close(aStdoutPipe[PIPE_WRITE]);  
 
    // run child process image 
    // replace this with any exec* function find easier to use ("man exec") 
    nResult = execve(szCommand, aArguments, aEnvironment); 
 
    // if we get here at all, an error occurred, but we are in the child 
    // process, so just exit 
    exit(nResult); 
  }  
  else if (nChild > 0) 
  { 
    // parent continues here 
 
    // close unused file descriptors, these are for child only 
    close(aStdinPipe[PIPE_READ]); 
    close(aStdoutPipe[PIPE_WRITE]);  

    pthread_create(&ReadThread, NULL, ( void* ) ReadTask, NULL );
    if(Debug > 1)fprintf(stderr, "Read thread started\n");
            
    ClearScr();
    while(1)
    {
      KeybdEdit();
    }
    // done with these in this example program, you would normally keep these 
    // open of course as long as you want to talk to the child 
    close(aStdinPipe[PIPE_WRITE]); 
    close(aStdoutPipe[PIPE_READ]); 
  }  
  else 
  { 
    // failed to create child 
    close(aStdinPipe[PIPE_READ]); 
    close(aStdinPipe[PIPE_WRITE]); 
    close(aStdoutPipe[PIPE_READ]); 
    close(aStdoutPipe[PIPE_WRITE]); 
  } 
  return nChild; 
}

void INThandler(int sig)
{
  signal(sig, SIG_IGN);
  closeFramebuffer();
  exit(0);
}

void Usage(void)
{
  fprintf(stderr, "\nUsage :\n    consloe -OPTION\n");
  fprintf(stderr, "\tWhere OPTION is: \n");
  fprintf(stderr, "\t-v            - Print version number.\n");
  fprintf(stderr, "\t-bx           - Select frame buffer number x (from /dev/fbx).\n");
  fprintf(stderr, "\t                    Default \"/dev/fb1\".\n");
  fprintf(stderr, "\t-tx           - Select LCD TFT device number x(from /dev/input/eventx).\n");
  fprintf(stderr, "\t                    Default \"ADS7846 Touchscreen\".\n");
  fprintf(stderr, "\t-L            - List frame buffer device(s).\n");
  fprintf(stderr, "\t-l            - List LCD TFT device(s).\n");
  fprintf(stderr, "\t-fx           - Use font number x (1 to 3).\n");
  fprintf(stderr, "\t-dx           - Set debug level x.\n");
  fprintf(stderr, "\t--help, -h    - This screen.\n");
  exit(0);
}

void ParseArguments(int argc, char *argv[])
{
  int i;

  for(i = 1; i < argc; i++)
  {
    if(argv[i][0] == '-' )
    {
      if(!strcmp(argv[i], "--help"))
        Usage();
      else
        switch(argv[i][1])
        { 
          case 'v' :
            printf("console version %s\n", VERSION);
            exit(0);
                                        
          case 'h' :
            Usage();
            break;
                                        
          case 'b' :
            if(isdigit((unsigned char)argv[i][2]))
            {
              FbDeviceNumber = atoi(&argv[i][2]);
              if(Debug) fprintf(stderr, "FB device number = %d\n", FbDeviceNumber);
            }
            break;
                                        
          case 't' :
            if(isdigit((unsigned char)argv[i][2]))
            {
              DeviceNumber = atoi(&argv[i][2]);
              if(Debug) fprintf(stderr, "Device number = %d\n", DeviceNumber);
            }
            break;
                                        
          case 'l' :
            DeviceListMode = 1;    
            printf("List devices\n");
            break;
                                        
          case 'L' :
            FbDeviceListMode = 1;    
            printf("List frame buffers\n");
            break;
                                        
          case 'f' :
            if(isdigit((unsigned char)argv[i][2]))
            {
              FontNo = atoi(&argv[i][2]) - 1;
              if(Debug) fprintf(stderr, "Font No = %d\n", FontNo + 1);
            }
            break;
                                        
          case 'd' :
            if(isdigit((unsigned char)argv[i][2]))
            {
              Debug = atoi(&argv[i][2]);
              if(Debug) fprintf(stderr, "Debug = %d\n", Debug);
            }
            break;

          default :
            Usage();
            break;
      }
    }
  }
}
                                                                    
int main(int argc, char *argv[], char * envp[])
{
  char* const args[] = { NULL };

  ParseArguments(argc, argv);
  signal(SIGINT, INThandler);
  sem_init(&CurMutex, 0, 1);
                  
  system("setterm -cursor off");

  if(openTouchScreen("ADS7846 Touchscreen", &screenXmin,&screenXmax,&screenYmin,&screenYmax) != 1)
  {
    if(DeviceListMode) exit(0);
    perror("Error opening touch screen ADS7846 Touchscreen\n");
    exit(0);
  }
  
  if(DeviceListMode) exit(0);
   
  //getTouchScreenDetails("ADS7846 Touchscreen", &screenXmin,&screenXmax,&screenYmin,&screenYmax);
   
  framebufferInitialize(&ScreenXres,&ScreenYres);

  if(Debug > 1)printf("Max %d Min %d Res %d\n", screenXmax, screenXmin, ScreenXres);
  scaleXvalue = ((float)screenXmax-screenXmin) / ScreenXres;
  if(Debug > 1)printf ("X Scale Factor = %f\n", scaleXvalue);

  if(Debug > 1)printf("Max %d Min %d Res %d\n", screenYmax, screenYmin, ScreenYres);
  scaleYvalue = ((float)screenYmax-screenYmin) / ScreenYres;
  if(Debug > 1)printf ("Y Scale Factor = %f\n", scaleYvalue);

  SetFontIndex(FontNo);

  ScrLines = 120 / Font->Height;
  if(Debug > 1)printf("Lines = %d pixels = %d\n", ScrLines, Font->Height);

  createChild("/bin/bash", args, envp);
  sem_destroy(&CurMutex);
}

