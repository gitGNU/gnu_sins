/*
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
 */

#include "sins.h"

/* extern data */
extern long delay;

/* public functions */
void set_delay (int);
void lower_delay (void);
void higher_delay (void);
int CMD_delay (char *);

void
lower_delay ()
{
  /*delay -= delay / 2 + 1;*/
  delay -= DELAYSTEP;
  if (delay < MINDELAY) delay = MINDELAY;
  message("Delay %d", delay);
}

void
higher_delay ()
{
  /*delay += delay / 2 + 1;*/
  delay += DELAYSTEP;
  if (delay > MAXDELAY) delay = MAXDELAY;
  message("Delay %d", delay);
}

void
set_delay (int ms)
{
  if (ms > MAXDELAY) delay = MAXDELAY;
  else if (ms < MINDELAY) delay = MINDELAY;
  else delay = ms;
  message("Delay %d", delay);
}

/* 'delay' command handler */
int
CMD_delay (char *s)
{
  if ( ! s || ! *s )
  {
    message("Delay %d", delay);
  }
  else
  {
    set_delay (atoi (s));
  }
  return 1;
}
