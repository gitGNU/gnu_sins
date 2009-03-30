/*
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
 */

#include "sins.h"
#include <sys/stat.h>
#include <unistd.h>

#undef DEBUG_MALLOC

/* public functions */
int rand_between (int low, int high);
void move_cell (cell * from, DIRECTION dir, int steps);
int cell_distance (cell *from, cell *to);
void *safe_malloc (size_t size, char *reason);
void *safe_calloc (size_t nmemb, size_t size, char *reason);
void *safe_realloc (void *ptr, size_t size, char *reason);
int CMD_path (char *);

/* public data */
char path[SEARCHPATHLEN] = DEFAULT_SEARCHPATH;

int
rand_between (int low, int high)
{
  int ret;
  ret =
    (int) ((double) ((high + 1) - low) * (rand () / (RAND_MAX + 1.0))) + low;

  return ret;
}

/*
 * take the ``cell'' ``steps'' steps in direction ``dir''.
 * wrap around the screen.
 */
void
move_cell (cell * from, DIRECTION dir, int steps)
{
  if (dir == UP)
    from->y = (ARENA_HEIGHT + (from->y - steps)) % ARENA_HEIGHT;

  else if (dir == LEFT)
    from->x = (ARENA_WIDTH + (from->x - steps)) % ARENA_WIDTH;

  else if (dir == DOWN)
    from->y = (from->y + steps) % ARENA_HEIGHT;

  else if (dir == RIGHT)
    from->x = (from->x + steps) % ARENA_WIDTH;

}

void *
safe_malloc (size_t size, char *reason)
{
  void *ret;

#ifdef DEBUG_MALLOC
  fprintf (stderr, "safe_malloc: allocating %i bytes of memory for %s\n",
	   size, reason);
#endif

  ret = malloc (size);
  if (!ret)
    {
      perror ("malloc");
      finish ();
    }

  return ret;
}

void *
safe_calloc (size_t nmemb, size_t size, char *reason)
{
  void *ret;

#ifdef DEBUG_MALLOC
  fprintf (stderr,
	   "safe_calloc: allocating %i memory segments of %i bytes for %s\n",
	   nmemb, size, reason);
#endif

  ret = calloc (nmemb, size);
  if (!ret)
    {
      perror ("calloc");
      finish ();
    }

  return ret;
}

void *
safe_realloc (void *ptr, size_t size, char *reason)
{
  void *ret;

#ifdef DEBUG_MALLOC
  fprintf (stderr, "safe_realloc: allocating %i bytes of memory for %s\n",
	   size, reason);
#endif

  ret = realloc (ptr, size);
  if ( ! ret )
  {
    perror ("realloc");
    finish ();
  }

  return ret;
}

/*
 * Compute *shortest* distance between two cells
 */
int
cell_distance (cell *f, cell *t)
{
  int xdist, ydist, dist;
  int l, h;

  l=ARENA_WIDTH;
  h=ARENA_HEIGHT;

  xdist = abs(t->x-f->x);
  if ( xdist > l/2 ) xdist = l-xdist;

  ydist = abs(t->y-f->y);
  if ( ydist > h/2 ) ydist = h-ydist;

  dist = xdist+ydist;
#define DEBUG_DISTANCES
#ifdef DEBUG_DISTANCES
  fprintf(stderr, "(%i,%i) <-> (%i,%i) == %i\n", f->x, f->y, t->x, t->y, dist);
#endif

  return dist;
}

/* Find filename in PATH */
char *
find_file (char *name)
{
  char pathbuf[strlen(path)+1];
  char *bp = NULL;
  struct stat statbuf;
  static char fname[MAXPATHLEN];
  char *dir;

  /*
   * Search for file in sourcepath
   */
  strcpy (pathbuf, path);
  bp = pathbuf;
  while ( (dir=strtok(bp, ":")) )
  {
    bp = NULL;
    if ( dir[0] == '~' )
    {
      strncpy(fname, getenv("HOME"), MAXPATHLEN);
      if ( strlen(dir) > 1 )
      {
        dir++;
        if ( strlen(fname) >= MAXPATHLEN-strlen(dir) ) continue;
        strncpy(fname, dir, MAXPATHLEN);
      }
    }
    else
    {
      strncpy(fname, dir, MAXPATHLEN);
    }
    if ( strlen(fname) >= MAXPATHLEN-strlen(name)-1 ) continue;
    strcat(fname, "/");
    strcat(fname, name);
    if ( ! stat(fname, &statbuf) ) return fname;
  }

  return NULL;
}

int
CMD_path (char *string)
{
  if ( string )
  {
    strncpy (path, string, SEARCHPATHLEN);
    path[SEARCHPATHLEN-1] = '\0';
  }

  message("path %s", path);

  return 1;
}
