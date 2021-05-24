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
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include "console.h"

#define FROM_FRAME_BUFFER_C
#include "TftLcd.h"
#undef FROM_FRAME_BUFFER_C

#include "font_8X8.h"
#include "font_8X11.h"
#include "font_8X13.h"

extern int Debug;

struct StructFont Fonts[] = {
  {
    FONT_8X8_HEIGHT, 
    FONT_8X8_WIDTH, 
    &Font_8X8[0][0]
  }, {
    FONT_8X11_HEIGHT, 
    FONT_8X11_WIDTH, 
    &Font_8X11[0][0]
  }, {
    FONT_8X13_HEIGHT, 
    FONT_8X13_WIDTH, 
    &Font_8X13[0][0]
  }
};

struct StructFont *Font = &Fonts[2];

void SetFontIndex(int Idx)
{
  Font = &Fonts[Idx];
}

void put_pixel_16bpp(int x, int y, int r, int g, int b)
{
  unsigned int pix_offset;
  unsigned short c;

  // calculate the pixel's byte offset inside the buffer
  pix_offset = x*2 + y * fix.line_length;
  if(pix_offset > (screensize - 2))
  {
    printf("Out of bound\n");
    return;
  }
  //some magic to work out the color
  c = ((r / 8) << 11) + ((g / 4) << 5) + (b / 8);

  // write 'two bytes at once'
  *((unsigned short*)(fbp + pix_offset)) = c;
}

void put_pixel(int x, int y, unsigned short c)
{
  unsigned int pix_offset;

  // calculate the pixel's byte offset inside the buffer
  pix_offset = x*2 + y * fix.line_length;
  if(pix_offset > (screensize - 2))
  {
    printf("OUT OF BOUND\n");
    return;
  }

  // write 'two bytes at once'
  *((unsigned short*)(fbp + pix_offset)) = c;
}

unsigned short get_pixel(int x, int y)
{
  unsigned int pix_offset;
  unsigned short c;

  // calculate the pixel's byte offset inside the buffer
  pix_offset = x*2 + y * fix.line_length;
  if(pix_offset > (screensize - 2))
  {
    printf("Out Of Bound\n");
    return(0);
  }

  // get 'two bytes at once'
  c = *((unsigned short*)(fbp + pix_offset));
  return(c);
}

void drawSquare(int x, int y, int height, int width,  int c)
{
  int h = 0;
  int w = 0;
  for(h = 0; h < height; h++)
    for(w = 0; w < width; w++)
      put_pixel_16bpp( h+(x-2), w+(y-2) , def_r[c], def_g[c], def_b[c]);

}

void drawHline(int x, int y, int width,  int c)
{
  int w = 0;
  for(w = 0; w < width; w++)
    put_pixel_16bpp(x, y + w, def_r[c], def_g[c], def_b[c]);
}

void drawVline(int x, int y, int height,  int c)
{
  int h = 0;
  for(h = 0; h < height; h++)
    put_pixel_16bpp(x + h, y, def_r[c], def_g[c], def_b[c]);

}

void drawBox(int x, int y, int height, int width,  int c)
{
  drawVline(x, y, height, c);
  drawHline(x, y, width,  c);

  drawHline(x + height, y,         width,  c);
  drawVline(x,          y + width, height, c);
}

void ClearScr(void)
{
  int x, y;
   
  //clear framebuffer
  for(x = 0; x <var.xres; x++)
    for(y = 0; y < var.yres; y++)
      put_pixel_16bpp(x, y, 0, 0, 0);
}

void ScrollSubScr(int start, int end)
{
  int x, y;
  unsigned short c;
   
  for(x = 0; x < var.xres; x++)
  {
    for(y = start; y < end - Font->Height; y++)
    {
      c = get_pixel(x, y + Font->Height);
      put_pixel(x, y, c);
    }
    for(y = end - Font->Height; y < end; y++)
    {
      put_pixel(x, y, 0);
    }
  }
}

void ClearSubScr(int start, int end)
{
 int x, y;
  
 //clear framebuffer
 for(x = 0; x < var.xres; x++)
   for(y = start; y < end; y++)
     put_pixel_16bpp(x, y, 0, 0, 0);
}

