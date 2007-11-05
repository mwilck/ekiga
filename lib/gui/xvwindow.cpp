
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
 *                         XVWindow.cpp  -  description
 *                         ----------------------------
 *   begin                : Sun May 7 2006
 *   copyright            : (C) 2006 by Matthias Schneider <ma30002000@yahoo.de>
 *   description          : High-level class offering X-Video hardware 
 *                          acceleration.
 */


#include "xvwindow.h"

#include <object.h>

#define GUID_I420_PLANAR 0x30323449
#define GUID_YV12_PLANAR 0x32315659

#define wm_LAYER         1
#define wm_FULLSCREEN    2
#define wm_STAYS_ON_TOP  4
#define wm_ABOVE         8
#define wm_BELOW         16
#define wm_NETWM (wm_FULLSCREEN | wm_STAYS_ON_TOP | wm_ABOVE | wm_BELOW)

#define WIN_LAYER_ONBOTTOM    2
#define WIN_LAYER_NORMAL      4
#define WIN_LAYER_ONTOP       6
#define WIN_LAYER_ABOVE_DOCK 10

#define _NET_WM_STATE_REMOVE  0  /* remove/unset property */
#define _NET_WM_STATE_ADD     1  /* add/set property */
#define _NET_WM_STATE_TOGGLE  2  /* toggle property */

#define MWM_HINTS_FUNCTIONS   (1L << 0)
#define MWM_HINTS_DECORATIONS (1L << 1)
#define MWM_FUNC_RESIZE       (1L << 1)
#define MWM_FUNC_MOVE         (1L << 2)
#define MWM_FUNC_MINIMIZE     (1L << 3)
#define MWM_FUNC_MAXIMIZE     (1L << 4)
#define MWM_FUNC_CLOSE        (1L << 5)
#define MWM_DECOR_ALL         (1L << 0)
#define MWM_DECOR_MENU        (1L << 4)

#define PIP_RATIO_WIN  3
#define PIP_RATIO_FS   5
#define DEFAULT_X 1
#define DEFAULT_Y 1

typedef struct
{
  int flags;
  long functions;
  long decorations;
  long input_mode;
  long state;
} MotifWmHints;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>

#ifdef HAVE_SHM
#include <sys/shm.h>
#include <sys/ipc.h>
#endif

XVWindow::XVWindow()
{
  // initialize class variables
  _master = NULL;
  _slave = NULL;
  _XVPort = 0;
  _state.fullscreen = false;
  _state.ontop = false;
  _state.decoration = true;
  _state.origLayer=0;
  _display = NULL;
  _XVWindow = 0;
  _gc = NULL;
  _isInitialized = false;
  _XVImage = NULL;
  _embedded = false;
  _paintColorKey = false;
  _useShm = false;
#ifdef HAVE_SHM
  _XShmInfo.shmaddr = NULL;
#endif
}


