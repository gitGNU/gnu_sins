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
#include <stdlib.h>		/* free... */

/*
 * extern data 
 */
extern snake *zoo[];

/*
 * private functions 
 */
static void move_head (snake *);
static void eat_tail (snake *);
static void move_single_snake (snake *);
static int snake_crash (snakebody * s1, snakebody * s2);
static void eat_exsnake (snake * s);

/*
 * public functions 
 */
void move_snakes (void);

/*
 * private data
 */
static snakebody *sbptr = 0;

/*
 * move each snake 
 */
void
move_snakes ()
{
  int i;

  for (i=0; i<MAXPLAYERS; i++)
  {
    /* empty slot */
    if ( zoo[i] == NULL ) continue;

    /* dead snake */
    if (zoo[i]->died) continue;

    move_single_snake (zoo[i]);
  }

}

/*
 * move head, eat tail 
 */
void
move_single_snake (snake * s)
{
  /* move_head if not stopped */
  move_head (s);

  /* finish here if snake died */
  if (s->died) return;

  /* eat the tail if not growing */
  if (!s->grow) eat_tail (s);
  else s->grow--;
}


/*
 * move the snake's head 
 */
static void
move_head (snake * s)
{
  snakebody *newhead;
  sprite *sp;

  /*
   * allocate memory for new chain 
   */
  if (sbptr)
  {
    newhead = sbptr;
    sbptr = NULL;
  }
  else
  {
    newhead = (snakebody *) safe_malloc (sizeof (snakebody), "new snake head");
  }

  /*
   * set sprite type 
   */
  newhead->type = SNAKE;

  /*
   * set self reference 
   */
  newhead->self = s;

  /*
   * initialize newhead coordinates to old head's 
   */
  newhead->cell.x = s->head->cell.x;
  newhead->cell.y = s->head->cell.y;

  /*
   * move head one step in the desired direction 
   */
  move_cell (&(newhead->cell), s->dir, 1);

  /*
   * get sprite below the new head 
   */
  sp = ARENAGET (newhead->cell.y, newhead->cell.x);

  /*
   * check if snake have hitten another snake 
   */
  if (IS_SNAKE (sp))
  {
    if (snake_crash (newhead, (snakebody *) sp)) return;
  }

  /*
   * check if snake have hitten a wall
   */
  else if (IS_WALL(sp))
  {
    snake_kill ( s );
    return;
  }
  
  /*
   * check if snake have eaten something 
   */
  else if (IS_FRUIT (sp))
  {				

    int fruitval = FRUITVAL ((fruit *) sp);

    s->grow += fruitval;

    /* magic fruit (0 valued) */
    if (!fruitval)
	  {
	    s->score += 10;
	    snake_trim (s);
	  }

    /* normal fruit */
    else s->score += fruitval;

    init_fruit ();

  }

  /* eat a snake rest */
  else if (IS_EXSNAKE (sp)) eat_exsnake (s);

  /* set the new snake's head */
  s->head->prev = newhead;
  newhead->next = s->head;
  newhead->prev = NULL;
  s->head = newhead;

  /*
   * print the new head 
   * do it (for now) to avoid transparency of 1-lenght snakes 
   */
  ARENAPUT (newhead->cell.y, newhead->cell.x, (sprite *) newhead);

  /* increment the new snake's length */
  s->length++;

  /* Camera follows this snake */
  if ( arena.protagonist == s->playernum )
  {
  	arena.x0 = (s->head->cell.x)-(arena.cols/2);
  	arena.y0 = (s->head->cell.y)-(arena.lines/2);
  }


}


/*
 * eat snake's tail 
 */
static void
eat_tail (snake * s)
{
  /*
   * chain pointer 
   */
  snakebody *ptr;

  /*
   * delete the old tail before its coordinates change 
   */
  /*
   * unless there is something else below it 
   */
  /*
   * this shouldn't be done by this module 
   */
  ptr = (snakebody *) ARENAGET (s->tail->cell.y, s->tail->cell.x);
  if (ptr && ptr->self == s && ptr != ptr->self->head)
    ARENAPUT (s->tail->cell.y, s->tail->cell.x, NULL);

  /*
   * find the chain before the tail 
   */
  ptr = s->tail->prev;

  /*
   * make it the new tail pointer 
   */
  s->tail = ptr;

  /*
   * return memory of the old tail to the sb memory pointer
   */
  sbptr = s->tail->next;

  /*
   * set the new tail pointer to NULL 
   */
  s->tail->next = NULL;

  /*
   * decrement snake's lenght 
   */
  s->length--;

  /*
   * if snake disappeared, finish it 
   */
  if (s->length == 0)
    snake_kill (s);
}


int
snake_crash (snakebody * s1, snakebody * s2)
{

  /* s2 is neither the tail, nor the head */
  if (s2 != s2->self->tail && s2 != s2->self->head)
  {
    snake_kill (s1->self);
    return 1;
  }

  /* s2 is the tail ...   */
  if (s2 == s2->self->tail)
  {

    /* ... of an already moved snake */
    if (s2->self->playernum < s1->self->playernum)
	  {
	    snake_kill (s1->self);
	  }

    /* ... of a yet to move (but growing) snake */
    else if (s2->self->grow)
	  {
	    snake_kill (s1->self);
	  }

  }

  /* s2 is the head ... */
  if (s2 == s2->self->head)
  {

    /* ... of an already moved snake */
    if ( s2->self->playernum < s1->self->playernum
      || IS_OPPOSITE (s1->self->dir, s2->self->dir) )
	  {
	    /* kill both snakes */
	    snake_kill (s1->self);
	    snake_kill (s2->self);
	  }

    /* ... of a yet to move snake */
    else if (s2->self->length > 1)
	  {
	    snake_kill (s1->self);
	  }

  }

  if (s1->self->died) return 1;

  return 0;
}

void
eat_exsnake (snake * s)
{
  s->score++;
}

