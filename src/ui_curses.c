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
 * file: ui_curses.c *  purpose: handle curses graphics 
 */

#include "sins.h"
#include <curses.h>
#include <ctype.h>

#ifdef HAVE_READLINE_H
#include <readline.h>
#else
#ifdef HAVE_READLINE_READLINE_H
#include <readline/readline.h>
#endif
#endif

#define ARENA_X_OFFSET 1
#define ARENA_Y_OFFSET 1

/*
 * public functions 
 */
int ui_init (void);
int ui_initarena (ARENA *);
void ui_finish (void);
void ui_drawarena (ARENA *);
void ui_drawmessage (char *);
char *ui_prompt (char *);
int ui_getkey (void);
int CMD_colors (char *line);

/*
 * private functions 
 */
static int init_screen (void);
static void make_arena (ARENA *);
static void make_promptwin (void);
static void draw_motion (void);
static void init_color_pairs (void);

/* private data */
static WINDOW *arenawin;
static WINDOW *promptwin;
static chtype *arenamap; /* picture indexes */
static int use_colors = 0;
static int term_has_colors = 0;
static int term_can_change_color = 0;

/* public data */
int screen_initialized = 0;

/* Initialize UI module */
int
ui_init ()
{

  if (!register_command ("colors", &CMD_colors,
			 "colors [ ON | OFF ]")) return 0;

  return 1;
}

/*
 * initialize the graphics 
 */
int
ui_initarena (ARENA *arena)
{

  /*
   * init screen 
   */
  if (!init_screen ())
    {
      fputs ("can't initialize graphics\n", stderr);
      endwin();
      exit (1);
    }

  /*
   * make arena window 
   */
  make_arena (arena);

  /*
   * make prompt window
   */
  make_promptwin ();

  /* Set screen initialized flag */
  screen_initialized = 1;

  return 1;

}

/*
 * curses version of drawing function 
 */
void
ui_drawmessage (char *str)
{
  int y, x;

  mvwaddstr (arenawin, ARENA_Y_OFFSET+ARENA_HEIGHT, 5, "* ");
  waddnstr (arenawin, str, ARENA_WIDTH-10);
  waddstr (arenawin, " *");
  do
  {
    getyx (arenawin, y, x);
    waddch (arenawin, '-');
  }
  while (x < ARENA_WIDTH);

  touchwin (arenawin);
  wrefresh (arenawin);
}

/*
 * curses version of arena drawing 
 */
void
ui_drawarena (ARENA *arena)
{
	int y, x, val;
	sprite *sp;
	chtype look;
	int should_redraw = 0;
	static int x0 = 0;
	static int y0 = 0;

	if ( x0 != arena->x0 || y0 != arena->y0 )
	{
		x0 = arena->x0;
		y0 = arena->y0;
		should_redraw = 1;
	}

	for (y = 0; y < arena->lines; y++)
	{
		for (x = 0; x < arena->cols; x++)
		{
			int drawx, drawy;

			sp = GETSPRITE(arena,y,x);

			if ( !sp ) look = ' ';
			else
			{
				switch (sp->type)
				{
					case SNAKE:
						val = ((snakebody *) (sp))->self->playernum;
						look = (val+48) | A_REVERSE;
						if ( use_colors ) look |= COLOR_PAIR(val);
						break;

					case EXSNAKE:
						look = '.';
						break;

					case FRUIT:
						look = ((fruit *) (sp))->value;
						val = FRUITVAL ((fruit *) sp);
						if ( use_colors ) look |= COLOR_PAIR ( (10+val) );
						break;

					/* This is for future implementations */
					case WALL:
						look = '#';
						break;

					default:
						look = '?';
						break;
				}
			}

			if ( !should_redraw && *(arenamap+y*arena->cols+x) == look )
				continue;
			*(arenamap+y*arena->cols+x) = look;

			drawx=(arena->cols+(x-x0))%arena->cols;
			drawy=(arena->lines+(y-y0))%arena->lines;
			drawy+=ARENA_Y_OFFSET;
			drawx+=ARENA_X_OFFSET;
			if ( ERR == mvwaddch (arenawin, drawy, drawx, look) )
			{
				message("mvwaddch returned ERR");
			}

		} /* x loop */
	} /* y loop */

	draw_motion();

	touchwin (arenawin); 
	wrefresh (arenawin);
}

/*
 * activity widget 
 */
static void
draw_motion (void)
{
  static char sprite[] = { '-', '/', '|', '\\' };
  static int spriteidx = 0;

  if (++spriteidx == 4)
    spriteidx = 0;
  mvwaddch (arenawin, 0, 0, sprite[spriteidx]);
}


/************ PRIVATE FUNCTIONS **********************/

/*
 * initialize the screen  
 */
