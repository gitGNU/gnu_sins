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
 * $Id: ui_vga2.c,v 1.2 2004/07/28 13:33:13 strk Exp $
 ****************************************************************************
 *
 * purpose: handle vga graphics 
 *
 ****************************************************************************/

#include "sins.h"
#include "ui_picts.h"
#include <vga.h>
#include <vgagl.h>

#define BORDERSIZE 1

/* public functions */
int ui_init ();
int ui_initarena (ARENA *);
void ui_finish (void);
void ui_drawarena (ARENA *);
void ui_drawmessage (char *);
char *ui_prompt (char *);
int ui_getkey (void);
int CMD_loadpictset (char *line);

/* private functions */
static void draw_borders(ARENA *);
static void init_background(ARENA *);
static void draw_box (int sx, int sy, int color);
static void draw_pict (int sx, int sy, int pictnum);
static void init_fonts (void);
static int *load_pictset (char *filename);
static void show_modeinfo (int vmode);

/* private data */
static char *arenamap; /* picture indexes */
static int spriteheight;
static int spritewidth;
static int *pictset;

/* public data */
int screen_initialized = 0;

/* Initialize UI module */
int
ui_init ()
{
  if (!register_command ("loadpictset", &CMD_loadpictset,
			 "loadpictset [<fname>]")) return 0;

  pictset = defpictset;

  return 1;
}

/* initialize the graphics */
int
ui_initarena (ARENA *arena)
{

  int xdim, ydim;
  int vmode;

  /* what video mode should we use ? */
  vmode = vga_getdefaultmode();
  if ( !vga_hasmode(vmode) )
  {
    /* find out higher supported vmode */
    for (vmode=vga_lastmodenumber(); vmode; vmode--)
    {
      if (vga_hasmode(vmode))
      {
        break;
      }
    }
  }

  /* no mode found (weird) */
  if (!vmode)
  {
    message("No videomode supported (?)");
    finish();
  }

  /*vmode = 1;*/
  message("VGA videomode set to %i", vmode);
  show_modeinfo(vmode);

  /* initialize vga screen */
  vga_init();
  vga_setmode(vmode);

  /* initialize the vgagl context (needed for font use) */
  gl_setcontextvga(vmode);

  xdim = vga_getxdim();
  ydim = vga_getydim()-12; /* keep space for font messages */

  /* Set arena dimensions */
  if ( arena->lines || arena->cols ) message("ignoring -l and -c switches");
  if ( !spritewidth ) spritewidth=8;
  if ( !spriteheight ) spriteheight=8;
  arena->lines = ydim/spriteheight - 2*BORDERSIZE;
  arena->cols = xdim/spritewidth - 2*BORDERSIZE;

  /* initialize the arena maps (used for optimization purposes) */
  arenamap = (char *)safe_malloc( arena->lines*arena->cols*sizeof(int),
    "ui_vga2: ui_drawarena: arenamap");
  memset(arenamap, -1, arena->lines * arena->cols);

  init_background(arena);
  draw_borders(arena);
  init_fonts();

  /* Set screen initialized flag */
  screen_initialized = 1;

  return 1;

}

/* vga version of message drawing function */
void
ui_drawmessage (char *str)
{
  int ydim = vga_getydim();
  int xdim = vga_getxdim();
  int maxlen = xdim/8;

  /* truncate the string to the maximum length allowed */
  if ( strlen(str) > maxlen ) *(str+maxlen) = '\0';

  /* something is wrong here */
  gl_fillbox(0, ydim-12, xdim, 12, 0);
  gl_write(0, ydim-12, str);

  return;
}

