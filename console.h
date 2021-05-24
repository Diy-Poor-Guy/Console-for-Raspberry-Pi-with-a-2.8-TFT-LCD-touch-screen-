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
extern int DeviceNumber;
extern int FbDeviceNumber;
extern int DeviceListMode;
extern int FbDeviceListMode;
extern int FontNo;
extern int Debug;

extern void ScrollUp(void);
extern void CursorOn(void);
extern void CursorOff(void);
extern void TogleCursor(void);
extern void PutScreenChar(char c, int Col);
extern void SendToBash(char c);
extern void ReadTask(void);
extern int createChild(const char* szCommand, char* const aArguments[], char* const aEnvironment[]) ;
extern void INThandler(int sig);
extern void Usage(void);
extern void ParseArguments(int argc, char *argv[]);

