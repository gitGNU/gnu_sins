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
 * $Id: ui_TEMPLATE.c,v 1.2 2004/07/28 13:33:13 strk Exp $
 ****************************************************************************
 *
 * purpose: show how to handle UI 
 *
 ****************************************************************************/

#include "sins.h"


/* public functions */
int ui_init (void);
int ui_initarena (ARENA *);
void ui_finish (void);
void ui_drawarena (ARENA *);
void ui_drawmessage (char *);
char *ui_prompt (char *);
int ui_getkey (void);

/* public data */
int screen_initialized = 0;

/* Initialize UI module */
int
ui_init ()
{
  return 1;
}

/* initialize the graphics */
int
ui_initarena (ARENA *arena)
{
  return 1;
}

/* message drawing function */
void
ui_drawmessage (char *str)
{
}

/* arena drawing */
void
ui_drawarena (ARENA *arena)
{
}


void
ui_finish ()
{
}


/*
 * Get next key in input buffer
 */
int
ui_getkey ()
{
  return -1;
}

/*
 * prompt the user for a string
 * the line returnered is allocated with malloc(3)
 * so the caller must free it when finished with it. 
 */
char *
ui_prompt (char *prompt_string)
{
  return NULL;
}