int framebufferIList(int *xres, int *yres)
{
  char fbdevice[11];
  int i;
  
  for(i = 0; i < 10; i++)
  {
    sprintf(&fbdevice[0], "/dev/fb%d", i);
    fb = open(fbdevice, O_RDWR);
    if (fb == -1) 
    {
      continue;
    }
  
    if (ioctl(fb, FBIOGET_FSCREENINFO, &fix) < 0) 
    {
      perror("ioctl FBIOGET_FSCREENINFO");
      close(fb);
      continue;
    }
  
    if (ioctl(fb, FBIOGET_VSCREENINFO, &var) < 0) 
    {
      perror("ioctl FBIOGET_VSCREENINFO");
      close(fb);
      continue;
    }
    printf("Framebuffer %s resolution;\n", fbdevice);
    memcpy(&orig_var, &var, sizeof(struct fb_var_screeninfo));
    printf("%dx%d, %d bpp\n\n\n", var.xres, var.yres, var.bits_per_pixel );
  }
  exit(0);
}


int framebufferInitialize(int *xres, int *yres)
{
  char fbdevice[11] = "/dev/fb1" ;

  if(FbDeviceListMode) 
    framebufferIList(xres, yres);

  if(FbDeviceNumber != -1)
    sprintf(&fbdevice[0], "/dev/fb%d", FbDeviceNumber);
  fb = open(fbdevice, O_RDWR);
  if(fb == -1) 
  {
    perror("open fbdevice");
    return -1;
  }

  if(ioctl(fb, FBIOGET_FSCREENINFO, &fix) < 0) 
  {
    perror("ioctl FBIOGET_FSCREENINFO");
    close(fb);
    return -1;
  }

  if(ioctl(fb, FBIOGET_VSCREENINFO, &var) < 0) 
  {
    perror("ioctl FBIOGET_VSCREENINFO");
    close(fb);
    return -1;
  }

  if(Debug) printf("Original %dx%d, %dbpp\n", var.xres, var.yres, 
         var.bits_per_pixel );

  memcpy(&orig_var, &var, sizeof(struct fb_var_screeninfo));



  if(Debug) printf("Framebuffer %s%s%s resolution;\n",KYEL, fbdevice, KWHT);

  if(Debug) printf("%dx%d, %d bpp\n\n\n", var.xres, var.yres, var.bits_per_pixel );

  // map framebuffer to user memory 
  screensize = fix.smem_len;
  fbp = (char*)mmap(0, 
                    screensize, 
                    PROT_READ | PROT_WRITE, 
                    MAP_SHARED, 
                    fb, 0);
  if((int)fbp == -1) 
  {
    printf("Failed to mmap.\n");
  }

  *xres = var.xres;
  *yres = var.yres;

  ClearScr();
  return(0);
}

void closeFramebuffer()
{

  int x, y;
  for(x = 0; x < var.xres; x++)
    for(y = 0; y < var.yres;  y++)
      put_pixel_16bpp(x, y, 0, 0, 0);

  munmap(fbp, screensize);
  if(ioctl(fb, FBIOPUT_VSCREENINFO, &orig_var)) 
  {
    printf("Error re-setting variable information.\n");
  }
  close(fb);

}

void invert_char(int x, int y)
{
  int i,j;

  for(i = 0; i < Font->Height; i++) 
  {
    for(j = 0; j < Font->Width; j++)
    {
      if(get_pixel(x + j,y + i))
      {
        put_pixel(x + j, y + i, 0);
      }
      else
      {
        put_pixel(x + j, y + i, 0xffff);
      }
    }
  }
}

void erase_char(int x, int y)
{
  int i,j;

  for(i = 0; i < Font->Height; i++) 
  {
    for(j = 0; j < Font->Width; j++)
    {
      put_pixel_16bpp(x+j,  y+i, 0, 0, 0);
    }
  }
}

void put_char(int x, int y, int c, int col)
{
  int i, j, bits;
  
  for(i = 0; i < Font->Height; i++) 
  {
    bits = *((unsigned char *)(Font->Data + (c * Font->Height + i)));
    for(j = 0; j < Font->Width; j++, bits <<= 1)
    {
      if(bits & 0x80)
      {
        put_pixel_16bpp(x+j,  y+i, def_r[col], def_g[col], def_b[col]);
      }
    }
  }
}

void put_string(int x, int y, char *s, unsigned colidx)
{
  int i;
  for(i = 0; *s; i++, x += (Font->Width + 1), s++)
  {
    if(x >= (var.xres - 8))
      break;
    put_char (x, y, *s, colidx);
  }
}