int 
XVWindow::Init (Display* dp, 
                Window rootWindow, 
                GC gc, 
                int x, 
                int y,
                int windowWidth, 
                int windowHeight, 
                int imageWidth, 
                int imageHeight)
{
  // local variables needed for creation of window 
  // and initialization of XV extension
  unsigned int ver = 0;
  unsigned int rel = 0;
  unsigned int req = 0;
  unsigned int ev = 0;
  unsigned int err = 0;
  int ret = 0;
  int depth = 0;

  XSetWindowAttributes xswattributes;
  XWindowAttributes xwattributes;
  XVisualInfo xvinfo;

  _display = dp;
  _rootWindow = rootWindow;

  XLockDisplay (_display);

  // initialize atoms
  WM_DELETE_WINDOW = XInternAtom (_display, "WM_DELETE_WINDOW", False);
  XA_WIN_PROTOCOLS = XInternAtom (_display, "_WIN_PROTOCOLS", False);
  XA_NET_SUPPORTED = XInternAtom (_display, "_NET_SUPPORTED", False);
  XA_NET_WM_STATE = XInternAtom (_display, "_NET_WM_STATE", False);
  XA_NET_WM_STATE_FULLSCREEN = XInternAtom (_display, "_NET_WM_STATE_FULLSCREEN", False);
  XA_NET_WM_STATE_ABOVE = XInternAtom (_display, "_NET_WM_STATE_ABOVE", False);
  XA_NET_WM_STATE_STAYS_ON_TOP = XInternAtom (_display, "_NET_WM_STATE_STAYS_ON_TOP", False);
  XA_NET_WM_STATE_BELOW = XInternAtom (_display, "_NET_WM_STATE_BELOW", False);
  XV_SYNC_TO_VBLANK = None;
  XV_COLORKEY = None;
  XV_AUTOPAINT_COLORKEY = None;
  

  XSync (_display, false);

  // check if SHM XV window is possible
  ret = XvQueryExtension (_display, &ver, &rel, &req, &ev, &err);
  PTRACE(4, "XVideo\tXvQueryExtension: Version: " << ver << " Release: " << rel 
         << " Request Base: " << req << " Event Base: " << ev << " Error Base: " << err  );

  if (Success != ret) {
    if (ret == XvBadExtension)
      PTRACE(1, "XVideo\tXvQueryExtension failed - XvBadExtension");
    else if (ret == XvBadAlloc)
      PTRACE(1, "XVideo\tXvQueryExtension failed - XvBadAlloc");
    else
      PTRACE(1, "XVideo\tXQueryExtension failed");
    XUnlockDisplay (_display);
    return 0;
  }
  
  // Find XV port
  _XVPort = FindXVPort ();
  if (!_XVPort) {
    PTRACE(1, "XVideo\tFindXVPort failed");
    XUnlockDisplay(_display);
    return 0;
  } 

  PTRACE(4, "XVideo\tUsing XVideo port: " << _XVPort);

  XV_SYNC_TO_VBLANK = GetXVAtom("XV_SYNC_TO_VBLANK");
  XV_COLORKEY = GetXVAtom( "XV_COLORKEY" );
  XV_AUTOPAINT_COLORKEY = GetXVAtom( "XV_AUTOPAINT_COLORKEY" );    

  if ( !InitColorkey() )
  {
    PTRACE(1, "XVideo\tColorkey initialization failed");
    XUnlockDisplay(_display);
    return 0; 
  } 

  if (XV_SYNC_TO_VBLANK != None)
    if (XvSetPortAttribute(_display, _XVPort, XV_SYNC_TO_VBLANK, 1) == Success)
      PTRACE(4, "XVideo\tVertical sync successfully activated" );
     else
      PTRACE(4, "XVideo\tFailure when trying to activate vertical sync" );
  else
    PTRACE(4, "XVideo\tVertical sync not supported");

  if (!checkMaxSize (imageWidth, imageHeight)) {
    PTRACE(1, "XVideo\tCheck of image size failed");
    XUnlockDisplay(_display);
    return 0; 
  }

  XGetWindowAttributes (_display, _rootWindow, &xwattributes);
  depth = checkDepth(xwattributes.depth);
  XMatchVisualInfo (_display, DefaultScreen (_display), depth, TrueColor, &xvinfo);

  // define window properties and create the window
  xswattributes.colormap = XCreateColormap (_display, _rootWindow, xvinfo.visual, AllocNone);
  xswattributes.event_mask = StructureNotifyMask | ExposureMask;

  xswattributes.background_pixel = WhitePixel (_display, DefaultScreen (_display));

  xswattributes.border_pixel = WhitePixel (_display, DefaultScreen (_display));

  _XVWindow = XCreateWindow (_display, _rootWindow, x, y, windowWidth, windowHeight, 
                             0, xvinfo.depth, InputOutput, xvinfo.visual, 
                             CWBackPixel | CWBorderPixel | CWColormap | CWEventMask,  &xswattributes);

  // define inputs events
  XSelectInput (_display, _XVWindow, StructureNotifyMask | KeyPressMask | ButtonPressMask);

  XUnlockDisplay (_display);

  SetSizeHints (DEFAULT_X,DEFAULT_Y, imageWidth, imageHeight, windowWidth, windowHeight);
  
  XLockDisplay (_display);  

  // map the window
  XMapWindow (_display, _XVWindow);

  XSetWMProtocols (_display, _XVWindow, &WM_DELETE_WINDOW, 1);

  // Checking if we are going embedded or not
  if (gc) {
    _gc = gc; 
    _embedded = true;
  }
  else {
    _gc = XCreateGC (_display, _XVWindow, 0, 0);
    _embedded = false;
  }

#ifdef HAVE_SHM
   if (XShmQueryExtension (_display)) {
     _useShm = true;
     PTRACE(1, "XVideo\tXQueryShmExtension success");
   }
   else {
     _useShm = false;
     PTRACE(1, "XVideo\tXQueryShmExtension failed");
   }

  if (_useShm)
    ShmAttach(imageWidth, imageHeight);

  if (!_useShm) {
#endif
    _XVImage = (XvImage *) XvCreateImage( _display, _XVPort, GUID_YV12_PLANAR, 0, imageWidth, imageHeight);
    if (!_XVImage) {
      XUnlockDisplay (_display);
      return 0;
    }
    _XVImage->data = (char*) malloc(_XVImage->data_size);
    XSync(_display, False);
    PTRACE(1, "XVideo\tNot using SHM extension");
#ifdef HAVE_SHM
  }
  else {
      PTRACE(1, "XVideo\tUsing SHM extension");
  }
#endif


  _isInitialized = true;
  XUnlockDisplay (_display);

  // detect the window manager type
  _wmType = GetWMType ();
  CalculateSize (windowWidth, windowHeight, true);

  return 1;
}


XVWindow::~XVWindow()
{
#ifdef HAVE_SHM
    if (_useShm) {
      if (_isInitialized && _XShmInfo.shmaddr) {
        XLockDisplay (_display);
        XShmDetach (_display, &_XShmInfo);
        shmdt (_XShmInfo.shmaddr);
        XUnlockDisplay (_display);
      }
    } else
#endif
    {
      if ((_XVImage) && (_XVImage->data))
        free (_XVImage->data);
    }
  
  if (_XVImage) {

    XLockDisplay (_display);
    XFree (_XVImage);
    XUnlockDisplay (_display);
  }

  if (!_embedded && _gc) {
  
    XLockDisplay (_display);
    XFreeGC (_display, _gc);
    XUnlockDisplay (_display);
  }

  if (_XVPort) {
  
    XLockDisplay (_display);
    XvUngrabPort (_display, _XVPort, CurrentTime);
    XUnlockDisplay (_display);
  }

  if (_XVWindow) {
  
    XLockDisplay (_display);
    XUnmapWindow (_display, _XVWindow);
    XDestroyWindow (_display, _XVWindow);
    XFlush (_display);
    XUnlockDisplay (_display);
  }
}


