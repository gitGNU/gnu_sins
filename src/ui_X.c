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

/*
 * file: ui_X.c *  purpose: handle X graphics 
 */

#include "sins.h"
#include "ui_picts.h"
#include <X11/Xlib.h> 
#include <X11/Xutil.h>   /* XSizeHints */
#include <X11/keysym.h>  /* for a perfect(?) use of keyboard events */
#include <string.h>

/*
 * Define this to try double-buffering as suggested by Tronche.
 * A lot more memory is required on the server side for this since
 * the arena area is duplicated. On the other hand there are less
 * exposure-handling paranoias since nothing would mess with the
 * pixmap integrity so it's enought to copy it to the window itself
 * everytime it's needed.
 *
 * Of course, this will also slow down performances.
 *
 */
#undef USE_ARENAWIN_PIXMAP

/*
 * Define this to use root window as the arena
 * ( no keyboard control in this case )
 */
#undef USE_ROOT_WINDOW

/*
 * Define this to tell the window manager we
 * prefer to keep out initial window size
 * ( resizes would just force a redraw )
 */
#define USE_SIZE_HINTS

/*
 * Define this to declare a Class and Resource
 * name for the application. You can use these
 * names to set parameters of the WM such as
 * border width, colors and the like ...
 * Class can also be used to set parameters queriable
 * by the application itself.
 *
 *   *TODO* should probably always use class hints and
 *   ask here just for the class and resource names ...
 */
#define USE_CLASS_HINTS

/*
 * Backingstore is the will to keep window graphics even at
 * not-viewable areas.... this speeds up the application 
 * reducing the number of redraws required. Backingstore
 * is not guaranteed to work ( could be dropped by the X
 * server when there's not enought memory available )
 *
 * Undefine this to reduce server memory usage or to 
 * test how would the game perform without it.
 * 
 */
#define USE_BACKINGSTORE

/*
 * Default font color 
 * may be overwritten with the `fontcolor' command
 */
#define DEFAULTFONTCOLOR "rgb:FFFF/FFFF/0000"

/*
 * Default font definition
 * may be overwritten with the `loadfont' command
 */
#define DEFAULTFONTDEF "*-misc-*"

/*
 * Print on stderr debugging informations about events
 */
#undef DEBUG_X_EVENTS

/*
 * Use XAfterFunction for debugging purpose
 */
#undef DEBUG_X_PROTOCOL

/*
 * Define this if you want the arena to be redrawn
 * when pictset changes (loadpictset command).
 * This produces a lot of X protocol requests, thus
 * augmenting network traffic and reducing game speed.
 * On the other hand not redrawing it is ugly !
 */
#define REDRAW_ARENA_ON_PICTSET_CHANGE

#define SPRITEINMESSAGEBOX(x,y) (\
  ( ((x)*spritewidth)               <= msgbox.bx + msgbox.bw  ) && \
  ( ((x)*spritewidth)+spritewidth   >= msgbox.bx              ) && \
  ( ((y)*spriteheight)              <= msgbox.by + msgbox.bh  ) && \
  ( ((y)*spriteheight)+spriteheight >= msgbox.by              ) )
  

#define MAXFONTCOLORDEFLEN 256
#define MAXFONTDEFLEN 256

/* public functions */
int ui_init ();
int ui_initarena (ARENA *);
void ui_finish (void);
void ui_drawarena (ARENA *);
void ui_drawmessage (char *);
char *ui_prompt (char *);
int ui_getkey (void);
int CMD_loadpictset (char *line);
int CMD_fontcolor (char *line);
int CMD_loadfont (char *pattern);

/* private functions */
static void init_background(ARENA *);
static int *load_pictset (char *filename);
#ifndef USE_ROOT_WINDOW
static Window create_arenawin (Display *dpy, int width, int height);
#endif
static void create_pictset_pixmap (void);
#ifdef USE_ARENAWIN_PIXMAP
static void create_arenawin_pixmap (void);
static void UpdateScreen(ARENA *arena);
#endif
#ifdef USE_BACKINGSTORE
static void enable_backingstore(Display *d, Window w);
#endif
static int send_pictset (void);
static int copy_pict (int x, int y, int pictnum);
static void redraw_arenarect(int rx, int ry, int rw, int rh);
static void create_graphic_contexts (void);
static void init_colors (void);
static int load_font (char *);
static int font_color (char *);
#ifdef DEBUG_X_PROTOCOL
static int afterfunc (Display *dpy);
#endif

