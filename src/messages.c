/****************************************************************************
 *
 *    snake - a videogame derived from Q-Basic nibbles
 *    Copyright (C) 1999 "Sandro Santilli" <strk@keybit.net>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ****************************************************************************
 * $Id: messages.c,v 1.2 2004/07/28 13:33:13 strk Exp $
 ****************************************************************************/

#include "sins.h"
#include <stdarg.h>
#include <stdio.h>

/* Message timeout in milliseconds */
#define DEFAULT_MESSAGE_TIMEOUT 2000;

/*
 * public functions 
 */
void message (char *format, ...);
void expire_messages (void);

/*
 * private data 
 */
static int deftimer=DEFAULT_MESSAGE_TIMEOUT;
static int timer;

void
message (char *format, ...)
{
  char str[1024];
  va_list ap;
  extern int screen_initialized;

  va_start (ap, format);
  vsnprintf (str, 1024, format, ap);
  va_end(ap);

  if (screen_initialized)
  {
    ui_drawmessage (str);
    timer = deftimer;
  }
  else
  {
    fprintf (stderr, "%s\n", str);
  }
}

/*
 * expire message 
 */
void
expire_messages ()
{
  /* timer is in milliseconds */
  if (timer && (timer=timer-(delay/1000)-1) <= 0 )
  {
    ui_drawmessage ("");
    timer = 0;
  }
}
