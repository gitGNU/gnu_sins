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
#include <errno.h>

#undef AI_SCORES_DEBUG

/*
 * WEIGHTS 
 */
#define MAXSCORE		100

#define SNAKE_DISTANCE_WEIGHT(x)  (((brain *)x->brain)->snake_distance_weight)
#define FRUIT_DISTANCE_WEIGHT(x)  (((brain *)x->brain)->fruit_distance_weight)
#define RANDOM_WEIGHT(x)	  (((brain *)x->brain)->random_weight)
#define INERTIA_WEIGHT(x)	  (((brain *)x->brain)->inertia_weight)
#define SNAKE_PRESENCE_WEIGHT(x)  (((brain *)x->brain)->snake_presence_weight)
#define FREE_SPACE_WEIGHT(x)	  (((brain *)x->brain)->free_space_weight)
#define DEADSNAKE_PROXIMITY_WEIGHT(x)	  (((brain *)x->brain)->deadsnake_proximity_weight)

/*
 * local data and definitions 
 */
#ifdef AI_SCORES_DEBUG
static char Direction[4] = { 'l', 'u', 'r', 'd' };
#endif

typedef struct t_scored_dir
{
  DIRECTION dir;
  float score;
}
scored_dir;

typedef struct t_brain
{
  float snake_distance_weight;
  float fruit_distance_weight;
  float random_weight;
  float inertia_weight;
  float snake_presence_weight;
  float free_space_weight;
  float deadsnake_proximity_weight;
  scored_dir *sd[4];
}
brain;

/*
 * private functions 
 */
static DIRECTION play (snake * s);
static int distance (cell * from, cell * to, int dir);
static void score_by_random (snake *);
static void score_by_inertia (snake *);
static void score_by_snake_distance (snake *);
static void score_by_free_space (snake *);
static void score_by_snake_presence (snake *, fruit *);
static void score_by_fruit_distance (snake *, fruit *);
static void score_by_deadsnake_proximity (snake *);
static void sort_direction_by_score (snake *);
static brain *make_brain (float, float, float, float, float, float, float);
#ifdef AI_SCORES_DEBUG
static void dump_scores (snake *);
#endif

/* public functions */
int ai_module_init (void);
int CMD_ai (char *);

/*
 * module initialization function 
 */
int
ai_module_init ()
{
  if (!register_command ("ai", &CMD_ai,
			 "ai [<snk>] [<frt>] [<rnd>] [<inrt>] [<snkp>] [<sqr>] [<dsnk>]"))
    return 0;

  return 1;
}

/*
 * handle the 'ai' command 
 */
int
CMD_ai (char *line)
{
  snake *mysnake;
  int ret;
  char format[] = "%f %f %f %f %f %f %f";
  float w_snake_distance = 1;
  float w_fruit_distance = 1;
  float w_random = 1;
  float w_inertia = 1;
  float w_snake_presence = 1;
  float w_free_space = 1;
  float w_deadsnake = 1;

  /*
   * read record 
   */
  if ( line )
  {
    ret = sscanf (line, format,
		  &w_snake_distance,
		  &w_fruit_distance, &w_random,
		  &w_inertia, &w_snake_presence, &w_free_space, &w_deadsnake);
  }

/* maintain default values for parameters not given
  if (ret != 7)
    return -1;
*/

  /*
   * make a new player 
   */
  mysnake = snake_make ();

  /*
   * set player function 
   */
  mysnake->play = &play;

  /*
   * make snake's brain 
   */
  mysnake->brain = (brain *) make_brain (w_snake_distance,
					 w_fruit_distance, w_random,
					 w_inertia, w_snake_presence,
					 w_free_space, w_deadsnake);

  /*
   * register new player 
   */
  register_player (mysnake);

  return 1;
}

/*
 * score each direction based on given weights. pick the best 
 */