/* private data */
static char *arenamap; /* picture indexes */
static int spriteheight;
static int spritewidth;
static int *pictset;
static int pictsetsent = 0;
static int coloridx[16]; /* index of default 16 colors */

static Display *dpy;
static Screen *scr;
static Window arenawin;
static Pixmap pictset_pixmap;
#ifdef USE_ARENAWIN_PIXMAP
static Pixmap arenawin_pixmap;
#endif
static GC arenagc; 
static int ydim, xdim, oldydim, oldxdim;
static GC messagegc; 
static Font font = -1;
static char fontdef[MAXFONTDEFLEN] = DEFAULTFONTDEF;
static char fontcolordef[MAXFONTCOLORDEFLEN] = DEFAULTFONTCOLOR;
static int fontcolor = -1;
static struct 
{
  short bx, by;          /* box origin */
  unsigned short bw, bh; /* box size */
  short fx, fy;          /* font origin */
  Region reg;            /* actual message region */
} msgbox;

/* public data */
int screen_initialized = 0;

/* Initialize UI module */
int
ui_init ()
{
  if (!register_command ("loadpictset", &CMD_loadpictset,
			 "loadpictset [<fname>]")) return 0;

  if (!register_command ("fontcolor", &CMD_fontcolor,
			 "fontcolor [<colordef>]")) return 0;

  if (!register_command ("loadfont", &CMD_loadfont,
			 "loadfont [<pattern>]")) return 0;

  pictset = defpictset;

  return 1;
}

/* initialize the graphics */
int
ui_initarena (ARENA *arena)
{

  /* Connect to the X server */
  if ( ! (dpy=XOpenDisplay(NULL)) ) 
  {
    message ("Can't open display: %s", getenv("DISPLAY"));
    finish();
  }

#ifdef DEBUG_X_PROTOCOL
  XSetAfterFunction(dpy, &afterfunc);
#endif

  scr = DefaultScreenOfDisplay(dpy);

  /* default dimensions */
#ifdef USE_ROOT_WINDOW
  xdim = scr->width;
  ydim = scr->height;
#else
  xdim = (scr->width) * 0.7;
  ydim = (scr->height) * 0.7;
#endif

  /* Set window dimensions based on arena and sprites dimensions */
  if ( !spritewidth ) spritewidth=8;
  if ( !spriteheight ) spriteheight=8;
  if ( arena->lines )
  {
    /* set window height based on arena and sprite heights */
    ydim = spriteheight * ( arena->lines );
  }
  else
  {
    /* set arena height based on window and sprite heights */
    arena->lines = ydim/spriteheight;
    ydim -= ydim%spriteheight;
  }

  if ( arena->cols )
  {
    /* set window width based on arena and sprite widths */
    xdim = spritewidth * ( arena->cols );
  }
  else 
  {
    /* set arena width based on window and sprite widths */
    arena->cols = xdim/spritewidth;
    xdim -= xdim%spritewidth;
  }


  /* remember this original "dimensions" */
  oldxdim = xdim;
  oldydim = ydim;

message("xdim=%d ydim=%d", xdim, ydim);
  create_pictset_pixmap();
#ifdef USE_ARENAWIN_PIXMAP
  create_arenawin_pixmap();
#endif


#ifdef DEBUG_X_PROTOCOL
  fputs("Setting up arena window", stderr);
#endif

#ifdef USE_ROOT_WINDOW
  arenawin = DefaultRootWindow(dpy);
  XSelectInput(dpy, arenawin, ExposureMask);
#else
  arenawin = create_arenawin(dpy, xdim, ydim);
#endif

  /* We should not rely on this */
#ifdef USE_BACKINGSTORE
  enable_backingstore(dpy, arenawin);
#endif

#ifdef DEBUG_X_PROTOCOL
  fputs(" done!\n", stderr);
#endif

#ifdef DEBUG_X_PROTOCOL
  fputs("Setting up graphic contexts", stderr);
#endif

  create_graphic_contexts();

#ifdef DEBUG_X_PROTOCOL
  fputs(" done!\n", stderr);
#endif

#ifdef DEBUG_X_PROTOCOL
  fputs("Setting up colors", stderr);
#endif

  init_colors();

#ifdef DEBUG_X_PROTOCOL
  fputs(" done!\n", stderr);
#endif


#ifdef DEBUG_X_PROTOCOL
  fputs("Loading font", stderr);
#endif

  /* load font */
  if ( ! load_font(fontdef) )
  {
    finish();
  }

#ifdef DEBUG_X_PROTOCOL
  fputs("  done!\n", stderr);
#endif

  /* Set font color */
  if ( ! font_color (fontcolordef) )
  {
    finish();
  }

  send_pictset();

  init_background(arena);

  /*
   * cache server's KeySym now ... just to avoid later delay.
   * there should be a better way, but i'm an hacker after all ...
   */
  XKeycodeToKeysym(dpy, 10, 0);

  /* initialize the arena maps (used for optimization purposes) */
  arenamap = (char *)safe_malloc( arena->lines*arena->cols*sizeof(int),
    "ui_X: ui_initarena: arenamap");
  memset(arenamap, 0, arena->lines * arena->cols * sizeof(int));

  /* Set screen initialized flag */
  screen_initialized = 1;

  return 1;

}

