/****************************************************************************
 * 
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
 ****************************************************************************
 *
 * file: ui_vga.c *  purpose: handle vga graphics 
 *
 ****************************************************************************
 *
 * $Log: ui_vga.c,v $
 * Revision 1.3  2003/12/19 19:00:58  strk
 * various small changes
 *
 * Revision 1.2  2003/11/10 21:19:06  strk
 * removed multi-line string
 *
 ****************************************************************************/



#include "sins.h"
#include <vga.h>

/* public functions */
int ui_init ();
int ui_initarena (ARENA *);
void ui_finish (void);
void ui_drawarena (ARENA *);
void ui_drawmessage (char *);
char *ui_prompt (char *);
int ui_getkey (void);

int CMD_spritesize (char *line);
int CMD_border (char *line);

/* private functions */
static void draw_borders(ARENA *);
static void draw_sprite (int sx, int sy, int color);

/* private data */
static char *arenamap; /* picture indexes */
static int spriteheight;
static int spritewidth;
static int border;
static vga_modeinfo *modeinfo;
static int vmode;

/* public data */
int screen_initialized = 0;


/* Initialize UI module */
int
ui_init ()
{

  if (!register_command ("spritesize", &CMD_spritesize,
			 "spritesize <width> <height>")) return 0;

  if (!register_command ("border", &CMD_border,
			 "border <num>")) return 0;

  /* set default dimensions */
  spritewidth = 4;
  spriteheight = 8;
  border = 1;

  /* what video mode should we use ? */
  vmode = vga_getdefaultmode();
  if ( !vga_hasmode(vmode) )
  {
    /* find out higher supported vmode */
    for (vmode=vga_lastmodenumber(); vmode; vmode--)
    {
      if (vga_hasmode(vmode)) break;
    }
  }

  /* no mode found (weird) */
  if (!vmode)
  {
    message("No videomode supported (?)");
    finish();
  }

  /* Get informations about videomode */
  modeinfo = vga_getmodeinfo(vmode);

  message("VGA videomode set to %i", vmode);

  return 1;
}

/* initialize the graphics */
int
ui_initarena (ARENA *arena)
{

  int maxcols, maxlines;

  /* Find maximum lines and columns allowed with this
     spritesize and border */
  maxcols = (modeinfo->width/spritewidth)-2*border;
  maxlines = (modeinfo->height/spriteheight)-2*border;

  if ( maxcols <= 0 || maxlines <= 0 )
  {
    message("There is not enought space for the arena!");
    return 0;
  }

  if ( arena->cols )
  {
      if ( arena->cols > maxcols )
      {
        message("Too many columns for this setup (try specifying fewer cols)");
        return 0;
      }
  }
  else arena->cols = maxcols;

  if ( arena->lines )
  {
      if ( arena->lines > maxlines )
      {
        message("Too many lines for this setup (try specifying fewer lines)");
        return 0;
      }
  }
  else arena->lines = maxlines;


  fprintf(stderr, "\n\
modeinfo->width: %i\n\
modeinfo->height: %i\n\
arena->cols: %i\n\
arena->lines: %i\n\
spritewidth: %i\n\
spriteheight: %i\n\
border: %i\n",
modeinfo->width,
modeinfo->height,
arena->cols,
arena->lines,
spritewidth,
spriteheight,
border);
  /*exit(0);*/

  /* initialize vga screen */
  vga_init();
  vga_setmode(vmode);

  /* set screen initialized flag */
  screen_initialized = 1;

  /* initialize the arena maps (used for optimization purposes) */
  arenamap = (char *)safe_malloc( arena->lines*arena->cols*sizeof(int),
    "ui_X: ui_drawarena: arenamap");
  memset(arenamap, -1, arena->lines * arena->cols);

  draw_borders(arena);

  return 1;

}

/* vga version of message drawing function */
void
ui_drawmessage (char *str)
{
  /*TODO* find out how to write messages somewhere */
  fprintf (stderr, "%s\n", str);
  return;
}

/* arena drawing */
void
ui_drawarena (ARENA *arena)
{
  int y, x;
  sprite *sp;
  int pictidx;

  for (y = 0; y < arena->lines; y++)
  {
    for (x = 0; x < arena->cols; x++)
    {

      sp = GETSPRITE(arena,y,x);

      if (sp) switch (sp->type)
      {
        case SNAKE:
          pictidx = 4;
          break;  
        case EXSNAKE:
          pictidx = 8;
          break; 
        case FRUIT:
          if ( ((fruit *)(sp))->value == 48 ) pictidx = 2;
          else pictidx = 1;
          break;
        case WALL:
          pictidx = 1;
          break;
        default:
          pictidx = 0;
          break;
      }
      else pictidx = 0;

      if ( *(arenamap+y*arena->cols+x) == pictidx ) continue;
      *(arenamap+y*arena->cols+x) = pictidx;

      draw_sprite(x+border, y+border, pictidx);
    }
  }
}



void
ui_finish ()
{
  if ( screen_initialized ) vga_setmode(TEXT);
}


/*
 * Get next key in input buffer
 */
int
ui_getkey ()
{
  int ret;
  
  ret=vga_getkey();
  if (!ret) return -1;
  return ret;
}

/*
 * prompt the user for a string
 * the line returnered is allocated with malloc(3)
 * so the caller must free it when finished with it. 
 */
char *
ui_prompt (char *prompt_string)
{
  return NULL;
}

int
CMD_spritesize (char *line)
{
  int width, height;

  if ( screen_initialized )
  {
    message("can't change sprite size after screen initialization");
    return 0;
  }

  if ( 2 != sscanf(line, "%i %i", &width, &height) )
    return -1;

  if ( ! width || width > modeinfo->width ||
    ! height || height > modeinfo->height )
  {
    message("sprite size out of range 1x1 - %ix%i",
      modeinfo->width,modeinfo->height);
    return 0;
  }
  
  spritewidth = width;
  spriteheight = height;

  return 0;
}

int
CMD_border (char *line)
{
  int size;
  int maxsize;

  if ( screen_initialized )
  {
    message("can't change border size after screen initialization");
    return 0;
  }

  if ( 1 != sscanf(line, "%i", &size) )
    return -1;

  maxsize = modeinfo->width/3;
  if ( maxsize > modeinfo->height/3 ) maxsize = modeinfo->height/3;

  if ( size > maxsize )
  {
    message("border size must be smaller then %i", maxsize);
    return 0;
  }

  border = size;
  return 0;
}

/************ PRIVATE FUNCTIONS **********************/

static void
draw_borders (ARENA *arena)
{
  int x,y,i;
  int bordercolor = 7;


  for (x=0; x<arena->cols+2*border; x++)
  {
      for (i=0; i<border; i++)
      {
        draw_sprite(x,i,bordercolor);
        draw_sprite(x,arena->lines+2*border-i-1,bordercolor);
      }
  }

  for (y=0; y<arena->lines+2*border; y++)
  {
      for (i=0; i<border; i++)
      {
        draw_sprite(i,y,bordercolor);
        draw_sprite(arena->cols+2*border-i-1,y,bordercolor);
      }
  }
}

static void
draw_sprite (int sx, int sy, int color)
{
  int x,y,i,j;

  vga_setcolor(color);
  for (x=sx*spritewidth, i=0; i<spritewidth; i++, x++)
  {
    for (y=sy*spriteheight, j=0; j<spriteheight; y++, j++)
    {
      vga_drawpixel(x,y);
    }
  }
  vga_setcolor(vga_white());
}