static DIRECTION
play (snake * s)
{
  cell *headcell;
  cell tmpcell;
  int i, j, danger;
  DIRECTION dir;
  scored_dir **sd = ((brain *) s->brain)->sd;
  sprite *sp;
  cell tmpcell2;

  headcell = &(s->head->cell);

  /*
   * init scored_dir array 
   */
  LOOPDIRECTION(i)
  {
    sd[i]->dir = i;
    sd[i]->score = 0;
  }


  /*
   * score by snake distance 
   */
  score_by_snake_distance (s);

  /*
   * score by space occupation 
   */
  score_by_free_space (s);

  /*
   * score by fruit distance 
   */
  score_by_fruit_distance (s, &frt);

  /*
   * score by deadsnake proximity 
   */
  score_by_deadsnake_proximity (s);

  /*
   * score by snake presence 
   */
  score_by_snake_presence (s, &frt);

  /*
   * score by random ... 
   */
  score_by_random (s);

  /*
   * score by inertia 
   */
  score_by_inertia (s);

  /*
   * sort directions by score 
   */
  sort_direction_by_score (s);

#ifdef AI_SCORES_DEBUG
  /*
   * dump scored (do it after sorting if you like)... 
   */
  dump_scores (s);
#endif

  /* just waste some time... */
  /* for (i=0; i<200000; i++); */

  /*
   * find first non mortal direction (in order of score) 
   */
  LOOPDIRECTION (i)
  {
    dir = ((brain *)s->brain)->sd[i]->dir;
    danger = 0;

    memcpy (&tmpcell, headcell, sizeof (cell));
    move_cell (&tmpcell, dir, 1);

    /* there is a snake in that position */
    if ( IS_SNAKE (ARENAGET (tmpcell.y, tmpcell.x)) )
    {
      continue;
    }

    /* there is a wall in that position */
    if ( IS_WALL (ARENAGET (tmpcell.y, tmpcell.x)) )
    {
      continue;
    }

    /* check near sprites */
    LOOPDIRECTION (j)
    {
      /* that's me :) */
      if ( IS_OPPOSITE(dir, j) )
      {
        continue;
      }

      memcpy (&tmpcell2, &tmpcell, sizeof (cell));
      move_cell (&tmpcell2, j, 1);
      sp = ARENAGET (tmpcell2.y, tmpcell2.x);

      /* there is a snake there */
      if ( IS_SNAKE(sp) && ((snakebody *)sp)->self->head == (snakebody *)sp )
      {
              danger++;
      }

	  }

    if ( ! danger ) return ((brain *) s->brain)->sd[i]->dir;
	  continue;
  }

  /* if no free directions are found keep going your direction */
  return s->dir;

}

/*
 * return distance to ortogonal position between ``from'' and ``to'' * going
 * in given direction ``dir'' 
 */
static int
distance (cell * from, cell * to, int dir)
{
  int dist = 0;

  switch (dir)
    {
    case UP:
      dist = from->y - to->y;
      if (dist < 0) dist = ARENA_HEIGHT + dist;
      break;
    case DOWN:
      dist = to->y - from->y;
      if (dist < 0) dist = ARENA_HEIGHT + dist;
      break;
    case LEFT:
      dist = from->x - to->x;
      if (dist < 0) dist = ARENA_WIDTH + dist;
      break;
    case RIGHT:
      dist = to->x - from->x;
      if (dist < 0) dist = ARENA_WIDTH + dist;
      break;
    }

  return dist;
}



/*
 * random scoring 
 */
static void
score_by_random (snake * s)
{
  int i;
  float weight = RANDOM_WEIGHT (s);
  scored_dir **sdir = ((brain *) (s->brain))->sd;

  /* i = rand_between(0,3); */
  LOOPDIRECTION (i)
    sdir[i]->score += rand_between (0, MAXSCORE) * weight;
}

/*
 * score by inertia 
 */
static void
score_by_inertia (snake * s)
{
  float weight = INERTIA_WEIGHT (s);
  DIRECTION *dir;
  scored_dir **sdir = ((brain *) (s->brain))->sd;

  dir = &(s->dir);

  sdir[*dir]->score += MAXSCORE * weight;
}