/* X version of message drawing function */
void
ui_drawmessage (char *str)
{
  int should_delete, newmessage;
  static char *lastmessage;

  /* No string provided... */
  if ( ! str )
  {
    /* Nothing to do */
    if ( ! lastmessage || ! *lastmessage ) return;

    /* Using last message */
    should_delete = 0;
    newmessage = 0;
    str = lastmessage;
  }

  else /* string provided */
  {
    if ( lastmessage )
    {

      if ( strncmp(lastmessage, str, strlen(lastmessage)) )
        should_delete = 1;

      else should_delete = 0;

      if ( strcmp(lastmessage, str) ) newmessage = 1;
      else newmessage = 0;

      if ( strlen(str) > strlen(lastmessage) )
      {
        lastmessage = safe_realloc(lastmessage, strlen(str)+1,
          "ui_drawmessage: lastmessage");
      }
    }
    else
    {
      lastmessage = safe_malloc(strlen(str)+1, "ui_drawmessage: lastmessage");
      newmessage = 1;
      should_delete = 0;
    }
    strcpy(lastmessage, str);
  }


  /* no font loaded ... */
  if ( font == 0 )
  {
    if ( newmessage ) fprintf(stderr, "%s\n", str);
    return;
  }

  /* Clear previously drawn string if needed */
  if ( should_delete )
  {

    redraw_arenarect(msgbox.bx, msgbox.by,
      msgbox.bx+msgbox.bw, msgbox.by+msgbox.bh);

  }

#ifdef USE_ARENAWIN_PIXMAP
  XDrawString(dpy, arenawin_pixmap, messagegc, msgbox.fx,
    msgbox.fy, str, strlen(str));
  UpdateScreen(&arena);
  //XCopyArea(dpy, arenawin_pixmap, arenawin, arenagc, 0, 0, xdim, ydim, 0, 0);
#else
  XDrawString(dpy, arenawin, messagegc, msgbox.fx,
    msgbox.fy, str, strlen(str));
#endif

  /* Update message region */
  /* .... */
  
  return;
}


/*
 * TODO: support camera control when using pixmap
 */