void 
XVWindow::PutFrame (uint8_t* frame, 
                    uint16_t width, 
                    uint16_t height)
{
  XEvent event;

  if (!_XVImage) 
    return;
  
  if (width != _XVImage->width || height != _XVImage->height) {
     PTRACE (1, "XVideo\tDynamic switching of resolution not supported\n");
     return;
  }

  XLockDisplay (_display);


  if (!_embedded) {

    // event handling
    while (XPending (_display)) {

      XNextEvent (_display, &event);
      XUnlockDisplay (_display);
      if (event.type == ClientMessage) {
        // If "closeWindow" is clicked do nothing right now 
        // (window is closed from the GUI)
      }
      
      // the window size has changed
      if (event.type == ConfigureNotify) {
        
        XConfigureEvent *xce = (XConfigureEvent *) &event;
        
        // if a slave window exists it has to be resized as well
        if (_slave) 
          _slave->SetWindow (xce->width - (int) (xce->width / ( _state.fullscreen ? PIP_RATIO_FS : PIP_RATIO_WIN)),
                             xce->height - (int) (_slave->GetYUVHeight () * xce->width / ( _state.fullscreen ? PIP_RATIO_FS :  PIP_RATIO_WIN) / _slave->GetYUVWidth ()),
                             (int) (xce->width / ( _state.fullscreen ? PIP_RATIO_FS :  PIP_RATIO_WIN)),
                             (int) (_slave->GetYUVHeight () * xce->width / ( _state.fullscreen ? PIP_RATIO_FS :  PIP_RATIO_WIN) / _slave->GetYUVWidth ()));
        
        CalculateSize (xce->width, xce->height, true);
        if( _paintColorKey ) {

          XSetForeground( _display, _gc, _colorKey );
          XFillRectangle( _display, _XVWindow, _gc, _state.curX, _state.curY, _state.curWidth, _state.curHeight);
        }
      }

      if ((event.type == Expose) && (_paintColorKey)) {

        XSetForeground( _display, _gc, _colorKey );
        XFillRectangle( _display, _XVWindow, _gc, _state.curX, _state.curY, _state.curWidth, _state.curHeight);
      }

      // a key is pressed
      if (event.type == KeyPress) {
        
        XKeyEvent *xke = (XKeyEvent *) &event;
        switch (xke->keycode) {
          case 41:  
            if (_master) 
              _master->ToggleFullscreen (); 
            else 
              ToggleFullscreen (); // "f"
            break;
          case 40:  
            if (_master) 
              _master->ToggleDecoration (); 
            else 
              ToggleDecoration (); // "d"
            break;
          case 32:  
            if (_master) 
              _master->ToggleOntop (); 
            else 
              ToggleOntop ();      // "o"
            break;
          case 9:   
            if (_master) { 
              if (_master->IsFullScreen ()) 
                _master->ToggleFullscreen(); 
            } // esc
            else { 
              if (IsFullScreen ()) 
                ToggleFullscreen(); 
            }
            break;
	default:
	  break;
        }
      }

      // a mouse button is clicked
      if (event.type == ButtonPress) {

        if (_master)
          if (!_master->HasDecoration())
          _master->ToggleDecoration();
          else
          _master->ToggleFullscreen();
        else 
          if (!_state.decoration)
            ToggleDecoration();
          else
            ToggleFullscreen();
      }

      XLockDisplay (_display);
    }
  }

  if (_XVImage->pitches [0] ==_XVImage->width
      && _XVImage->pitches [2] == (int) (_XVImage->width / 2) 
      && _XVImage->pitches [1] == (int) (_XVImage->width / 2)) {
  
    memcpy (_XVImage->data, 
            frame, 
            (int) (_XVImage->width * _XVImage->height));
    memcpy (_XVImage->data + (int) (_XVImage->width * _XVImage->height), 
            frame + _XVImage->offsets [2], 
            (int) (_XVImage->width * _XVImage->height / 4));
    memcpy (_XVImage->data + (int) (_XVImage->width * _XVImage->height * 5 / 4), 
            frame + _XVImage->offsets [1], 
            (int) (_XVImage->width * _XVImage->height / 4));
  } 
  else {
  
    unsigned int i = 0;
    int width2 = (int) (_XVImage->width / 2);

    uint8_t* dstY = (uint8_t*) _XVImage->data;
    uint8_t* dstV = (uint8_t*) _XVImage->data + (_XVImage->pitches [0] * _XVImage->height);
    uint8_t* dstU = (uint8_t*) _XVImage->data + (_XVImage->pitches [0] * _XVImage->height) 
                                              + (_XVImage->pitches [1] * (_XVImage->height/2));

    uint8_t* srcY = frame;
    uint8_t* srcV = frame + (int) (_XVImage->width * _XVImage->height * 5 / 4);
    uint8_t* srcU = frame + (int) (_XVImage->width * _XVImage->height);

    for (i = 0 ; i < (unsigned int)_XVImage->height ; i+=2) {

      memcpy (dstY, srcY, _XVImage->width); 
      dstY +=_XVImage->pitches [0]; 
      srcY +=_XVImage->width;
      
      memcpy (dstY, srcY, _XVImage->width); 
      dstY +=_XVImage->pitches [0]; 
      srcY +=_XVImage->width;
      
      memcpy (dstV, srcV, width2); 
      dstV +=_XVImage->pitches [1]; 
      srcV += width2;
      
      memcpy(dstU, srcU, width2); dstU+=_XVImage->pitches [2]; 
      srcU += width2;
    }
  }
#ifdef HAVE_SHM
  if (_useShm) {
    XvShmPutImage (_display, _XVPort, _XVWindow, _gc, _XVImage, 
                  0, 0, _XVImage->width, _XVImage->height, 
                  _state.curX, _state.curY, _state.curWidth, _state.curHeight, false);
  }
  else
#endif
  {
    XvPutImage (_display, _XVPort, _XVWindow, _gc, _XVImage, 
                  0, 0, _XVImage->width, _XVImage->height, 
                  _state.curX, _state.curY, _state.curWidth, _state.curHeight);
  }

  XSync (_display, false);
  XUnlockDisplay (_display);
}