/*
 * score by snake distance 
 */
static void
score_by_snake_distance (snake * s)
{
  int dist;
  float maxdist;
  cell *from;
  cell tmpcell;
  float weight = SNAKE_DISTANCE_WEIGHT (s);
  scored_dir **dir = ((brain *) (s->brain))->sd;

  from = &(s->head->cell);

  /*
   * horizontal distances 
   */
  maxdist = ARENA_WIDTH;

  /*
   * right 
   */
  dist = 0;
  memcpy (&tmpcell, from, sizeof (cell));
  do
    {
      move_cell (&tmpcell, RIGHT, 1);
      dist++;
      if (IS_SNAKE (ARENAGET (tmpcell.y, tmpcell.x)))
	break;
    }
  while (tmpcell.x != from->x);
  dir[RIGHT]->score += (dist / maxdist) * MAXSCORE * weight;

  /*
   * left 
   */
  dist = 0;
  memcpy (&tmpcell, from, sizeof (cell));
  do
    {
      move_cell (&tmpcell, LEFT, 1);
      dist++;
      if (IS_SNAKE (ARENAGET (tmpcell.y, tmpcell.x)))
	break;
    }
  while (tmpcell.x != from->x);
  dir[LEFT]->score += (dist / maxdist) * MAXSCORE * weight;


  /*
   * vertical distances 
   */
  maxdist = ARENA_HEIGHT;

  /*
   * up 
   */
  dist = 0;
  memcpy (&tmpcell, from, sizeof (cell));
  do
    {
      move_cell (&tmpcell, UP, 1);
      dist++;
      if (IS_SNAKE (ARENAGET (tmpcell.y, tmpcell.x)))
	break;
    }
  while (tmpcell.y != from->y);
  dir[UP]->score += (dist / maxdist) * MAXSCORE * weight;

  /*
   * down 
   */
  dist = 0;
  memcpy (&tmpcell, from, sizeof (cell));
  do
    {
      move_cell (&tmpcell, DOWN, 1);
      dist++;
      if (IS_SNAKE (ARENAGET (tmpcell.y, tmpcell.x)))
	break;
    }
  while (tmpcell.y != from->y);
  dir[DOWN]->score += (dist / maxdist) * MAXSCORE * weight;
}

/*
 * score by free space 
 */
static void
score_by_free_space (snake * s)
{
  int square_side = 5;		/* be odd! */
  cell tmpcell;
  cell *from;
  int half_side;
  int c, d;
  int freespace;
  int maxspace;
  int weight = FREE_SPACE_WEIGHT (s);
  scored_dir **dir = ((brain *) (s->brain))->sd;

  from = &(s->head->cell);

  half_side = (square_side / 2);
  maxspace = square_side * square_side;

  /*
   * right 
   */
  freespace = maxspace;
  tmpcell.x = from->x;
  for (move_cell (&tmpcell, RIGHT, 1), c = 0;
       c < square_side; c++, move_cell (&tmpcell, RIGHT, 1))
    {
      tmpcell.y = from->y;
      for (move_cell (&tmpcell, UP, 1), d = 0;
	   d < square_side; d++, move_cell (&tmpcell, UP, 1))
	{
	  if (IS_SNAKE (ARENAGET (tmpcell.y, tmpcell.x)))
	    {
	      freespace--;
	    }
	}
    }
  dir[RIGHT]->score += freespace * weight;

  /*
   * left 
   */
  freespace = maxspace;
  tmpcell.y = from->y;
  for (move_cell (&tmpcell, UP, half_side), c = 0;
       c < square_side; c++, move_cell (&tmpcell, DOWN, 1))
    {
      tmpcell.x = from->x;
      for (move_cell (&tmpcell, LEFT, square_side), d = 0;
	   d < square_side; d++, move_cell (&tmpcell, RIGHT, 1))
	{
	  if (IS_SNAKE (ARENAGET (tmpcell.y, tmpcell.x)))
	    freespace--;
	}
    }
  dir[LEFT]->score += freespace * weight;

  /*
   * up 
   */
  freespace = maxspace;
  tmpcell.x = from->x;
  for (move_cell (&tmpcell, LEFT, half_side), c = 0;
       c < square_side; c++, move_cell (&tmpcell, RIGHT, 1))
    {
      tmpcell.y = from->y;
      for (move_cell (&tmpcell, UP, 1), d = 0;
	   d < square_side; d++, move_cell (&tmpcell, UP, 1))
	{
	  if (IS_SNAKE (ARENAGET (tmpcell.y, tmpcell.x)))
	    freespace--;
	}
    }
  dir[UP]->score += freespace * weight;

  /*
   * down 
   */
  freespace = maxspace;
  tmpcell.y = from->y;
  for (move_cell (&tmpcell, DOWN, 1), c = 0;
       c < square_side; c++, move_cell (&tmpcell, DOWN, 1))
    {
      tmpcell.x = from->x;
      for (move_cell (&tmpcell, LEFT, half_side), d = 0;
	   d < square_side; d++, move_cell (&tmpcell, RIGHT, 1))
	{
	  if (IS_SNAKE (ARENAGET (tmpcell.y, tmpcell.x)))
	    freespace--;
	}
    }
  dir[DOWN]->score += freespace * weight;
}