/* arena drawing */
/**** DEFAULT SVGALIB PALETTE 
0      - black.
1      - blue.
2      - green.
3      - cyan.
4      - red.
5      - magenta.
6      - brown.
7      - grey.
8      - dark grey (light black).
9      - light blue.
10     - light green.
11     - light cyan.
12     - light red.
13     - light magenta.
14     - yellow (light brown).
15     - white (light grey).
***************************************************/
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
          pictidx = PICT_SNAKE0+((snakebody *)(sp))->self->playernum;
          break;  
        case EXSNAKE:
          pictidx = PICT_EXSNAKE;
          break; 
        case FRUIT:
          pictidx = PICT_FRUIT0+(((fruit *)(sp))->value)-48;
          break;
        case WALL:
          pictidx = PICT_WALL;
          break;
        default:
          pictidx = PICT_UNKNOWN;
          break;
      }
      else pictidx = PICT_EMPTY;

      if ( *(arenamap+y*arena->cols+x) == pictidx ) continue;
      *(arenamap+y*arena->cols+x) = pictidx;

      draw_pict(x,y,pictidx);
    }
  }
}



void
ui_finish ()
{
  vga_setmode(TEXT);
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
  int xdim = vga_getxdim();
  char *cmdline;
  int key, len=0;

  cmdline = safe_malloc(MAXCMDLINELEN, "prompt input buffer");
  memset(cmdline, '\0', MAXCMDLINELEN);

  ui_drawmessage(": ");

  while (1)
  {

    key=vga_getkey();
    if (!key) continue;

    /* max command line lenght reached */
    if ( len > MAXCMDLINELEN ) break;

    if ( len > xdim-2 )
    {
      /* should scroll left */
      break;
    }

    if ( key == '\n' ) break;

    /* handle backspace */
    if ( key == 127 )
    {
        if (len) cmdline[--len] = '\0';
    }

    else cmdline[len++] = key;

    message(": %s", cmdline);

  }

  ui_drawmessage("");

  return cmdline;
}

int
CMD_loadpictset (char *line)
{
  char fname[MAXPATHLEN];
  char *fpath;
  int ret = 0;
  int *newset;

  if ( line ) ret = sscanf (line, "%s", fname);
  if ( ret != 1 )
  {
    /* set default pictset (should instead complain about wrong num of args?) */
    pictset = defpictset;
    message("default pictset loaded");
    return 1;
  }

  if ( fname[0] != '/' )
  {
    if ( (fpath=find_file(fname)) == NULL )
    {
            message ("Can't find file %s in path %s", fname, path);
            return 0;
    }
  }
  else
  {
    fpath = fname;
  }

  newset = load_pictset(fpath);
  if ( !newset )
  {
    message("Can't load pictset from %s", fpath);
    return 0;
  }

  if ( pictset != defpictset ) free(pictset);
  pictset = newset;

  message("pictset %s loaded", fname);
  return 1;
}

/************ PRIVATE FUNCTIONS **********************/

static void
init_background(ARENA *arena)
{
  int y,x;

  /* initialize background */
  for (y = 0; y < arena->lines; y++)
  {
    for (x = 0; x < arena->cols; x++)
    {
      draw_pict(x,y,PICT_EMPTY);
    }
  }
}

static void
draw_borders (ARENA *arena)
{
  int x,y,i;
  int bordercolor = 7;


  for (x=0; x<arena->cols+2*BORDERSIZE; x++)
  {
      for (i=0; i<BORDERSIZE; i++)
      {
        draw_box(x,i,bordercolor);
        draw_box(x,arena->lines+2*BORDERSIZE-i-1,bordercolor);
      }
  }

  for (y=0; y<arena->lines+2*BORDERSIZE; y++)
  {
      for (i=0; i<BORDERSIZE; i++)
      {
        draw_box(i,y,bordercolor);
        draw_box(arena->cols+2*BORDERSIZE-i-1,y,bordercolor);
      }
  }
}

static void
draw_box (int sx, int sy, int color)
{
  int x,y,i,j;
  int white = vga_white();

  vga_setcolor(color);
  for (x=sx*spritewidth, i=0; i<spritewidth; i++, x++)
  {
    for (y=sy*spriteheight, j=0; j<spriteheight; y++, j++)
    {
      vga_drawpixel(x,y);
    }
  }
  vga_setcolor(white);
}