void 
XVWindow::ToggleFullscreen ()
{
  Window childWindow;
  XWindowAttributes xwattributes;

  int newX = 0;
  int newY = 0;
  int newWidth = 0;
  int newHeight = 0;

  if (_state.fullscreen) {
    
    // not needed with EWMH fs
    if (!(_wmType & wm_FULLSCREEN)) {
      
      newX = _state.oldx;
      newY = _state.oldy;
      newWidth = _state.oldWidth;
      newHeight = _state.oldHeight;
      SetDecoration (true);
    }

    // removes fullscreen state if wm supports EWMH
    SetEWMHFullscreen (_NET_WM_STATE_REMOVE);
  } 
  else {

    // sets fullscreen state if wm supports EWMH
    SetEWMHFullscreen (_NET_WM_STATE_ADD);

    // not needed with EWMH fs - save window coordinates/size 
    // and discover fullscreen window size
    if (!(_wmType & wm_FULLSCREEN)) {

      XLockDisplay (_display);

      newX = 0;
      newY = 0;
      newWidth = DisplayWidth (_display, DefaultScreen (_display));
      newHeight = DisplayHeight (_display, DefaultScreen (_display));

      SetDecoration (false);
      XFlush (_display);

      XTranslateCoordinates (_display, _XVWindow, RootWindow (_display, DefaultScreen (_display)), 
                             0, 0, &_state.oldx, &_state.oldy, &childWindow);

      XGetWindowAttributes (_display, _XVWindow, &xwattributes);
      XUnlockDisplay (_display);
      
      _state.oldWidth = xwattributes.width;
      _state.oldHeight = xwattributes.height;
    }
  }
  
  // not needed with EWMH fs - create a screen-filling window on top 
  // and turn of decorations
  if (!(_wmType & wm_FULLSCREEN)) {

    SetSizeHints (newX, newY, _XVImage->width, _XVImage->height, newWidth, newHeight);

    XLockDisplay (_display);
    SetLayer (!_state.fullscreen ? 0 : 1);
    XMoveResizeWindow (_display, _XVWindow, newX, newY, newWidth, newHeight);
    XUnlockDisplay (_display);
  }

  // some WMs lose ontop after fullscreeen
  if (_state.fullscreen & _state.ontop)
    SetLayer (1);

  XLockDisplay (_display);
  XMapRaised (_display, _XVWindow);
  XRaiseWindow (_display, _XVWindow);
  XFlush (_display);
  XUnlockDisplay (_display);

  _state.fullscreen = !_state.fullscreen;
}


void 
XVWindow::ToggleDecoration ()
{
  SetDecoration (!_state.decoration);
}


void 
XVWindow::SetEWMHFullscreen (int action)
{
  if (_wmType & wm_FULLSCREEN) {

    // create an event event to toggle fullscreen mode
    XEvent xev;
    
    xev.xclient.type = ClientMessage;
    xev.xclient.serial = 0;
    xev.xclient.send_event = True;
    xev.xclient.message_type = XInternAtom (_display, "_NET_WM_STATE", False);
    xev.xclient.window = _XVWindow;
    xev.xclient.format = 32;
    
    xev.xclient.data.l[0] = action;
    xev.xclient.data.l[1] = XInternAtom (_display, "_NET_WM_STATE_FULLSCREEN", False);
    xev.xclient.data.l[2] = 0;
    xev.xclient.data.l[3] = 0;
    xev.xclient.data.l[4] = 0;

    // send the event to the window
    XLockDisplay (_display);
    if (!XSendEvent (_display, _rootWindow, FALSE, SubstructureRedirectMask | SubstructureNotifyMask, &xev))
      PTRACE(1, "XVideo\tSetEWMHFullscreen failed");
    XUnlockDisplay (_display);
  }
}


void 
XVWindow::ToggleOntop ()
{
  SetLayer (_state.ontop ? 0 : 1);
  _state.ontop = !_state.ontop;
}