void
ui_drawarena (ARENA *arena)
{
  int y, x;
  sprite *sp;
  int pictidx;
  short should_redraw = 0;
  short should_redraw_message = 0;
  short should_redraw_sprite;
  XEvent event;
  static int x0=0;
  static int y0=0;
#ifdef USE_ARENAWIN_PIXMAP 
  short need_update = 0;
#else
  Region expreg;
  XRectangle rectbuf;

  expreg = XCreateRegion();
#endif

  if ( x0 != arena->x0 || y0 != arena->y0 )
  {
    x0 = arena->x0;
    y0 = arena->y0;
#ifndef USE_ARENAWIN_PIXMAP
    should_redraw = 1;
#else
    need_update = 1;
#endif
  }

#ifndef USE_ROOT_WINDOW
  while ( XCheckWindowEvent(dpy, arenawin, StructureNotifyMask|ExposureMask, &event) )
#else
  while ( XCheckWindowEvent(dpy, arenawin, ExposureMask, &event) )
#endif
  {
    switch(event.type)
    {

#ifndef USE_ROOT_WINDOW
      case (ConfigureNotify):

#ifdef DEBUG_X_EVENTS
        fprintf(stderr, "ui_X.c: Event ConfigureNotify (width=%i, height=%i)\n",
          event.xconfigure.width, event.xconfigure.height);
#endif
        if (event.xconfigure.width != oldxdim || event.xconfigure.height != oldydim)
        {
          oldxdim = event.xconfigure.width;
          oldydim = event.xconfigure.height;
#ifndef USE_ARENAWIN_PIXMAP
          should_redraw = 1;
#else
          need_update = 1;
#endif
        }
        break;
#endif

      case (Expose):
#ifdef DEBUG_X_EVENTS
        fprintf(stderr, "ui_X.c: Event Expose (x=%i, y=%i, w=%i, h=%i)\n", event.xexpose.x,
          event.xexpose.y, event.xexpose.width, event.xexpose.height);
#endif

#ifndef USE_ARENAWIN_PIXMAP
        /*
         * Instead of redrawing on each expose event,
         * compute a catch-all-exposed region to use at
         * the end of the switch ...
         */
        rectbuf.x = event.xexpose.x;
        rectbuf.y = event.xexpose.y;
        rectbuf.width = event.xexpose.width;
        rectbuf.height = event.xexpose.height;
        XUnionRectWithRegion(&rectbuf, expreg, expreg);
#else
        need_update = 1;
#endif

        break;
        
      case UnmapNotify:
#ifdef DEBUG_X_EVENTS
        fprintf(stderr, "ui_X.c: Event UnmapNotify\n");
#endif
        pause_game();
        break;

      case MapNotify:
#ifdef DEBUG_X_EVENTS
        fprintf(stderr, "ui_X.c: Event MapNotify\n");
#endif
        /* resume_game(); */
        break;

      default:
#ifdef DEBUG_X_EVENTS
        fprintf(stderr, "ui_X.c: Event %i\n", event.type);
#endif
        break;
    }
  }

  /* Send pictset to the display if needed 
   * check it here to allow `loadpictset' command
   * before screen initialization.
   * We need this check also to know wheter to redraw
   * the whole arena ...
   */
  if ( ! pictsetsent )
  {
    send_pictset(); 
#ifdef REDRAW_ARENA_ON_PICTSET_CHANGE
    should_redraw = 1;
#endif
  }

  for (y = 0; y < arena->lines; y++)
  {
    for (x = 0; x < arena->cols; x++)
    {

      should_redraw_sprite = 0;
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

      /* All sprites need redrawing */
      if ( should_redraw )
      {
        should_redraw_sprite = 1;
        should_redraw_message = 1;
      }

      /* Sprite changes from last drawing */
      if ( *(arenamap+y*arena->cols+x) != pictidx )
      {
        should_redraw_sprite = 1;
        *(arenamap+y*arena->cols+x) = pictidx;
      }

#ifndef USE_ARENAWIN_PIXMAP
      /* Sprite overlaps exposed region */
      if ( XRectInRegion(expreg, x*spritewidth, y*spriteheight,
            spritewidth, spriteheight) )
      {
        should_redraw_sprite = 1;
      }
#endif

      if ( should_redraw_sprite ) 
      {
#ifdef USE_ARENAWIN_PIXMAP 
        need_update = 1;
        copy_pict(x, y, pictidx);
#else
        copy_pict((arena->cols+(x-x0))%arena->cols, (arena->lines+(y-y0))%arena->lines, pictidx);
#endif

        if ( SPRITEINMESSAGEBOX(x,y) )
        {
          should_redraw_message = 1;
        }
      }

    }
  }

#ifndef USE_ARENAWIN_PIXMAP
  XDestroyRegion(expreg);
#endif

  if ( should_redraw_message ) 
  {
    ui_drawmessage(NULL);
#ifdef USE_ARENAWIN_PIXMAP 
    need_update = 2;
#endif
  }

#ifdef USE_ARENAWIN_PIXMAP
  if ( need_update )
  {
    UpdateScreen(arena);
/* fprintf(stderr, "Need update = %i\n", need_update); */
  }
#endif

  XSync(dpy, False);
  XFlush(dpy);

}


void
ui_finish ()
{
  if ( screen_initialized )
  {
#ifdef USE_ARENAWIN_PIXMAP
    XFreePixmap(dpy, arenawin_pixmap);
#endif
    XFreePixmap(dpy, pictset_pixmap);
    XDestroyWindow(dpy, arenawin);
    XCloseDisplay(dpy);
  }
}


