
/*  pixbuf_to_hicon.c
 *
 *  GnomeMeeting -- A Video-Conferencing application
 *  Copyright (C) 2000-2006 Damien Sandras
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *
 *  Authors: Julien Puydt <jpuydt@free.fr>
 */

/*
 *                         pixbuf_to_hicon.c  -  description
 *                         ------------------------
 *   begin                : Sat Jan 7 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Implementation of an helper function
 */

#include "pixbuf_to_hicon.h"

#define WIN32_GDI_FAILED g_warning

/* the rest of that code was directly copy-pasted from gtk+'s sources
 * (more specifically gdk/win32/gdkcursor-win32.c), hence falls under
 * the same licence, with the same copyrights. It will disappear from
 * gnomemeeting's own cvs when a version of GTK+ with GtkStatusIcon will be
 * released.
 */

typedef struct { 
  DWORD        bV5Size; 
  LONG         bV5Width; 
  LONG         bV5Height; 
  WORD         bV5Planes; 
  WORD         bV5BitCount; 
  DWORD        bV5Compression; 
  DWORD        bV5SizeImage; 
  LONG         bV5XPelsPerMeter; 
  LONG         bV5YPelsPerMeter; 
  DWORD        bV5ClrUsed; 
  DWORD        bV5ClrImportant; 
  DWORD        bV5RedMask; 
  DWORD        bV5GreenMask; 
  DWORD        bV5BlueMask; 
  DWORD        bV5AlphaMask; 
  DWORD        bV5CSType; 
  CIEXYZTRIPLE bV5Endpoints; 
  DWORD        bV5GammaRed; 
  DWORD        bV5GammaGreen; 
  DWORD        bV5GammaBlue; 
  DWORD        bV5Intent; 
  DWORD        bV5ProfileData; 
  DWORD        bV5ProfileSize; 
  DWORD        bV5Reserved; 
} BITMAPV5HEADER;


gboolean
_gdk_win32_pixbuf_to_hicon_supports_alpha (void)
{
  static gboolean is_win_xp=FALSE, is_win_xp_checked=FALSE;

  if (!is_win_xp_checked)
    {
      is_win_xp_checked = TRUE;

      if (!G_WIN32_IS_NT_BASED ())
	is_win_xp = FALSE;
      else
	{
	  OSVERSIONINFO version;

	  memset (&version, 0, sizeof (version));
	  version.dwOSVersionInfoSize = sizeof (version);
	  is_win_xp = GetVersionEx (&version)
	    && version.dwPlatformId == VER_PLATFORM_WIN32_NT
	    && (version.dwMajorVersion > 5
		|| (version.dwMajorVersion == 5 && version.dwMinorVersion >= 1));
	}
    }
  return is_win_xp;
}


static HBITMAP
create_alpha_bitmap (gint width,
		     gint height,
		     guchar **outdata)
{
  BITMAPV5HEADER bi;
  HDC hdc;
  HBITMAP hBitmap;

  ZeroMemory (&bi, sizeof (BITMAPV5HEADER));
  bi.bV5Size = sizeof (BITMAPV5HEADER);
  bi.bV5Width = width;
  bi.bV5Height = height;
  bi.bV5Planes = 1;
  bi.bV5BitCount = 32;
  bi.bV5Compression = BI_BITFIELDS;
  /* The following mask specification specifies a supported 32 BPP
   * alpha format for Windows XP (BGRA format).
   */
  bi.bV5RedMask   = 0x00FF0000;
  bi.bV5GreenMask = 0x0000FF00;
  bi.bV5BlueMask  = 0x000000FF;
  bi.bV5AlphaMask = 0xFF000000;

  /* Create the DIB section with an alpha channel. */
  hdc = GetDC (NULL);
  if (!hdc)
    {
      WIN32_GDI_FAILED ("GetDC");
      return NULL;
    }
  hBitmap = CreateDIBSection (hdc, (BITMAPINFO *)&bi, DIB_RGB_COLORS,
			      (PVOID *) outdata, NULL, (DWORD)0);
  if (hBitmap == NULL)
    WIN32_GDI_FAILED ("CreateDIBSection");
  ReleaseDC (NULL, hdc);

  return hBitmap;
}


