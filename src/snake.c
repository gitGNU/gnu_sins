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
 *
 * file: snake.c *  purpose: manipulation functions for the snake object
 *
 ****************************************************************************
 *
 * $Log: snake.c,v $
 * Revision 1.4  2004/07/28 13:33:13  strk
 * Added copyright statment and permission to copy header in all files.
 * Added a reference to source URL into compat.c file.
 *
 * Revision 1.3  2004/04/13 07:30:32  strk
 * removed warning about multiple initializzation of snakes
 *
 * Revision 1.2  2003/12/19 19:00:58  strk
 * various small changes
 *
 ****************************************************************************/

#include "sins.h"
#include <stdlib.h>		/* malloc, free ...   */

/*
 * Define QUIT_ON_LAST_DEATH if you want the game to 
 * finish when everybody is dead
 */
#undef QUIT_ON_LAST_DEATH

/*
 * public function 
 */
void snake_kill (snake *);
void snake_destroy (snake *);
void snake_reset (snake *);
void snake_init (snake *);
void snake_trim (snake *);
snake *snake_make (void);
int CMD_kill (char *);
int CMD_reset (char *);
int CMD_trim (char *);

/*
 * private data
 */

/* this is used as a fake sprite for snake's cadavers */
static sprite exsnake = { EXSNAKE, {0}, };

/* kill a snake */
void
snake_kill (snake * s)
{
#ifdef QUIT_ON_LAST_DEATH
  extern ARENA arena;
  int i, onealive = 0;
#endif
  snakebody *ptr;

  /* Call the killer function of the snake (think better about it) */
  /* if ( s->kill ) s->kill(s); */

  /*
   * mark it dead 
   */
  s->died = 1;

  /*
   * place EXSNAKE sprites on the arena board
   */
  for (ptr = s->head; ptr != NULL; ptr = ptr->next)
  {
      ARENAPUT (ptr->cell.y, ptr->cell.x, &exsnake);
  }


#ifdef QUIT_ON_LAST_DEATH
  for (i=0; i<MAXPLAYERS; i++)
  {
    /* empty slot */
    if (zoo[i] == NULL) continue;

    /* still one alive ! */
    if (!zoo[i]->died) onealive = 1;
  }

  if (!onealive) finish ();
#endif

}

/* kill command */
int
CMD_kill (char *s)
{
  int snakenum;

  if (!s)
    return -1;

  snakenum = atoi (s);

  if (snakenum < 0 || snakenum >= MAXPLAYERS)
    {
      message ("CMD_kill: snake number out of range: %i", snakenum);
      return 0;
    }

  if (!zoo[snakenum])
    {
      message ("CMD_kill: %i: no such snake", snakenum);
      return 0;
    }

  if (zoo[snakenum]->died)
    {
      message ("CMD_kill: snake %i already dead", snakenum);
      return 0;
    }

  message ("CMD_kill: snake %i killed at user request", snakenum);
  snake_kill (zoo[snakenum]);
  return 1;
}

/* destroy a snake (free all memory associated with it) */
void
snake_destroy (snake * s)
{
#ifdef QUIT_ON_LAST_DEATH
  extern ARENA arena;
  int i,onealive;
#endif
  snakebody *ptr, *oldptr, *onarena;
  extern int numplayers;

  /* Call the destroyer function of the snake (think better about it) */
  /* if ( s->destroy ) s->destroy(s); */

  /*
   * free space allocated for snake's body 
   */
  ptr = s->head;
  while (ptr != NULL)
  {
    oldptr = ptr;
    ptr = ptr->next;
    onarena = (snakebody *) ARENAGET (oldptr->cell.y, oldptr->cell.x);
    if (onarena && onarena->self == s)
	    ARENAPUT (oldptr->cell.y, oldptr->cell.x, NULL);
    free (oldptr);
  }

  /* destroy snake's brain (should call a snake destroy funcion?) */
  free(s->brain);

  /* decrement players counter */
  numplayers--;

  /* set this snake's slot to NULL */
  zoo[s->playernum] = NULL;

  /* destroy snake structure */
  free(s);

#ifdef QUIT_ON_LAST_DEATH
  onealive = 0;
  for (i=0; i<MAXPLAYERS; i++)
  {
    /* empty slot */
    if ( zoo[i] == NULL ) continue;

    /* still one alive! */
    if (!zoo[i]->died)
    {
      onealive = 1;
      break;
    }
  }

  if (!onealive) finish ();
#endif

}

/* destroy command */
int
CMD_destroy (char *s)
{
  int snakenum;

  if (!s)
    return -1;

  snakenum = atoi (s);

  if (snakenum < 0 || snakenum >= MAXPLAYERS)
    {
      message ("CMD_destroy: snake number out of range: %i", snakenum);
      return 0;
    }

  if (!zoo[snakenum])
    {
      message ("CMD_destroy: %i: no such snake", snakenum);
      return 0;
    }

  message ("CMD_destroy: snake %i destroyed at user request", snakenum);
  snake_destroy (zoo[snakenum]);
  return 1;
}

/*
 * reset snake body 
 */
