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
 * $Id: mod_TEMPLATE.c,v 1.2 2004/07/28 13:33:13 strk Exp $
 ****************************************************************************/


#include "sins.h"


/*
 * local data types 
 */
typedef struct t_brain
{
  DIRECTION preferred_dir;
}
brain;

/*
 * private functions 
 */
static DIRECTION myplay (snake * mysnake);

int
TEMPLATE_module_init ()
{
  snake *mysnake1, *mysnake2;
  brain *mybrain1, *mybrain2;
  int player1, player2;

  mybrain1 = (brain *) safe_malloc (sizeof (brain), "new TEMPLATE brain");
  mybrain2 = (brain *) safe_malloc (sizeof (brain), "new TEMPLATE brain");

  mybrain1->preferred_dir = UP;
  mybrain2->preferred_dir = DOWN;

  mysnake1 = snake_make ();
  mysnake2 = snake_make ();

  mysnake1->brain = mybrain1;
  mysnake2->brain = mybrain2;

  mysnake1->play = &myplay;
  mysnake2->play = &myplay;

  player1 = register_player (mysnake1);
  player2 = register_player (mysnake2);

  return 0;
}

static DIRECTION
myplay (snake * mysnake)
{
  brain *mybrain;

  /*
   * you MUST cast this 
   */
  mybrain = (brain *) mysnake->brain;

  /*
   * this is how you set the new direction 
   */
  return mybrain->preferred_dir;
}
