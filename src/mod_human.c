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
#include "mod_human.h"
#include <stdio.h>


/*
 * private functions 
 */
static DIRECTION play (snake * s);
static brain *make_brain (int kl, int ku, int kr, int kd, int ks, int kg);
static int _getqueue (snake * s);
static int _putqueue (snake * s, int key);

/*
 * public functions 
 */
int human_module_init (void);
int CMD_human (char *);


int
human_module_init ()
{
  if (!register_command ("human", &CMD_human,
			 "human <left> <up> <right> <down>"))
    return 0;
  return 1;
}

static DIRECTION
play (snake * s)
{

  int typein;
  brain *mybrain = (brain *) s->brain;

  /* fill the snake's key queue */
  while ((typein = fetchtypein ()) != -1)
  {

    if (
      typein == mybrain->kl
      || typein == mybrain->ku 
      || typein == mybrain->kr 
      || typein == mybrain->kd
    ) if (!_putqueue (s, typein)) break;

  }


  /* Get next queued key */
  typein = _getqueue (s);

  /* Key left */
  if (typein == mybrain->kl) return LEFT;

  /* Key up */
  else if (typein == mybrain->ku) return UP;

  /* Key right */
  else if (typein == mybrain->kr) return RIGHT;

  /* Key down */
  else if (typein == mybrain->kd) return DOWN;

  return s->dir;
}




/*
 * allocate a brain structure 
 */
static brain *
make_brain (int kl, int ku, int kr, int kd, int ks, int kg)
{
  brain *mybrain;

  mybrain = (brain *) safe_malloc (sizeof (brain), "new human brain");

  mybrain->kl = kl;		/* left key */
  mybrain->ku = ku;		/* up key */
  mybrain->kr = kr;		/* right key */
  mybrain->kd = kd;		/* down key */
  mybrain->ks = ks;		/* stop key */
  mybrain->kg = kg;		/* grow key */

  mybrain->kqueue = safe_calloc (1, sizeof (queue), "new human queue");

  return mybrain;
}



/*
 * KEY QUEUE ******************************
 */


static int
_putqueue (snake * s, int key)
{
  qelem *new;
  queue *q = ((brain *) (s->brain))->kqueue;

  /*
   * Maximum queue lenght reached 
   */
  if (q->len == MAXQUEUELEN)
    return 0;

  /*
   * Allocate space for the new queue element 
   */
  new = safe_malloc (sizeof (qelem), "new human queue element");

  /*
   * Set value and pointers of the new element 
   */
  new->key = key;
  new->next = NULL;

  /*
   * Link it to the last element if any 
   */
  if (q->last)
    q->last->next = new;

  /*
   * This is the first element ... 
   */
  if (!q->first)
    q->first = new;

  /*
   * Make this the last element 
   */
  q->last = new;

  /*
   * Increment queue lenght 
   */
  q->len++;

  return 1;
}

/*
 * Returns -1 on empty queue.
 */
static int
_getqueue (snake * s)
{
  int ret;
  qelem *tmp;
  queue *q = ((brain *) (s->brain))->kqueue;

  /*
   * Queue is empty 
   */
  if (!q->first)
    return -1;

  ret = q->first->key;

  tmp = q->first->next;		/*
				 * This is NULL when first == last 
				 */
  free (q->first);
  q->first = tmp;


  /*
   * first == last 
   */
  if (!tmp)
    q->last = NULL;

  /*
   * Decrease queue lenght 
   */
  q->len--;

  return ret;
}

/*
 * handler for the `human' command 
 */
int
CMD_human (char *line)
{
  snake *mysnake;
  int ret;
  char format[] = "%c %c %c %c";
  char ku, kl, kr, kd;		/* snake parameters */

  if (!line)
    return -1;

  ret = sscanf (line, format, &kl, &ku, &kr, &kd);
  if (ret != 4)
    return -1;

  mysnake = snake_make ();
  mysnake->brain = make_brain (kl, ku, kr, kd, 0, 0);
  mysnake->play = &play;
  register_player (mysnake);

  return 1;
}