void
snake_reset (snake * s)
{
  snakebody *ptr, *oldptr, *onarena;
  int x, y, dir;
  cell newcell;

  /*
   * Get a random position and direction for the new body 
   */
  newcell.x = x = rand_between (0, ARENA_WIDTH - 1);
  newcell.y = y = rand_between (0, ARENA_HEIGHT - 1);
  while (ARENAGET (newcell.y, newcell.x))
  {
    move_cell(&newcell, RIGHT, 1);
    if (newcell.x == x)
    {
      move_cell(&newcell, DOWN, 1);
      if (newcell.y == y)
      {
        fprintf(stderr, "No space left on arena for a snake reset\n");
        finish();
      }
    }
  }
  dir = rand_between (0, 3);

  /* Free space allocated for old body */
  ptr = s->head;
  while (ptr != NULL)
  {
      oldptr = ptr;
      ptr = ptr->next;
      onarena = (snakebody *) ARENAGET (oldptr->cell.y, oldptr->cell.x);
      if (onarena && onarena->self == s)
	ARENAPUT (oldptr->cell.y, oldptr->cell.x, NULL);
      free (oldptr);
  }

  /*
   * Create the new body 
   */
  s->head = s->tail = (snakebody *) safe_malloc (sizeof (snakebody),
						 "new snake's head/tail (reset)");
  s->head->type = SNAKE;
  s->head->next = NULL;
  s->head->prev = NULL;
  s->head->cell.y = y;
  s->head->cell.x = x;
  s->head->self = s;

  /* Assign a direction */
  s->dir = dir;

  /*
   * assign initial length 
   */
  s->length = 1;

  /*
   * give it a life again (in case it was dead) 
   */
  s->died = 0;

  /*
   * reset the score (no Karma here...) 
   */
  s->score = 0;
}

/*
 * reset command
 */
int
CMD_reset (char *s)
{
  int snakenum;

  if (!s)
    return -1;

  snakenum = atoi (s);

  if (snakenum < 0 || snakenum >= MAXPLAYERS)
    {
      message ("CMD_reset: snake number out of range: %i", snakenum);
      return 0;
    }

  if (!zoo[snakenum])
    {
      message ("CMD_reset: %i: no such snake", snakenum);
      return 0;
    }

  if (!zoo[snakenum]->died)
    {
      message ("CMD_reset: snake %i still alive", snakenum);
      return 0;
    }

  message ("CMD_reset: snake %i reset at user request", snakenum);
  snake_reset (zoo[snakenum]);
  return 1;
}

/*
 * Initialize snake position (can only be done for newly registered players)
 */
void
snake_init (snake * s)
{
  int x, y, dir;
  cell newcell;

  /* Check wheter the snake was already initialized */
  if ( s->length )
  {
    //message("Snake %i already initialized", s->playernum);
    return;
  }

  /* Allocate space for head/tail of the snake */
  s->head = s->tail = (snakebody *) safe_malloc (sizeof (snakebody),
						 "initialized snake's head/tail (snake_init)");

  s->head->type = SNAKE;
  s->head->next = NULL;
  s->head->prev = NULL;

  /* Get a random position and direction for the new body */
  newcell.x = x = rand_between (0, ARENA_WIDTH - 1);
  newcell.y = y = rand_between (0, ARENA_HEIGHT - 1);
  while (ARENAGET (newcell.y, newcell.x))
  {
    move_cell(&newcell, RIGHT, 1);
    if (newcell.x == x)
    {
      move_cell(&newcell, DOWN, 1);
      if (newcell.y == y)
      {
        fprintf(stderr, "No space left on arena for a snake reset\n");
        finish();
      }
    }
  }
  dir = rand_between (0, 3);

  /* Assign initial position */
  s->head->cell.y = newcell.y;
  s->head->cell.x = newcell.x;
  s->head->self = s;

  /* Assign a direction */
  s->dir = dir;

  /* Assign initial length */
  s->length = 1;

  /* Reset the score (no Karma here...) */
  s->score = 0;

  /* Make it alive */
  s->died = 0;
}



/*
 * trim a snake 
 */
void
snake_trim (snake * s)
{
  snakebody *ptr, *oldptr;

  /*
   * clean up the snake screen (?is this needed?) * free memory used by the 
   * body 
   */
  ptr = s->head->next;
  while (ptr)
    {
      oldptr = ptr;
      ptr = ptr->next;
      ARENAPUT (oldptr->cell.y, oldptr->cell.x, &exsnake);
      free (oldptr);
    }

  /*
   * reset length 
   */
  s->length = 1;

  /*
   * set tail 
   */
  s->tail = s->head;

  /*
   * link to NULL 
   */
  s->head->next = NULL;
  s->head->prev = NULL;
}

/*
 * trim command
 */
int
CMD_trim (char *s)
{
  int snakenum;

  if (!s)
    return -1;

  snakenum = atoi (s);

  if (snakenum < 0 || snakenum >= MAXPLAYERS)
    {
      message ("CMD_trim: snake number out of range: %i", snakenum);
      return 0;
    }

  if (!zoo[snakenum])
    {
      message ("CMD_trim: %i: no such snake", snakenum);
      return 0;
    }

  if (zoo[snakenum]->died)
    {
      message ("CMD_trim: snake %i is dead", snakenum);
      return 0;
    }

  message ("CMD_trim: snake %i trimmered at user request", snakenum);
  snake_trim (zoo[snakenum]);
  return 1;
}


/*
 * Allocate space for a new (1-cell-big) snake.
 * Initialize snake's structure members.
 * If screen is initialized assign a random position
 * else set coordinates to 0 and mark it dead
 */
snake *
snake_make (void)
{
  snake *new;			/* new snake pointer */

  /* Allocate memory for the new snake */
  new = (snake *) safe_malloc (sizeof (snake), "new snake");

  new->head = NULL;

  /* Set snake's length to 0 (needs to be initialized) */
  new->length = 0;

  /* Not growing */
  new->grow = 0;

  if ( screen_initialized )
  {
    snake_init(new);
  }

  /* Return the new snake */
  return new;
}