/*
 * Get next key in input buffer
 *
 * OK. We should inspect keyboard status here.
 * It's not that easy. The calling function expect ONE and only ONE char
 * to be returned at a time. It's used to ``consume'' the input buffer
 * before proceding. We need to feed it with all the key pressed up to
 * now I guess... How ?!
 *
 */
int
ui_getkey ()
{
  int retkey = -1;

#ifndef USE_ROOT_WINDOW
  int key = -1;
  XEvent event;

  while ( XCheckWindowEvent(dpy, arenawin, KeyPressMask|KeyReleaseMask, &event) )
  {

#ifdef DEBUG_X_EVENTS
    switch (event.type)
    {
      case KeyPress:
        fprintf(stderr, "ui_X.c: Event KeyPress (keycode=%i)\n", event.xkey.keycode);
        break;
      case KeyRelease:
        fprintf(stderr, "ui_X.c: Event KeyRelease (keycode=%i)\n", event.xkey.keycode);
        break;
      default:
        fprintf(stderr, "ui_X.c: Event %i\n", event.type);
        break;
    }
#endif

    key = XKeycodeToKeysym(dpy, event.xkey.keycode, event.xkey.state);

#if 0 // DEBUG KEYSYMS ?
    fprintf(stderr, "ui_getkey: keysym %i / keycode %i\n",
		    key, event.xkey.keycode);
#endif

    if ( event.type == KeyPress )
    {

      /* Camera movements */
      if ( key == XK_Up ) arena.y0--;
      else if ( key == XK_Down ) arena.y0++;
      else if ( key == XK_Right ) arena.x0++;
      else if ( key == XK_Left ) arena.x0--;

      /* ENTER KEY */
      if ( key == 65293 ) retkey = '\n';
      else if ( key == 65288 ) retkey = 127;

      /* I found this to be working for HIGH-valued keysyms 
       * My nice game segfaults if this function returns numbers
       * higher then 255 (who knows why ? -- me, a char type is expected)
       */
      /*else if ( key > 256 ) retkey = key % 256; */
      else if ( key > 256 ) retkey = -1;
      else retkey = key;

      break;
    }

  }
#endif

  return retkey;
}

/*
 * prompt the user for a string
 * the line returnered is allocated with malloc(3)
 * so the caller must free it when finished with it. 
 */