static void
draw_pict (int sx, int sy, int pictnum)
{
  int x,y,i,j;
  /*int white = vga_white();*/
  int *pict;

  sx=(sx+BORDERSIZE)*spritewidth;
  sy=(sy+BORDERSIZE)*spriteheight;
  pict=pictset+pictnum*spritewidth*spriteheight;

  for (x=sx, i=0; i<spritewidth; i++, x++)
  {
    for (y=sy, j=0; j<spriteheight; y++, j++)
    {
      vga_setcolor(*(pict+(j*spritewidth)+i));
      vga_drawpixel(x,y);
    }
  }
}

static void
init_fonts ()
{
  gl_setfont(8, 8, gl_font8x8);
  gl_setwritemode(FONT_COMPRESSED + WRITEMODE_OVERWRITE);
  gl_setfontcolors(0, vga_white());
}

static int *
load_pictset (char *filename)
{
  FILE *stream;
  int idx = 0;
  int quoted = 0;
  int ch;
  int *set;
  int w, h;

  stream = fopen(filename, "r");
  if ( !stream ) return NULL;

  /* check sprite size */
  if ( fscanf(stream, "#%ix%i\n", &w, &h) != 2 )
  {
    message("Invalid pictset format (missing '#WxH' on line 1)");
    return NULL;
  }

  if ( screen_initialized && (w != spritewidth || h != spriteheight) )
  {
    message("Can't change sprite size while running");
    return NULL;
  }

  spritewidth = w;
  spriteheight = h;
  set = safe_malloc(w*h*PICTSETLEN*sizeof(int), "New pictset");

  while (!feof(stream))
  {
    if ( idx >= w*h*PICTSETLEN ) break;
    ch=getc(stream);
    if (ch) switch(ch)
    {
      case '\n':
      case '\r':
        quoted=0;
        break;
      case ' ':
      case '\t':
        break;
      case '#':
        quoted=1;
        break;
      default:
        if ( !quoted )
        {
          *(set+idx) = ch-97;
          idx++;
        }
        break;
    }
  }
  fclose(stream);
  return set;
}

static void
show_modeinfo (int vmode)
{
  vga_modeinfo *vmodeinfo;

  vmodeinfo=vga_getmodeinfo(vmode);

  message("[VIDEOMODE %i]", vmode);
  message("  width: %i", vmodeinfo->width);
  message("  height: %i", vmodeinfo->height);
  message("  bytesperpixel: %i", vmodeinfo->bytesperpixel);
  message("  colors: %i", vmodeinfo->colors);
  message("  linewidth: %i", vmodeinfo->linewidth);
  message("  maxlogicalwidth: %i", vmodeinfo->maxlogicalwidth);
  message("  startaddressrange: %i", vmodeinfo->startaddressrange);
  message("  maxpixels: %i", vmodeinfo->maxpixels);
  message("  haveblit: %i", vmodeinfo->haveblit);
  message("  flags: %i", vmodeinfo->flags);
  if ( vmodeinfo->flags&HAVE_RWPAGE ) message("    HAVE_RWPAGE");
  if ( vmodeinfo->flags&IS_INTERLACED ) message("    IS_INTERLACED");
  if ( vmodeinfo->flags&IS_MODEX ) message("    IS_MODEX");
  if ( vmodeinfo->flags&IS_DYNAMICMODE ) message("    IS_DYNAMICMODE");
  if ( vmodeinfo->flags&CAPABLE_LINEAR ) message("    CAPABLE_LINEAR");
  if ( vmodeinfo->flags&IS_LINEAR ) message("    IS_LINEAR");
  if ( vmodeinfo->flags&RGB_MISORDERED ) message("    RGB_MISORDERED");
  if ( vmodeinfo->flags&HAVE_EXT_SET ) message("    HAVE_EXT_SET");
  if ( vmodeinfo->flags&EXT_INFO_AVAILABLE )
  {
    message("    EXT_INFO_AVAILABLE");
    message("      chiptype: %i", vmodeinfo->chiptype);
    message("      memory: %i", vmodeinfo->memory);
    message("      linewidth_unit: %i", vmodeinfo->linewidth_unit);
    message("      linear_aperture: (char *) ?");
    message("      aperture_size: %i", vmodeinfo->aperture_size);
  }

}