/*
 * score by snake presence beetween from and to 
 */
static void
score_by_snake_presence (snake * s, fruit * frt)
{
  int snakefound;
  cell *from;
  cell *to;
  cell tmpcell;
  float weight = SNAKE_PRESENCE_WEIGHT (s);
  scored_dir **dir = ((brain *) (s->brain))->sd;

  from = &(s->head->cell);
  to = &(frt->cell);

  /*
   * right 
   */
  snakefound = 0;
  memcpy (&tmpcell, from, sizeof (cell));
  do
    {
      move_cell (&tmpcell, RIGHT, 1);
      if (IS_SNAKE (ARENAGET (tmpcell.y, tmpcell.x)))
	{
	  snakefound = 1;
	  break;
	}
    }
  while (tmpcell.x != from->x);
  if (!snakefound)
    dir[RIGHT]->score += MAXSCORE * weight;

  /*
   * left 
   */
  snakefound = 0;
  memcpy (&tmpcell, from, sizeof (cell));
  do
    {
      move_cell (&tmpcell, LEFT, 1);
      if (IS_SNAKE (ARENAGET (tmpcell.y, tmpcell.x)))
	{
	  snakefound = 1;
	  break;
	}
    }
  while (tmpcell.x != from->x);
  if (!snakefound)
    dir[LEFT]->score += MAXSCORE * weight;

  /*
   * up 
   */
  snakefound = 0;
  memcpy (&tmpcell, from, sizeof (cell));
  do
    {
      move_cell (&tmpcell, UP, 1);
      if (IS_SNAKE (ARENAGET (tmpcell.y, tmpcell.x)))
	{
	  snakefound = 1;
	  break;
	}
    }
  while (tmpcell.y != from->y);
  if (!snakefound)
    dir[UP]->score += MAXSCORE * weight;

  /*
   * down 
   */
  snakefound = 0;
  memcpy (&tmpcell, from, sizeof (cell));
  do
    {
      move_cell (&tmpcell, DOWN, 1);
      if (IS_SNAKE (ARENAGET (tmpcell.y, tmpcell.x)))
	{
	  snakefound = 1;
	  break;
	}
    }
  while (tmpcell.y != from->y);
  if (!snakefound)
    dir[DOWN]->score += MAXSCORE * weight;

}

/*
 * score by fruit distace 
 */
