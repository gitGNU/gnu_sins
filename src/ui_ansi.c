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
 * $Id: ui_ansi.c,v 1.3 2009/03/30 09:47:07 strk Exp $
 ****************************************************************************/


/*
 * file: ui_ansi.c *  purpose: handle ansi UI 
 */

#include "sins.h"
#include <stdio.h>

#define LINES 25
#define COLS 80

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
static char *arenamap; /* picture indexes */

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
  int x, y;

  /* default screen size */
  if ( ! arena->lines ) arena->lines = LINES-2;
  if ( ! arena->cols ) arena->cols = COLS-2;

  /* initialize the arena maps (used for optimization purposes) */
  arenamap = (char *)safe_malloc( arena->lines*arena->cols*sizeof(int),
    "ui_ansi: ui_drawarena: arenamap");
  memset(arenamap, -1, arena->lines * arena->cols);

  /* clean screen, save terminal state */
  printf("\033[H\033[J\033[s");

  /* draw borders */
  for (x=0; x<COLS; x++)
  {
      printf("\033[%i;%iH+", LINES, x);
      printf("\033[0;%iH+", x);
  }
  for (y=0; y<LINES; y++)
  {
      printf("\033[%i;%iH+", y, COLS);
      printf("\033[%i;0H+", y);
  }
  fflush(stdout);

  screen_initialized = 1;

  return 1;
}

/* message drawing function */
void
ui_drawmessage (char *str)
{
  printf("\033[%i;0H%s", arenalines(), str);
  fflush(stdout);
}

/* arena drawing */
void
ui_drawarena (ARENA *arena)
{
  int y,x;
  sprite *sp;
  char look;
  char *attrs = '\0';

  for (y=0; y<arena->lines; y++)
  {
    for (x=0; x<arena->cols; x++)
    {
      sp = GETSPRITE(arena,y,x);

      if (sp) switch (sp->type)
      {
        case SNAKE:
          look = ((snakebody *) (sp))->self->playernum + 48;
          attrs="32;41";
          break;  
        case EXSNAKE:
          look = '.';
          attrs="34"; /* blink ;) */
          break; 
        case FRUIT:
		      look = ((fruit *) (sp))->value;
          attrs="34;1";
          break;
        case WALL:
          look = '#';
          attrs="37";
          break;
        default:
          look = '?';
          break;
      }
      else
      {
        attrs = "0;49;39";
        look = ' ';
      }

      if ( *(arenamap+y*arena->cols+x) == look ) continue;
      *(arenamap+y*arena->cols+x) = look;

      printf("\033[%i;%iH\033[0;49;39m\033[%sm%c", y+2, x+2, attrs, look);

    }
  }

  /* send cursor to the top/left corner */
  printf("\033[1;1H");
  fflush(stdout);
}


void
ui_finish ()
{
  /* Restore saved terminal state */
  printf("\033[H\033[J");
  printf("\033[u\n");
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