void 
XVWindow::SetLayer (int layer)
{
  char *state = NULL;

  Window mRootWin = RootWindow (_display, DefaultScreen (_display));
  XClientMessageEvent xev;
  memset (&xev, 0, sizeof(xev));

  if (_wmType & wm_LAYER) {

    if (!_state.origLayer) 
      _state.origLayer = GetGnomeLayer ();
    
    xev.type = ClientMessage;
    xev.display = _display;
    xev.window = _XVWindow;
    xev.message_type = XA_WIN_LAYER;
    xev.format = 32;
    xev.data.l [0] = layer ? WIN_LAYER_ABOVE_DOCK : _state.origLayer;
    xev.data.l [1] = CurrentTime;
    PTRACE(4, "XVideo\tLayered style stay on top (layer " << xev.data.l[0] << ")");
    
    XLockDisplay (_display);
    XSendEvent (_display, mRootWin, FALSE, SubstructureNotifyMask, (XEvent *) &xev);
    XUnlockDisplay (_display);

  } 
  else if (_wmType & wm_NETWM) {

    xev.type = ClientMessage;
    xev.message_type = XA_NET_WM_STATE;
    xev.display = _display;
    xev.window = _XVWindow;
    xev.format = 32;
    xev.data.l [0] = layer;

    if (_wmType & wm_STAYS_ON_TOP) 
      xev.data.l [1] = XA_NET_WM_STATE_STAYS_ON_TOP;
    else 
      if (_wmType & wm_ABOVE) 
        xev.data.l [1] = XA_NET_WM_STATE_ABOVE;
    else 
      if (_wmType & wm_FULLSCREEN) 
        xev.data.l [1] = XA_NET_WM_STATE_FULLSCREEN;
    else 
      if (_wmType & wm_BELOW) 
        xev.data.l [1] = XA_NET_WM_STATE_BELOW;

    XLockDisplay (_display);
    XSendEvent (_display, mRootWin, FALSE, SubstructureRedirectMask, (XEvent *) &xev);
    state = XGetAtomName (_display, xev.data.l [1]);
    PTRACE(4, "XVideo\tNET style stay on top (layer " << layer << "). Using state " << state );
    XFree (state);
    XUnlockDisplay (_display);
  }
}


int 
XVWindow::GetWMType ()
{
  Atom *args = NULL;

  unsigned int i = 0;
  int wmType = 0;
  int metacityHack = 0;
  unsigned long nitems = 0;

  // check if WM supports layers
  if (GetWindowProperty (XA_WIN_PROTOCOLS, &args, &nitems)) {
    
    PTRACE(4, "XVideo\tDetected WM supports layers");
    for (i = 0; i < nitems; i++) {
      
      if (args [i] == XA_WIN_LAYER) {
        wmType |= wm_LAYER;
        metacityHack |= 1;
      } 
      else 
        metacityHack |= 2;
    }

    XLockDisplay (_display);
    XFree (args);
    XUnlockDisplay (_display);

    // metacity WM reports that it supports layers, 
    // but it is not really true :-)
    if (wmType && metacityHack == 1) {
      wmType ^= wm_LAYER;
      PTRACE(4, "XVideo\tUsing workaround for Metacity bug");
    }
  }

  // NETWM
  if (GetWindowProperty (XA_NET_SUPPORTED, &args, &nitems)) {
    
    PTRACE(4, "XVideo\tDetected wm supports NetWM.");

    for (i = 0; i < nitems; i++) 
      wmType |= GetSupportedState (args[i]);
    
    XLockDisplay (_display);
    XFree (args);
    XUnlockDisplay (_display);
  }

  // unknown WM
  if (wmType == 0) 
    PTRACE(4, "XVideo\tUnknown wm type...");
  
  return wmType;
}


void 
XVWindow::GetWindow (int *x, 
                     int *y, 
                     unsigned int *windowWidth, 
                     unsigned int *windowHeight)
{
  unsigned int ud = 0; 
  Window _dw;
  
  int oldx = 0; 
  int oldy = 0;
  
  Window root;
  bool decoration = false;
  
  decoration = _state.decoration;
  SetDecoration (false);

  XLockDisplay (_display);
  XSync (_display,false); 
  XGetGeometry (_display, _XVWindow, &root, &oldx, &oldy, windowWidth, windowHeight, &ud, &ud);
  XTranslateCoordinates (_display, _XVWindow, root, oldx, oldy, x, y, &_dw);
  XUnlockDisplay (_display);

  SetDecoration (decoration);
}


void 
XVWindow::SetWindow (int x, 
                     int y, 
                     unsigned int windowWidth, 
                     unsigned int windowHeight)
{
  XLockDisplay (_display);
  XMoveResizeWindow (_display, _XVWindow, x, y, windowWidth, windowHeight);
  XUnlockDisplay (_display);
  CalculateSize (windowWidth, windowHeight, true);
}


int 
XVWindow::GetWindowProperty (Atom type, 
                             Atom **args, 
                             unsigned long *nitems)
{
  int format = 0;
  unsigned long bytesafter = 0;
  int ret = 0;

  XLockDisplay(_display);
  ret = (Success == XGetWindowProperty (_display, _rootWindow, type, 0, 16384, false,
         AnyPropertyType, &type, &format, nitems, &bytesafter, (unsigned char **) args) && *nitems > 0);
  XUnlockDisplay(_display);

  return ret; 
}


int 
XVWindow::GetSupportedState (Atom atom)
{
  if (atom == XA_NET_WM_STATE_FULLSCREEN)
    return wm_FULLSCREEN;

  if (atom == XA_NET_WM_STATE_ABOVE)
    return wm_ABOVE;

  if (atom == XA_NET_WM_STATE_STAYS_ON_TOP)
    return wm_STAYS_ON_TOP;

  if (atom==XA_NET_WM_STATE_BELOW)
    return wm_BELOW;

  return 0;
}


