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
#include <math.h>

/*
 * Default percentual probability of magic fruit occurrence.
 * Set to 0 to have NO magic fruits.
 * Set to 100 to have ONLY magic fruits.
 */
/*
#define DEFAULT_MAGIC_FRUIT_PROBABILITY 0 
#define DEFAULT_MAGIC_FRUIT_PROBABILITY 100 
*/
#define DEFAULT_MAGIC_FRUIT_PROBABILITY 8

/* public data */
fruit frt = {
  FRUIT,			/* type */
  {0, 0},			/* cell */
  0
};

/* private data */
static int magic_fruit_probability = DEFAULT_MAGIC_FRUIT_PROBABILITY;


void
init_fruit ()
{
  int value;
  cell newcell;
  int y, x;

  /* we will hang forever here when all space is busy */
  newcell.y = y = rand_between (0, ARENA_HEIGHT - 1);
  newcell.x = x = rand_between (0, ARENA_WIDTH - 1);
  while (ARENAGET (newcell.y, newcell.x))
  {
    move_cell(&newcell, RIGHT, 1);
    if (newcell.x == x)
    {
      move_cell(&newcell, DOWN, 1);
      if (newcell.y == y)
      {
        fprintf(stderr, "No space left on arena for a new fruit\n");
        finish();
      }
    }
  }

  frt.cell.y = newcell.y;
  frt.cell.x = newcell.x;

  if (rand_between (1, 100) <= magic_fruit_probability)
    value = '0';
  else
    value = rand_between (49, 57);	/* ascii 1 to 9 */

  frt.value = value;

  /* message("new fruit at: %2.2i, %2.2i", newcell.y, newcell.x); */

  ARENAPUT (newcell.y, newcell.x, (sprite *) & frt);

}

int
CMD_mfprob (char *arg)
{
	int mfp;

	if ( !arg ) return -1;

	mfp = atoi(arg);
	if ( mfp > 100 ) mfp = 100;
	else if ( mfp < 0 ) mfp = 0;
	magic_fruit_probability = mfp;

	return 1;
}