static HBITMAP
create_color_bitmap (gint     width,
		     gint     height,
		     guchar **outdata,
		     gint     bits)
{
  struct {
    BITMAPV4HEADER bmiHeader;
    RGBQUAD bmiColors[2];
  } bmi;
  HDC hdc;
  HBITMAP hBitmap;

  ZeroMemory (&bmi, sizeof (bmi));
  bmi.bmiHeader.bV4Size = sizeof (BITMAPV4HEADER);
  bmi.bmiHeader.bV4Width = width;
  bmi.bmiHeader.bV4Height = height;
  bmi.bmiHeader.bV4Planes = 1;
  bmi.bmiHeader.bV4BitCount = bits;
  bmi.bmiHeader.bV4V4Compression = BI_RGB;

  /* when bits is 1, these will be used.
   * bmiColors[0] already zeroed from ZeroMemory()
   */
  bmi.bmiColors[1].rgbBlue = 0xFF;
  bmi.bmiColors[1].rgbGreen = 0xFF;
  bmi.bmiColors[1].rgbRed = 0xFF;

  hdc = GetDC (NULL);
  if (!hdc)
    {
      WIN32_GDI_FAILED ("GetDC");
      return NULL;
    }
  hBitmap = CreateDIBSection (hdc, (BITMAPINFO *)&bmi, DIB_RGB_COLORS,
			      (PVOID *) outdata, NULL, (DWORD)0);
  if (hBitmap == NULL)
    WIN32_GDI_FAILED ("CreateDIBSection");
  ReleaseDC (NULL, hdc);

  return hBitmap;
}


static gboolean
pixbuf_to_hbitmaps_alpha_winxp (GdkPixbuf *pixbuf,
				HBITMAP   *color,
				HBITMAP   *mask)
{
  /* Based on code from
   * http://www.dotnet247.com/247reference/msgs/13/66301.aspx
   */
  HBITMAP hColorBitmap, hMaskBitmap;
  guchar *indata, *inrow;
  guchar *colordata, *colorrow, *maskdata, *maskbyte;
  gint width, height, i, j, rowstride;
  guint maskstride, mask_bit;

  width = gdk_pixbuf_get_width (pixbuf); /* width of icon */
  height = gdk_pixbuf_get_height (pixbuf); /* height of icon */

  hColorBitmap = create_alpha_bitmap (width, height, &colordata);
  if (!hColorBitmap)
    return FALSE;
  hMaskBitmap = create_color_bitmap (width, height, &maskdata, 1);
  if (!hMaskBitmap)
    {
      DeleteObject (hColorBitmap);
      return FALSE;
    }

  /* MSDN says mask rows are aligned to "LONG" boundaries */
  maskstride = (((width + 31) & ~31) >> 3);

  indata = gdk_pixbuf_get_pixels (pixbuf);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  for (j=0; j<height; j++)
    {
      colorrow = colordata + 4*j*width;
      maskbyte = maskdata + j*maskstride;
      mask_bit = 0x80;
      inrow = indata  + (height-j-1)*rowstride;
      for (i=0; i<width; i++)
	{
	  colorrow[4*i+0] = inrow[4*i+2];
	  colorrow[4*i+1] = inrow[4*i+1];
	  colorrow[4*i+2] = inrow[4*i+0];
	  colorrow[4*i+3] = inrow[4*i+3];
	  if (inrow[4*i+3] == 0)
	    maskbyte[0] |= mask_bit;	/* turn ON bit */
	  else
	    maskbyte[0] &= ~mask_bit;	/* turn OFF bit */
	  mask_bit >>= 1;
	  if (mask_bit == 0)
	    {
	      mask_bit = 0x80;
	      maskbyte++;
	    }
	}
    }

  *color = hColorBitmap;
  *mask = hMaskBitmap;

  return TRUE;
}


