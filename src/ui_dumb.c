/****************************************************************************
 *
 *    sins - a videogame derived from Q-Basic nibbles
 *
 *    Copyright (C) 1999-2009 "Sandro Santilli" <strk@keybit.net>
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
 * $Id: ui_dumb.c,v 1.3 2009/03/30 09:47:08 strk Exp $
 ****************************************************************************/


/*
 * file: ui_TEMPLATE.c *  purpose: handle dumb UI 
 */

#include "sins.h"
#include <stdio.h>

#define LINES 25
#define COLS 80
#define XOFFSET 1
#define YOFFSET 1

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

/* private data */
static char *screenboard;

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
  int y,x;

  /* won't work with sizes other then defined one */
  if ( arena->lines || arena->cols )
  {
    message("ignoring specified screen size...");
  }

  arena->lines = LINES-2;
  arena->cols = COLS-2;

  /* initialize the screenboard */
  screenboard = safe_malloc(LINES*COLS+1, "ui_dumb: screenboard");
  memset(screenboard, ' ', LINES*COLS);

  /* zero-end the char array */
  screenboard[LINES*COLS] = 0;

  /* draw borders */
  for (y=1; y<LINES-1; y++)
  {
    screenboard[y*COLS] = '|';
    screenboard[y*COLS+COLS-1] = '|';
  }
  for (x=1; x<COLS-1; x++)
  {
    screenboard[x] = '-';
    screenboard[(LINES-1)*COLS+x] = '-';
  }

  screen_initialized = 1;

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
  int y,x;
  sprite *sp;
  char look;

  for (y=0; y<arena->lines; y++)
  {
    for (x=0; x<arena->cols; x++)
    {
      sp = GETSPRITE(arena,y,x);

      if (sp) switch (sp->type)
      {
        case SNAKE:
          look = '*';
          break;  
        case EXSNAKE:
          look = '.';
          break; 
        case FRUIT:
		      look = ((fruit *) (sp))->value;
          break;
        case WALL:
          look = '#';
          break;
        default:
          look = '?';
          break;
      }
      else look = ' ';

      /*look = '.';*/
      screenboard[(y+YOFFSET)*COLS+(x+XOFFSET)] = look;
    }
  }

  /* It would be nicer if we knew how to clear... */

  /* ANSI cursor move */
  /*printf("\033[0;0r");*/

  /*printf("screenboard len: %i\n", strlen(screenboard));*/
  printf("%s", (char *)screenboard);
  fflush(stdout);
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