static int
init_screen ()
{
  /*
   * initialize the COLS and LINES vars 
   */
  if ( initscr () == NULL )
    return 0;

  /*
   * make cursor invisible 
   */
  if (curs_set (0) == ERR)
    return 0;

  /*
   * don't echo input characthers 
   */
  if (noecho () == ERR)
    return 0;

  /*
   * make each carachter break input reads 
   */
  if (cbreak () == ERR)
    return 0;

  /*
   * make getch() be a non-blocking call  
   */
  if (nodelay (stdscr, TRUE) == ERR)
    return 0;

  /*
   * redraw completely on refresh 
   */
  clearok (stdscr, TRUE);


  /*
   * check color capabilities
   */
  if ( has_colors() )
  {

    if ( ERR == start_color() )
    {
      message("Can't start colors");
    }

    else
    {
      term_has_colors = 1;
      use_colors = 1;
      init_color_pairs();

      if ( can_change_color() )
      {
        term_can_change_color = 1;
        message("The terminal supports colors redefinition");
      }
      else
      {
        message("The terminal supports colors");
      }
    }
  }
  else
  {
      message("The terminal does not support colors");
  }

  /*
   * Enable keypad
   */
  keypad(stdscr, TRUE);

  /*
   * success 
   */
  return 1;
}

/*
 * make prompt window
 */
static void
make_promptwin ()
{
  promptwin = newwin (1, ARENA_WIDTH+2, ARENA_HEIGHT+1, 0);
}

/*
 * make arena window 
 */
static void
make_arena (ARENA *arena)
{

  /*
   * Set arena dimensions if not already set
   */
  if ( ! arena->lines ) arena->lines = LINES-2;
  if ( ! arena->cols ) arena->cols = COLS-2;

  /*
   * Check arena dimensions fitness
   */
  if ( arena->lines > LINES-2 || arena->cols > COLS-2 )
  {
    message("Arena size (%i,%i) out of screen boundary (%i,%i)",
            arena->lines, arena->cols, LINES-2, COLS-2);
    finish();
  }

  /* initialize the arena maps (used for optimization purposes) */
  arenamap = (chtype *)safe_malloc( arena->lines*arena->cols*sizeof(chtype),
    "ui_curses: ui_initarena: arenamap");
  memset(arenamap, -1, arena->lines * arena->cols);

  /*
   * Actually create the arena window
   */
  arenawin = newwin (arena->lines+2, arena->cols+2, 0, 0);
  wborder (arenawin, '|', '|', '-', '-', '*', '*', '*', '*');

  /*
   * prevent screen from scrolling
   */
  scrollok(arenawin, FALSE);
}

/*
 * prompt the user for a string
 * the line returnered is allocated with malloc(3)
 * so the caller must free it when finished with it. 
 */
char *
ui_prompt (char *prompt_string)
{
  static char *cmd;
  static char prompt_prefix[] = "*--(";
  static char prompt_suffix[] = "): ";
  char ps[32]; /* prompt string */

  snprintf (ps, 32, "%s%s%s", prompt_prefix, prompt_string, prompt_suffix);

  curs_set (1);			/* show cursor */

#ifdef WITH_READLINE

  /* show prompt window */
  touchwin (promptwin);
  wrefresh (promptwin);

  reset_shell_mode ();
  cmd = readline (ps);
  reset_prog_mode ();

#else
  cmd = safe_malloc (MAXCMDLINELEN, "prompt input buffer");

  echo ();
  werase (promptwin);
  mvwaddstr (promptwin, 0, COLS - 5, "*---*");
  mvwaddstr (promptwin, 0, 0, ps);
  touchwin (promptwin);
  wrefresh (promptwin);
  wgetnstr (promptwin, cmd, COLS - strlen (ps) - 6);
  noecho ();
#endif

  curs_set (0);			/* hide cursor */

  /* standard screen is probabily scrolled */
  touchwin (stdscr);

  return cmd;
}

void
ui_finish ()
{
  endwin();
}


/*
 * Get next key in input buffer
 */
int
ui_getkey ()
{
  int key;
  key = getch();
  if ( key == KEY_DOWN )
  	arena.y0 = (arena.y0+1)%arena.lines;
  else if ( key == KEY_UP )
  	arena.y0 = (arena.lines+arena.y0-1)%arena.lines;
  else if ( key == KEY_RIGHT )
  	arena.x0 = (arena.x0+1)%arena.cols;
  else if ( key == KEY_LEFT )
  	arena.x0 = (arena.cols+arena.x0-1)%arena.cols;
  return key;
}

static void
init_color_pairs()
{
  int i, b, f;

  for (i=0; i<20; i++)
  {
    b = COLOR_BLACK;
    f = i%8; if ( ! f ) f = 1;
    if ( i/8 ) f |= A_BOLD;
    init_pair(i, f, b);
  }
}

int
CMD_colors (char *line)
{
  char ch;

  if ( ! line )
  {
    if ( use_colors ) message("colors ON");
    else message("colors OFF");
    return 1;
  }

  if ( strlen(line) < 2 ) return -1;
  
  ch = toupper(line[0]);
  if ( ch != 'O' ) return -1;

  ch = toupper(line[1]);
  if ( ch == 'N' )
  {
    if ( term_has_colors )
    {
      message("colors ON");
      use_colors = 1;
    }
    else
    {
      message("Terminal does not support colors");
    }
  }
  else
  {
    use_colors = 0;
    message("colors OFF");
  }
  
  return 1;
}