static gboolean
pixbuf_to_hbitmaps_normal (GdkPixbuf *pixbuf,
			   HBITMAP   *color,
			   HBITMAP   *mask)
{
  /* Based on code from
   * http://www.dotnet247.com/247reference/msgs/13/66301.aspx
   */
  HBITMAP hColorBitmap, hMaskBitmap;
  guchar *indata, *inrow;
  guchar *colordata, *colorrow, *maskdata, *maskbyte;
  gint width, height, i, j, rowstride, nc, bmstride;
  gboolean has_alpha;
  guint maskstride, mask_bit;

  width = gdk_pixbuf_get_width (pixbuf); /* width of icon */
  height = gdk_pixbuf_get_height (pixbuf); /* height of icon */

  hColorBitmap = create_color_bitmap (width, height, &colordata, 24);
  if (!hColorBitmap)
    return FALSE;
  hMaskBitmap = create_color_bitmap (width, height, &maskdata, 1);
  if (!hMaskBitmap)
    {
      DeleteObject (hColorBitmap);
      return FALSE;
    }

  /* rows are always aligned on 4-byte boundarys */
  bmstride = width * 3;
  if (bmstride % 4 != 0)
    bmstride += 4 - (bmstride % 4);

  /* MSDN says mask rows are aligned to "LONG" boundaries */
  maskstride = (((width + 31) & ~31) >> 3);

  indata = gdk_pixbuf_get_pixels (pixbuf);
  rowstride = gdk_pixbuf_get_rowstride (pixbuf);
  nc = gdk_pixbuf_get_n_channels (pixbuf);
  has_alpha = gdk_pixbuf_get_has_alpha (pixbuf);

  for (j=0; j<height; j++)
    {
      colorrow = colordata + j*bmstride;
      maskbyte = maskdata + j*maskstride;
      mask_bit = 0x80;
      inrow = indata  + (height-j-1)*rowstride;
      for (i=0; i<width; i++)
	{
	  if (has_alpha && inrow[nc*i+3] < 128)
	    {
	      colorrow[3*i+0] = colorrow[3*i+1] = colorrow[3*i+2] = 0;
	      maskbyte[0] |= mask_bit;	/* turn ON bit */
	    }
	  else
	    {
	      colorrow[3*i+0] = inrow[nc*i+2];
	      colorrow[3*i+1] = inrow[nc*i+1];
	      colorrow[3*i+2] = inrow[nc*i+0];
	      maskbyte[0] &= ~mask_bit;	/* turn OFF bit */
	    }
	  mask_bit >>= 1;
	  if (mask_bit == 0)
	    {
	      mask_bit = 0x80;
	      maskbyte++;
	    }
	}
    }

  *color = hColorBitmap;
  *mask = hMaskBitmap;

  return TRUE;
}


static HICON
pixbuf_to_hicon (GdkPixbuf *pixbuf,
		 gboolean   is_icon,
		 gint       x,
		 gint       y)
{
  ICONINFO ii;
  HICON icon;
  gboolean success;

  if (pixbuf == NULL)
    return NULL;

  if (_gdk_win32_pixbuf_to_hicon_supports_alpha() && gdk_pixbuf_get_has_alpha (pixbuf))
    success = pixbuf_to_hbitmaps_alpha_winxp (pixbuf, &ii.hbmColor, &ii.hbmMask);
  else
    success = pixbuf_to_hbitmaps_normal (pixbuf, &ii.hbmColor, &ii.hbmMask);

  if (!success)
    return NULL;

  ii.fIcon = is_icon;
  ii.xHotspot = x;
  ii.yHotspot = y;
  icon = CreateIconIndirect (&ii);
  DeleteObject (ii.hbmColor);
  DeleteObject (ii.hbmMask);
  return icon;
}


HICON
_gdk_win32_pixbuf_to_hicon (GdkPixbuf *pixbuf)
{
  return pixbuf_to_hicon (pixbuf, TRUE, 0, 0);
}

