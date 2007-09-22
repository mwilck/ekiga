
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2007 Damien Sandras
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Ekiga is licensed under the GPL license and as a special exception,
 * you have permission to link or otherwise combine this program with the
 * programs OPAL, OpenH323 and PWLIB, and distribute the combination,
 * without applying the requirements of the GNU GPL to the OPAL, OpenH323
 * and PWLIB programs, as long as you do follow the requirements of the
 * GNU GPL for all the rest of the software thus combined.
 */

/*
 *                         XVWindow.h  -  description
 *                         --------------------------
 *   begin                : Sun May 7 2006
 *   copyright            : (C) 2006 by Matthias Schneider <ma30002000@yahoo.de>
 *   description          : High-level class offering X-Video hardware 
 *                          acceleration.
 */


#ifndef XVWINDOW_H
#define XVWINDOW_H

#include <stdint.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/extensions/Xv.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/Xvlib.h>


/**
 * String: wrapper/helper.
 *
 * This class provides XVideo support under Linux if it is supported by the graphics hardware and driver.
 * XVideo makes use of hardware capabilities in order to do
 * - colorspace transformation
 * - scaling
 * - anti-aliasing
 *
 * This class features a fullscreen mode, an always-on-top mode and allows to enable and disable the window
 * manager decorations. A picture-in-picture functionality is provided by registering a second XVWindow class
 * window as a slave window. This class should work with most if not all window managers. It has to initialized
 * with the display and window where it shall appear and the original image and intial window size
 * After having been initialized successfully a frame is passed via PutFrame which takes care of the presentation.
 *
 * @author Matthias Schneider
 */
class XVWindow
{
public:

  XVWindow();

  ~XVWindow();
  
  int Init (Display *dp, 
            Window xvWindow, 
            GC gc, 
            int x, 
            int y, 
            int windowWidth, 
            int windowHeight, 
            int imageWidth, 
            int imageHeight);

  void PutFrame (uint8_t *frame, 
                 uint16_t width, 
                 uint16_t height);

  void ToggleFullscreen ();
 
  void ToggleOntop ();
  
  void ToggleDecoration ();

  void SetWindow (int x, 
                  int y, 
                  unsigned int windowWidth, 
                  unsigned int windowHeight);
  
  void GetWindow (int *x, 
                  int *y, 
                  unsigned int *windowWidth, 
                  unsigned int *windowHeight);

  bool IsFullScreen () const { return _state.fullscreen; };
 
  bool HasDecoration () const { return _state.decoration; };

  bool IsOntop () const { return _state.ontop; };

  Window GetWindowHandle () const { return _XVWindow; };

  int GetYUVWidth () const { return _XVImage->width; };

  int GetYUVHeight() const { return _XVImage->height; };

  void RegisterMaster (XVWindow *master) { _master = master; };

  void RegisterSlave (XVWindow *slave) { _slave = slave; };

private:

  Display *_display;
  
  Window _rootWindow;
  Window _XVWindow;
  
  unsigned int _XVPort;
  GC _gc;
  XvImage * _XVImage;
  XShmSegmentInfo _XShmInfo;
  
  int _wmType;
  bool _isInitialized;
  bool _embedded;

  typedef struct 
  {
    bool fullscreen;
    bool ontop;
    bool decoration;
    int oldx;
    int oldy;
    int oldWidth;
    int oldHeight;
    int curX;
    int curY;
    int curWidth;
    int curHeight;
    int origLayer;
  } State;

  State _state;
  XVWindow * _master;
  XVWindow * _slave;

  Atom XA_NET_SUPPORTED;
  Atom XA_WIN_PROTOCOLS;
  Atom XA_WIN_LAYER;
  Atom XA_NET_WM_STATE;
  Atom XA_NET_WM_STATE_FULLSCREEN;
  Atom XA_NET_WM_STATE_ABOVE;
  Atom XA_NET_WM_STATE_STAYS_ON_TOP;
  Atom XA_NET_WM_STATE_BELOW;
  Atom WM_DELETE_WINDOW;

  /**
   * Sets the layer for the window.
   */
  void SetLayer (int layer);

  /**
   * Fullscreen for ewmh WMs.
   */
  void SetEWMHFullscreen (int action);

  void SetDecoration (bool d);
  
  void SetSizeHints (int x, 
                     int y, 
                     int imageWidth, 
                     int imageHeight, 
                     int windowWidth, 
                     int windowHeight);

  /**
   * Detects window manager type.
   */
  int GetWMType ();

  int GetGnomeLayer ();

  /**
   * Tests an atom.
   */
  int GetSupportedState (Atom atom);

  /**
   * Returns the root window's.
   */
  int GetWindowProperty (Atom type, 
                         Atom **args, 
                         unsigned long *nitems);

  void CalculateSize (int width, 
                      int height, 
                      bool doAspectCorrection);

  unsigned int FindXVPort ();

  void DumpCapabilities (int port);
};

#endif //XVWINDOW_H