char *
ui_prompt (char *prompt_string)
{
  int key;
  int len = 0;
  char *cmdline;

  cmdline = safe_malloc (MAXCMDLINELEN, "prompt input buffer");
  memset(cmdline, '\0', MAXCMDLINELEN);

  message("%s: %s_", prompt_string, cmdline);
  ui_drawarena(&arena);

  while (1)
  {

    ui_drawarena(&arena);

    key=ui_getkey();
    if ( key == -1 ) continue;

    /* max command line lenght reached */
    if ( len > MAXCMDLINELEN ) break;

    if ( key == '\n' ) break;

    /* handle kill-line */
    //fprintf(stderr, "key is %d\n", key);

    /* handle backspace */
    if ( key == 127 )
    {
        if (len) cmdline[--len] = '\0';
    }

    else cmdline[len++] = key;
    message("%s: %s_", prompt_string, cmdline);

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
    pictsetsent = 0;
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

  pictsetsent = 0;

  message("pictset %s loaded", fname);
  return 1;
}

int
CMD_fontcolor (char *colordef)
{
  if ( ! colordef ) colordef = DEFAULTFONTCOLOR;

  if ( screen_initialized )
  {
    return font_color (colordef);
  }

  strncpy(fontcolordef, colordef, MAXFONTCOLORDEFLEN);
  fontcolordef[MAXFONTCOLORDEFLEN-1] = '\0';

  return 0;

}

int
CMD_loadfont (char *pattern)
{

  if ( ! pattern )
  {
    strncpy(fontdef, DEFAULTFONTDEF, MAXFONTDEFLEN);
    fontdef[MAXFONTDEFLEN-1] = '\0';
  }
  else
  {
    strncpy(fontdef, pattern, MAXFONTDEFLEN);
    fontdef[MAXFONTDEFLEN-1] = '\0';
  }

  if ( screen_initialized )
  {
    return load_font(fontdef);
  }

  return 0;
}


/************ PRIVATE FUNCTIONS **********************/


#ifdef USE_BACKINGSTORE
static void
enable_backingstore (Display *d, Window w)
{
  XSetWindowAttributes wa;
  wa.backing_store = Always;
  XChangeWindowAttributes(d, w, CWBackingStore, &wa);
}
#endif

#ifndef USE_ROOT_WINDOW
static Window
create_arenawin (Display *dpy, int width, int height)
{
  Window w;
  Cursor c;
  Pixmap cp;
  XColor color;
#ifdef USE_CLASS_HINTS
  XClassHint classhints;
#endif
#ifdef USE_SIZE_HINTS
  XSizeHints sizehints;
#endif

  w = XCreateSimpleWindow(dpy, DefaultRootWindow(dpy), 0, 0,
    width, height, 1, WhitePixelOfScreen(scr), None);

#ifdef USE_SIZE_HINTS
  /* Set Size Hints */
  sizehints.flags=PSize|PMinSize|PMaxSize;
  sizehints.min_width=sizehints.max_width=sizehints.base_width=width;
  sizehints.min_height=sizehints.max_height=sizehints.base_height=height;
  XSetWMNormalHints(dpy,w,&sizehints);
#endif

#ifdef USE_CLASS_HINTS
  /* Set class hints */
  classhints.res_name = "X-sins";
  classhints.res_class = "Games";
  XSetClassHint(dpy,w,&classhints);
#endif

  /* Create window's cursor */
  cp = XCreatePixmap(dpy, w, 1, 1, 1);
  c = XCreatePixmapCursor (dpy, cp, cp, &color, &color, 0, 0);
  XFreePixmap(dpy, cp);
  XDefineCursor(dpy, w, c);

  /* Give window a name */
  XStoreName (dpy, w, "X-Sins");

  /* We want to get MapNotify events */
  /* XSelectInput(dpy, w, KeyPressMask|FocusChangeMask|StructureNotifyMask); */
  XSelectInput(dpy, w, KeyPressMask|KeyReleaseMask|StructureNotifyMask|ExposureMask);

  /* "Map" the window (that is, make it appear on the screen) */
  XMapWindow(dpy, w);

  for(;;)
  {
    XEvent e;
    XNextEvent(dpy, &e);
    if (e.type == MapNotify) break;
  }

  /* I don't want an ExposeEvent now ! */
  XSync(dpy, True);


  return w;
}
#endif

#ifdef USE_ARENAWIN_PIXMAP
static void 
create_arenawin_pixmap (void)
{

  arenawin_pixmap = XCreatePixmap (dpy, DefaultRootWindow(dpy),
    xdim, ydim, DefaultDepthOfScreen(scr));

  message("Created arenawin pixmap %i", arenawin_pixmap);

}
#endif

static void 
create_pictset_pixmap (void)
{

#ifdef DEBUG_X_PROTOCOL
  fputs("Creating pictset pixmap", stderr);
#endif

  pictset_pixmap = XCreatePixmap (dpy, DefaultRootWindow(dpy), spritewidth,
    spriteheight*PICTSETLEN, DefaultDepthOfScreen(scr));

#ifdef DEBUG_X_PROTOCOL
  fputs(" done\n", stderr);
#endif

}

static void
init_background(ARENA *arena)
{
  
#ifdef USE_ARENAWIN_PIXMAP
  XFillRectangle(dpy, arenawin_pixmap, arenagc, 0, 0, xdim, ydim);
#else
  XFillRectangle(dpy, arenawin, arenagc, 0, 0, xdim, ydim);
#endif
  fprintf(stderr, "Background initialized\n");

}

/*
 * Copy a picture from the pictset pixmap to the specified
 * location on the arenawin
 */
static int
copy_pict (int x, int y, int pictnum)
{
  int sx, sy, dx, dy;

  sx = 0;
  sy = pictnum*spriteheight;
  dx=x*spritewidth;
  dy=y*spriteheight;

  /*
   * This generates NoExpose and GraphicsExpose event 
   * unless the GC has the graphics_exposures set to False
   */
#ifdef USE_ARENAWIN_PIXMAP
  XCopyArea(dpy, pictset_pixmap, arenawin_pixmap, arenagc,
    sx, sy, spritewidth, spriteheight, dx, dy);
#else
  XCopyArea(dpy, pictset_pixmap, arenawin, arenagc,
    sx, sy, spritewidth, spriteheight, dx, dy);
#endif

  return 1;
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

static int
send_pictset()
{
  Pixmap bgpix;
  int n;
  int color;
  XPoint points[16][spritewidth*spriteheight*PICTSETLEN]; /* Too large! */
  int numpoints[16];
  int dx, dy; /* destination coordinates */

  memset(numpoints, 0, sizeof(int)*16);
  for (n=0; n<PICTSETLEN*spritewidth*spriteheight; n++)
  {
    color = pictset[n];
    dx = n%spritewidth;
    dy = n/spritewidth;
    points[color][numpoints[color]].x = dx;
    points[color][numpoints[color]++].y = dy;
  }

#ifdef DEBUG_X_PROTOCOL
  fputs("Sending pictset", stderr);
#endif

  for (color=0; color<16; color++)
  {
    if ( numpoints[color] )
    {
      XSetForeground( dpy, arenagc, coloridx[color] );
      XDrawPoints( dpy, pictset_pixmap, arenagc, points[color],
        numpoints[color], CoordModeOrigin);
    }
  }

#ifdef DEBUG_X_PROTOCOL
  fputs("  done!\n", stderr);
#endif


#ifdef DEBUG_X_PROTOCOL
  fputs("Updating arenagc tile", stderr);
#endif

  /* Update arenagc tile */
  bgpix = XCreatePixmap (dpy, DefaultRootWindow(dpy), spritewidth,
    spriteheight, DefaultDepthOfScreen(scr));
  XCopyArea(dpy, pictset_pixmap, bgpix, arenagc,
    0, 0, spritewidth, spriteheight, 0, 0);
  XSetFillStyle(dpy, arenagc, FillTiled);
  XSetTile(dpy, arenagc, bgpix);
  XFreePixmap(dpy, bgpix);

#ifdef DEBUG_X_PROTOCOL
  fputs(" done!\n", stderr);
#endif

  pictsetsent = 1;

  return 1;
}

static void
redraw_arenarect(int rx, int ry, int rw, int rh)
{
  int ax, ay, aw, ah;
  int y, x;
  int pictidx;

  ax = rx/spritewidth;
  ay = ry/spriteheight;
  aw = rw/spritewidth;
  if ( rw%spritewidth ) aw++;
  ah = rh/spriteheight;
  if ( rh%spriteheight ) ah++;

  for (y = ay; y-ay <= ah && y < ARENA_HEIGHT; y++)
  {
    for (x = ax; x-ax <= aw && x < ARENA_WIDTH; x++)
    {
      pictidx = *(arenamap+y*arena.cols+x);
      copy_pict(x, y, pictidx);
    }
  }

}


static void
create_graphic_contexts()
{
  XGCValues gcval1;

  gcval1.graphics_exposures = False;


  /**************
   * Arena   GC *
   **************/

  /*
   * Create the Graphic Context
   * Set the graphics-exposures to False to avoid
   * NoExpose and GraphicsExpose Event on XCopyArea requests
   *
   * Anyway... NoExpose events suggest there are some problems with XCopyArea
   * ( unreachable source coordinates ? )
   */ 
  arenagc = XCreateGC(dpy, arenawin, GCGraphicsExposures, &gcval1);

  /* Set line width to 0 (an X faq states it might speed up operations ) */
  XSetLineAttributes(dpy, arenagc, 0, LineSolid, CapRound, JoinBevel);


  /**************
   * Message GC *
   **************/

  /* Create the graphic context for the messages */
  messagegc = XCreateGC(dpy, arenawin, GCGraphicsExposures, &gcval1);
  /* XSetPlaneMask(dpy, messagegc, 1<<2); */

}

static void
init_colors()
{
  Colormap cm;
  int i;
  XColor color;
  char *colornames[16] = {
    "rgb:0000/0000/0000", /* black */
    "rgb:0000/0000/AAAA", /* blue */
    "rgb:0000/8888/0000", /* green */
    "rgb:0000/8888/8888", /* cyan */
    "rgb:8888/0000/0000", /* red */
    "rgb:8888/8888/0000", /* magenta */
    "rgb:8888/0000/8888", /* brown */
    "rgb:8888/8888/8888", /* grey */

    "rgb:4444/4444/4444", /* light black */
    "rgb:0000/0000/FFFF", /* light blue */
    "rgb:0000/FFFF/0000", /* light green */
    "rgb:0000/FFFF/FFFF", /* light cyan */
    "rgb:FFFF/0000/0000", /* light red */
    "rgb:FFFF/FFFF/0000", /* light magenta */
    "rgb:FFFF/0000/FFFF", /* light brown */
    "rgb:FFFF/FFFF/FFFF", /* light grey */
  };

  cm = DefaultColormap(dpy, DefaultScreen(dpy));
  /* message("Default colormap = %i", cm); */

  for (i=0; i<16; i++)
  {
    if ( ! XParseColor(dpy, cm, colornames[i], &color) )
    {
      message("color %s not found", colornames[i]);
    }
    XAllocColor(dpy, cm, &color);
    coloridx[i] = color.pixel;
  }

}


/*
 * Load font
 */
static int
load_font (char *pattern)
{
  XFontStruct *fontinfo;
  char **fontnames;
  int foundfonts = 0;

  if ( ! pattern ) pattern = DEFAULTFONTDEF;

  if ( ! (fontnames=XListFontsWithInfo(dpy, pattern, 1, &foundfonts, &fontinfo)) || ! foundfonts )
  {
    message("No fonts matching pattern %s", pattern);
    return 0;
  }

  /* Clean the message area before messing with another font */
  ui_drawmessage("");

  /* Unload old font if any */
  if ( font != -1 ) XUnloadFont(dpy, font);

  font = XLoadFont(dpy, fontnames[0]);

  /* Update message box size */
  msgbox.bx = spritewidth;
  msgbox.by = ((ARENA_HEIGHT-1)*spriteheight)
    - (fontinfo->max_bounds.descent) - (fontinfo->max_bounds.ascent);
  msgbox.bw = (ARENA_WIDTH-2)*spritewidth;
  msgbox.bh = (fontinfo->max_bounds.descent) + (fontinfo->max_bounds.ascent);
  msgbox.fx = msgbox.bx-(fontinfo->min_bounds.lbearing);
  msgbox.fy = msgbox.by+(fontinfo->max_bounds.ascent);


  XSetFont(dpy, messagegc, font);

  message("Loaded font %s", fontnames[0]);

  XFreeFontInfo (fontnames, fontinfo, foundfonts);

  return 1;
}

/*
 * Font color
 */
static int
font_color (char *def)
{
  XColor color;
  Colormap cm;

  cm = DefaultColormap(dpy, DefaultScreen(dpy));
  if ( ! XParseColor(dpy, cm, def, &color) )
  {
      message("color %s not found", def);
      return 0;
  }
  XAllocColor(dpy, cm, &color);
  fontcolor = color.pixel;
  XSetForeground(dpy, messagegc, fontcolor);
  message("Font color changed to %s", def);
  return 1;
}

#ifdef DEBUG_X_PROTOCOL
static int afterfunc (Display *dpy)
{
  fputc('.', stderr);
  /* fprintf(stderr, "."); */
  /*
  fprintf(stderr, "last_request: %ld\n", XLastKnownRequestProcessed(dpy));
  */
  return 1;
}
#endif

#ifdef USE_ARENAWIN_PIXMAP
static void
UpdateScreen(ARENA *arena)
{
	int x0 = 10;
	int y0 = 10;

#if 0
	XCopyArea(dpy, arenawin_pixmap, arenawin, arenagc,
		0, 0,
		xdim, ydim,
		0, 0);

#else
	XCopyArea(dpy, arenawin_pixmap, arenawin, arenagc,
		x0*spritewidth, y0*spriteheight,
		xdim-spritewidth*x0,
		ydim-spriteheight*y0,
		0, 0);

	XCopyArea(dpy, arenawin_pixmap, arenawin, arenagc,
		0, y0*spriteheight,
		spritewidth*x0,
		ydim-spriteheight*y0,
		x0*spritewidth, 0);

	XCopyArea(dpy, arenawin_pixmap, arenawin, arenagc,
		x0*spritewidth, 0,
		xdim-spritewidth*x0,
		spriteheight*y0,
		0, y0*spriteheight);

	XCopyArea(dpy, arenawin_pixmap, arenawin, arenagc,
		0, 0,
		spritewidth*x0,
		spriteheight*y0,
		x0*spritewidth, y0*spriteheight);
#endif
}
#endif
