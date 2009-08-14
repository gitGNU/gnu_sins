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
 * $Id: mod_wall.c,v 1.4 2009/08/14 15:48:30 strk Exp $
 ****************************************************************************/


#include <ctype.h>
#include <math.h>
#include "sins.h"

#define MAXWALLS 100

/* wall block */
typedef struct t_wallblock
{
  SPRITE_TYPE type;
  struct t_wallblock *next;
  cell cell;
}
wallblock;

/* wall */
typedef struct t_wall
{
  wallblock *start;	/* wall first block */
}
wall;


/* public functions */
int wall_module_init (void);
int CMD_wall (char *);
int CMD_dewall (char *);

/* private data */
static wall *walls[MAXWALLS];

/* private functions */
static wall *create_wall (char *);
static int draw_wall (wall *);
static wall *make_wall_segment (int ax, int ay, int bx, int by);
  

int
wall_module_init ()
{
  if (!register_command("wall", &CMD_wall, "wall <ax> <ay>, <bx> <by> [try W for *x and H for *y]"))
    return 0;

  if (!register_command("dewall", &CMD_dewall, "dewall <#>"))
    return 0;

  return 1;
}

int
CMD_wall (char *spec)
{
  int i;
  int freeslot = -1;
  wall *newwall;

  /* Find first free slot of wall */
  for (i=0; i<MAXWALLS; i++)
  {
    if ( walls[i] == 0 )
    {
      freeslot = i;
      break;
    }
  }
  if ( freeslot == -1 )
  {
    message("No available slot for wall");
    return 0;
  }

  /* Syntax error */
  if ( !spec ) return -1;

  /* Can't work without an initialized screen */
  if ( ! screen_initialized ) return 0;

  /* Create the new wall */
  if ( ! (newwall=create_wall(spec)) ) return -1;

  walls[freeslot] = newwall;
  if ( ! draw_wall(newwall) ) return -1;

  return 1;
}

int
CMD_dewall (char *num)
{
  int wnum;

  /* Syntax errors */
  if ( !num ) return -1;
  if ( sscanf(num, "%d", &wnum) != 1 ) return -1;

  if ( wnum >= MAXWALLS ) {
    message("Walls are numbered from 0 to %d", MAXWALLS-1);
    return 0;
  }

  message("Should delete wall number %d", wnum);

  return 1;
}

/*
 * Return an integer being the result of evaluating
 * next expression from the input spec, and advance
 * the spec pointer to point to past the end of
 * expression.
 *
 * Return -1 and set *spec to NULL on malformed input.
 *
 */
static int
parse_int_expression(char **spec)
{
  int ret;
  char* endptr;

  /* Skip spaces */
  while (**spec && isspace(**spec)) ++(*spec);

  /* Check premature end */
  if ( ! **spec ) 
  {
    *spec = NULL;
    return -1;
  }

  /* Substitute W (arena width) label */
  if ( **spec == 'W' || **spec == 'w' )
  {
    ++(*spec);
    return ARENA_WIDTH-1;
  }

  /* Substitute H (arena height) label */
  if ( **spec == 'H' || **spec == 'h' )
  {
    ++(*spec);
    return ARENA_HEIGHT-1;
  }

  /* Parse a long */
  ret = strtol(*spec, &endptr, 10);

  fprintf(stderr, "spec=%s, ret=%d, endptr=%s\n", *spec, ret, endptr);

  *spec = endptr;

  return ret;
}

static wall *
create_wall (char *spec)
{
  int ax, ay, bx, by;
  wall *w;
  char *ptr = spec;

  // TODO: substitute W and H with width and height of arena

  // Get AX
  ax = parse_int_expression(&ptr);
  if ( ! ptr ) return NULL;

  // Get AY
  ay = parse_int_expression(&ptr);
  if ( ! ptr ) return NULL;

  // Skip comma
  while (*ptr && *ptr == ',') ++ptr;
  if ( ! ptr ) return NULL;

  // Get BX
  bx = parse_int_expression(&ptr);
  if ( ! ptr ) return NULL;

  // Get BY
  by = parse_int_expression(&ptr);

  fprintf(stderr, "A(%d,%d) B(%d,%d)\n", ax, ay, bx, by);
  w = make_wall_segment(ax, ay, bx, by);
  if ( ! w ) return NULL;
  return w;
}


/*
 * This has been taken from:
 *  http://www.geocities.com/CapeCanaveral/3439/tdr.zip
 */