static void
score_by_fruit_distance (snake * s, fruit * frt)
{
  float maxdist, dist;
  float weight = FRUIT_DISTANCE_WEIGHT (s);
  cell *from;
  cell *to;
  scored_dir **dir = ((brain *) (s->brain))->sd;

  from = &(s->head->cell);
  to = &(frt->cell);

  /*
   * vertical distances 
   */
  maxdist = ARENA_HEIGHT;

  dist = distance (from, to, DOWN);
  if (dist)
    {
      dir[DOWN]->score += (MAXSCORE - ((dist / maxdist) * MAXSCORE)) * weight;
    }

  dist = distance (from, to, UP);
  if (dist)
    {
      dir[UP]->score += (MAXSCORE - ((dist / maxdist) * MAXSCORE)) * weight;
    }

  /*
   * horizontal distances 
   */
  maxdist = ARENA_WIDTH;

  dist = distance (from, to, RIGHT);
  if (dist)
    {
      dir[RIGHT]->score +=
	(MAXSCORE - ((dist / maxdist) * MAXSCORE)) * weight;
    }

  dist = distance (from, to, LEFT);
  if (dist)
    {
      dir[LEFT]->score += (MAXSCORE - ((dist / maxdist) * MAXSCORE)) * weight;
    }
}

/*
 * score by dead snake proximity 
 */
static void
score_by_deadsnake_proximity (snake * s)
{
  float weight = DEADSNAKE_PROXIMITY_WEIGHT (s);
  cell tmpcell;
  int i;
  scored_dir **dir = ((brain *) (s->brain))->sd;

  LOOPDIRECTION (i)
  {
    memcpy (&tmpcell, &(s->head->cell), sizeof (cell));
    move_cell (&tmpcell, i, 1);
    if (IS_EXSNAKE (ARENAGET (tmpcell.y, tmpcell.x)))
	    dir[i]->score += MAXSCORE * weight;
  }
}


#ifdef AI_SCORES_DEBUG
static void
dump_scores (snake * s)
{
  char buf[50], buf2[15];
  int i = 0;
  scored_dir **dirs = ((brain *) s->brain)->sd;

  sprintf (buf, "%i): ", (char) s->playernum);
  LOOPDIRECTION (i)
  {
    snprintf (buf2, 15, "%c: %4.2f ",
		  Direction[dirs[i]->dir], dirs[i]->score);
    strncat (buf, buf2, 15);
  }

  fprintf (stderr, "%s\n", buf);
}
#endif

static void
sort_direction_by_score (snake * s)
{
  int changes, i;
  scored_dir *ptr;
  scored_dir **sdir = ((brain *) (s->brain))->sd;

  do
  {
    changes = 0;
    for (i=0; i<3; i++)
	  {
      if (sdir[i]->score < sdir[i + 1]->score)
      {
        changes++;
        ptr = sdir[i];
        sdir[i] = sdir[i + 1];
        sdir[i + 1] = ptr;
      }
    }
  }
  while (changes);
}


/*
 * make a new brain 
 */
static brain *
make_brain (float w_snake_distance, float w_fruit_distance,
	    float w_random, float w_inertia,
	    float w_snake_presence, float w_free_space, float w_deadsnake)
{
  brain *newbrain;
  int i;

  newbrain = (brain *) safe_malloc (sizeof (brain), "ai new brain");

  newbrain->snake_distance_weight = w_snake_distance;
  newbrain->fruit_distance_weight = w_fruit_distance;
  newbrain->random_weight = w_random;
  newbrain->inertia_weight = w_inertia;
  newbrain->snake_presence_weight = w_snake_presence;
  newbrain->free_space_weight = w_free_space;
  newbrain->deadsnake_proximity_weight = w_deadsnake;


  /*
   * init scored_dir array 
   */
  LOOPDIRECTION (i)
  {
    newbrain->sd[i] = (scored_dir *) safe_malloc (sizeof (scored_dir),
						    "ai brain->sd");
    newbrain->sd[i]->dir = i;
    newbrain->sd[i]->score = 0;
  }

  return newbrain;
}