void 
XVWindow::SetSizeHints (int x, 
                        int y, 
                        int imageWidth, 
                        int imageHeight, 
                        int windowWidth, 
                        int windowHeight)
{
  XSizeHints xshints;

  xshints.flags = PPosition | PSize | PAspect | PMinSize;

  xshints.min_aspect.x = imageWidth;
  xshints.min_aspect.y = imageHeight;
  xshints.max_aspect.x = imageWidth;
  xshints.max_aspect.y = imageHeight;

  xshints.x = x;
  xshints.y = y;
  xshints.width = windowWidth;
  xshints.height = windowHeight;
  xshints.min_width = 25;
  xshints.min_height = 25;
  
  XLockDisplay (_display);
  XSetStandardProperties (_display, _XVWindow, "Video", "Video", None, NULL, 0, &xshints);
  XUnlockDisplay (_display);
}


void 
XVWindow::SetDecoration (bool d)
{
  Atom motifHints;
  Atom mType;
  
  MotifWmHints setHints;
  MotifWmHints *getHints = NULL;
  
  unsigned char *args = NULL;

  int mFormat = 0;
  unsigned long mn = 0;
  unsigned long mb = 0;

  static unsigned int oldDecor = MWM_DECOR_ALL;
  static unsigned int oldFuncs = MWM_FUNC_MOVE | MWM_FUNC_CLOSE | MWM_FUNC_MINIMIZE | MWM_FUNC_MAXIMIZE | MWM_FUNC_RESIZE;

  XLockDisplay (_display);
  motifHints = XInternAtom (_display, "_MOTIF_WM_HINTS", 0);
  XUnlockDisplay (_display);

  if (motifHints != None) {

    memset (&setHints, 0, sizeof (setHints));

    if (!d) {

      XLockDisplay (_display);
      XGetWindowProperty (_display, _XVWindow, motifHints, 0, 20, False, motifHints, &mType, &mFormat, &mn, &mb, &args);
      getHints = (MotifWmHints*) args;

      if (getHints) {

        if (getHints->flags & MWM_HINTS_DECORATIONS) 
          oldDecor = getHints->decorations;
        
        if (getHints->flags & MWM_HINTS_FUNCTIONS) 
          oldFuncs = getHints->functions;
        
        XFree(getHints);
      }

      XUnlockDisplay(_display);
      
      setHints.decorations = 0;
    } 
    else {
    
      setHints.functions = oldFuncs;
      setHints.decorations = oldDecor;
    }

    setHints.flags = MWM_HINTS_FUNCTIONS | MWM_HINTS_DECORATIONS;
    
    XLockDisplay (_display);
    XChangeProperty (_display, _XVWindow, motifHints, motifHints, 32, PropModeReplace, (unsigned char *) &setHints, 5);
    XUnlockDisplay (_display);
    
    _state.decoration=!_state.decoration;
  }
}


int 
XVWindow::GetGnomeLayer ()
{
  Atom type;

  int format = 0;
  unsigned long count = 0;
  unsigned long bytesafter = 0;
  unsigned char *prop = NULL;

  long layer = WIN_LAYER_NORMAL;

  XLockDisplay (_display);
  if (XGetWindowProperty (_display, _XVWindow,XA_WIN_LAYER, 0, 16384, false, XA_CARDINAL, &type, &format, &count, &bytesafter, &prop) 
      == Success && prop) {
    
    if (type == XA_CARDINAL && format == 32 && count == 1) 
      layer = ((long *) prop) [0];

    XFree(prop);
  }
  XUnlockDisplay(_display);

  return layer;
}


void 
XVWindow::CalculateSize (int width, 
                         int height, 
                         bool doAspectCorrection) 
{
  if ( doAspectCorrection && ((width * _XVImage->height / _XVImage->width) > height)) {

    _state.curX = (int) ((width - (height * _XVImage->width / _XVImage->height)) / 2);
    _state.curY = 0;
    _state.curWidth = (int) (height * _XVImage->width / _XVImage->height);
    _state.curHeight = height;

  } 
  else if ( doAspectCorrection && ((height * _XVImage->width / _XVImage->height) > width)) {

    _state.curX = 0;
    _state.curY = (int) ((height - (width * _XVImage->height / _XVImage->width)) / 2);
    _state.curWidth = width;
    _state.curHeight = (int) (width * _XVImage->height / _XVImage->width);
  } 
  else {

    _state.curX = 0;
    _state.curY = 0;
    _state.curWidth = width;
    _state.curHeight = height;
  }
}


