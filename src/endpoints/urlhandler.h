
/* Ekiga -- A VoIP and Video-Conferencing application
 * Copyright (C) 2000-2006 Damien Sandras
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
 *                          urlhandler.h  -  description
 *                          ----------------------------
 *   begin                : Sat Jun 8 2002
 *   copyright            : (C) 2000-2006 by Damien Sandras
 *   description          : Multithreaded class to call a given URL or to
 *                          answer a call.
 *
 */


#ifndef _URLHANDLER_H_
#define _URLHANDLER_H_

#include "common.h"

class GMURL
{

 public:

  GMURL ();
  GMURL (PString);
  GMURL (const GMURL &);
  BOOL IsEmpty ();
  BOOL IsSupported ();
  BOOL Find (GMURL);
  PString GetType ();
  PString GetFullURL ();
  PString GetURL ();
  PString GetCanonicalURL ();
  PString GetCalltoServer ();
  PString GetCalltoEmail ();
  PString GetDefaultURL ();
  BOOL operator == (GMURL);
  BOOL operator != (GMURL);
  
 private:
  void Parse ();

  PString url;
  PString port;
  PString type;
  BOOL is_supported;
};


class GMURLHandler : public PThread
{
  PCLASSINFO(GMURLHandler, PThread);


public:

  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Initialise the parameters. GM will call the given URL
   *                 after having parsed it (or transfer to it).
   * PRE          :  The URL, transfer the call to the URL if true, else
   *                 call the url.
   */
  GMURLHandler (PString, BOOL = FALSE);

  
  /* DESCRIPTION  :  The destructor.
   * BEHAVIOR     :  /
   * PRE          :  /
   */
  ~GMURLHandler ();


  /* DESCRIPTION  :  The constructor.
   * BEHAVIOR     :  Parses the URL and establish the call if URL ok or
   *                 user found in ILS directory.
   * PRE          :  /
   */
  void Main ();


protected:

  GMURL url;
  BOOL answer_call;
  BOOL transfer_call;
  PMutex quit_mutex;
};
#endif
