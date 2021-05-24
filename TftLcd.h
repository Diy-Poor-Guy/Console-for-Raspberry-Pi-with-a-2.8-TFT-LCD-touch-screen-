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

#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#ifdef FROM_FRAME_BUFFER_C

struct fb_fix_screeninfo fix;
struct fb_var_screeninfo orig_var;
struct fb_var_screeninfo var;
char *fbp = 0;
int fb=0;
long int screensize = 0;
static unsigned short def_r[] =
    { 0,   0,   0,   0, 172, 172, 172, 168,
     84,  84,  84,  84, 255, 255, 255, 255};
static unsigned short def_g[] =
    { 0,   0, 168, 168,   0,   0,  84, 168,
     84,  84, 255, 255,  84,  84, 255, 255};
static unsigned short def_b[] =
    { 0, 172,   0, 168,   0, 172,   0, 168,
     84, 255,  84, 255,  84, 255,  84, 255};

#endif

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"


// default framebuffer palette
typedef enum {
  BLACK        =  0, /*   0,   0,   0 */
  BLUE         =  1, /*   0,   0, 172 */
  GREEN        =  2, /*   0, 172,   0 */
  CYAN         =  3, /*   0, 172, 172 */
  RED          =  4, /* 172,   0,   0 */
  PURPLE       =  5, /* 172,   0, 172 */
  ORANGE       =  6, /* 172,  84,   0 */
  LTGREY       =  7, /* 172, 172, 172 */
  GREY         =  8, /*  84,  84,  84 */
  LIGHT_BLUE   =  9, /* 124, 124, 255 */
  LIGHT_GREEN  = 10, /*  84, 255,  84 */
  LIGHT_CYAN   = 11, /*  84, 255, 255 */
  LIGHT_RED    = 12, /* 255,  84,  84 */
  LIGHT_PURPLE = 13, /* 255,  84, 255 */
  YELLOW       = 14, /* 255, 255,  84 */
  WHITE        = 15  /* 255, 255, 255 */
} COLOR_INDEX_T;

typedef struct StructFont{
  int Height;
  int Width; 
  void *Data;
}StructFont;

extern struct StructFont *Font;

void ClearScr(void);
void ClearSubScr(int start, int end);
void ScrollSubScr(int start, int end);
void put_pixel_16bpp(int x, int y, int r, int g, int b);
void drawSquare(int x, int y,int height, int width,  int c);
void drawBox(int x, int y,int height, int width,  int c);
int framebufferInitialize(int *xres, int *yres);
void closeFramebuffer();
void put_char(int x, int y, int c, int colidx);
void put_string(int x, int y, char *s, unsigned colidx);
void SetFontIndex(int Idx);

void invert_char(int x, int y);
void erase_char(int x, int y);
#endif