unsigned int 
XVWindow::FindXVPort () 
{
  XvAdaptorInfo *xvainfo = NULL;
  XvImageFormatValues *xviformats = NULL;

  unsigned int numXvainfo = 0;
  unsigned int i = 0;
  unsigned int candidateXVPort = 0;
  unsigned int busyPorts = 0;

  int numXviformats = 0;
  int j = 0;

  int result = 0;
  bool supportsYV12 = false;

  if (Success != XvQueryAdaptors (_display, _rootWindow, &numXvainfo, &xvainfo)) {
    PTRACE (1, "XVideo\tXQueryAdaptor failed");
    return 0;
  }

  for (i = 0; i < numXvainfo; i++) {

    char adaptorInfo [512];
    sprintf (adaptorInfo, "XVideo\t#%d, Adaptor: %s, type: %s%s%s%s%s, ports: %ld, first port: %ld", i,
             xvainfo[i].name,
             (xvainfo[i].type & XvInputMask)?"input | ":"",
             (xvainfo[i].type & XvOutputMask)?"output | ":"",
             (xvainfo[i].type & XvVideoMask)?"video | ":"",
             (xvainfo[i].type & XvStillMask)?"still | ":"",
             (xvainfo[i].type & XvImageMask)?"image | ":"",
             xvainfo[i].num_ports,
             xvainfo[i].base_id);
    PTRACE (4, adaptorInfo);

    if ((xvainfo[i].type & XvInputMask) && (xvainfo[i].type & XvImageMask)) {

      for (candidateXVPort = xvainfo [i].base_id ; candidateXVPort < (xvainfo [i].base_id + xvainfo [i].num_ports) ; ++candidateXVPort) {

        if (PTrace::CanTrace (4)) 
          DumpCapabilities (candidateXVPort);

        // Check if the Port supports YV12/YUV colorspace
        supportsYV12 = false;
        xviformats = XvListImageFormats (_display, candidateXVPort, &numXviformats); 

        for (j = 0 ; j < numXviformats ; j++)
          if (xviformats [j].id == GUID_YV12_PLANAR) 
            supportsYV12 = true;

        if (xviformats) 
          XFree (xviformats);

        if (!supportsYV12) {

          PTRACE(4, "XVideo\tPort " << candidateXVPort << " does not support YV12 colorspace");
        } 
        else {

          result = XvGrabPort (_display, candidateXVPort, CurrentTime);
          if ( result == Success) {

            PTRACE(4, "XVideo\tGrabbed Port: " << candidateXVPort);
            XvFreeAdaptorInfo (xvainfo);

            return candidateXVPort;
          } 
          else {

            switch (result) 
              {
              case XvInvalidTime: 
                PTRACE (4, "XVideo\tCould not grab port " << candidateXVPort << ": requested time is older than the current port time"); 
                break;

              case XvAlreadyGrabbed: 
                PTRACE (4, "XVideo\tCould not grab port " << candidateXVPort << ": port is already grabbed by another client"); 
                break;

              case XvBadExtension: 
                PTRACE (4, "XVideo\tCould not grab port " << candidateXVPort << ": XV extension is unavailable"); 
                break;

              case XvBadAlloc: 
                PTRACE (4, "XVideo\tCould not grab port " << candidateXVPort << ": XvGrabPort failed to allocate memory to process the request"); 
                break;

              case XvBadPort: 
                PTRACE(4, "XVideo\tCould not grab port " << candidateXVPort << ": port does not exist"); 
                break;

              default: 
                PTRACE(4, "XVideo\tCould not grab port " << candidateXVPort);
              }

            ++busyPorts;
          }
        }
      }
    }
  }

  if (busyPorts) 
    PTRACE(1, "XVideo\tCould not find any free Xvideo port - maybe other processes are already using them");
  else 
    PTRACE(1, "XVideo\tIt seems there is no Xvideo support for your video card available");
  XvFreeAdaptorInfo (xvainfo);

  return 0;
}


void 
XVWindow::DumpCapabilities (int port) 
{
  XvImageFormatValues *xviformats = 0;
  XvAttribute *xvattributes = NULL;
  XvEncodingInfo *xveinfo = NULL;

  unsigned int numXveinfo = 0;
  unsigned int i = 0;

  int numXvattributes = 0;
  int j = 0;
  int numXviformats = 0;

  char info[512];

  if (XvQueryEncodings (_display, port, &numXveinfo, &xveinfo) != Success) {
    PTRACE(4, "XVideo\tXvQueryEncodings failed\n");
    return;
  }

  for (i = 0 ; i < numXveinfo ; i++) {
    PTRACE(4, "XVideo\tEncoding List for Port " << port << ": "
           << " id="          << xveinfo [i].encoding_id
           << " name="        << xveinfo [i].name 
           << " size="        << xveinfo [i].width << "x" << xveinfo[i].height
           << " numerator="   << xveinfo [i].rate.numerator
           << " denominator=" << xveinfo [i].rate.denominator);
  }
  XvFreeEncodingInfo(xveinfo);

  PTRACE(4, "XVideo\tAttribute List for Port " << port << ":");
  xvattributes = XvQueryPortAttributes (_display, port, &numXvattributes);
  for (j = 0 ; j < numXvattributes ; j++) {
    PTRACE(4, "  name:       " << xvattributes [j].name);
    PTRACE(4, "  flags:     " << ((xvattributes [j].flags & XvGettable) ? " get" : "") << ((xvattributes [j].flags & XvSettable) ? " set" : ""));
    PTRACE(4, "  min_color:  " << xvattributes [j].min_value);
    PTRACE(4, "  max_color:  " << xvattributes [j].max_value);
  }

  if (xvattributes) 
    XFree (xvattributes);

  PTRACE (4, "XVideo\tImage format list for Port " << port << ":");
  xviformats = XvListImageFormats (_display, port, &numXviformats);
  for (j = 0 ; j < numXviformats ; j++) {

    sprintf (info, "  0x%x (%4.4s) %s, order: %s",
             xviformats [j].id,
             (char *) &xviformats [j].id,
             (xviformats [j].format == XvPacked) ? "packed" : "planar",
             xviformats [j].component_order);
    PTRACE(4, info);
  }

  if (xviformats) 
    XFree (xviformats);
}

Atom XVWindow::GetXVAtom ( char const * name )
{
  XvAttribute * attributes;
  int numAttributes = 0;
  int i;
  Atom atom = None;

  attributes = XvQueryPortAttributes( _display, _XVPort, &numAttributes );
  if( attributes != NULL ) {
  
    for ( i = 0; i < numAttributes; ++i ) {

      if ( strcmp(attributes[i].name, name ) == 0 ) {

        atom = XInternAtom( _display, name, False );
        break; 
      }
    }
    XFree( attributes );
  }

  return atom;
}