static wall *
make_wall_segment_copied (int x, int y, int x2, int y2)
{
 int dx,dy,long_d,short_d;
 int d,add_dh,add_dl;
 register int inc_xh,inc_yh,inc_xl,inc_yl;
 register int i;
  wall *w;
  wallblock *wb=NULL, *pwb=NULL; /* Wall block and previous */

 dx=x2-x; dy=y2-y;                          /* ranges */

 if(dx<0){dx=-dx; inc_xh=-1; inc_xl=-1;}    /* making sure dx and dy >0 */
 else    {        inc_xh=1;  inc_xl=1; }    /* adjusting increments */
 if(dy<0){dy=-dy; inc_yh=-1; inc_yl=-1;}
 else    {        inc_yh=1;  inc_yl=1; }

 if(dx>dy){long_d=dx; short_d=dy; inc_yl=0;}/* long range,&making sure either */
 else     {long_d=dy; short_d=dx; inc_xl=0;}/* x or y is changed in L case */

 d=2*short_d-long_d;                        /* initial value of d */
 add_dl=2*short_d;                          /* d adjustment for H case */
 add_dh=2*short_d-2*long_d;                 /* d adjustment for L case */

  w = safe_malloc(sizeof(wall), "new wall");
  memset(w, 0, sizeof(wall));

  /* for all points in longer range */
  for(i=0;i<=long_d;i++)                    
  {
    wb = safe_malloc(sizeof(wallblock), "new wall's block");
    wb->type = WALL;
    if ( ! w->start ) w->start = wb;

    if ( pwb ) pwb->next = wb;
    pwb = wb;

    /* rendering */
    wb->cell.x = x;
    wb->cell.y = y;

    if(d>=0){x+=inc_xh; y+=inc_yh; d+=add_dh;}/* previous point was H type */
    else    {x+=inc_xl; y+=inc_yl; d+=add_dl;}/* previous point was L type */
  }

  return w;
}

static wall *
make_wall_segment (int ax, int ay, int bx, int by)
{
  int sw;
  wall *w;
  double num, den;
  double u = 0;
  wallblock *wb=NULL, *pwb=NULL; /* Wall block and previous */
  int x, y;

  w = safe_malloc(sizeof(wall), "new wall");
  memset(w, 0, sizeof(wall));

  /* Line is only a point */
  if ( ax==bx && ay==by )
  {
    wb = safe_malloc(sizeof(wallblock), "new wall's block");
    wb->type = WALL;
    wb->cell.x = ax;
    wb->cell.y = ay;
    wb->next = NULL;
    w->start = wb;
    return w;
  }

  /* Swap values so to have A be upper-left and B lower-right */
  if ( ax > bx ) { sw = ax; ax=bx; bx=sw; }
  if ( ay > by ) { sw = ay; ay=by; by=sw; }

  num = by-ay;
  den = bx-ax;


  /*
   * Loop on longer side
   */
  u = num/den;
  fprintf(stderr, "%g/%g=%g\n", num,den,u);
  x=ax; y=ay;
  while ( 1 )
  {
    int finish=0;

    wb = safe_malloc(sizeof(wallblock), "new wall's block");
    wb->type = WALL;
    if ( ! w->start ) w->start = wb;

    if ( den >= num ) {
      y = (int)(rint(u*(x-ax)+ay));
      wb->cell.x = x;
      wb->cell.y = y;
      if ( ax < bx ) {
        if ( x++ >= bx ) finish=1;
      } else {
        if ( x-- <= bx ) finish=1;
      }
    } else {
      x = (int)(rint((y-ay)/u+ax));
      wb->cell.x = x;
      wb->cell.y = y;
      if ( ay < by ) {
        if ( y++ >= by ) finish=1;
      } else {
        if ( y-- <= by ) finish=1;
      }
    }

    fprintf(stderr, "cell: %d,%d\n", wb->cell.x, wb->cell.y);
    wb->next = NULL;
    if ( pwb ) pwb->next = wb;
    pwb = wb;
    if ( finish ) break;
  }

  return w;
}


static int
draw_wall (wall *w)
{
  int x,y;
  wallblock *ptr;

  for (ptr = w->start; ptr; ptr=ptr->next )
  {
    x = ptr->cell.x;
    y = ptr->cell.y;
    if ( ! ARENAGET(y,x) ) ARENAPUT(y, x, (sprite *)ptr); 
  }

  /* message("Wall drawing not implemented yet"); */
  return 1;
}

