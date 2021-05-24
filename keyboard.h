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
#define KEYBD_ALT        129
#define KEYBD_CTL        130
#define KEYBD_RETURN     133
#define KEYBD_BACKSPACE  134
#define KEYBD_LEFT       135
#define KEYBD_RIGHT      136
#define KEYBD_UP         131
#define KEYBD_DOWN       132
            

void DrawKeyboard(int Upper);
void HiglighButton(int x, int y, int w, int h, int borderColor);
void KeybdEditDisplay(char *Str, int StrPos);
void KeybdEdit(void);

