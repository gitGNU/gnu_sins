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
 ****************************************************************************/

#include "sins.h"

/*
 * public functions 
 */
int arenaput (int y, int x, sprite * sp);
sprite *arenaget (int y, int x);
int arenainit (void);
int arenalines (void);
int arenacols (void);
int CMD_camera (char *);

  

/* Utilities for modules */
int arenalines ()
{
  extern ARENA arena;
  return arena.lines;
}

int arenacols ()
{
  extern ARENA arena;
  return arena.cols;
}

int
arenainit ()
{
  arena.board = safe_malloc (arena.lines * arena.cols *
    sizeof (sprite*), "arena board");
  memset (arena.board, 0, arena.lines*arena.cols*sizeof(sprite*) );

  arena.init = 1;
  arena.protagonist = -1;

  return 1;
}

sprite *
arenaget (int y, int x)
{

  /* range check */
  if ( y >= arena.lines || y < 0 ) return NULL;
  if ( x >= arena.cols  || x < 0 ) return NULL; 

  /* what about this ?   */
  return *(arena.board + (y * arena.cols + x));

  /*
   * this is out of date ;) 
   */
  /*
   * return arena.board[y * arena.cols + x]; 
   */
}

int
arenaput (int y, int x, sprite * sp)
{

  /* range check */
  if ( y >= arena.lines || y < 0 ) return 0;
  if ( x >= arena.cols  || x < 0 ) return 0; 

  /* what about this ?   */
  *(arena.board + (y * arena.cols + x)) = sp;

  return 1;
}

int 
CMD_camera(char *s)
{
	int prot;

  if (!s) return -1;

	if ( 1 == sscanf(s, "on %d", &prot) )
  {
	  arena.protagonist = prot;
  }
	else if ( NULL != strstr(s, "off") )
  {
	  arena.protagonist = -1;
  }
  else return -1;

	return 1;
}