bool XVWindow::InitColorkey()
{
  if( XV_COLORKEY != None ) {

    if ( XvGetPortAttribute(_display,_XVPort, XV_COLORKEY, &_colorKey) == Success )
      PTRACE(4, "XVideo\tUsing colorkey " << _colorKey );
    else {

      PTRACE(1, "XVideo\tCould not get colorkey! Maybe the selected Xv port has no overlay." );
      return false; 
    }

    if ( XV_AUTOPAINT_COLORKEY != None ) {

      if ( XvSetPortAttribute( _display, _XVPort, XV_AUTOPAINT_COLORKEY, 1 ) == Success )
        PTRACE(4, "XVideo\tColorkey method: AUTOPAINT");
      else {

        _paintColorKey = true;
        PTRACE(4, "XVideo\tFailed to set XV_AUTOPAINT_COLORKEY");
        PTRACE(4, "XVideo\tColorkey method: MANUAL");
      }
    }
    else {

      _paintColorKey = true;
      PTRACE(4, "XVideo\tXV_AUTOPAINT_COLORKEY not supported");
      PTRACE(4, "XVideo\tColorkey method: MANUAL");
    }
  }
  else {

    PTRACE(4, "XVideo\tColorkey method: NONE");
  } 

  return true; 
}

bool XVWindow::checkMaxSize(unsigned int width, unsigned int height)
{
  XvEncodingInfo * xveinfo;
  unsigned int numXveinfo = 0;
  unsigned int i;
  bool ret = false;

  if (XvQueryEncodings (_display, _XVPort, &numXveinfo, &xveinfo) != Success) {

    PTRACE(4, "XVideo\tXvQueryEncodings failed\n");
    return false;
  }

  for (i = 0 ; i < numXveinfo ; i++) {

    if ( strcmp( xveinfo[i].name, "XV_IMAGE" ) == 0 ) {

      if ( (width <= xveinfo[i].width  ) ||
           (height <= xveinfo[i].height) )
        ret = true;
      else {

        PTRACE(1, "XVideo\tRequested resolution " << width << "x" << height 
	         << " higher than maximum supported resolution " 
		 << xveinfo[i].width << "x" << xveinfo[i].height);
        ret = false;
      }
      break;
    }
  }

  XvFreeEncodingInfo(xveinfo);
  return ret;
}

int  XVWindow::checkDepth (int depth) {
  if (depth != 15 && depth != 16 && depth != 24 && depth != 32)
      return (24);
  else 
    return (depth);
}

#ifdef HAVE_SHM
void XVWindow::ShmAttach(int imageWidth, int imageHeight)
{
  if (_useShm) {
    // create the shared memory portion
    _XVImage = XvShmCreateImage (_display, _XVPort, GUID_YV12_PLANAR, 0, imageWidth, imageHeight, &_XShmInfo);

    if (_XVImage == NULL) {
      PTRACE(1, "XVideo\tXShmCreateImage failed");
      _useShm = false;
    }

    if ((_XVImage) && (_XVImage->id != GUID_YV12_PLANAR)) {
      PTRACE(1, "XVideo\t  XvShmCreateImage returned a different colorspace than YV12");
      XFree (_XVImage);
      _useShm = false;
    }
  }

  if ( (_useShm) && (PTrace::CanTrace (4)) ) {
    int i = 0;
    PTRACE(4, "XVideo\tCreated XvImage (" << _XVImage->width << "x" << _XVImage->height 
          << ", data size: " << _XVImage->data_size << ", num_planes: " << _XVImage->num_planes);

    for (i = 0 ; i < _XVImage->num_planes ; i++) 
      PTRACE(4, "XVideo\t  Plane " << i << ": pitch=" << _XVImage->pitches [i] << ", offset=" << _XVImage->offsets [i]);
  }

  if (_useShm) {
    _XShmInfo.shmid = shmget (IPC_PRIVATE, _XVImage->data_size, IPC_CREAT | 0777);
    if (_XShmInfo.shmid < 0) {
      XFree (_XVImage);
      PTRACE(1, "XVideo\tshmget failed"); //strerror(errno)
      _useShm = FALSE;
    }
  }

  if (_useShm) {
    _XShmInfo.shmaddr = (char *) shmat(_XShmInfo.shmid, 0, 0);
    if (_XShmInfo.shmaddr == ((char *) -1)) {
      XFree (_XVImage);
      if (_XShmInfo.shmaddr != ((char *) -1))
        shmdt(_XShmInfo.shmaddr);
      PTRACE(1, "XVideo\tshmat failed"); //strerror(errno)
      _useShm = FALSE;
    }
  }

  if (_useShm) {
    _XVImage->data = _XShmInfo.shmaddr;
    _XShmInfo.readOnly = False;

    // Attaching the shared memory to the display
    if (!XShmAttach (_display, &_XShmInfo)) {
      XFree (_XVImage);
      if (_XShmInfo.shmaddr != ((char *) -1))
        shmdt(_XShmInfo.shmaddr);
      PTRACE(1, "XVideo\t  XShmAttach failed");
      _useShm = false;
    } 
  } 

  if (_useShm) {
    XSync(_display, False);
    shmctl(_XShmInfo.shmid, IPC_RMID, 0);
  }
}
#endif
