/*
 *    snake - a videogame derived from Q-Basic nibbles
 *    Copyright (C) 1999 Cyberandro J.K. Starrik
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


/*
 * local data types 
 */
typedef struct t_brain
{
  int preferred_dir;
}
brain;

/*
 * private functions 
 */
static void myplay (snake * mysnake);


int
long_module_init ()
{
  snake *mysnake;
  brain *mybrain;
  int player;

  mybrain = (brain *) safe_malloc (sizeof (brain), "long new brain");

  mybrain->preferred_dir = UP;

  mysnake = snake_make ();

  mysnake->brain = mybrain;

  mysnake->play = &myplay;

  player = register_player (mysnake);

  return 0;
}

static void
myplay (snake * mysnake)
{
  brain *mybrain;
  int i;

  /*
   * you MUST cast this 
   */
  mybrain = (brain *) mysnake->brain;

  /*
   * this is how you set the new direction 
   */
  mysnake->dir = mybrain->preferred_dir;

  /*
   * take a long time before deciding what to do 
   */
  for (i = 0; i < 900000; i++);
}
