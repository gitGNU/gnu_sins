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
 * file: main.c *  purpose: application entry point
 *
 ****************************************************************************
 *
 * $Log: main.c,v $
 * Revision 1.2  2003/12/19 19:00:58  strk
 * various small changes
 *
 ****************************************************************************/

#include "sins.h"

#include <stdio.h>
#include <time.h>		/* time (for srand) */
#include <sys/time.h>		/* itimer ...   */
#include <signal.h>		/* signal */
#include <setjmp.h>		/* setjmp, longjmp */

/*
 * extern functions 
 */
extern int ui_init (void);
extern int ui_initarena (ARENA *);
extern void ui_finish (void);
extern void ui_drawarena (ARENA *);

extern void list_commands (void);
extern void move_snakes (void);
extern void init_fruit (void);
extern int parse_input (void);
extern void init_modules (void);
extern int parse_cmdline_opts (int, char **);
extern void parse_cmdline_args (int, char **);
extern int source_rcfile ();
extern int CMD_delay (char *);
extern int CMD_mfprob (char *);
extern int CMD_kill (char *);
extern int CMD_destroy (char *);
extern int CMD_reset (char *);
extern int CMD_trim (char *);
extern int CMD_help (char *);
extern int CMD_source (char *);
extern int CMD_path (char *);
extern void expire_messages (void);
extern void rewindtypein (void);
extern void resettypein (void);

#ifndef DISABLE_CMDFIFO
extern int CMD_cmdfifo (char *);
extern void read_cmdfifo (void);
extern FILE *cmdfifo;
#endif

/*
 * Private functions 
 */
static void snake_play (void);
static void sigint (int);
static void sigsegv (int);
static int CMD_quit (char *);
#ifndef DISABLE_TIMEOUT
static void sigvtalrm (int);
#define PLAYERTIMEOUT 1
#endif
static void game_loop (void);


/*
 * Public function 
 */
int register_player (snake * s);
void finish (void);
void pause_game (void);
void resume_game (void);

/*
 * Public data 
 */
snake *zoo[MAXPLAYERS];		    /* snakes pointer */
int want_commandlist=0;       /* user requested only the commandlist */
int numplayers = 0;		        /* number of players */
long delay = DEFAULTDELAY;	  /* game delay in microseconds */
unsigned long int turn = 0;	/* game turn */
ARENA arena = { 0, 0, 0, 0 };
short game_paused = 0;

/*
 * Private data
 */
#ifndef DISABLE_TIMEOUT
static jmp_buf playloop;
#endif

int
main (int argc, char **argv)
{
  int optind;
  int i;

  /* Write zeros to zoo structure */
  memset(zoo, 0, sizeof(void *)); 

  /* Register core commands */
  register_command ("help", CMD_help, "help <command>");
  register_command ("source", CMD_source, "source <filename>");
  register_command ("delay", CMD_delay, "delay [<msec>]");
#ifndef DISABLE_CMDFIFO
  register_command ("cmdfifo", CMD_cmdfifo, "cmdfifo <path>");
#endif
  register_command ("kill", CMD_kill, "kill <snakenum>");
  register_command ("destroy", CMD_destroy, "destroy <snakenum>");
  register_command ("reset", CMD_reset, "reset <snakenum>");
  register_command ("trim", CMD_trim, "trim <snakenum>");
  register_command ("quit", CMD_quit, "quit");
  register_command ("mfprob", CMD_mfprob, "mfprob <percent>");
  register_command ("path", CMD_path, "path [<string>]");

  /* Parse command line options (-switches) */
  optind = parse_cmdline_opts (argc, argv);

  /*
   * initialize the modules before parsing command line args and config file
   * ( they may introduce new commands )
   */
  if ( !ui_init () ) finish();
  init_modules ();

  /*
   * Just the list of command
   * This has to be done after initialization of UI and other modules
   * otherwise we'd miss commands registered by them!
   */
  if ( want_commandlist )
  {
    list_commands();
    exit(0);
  }

  /* Parse configuration file */
  source_rcfile ();

  /*
   * parse command line arguments
   * parse it after configuration file
   * to override values specified there
   */
  parse_cmdline_args (argc - optind, argv + optind);

  /* Initialize graphics, arena and fruit */
  if ( !ui_initarena (&arena) ) finish();
  arenainit ();
  init_fruit ();

  /*
   * Set signal handlers
   */
  signal (SIGINT, sigint);
  signal (SIGSEGV, sigsegv);
#ifndef DISABLE_TIMEOUT
  signal (SIGVTALRM, sigvtalrm);	/* players timeout */
#endif

  /* initialize the random seed */
  srand (time (NULL));

  /* draw the arena once before starting the loop */
  ui_drawarena (&arena);

  /* initialize all snakes */
  for (i = 0; i<MAXPLAYERS; i++)
  {
    /* empty slot */
    if ( zoo[i] == NULL ) continue;

    snake_init(zoo[i]);
  }
  
  game_loop();

  finish ();
  exit (0);			/* finish will exit itself */
}

static void
game_loop ()
{
  int waiting = 0;
  struct timeval last, now, diff;

  /* initialize timing */
  if ( gettimeofday(&last, NULL) )
  {
      perror("gettimeofday");
      finish();
  }

  /* MAIN LOOP  */
  for (;;)
  {

    if ( gettimeofday(&now, NULL) )
    {
      perror("gettimeofday");
      finish();
    }
    diff.tv_sec = now.tv_sec - last.tv_sec;
    diff.tv_usec = now.tv_usec - last.tv_usec;
    /* if ( (now.tv_sec-last.tv_sec)*1000000 + (now.tv_usec-last.tv_usec) */
    if ( diff.tv_sec*1000000 + diff.tv_usec > delay )
    {
      waiting = 0;
      memcpy(&last, &now, sizeof(struct timeval));
    }
    else
    {
      waiting++;
    }

    /* parse keyboard input */
    parse_input ();

#ifndef DISABLE_CMDFIFO
    /* Read from command fifo */
    if (cmdfifo) read_cmdfifo ();
#endif

    /* draw the arena */
    ui_drawarena (&arena);

    if ( game_paused ) continue;

    if ( waiting ) continue;

    /* give players a turn to play */
    snake_play ();

    /* move the snakes */
    move_snakes ();

    /* increment bogus time counter */
    turn++;

    /* should be done by UIs */
    expire_messages ();

    /* usleep(delay*1000); */

  }

}

void
finish ()
{
  int i;

  /* call the UI cleaner */
  ui_finish();

  printf ("\n");

  /* print scores */
  for (i = 0; i<MAXPLAYERS; i++)
  {
    /* empty slot */
    if ( zoo[i] == NULL ) continue;

    printf ("snake %i: score(%li), length(%i)\n",
      i, zoo[i]->score, zoo[i]->length);
  }

  exit (0);
}


static void
snake_play (void)
{
  static int i; /* could be messed on longjump */
  int dir;

#ifndef DISABLE_TIMEOUT
  static struct itimerval playtimer = {
    {0, PLAYERTIMEOUT},		/*
				 * it_interval.{tv_sec,tv_usec} 
				 */
    {0, PLAYERTIMEOUT},		/*
				 * it_value.{tv_sec,tv_usec} 
				 */
  };
#endif


  for (i=0; i<MAXPLAYERS; i++)
  {
    /* empty slot */
    if (zoo[i] == NULL) continue;

#ifndef DISABLE_TIMEOUT
    if (setjmp (playloop))
	  {

	    /* reset timer */
	    setitimer (ITIMER_VIRTUAL, NULL, NULL);

	    message ("!! snake %i thinks too much !!", i);

	    continue;
	  }
#endif

    /* dead snake */
    if (zoo[i]->died) continue;

    /* rewind typein queue */
    rewindtypein ();

#ifndef DISABLE_TIMEOUT
    /* start timer */
    setitimer (ITIMER_VIRTUAL, &playtimer, NULL);
#endif

    /* call player function */
    dir=-1;
    dir=(*zoo[i]->play) (zoo[i]);

#ifndef DISABLE_TIMEOUT
    /* remove timer */
    setitimer (ITIMER_VIRTUAL, NULL, NULL);
#endif

    if ( dir == -1 ) continue;
    if ( zoo[i]->length > 1 && IS_OPPOSITE(zoo[i]->dir, dir) ) continue;
    zoo[i]->dir = dir;

  }

  /* reset typein queue */
  resettypein ();
}


/*
 * player modules must call this function at startup 
 */
int
register_player (snake * s)
{
  int playernum;

  /* find first available player slot */
  for (playernum=0; playernum<MAXPLAYERS; playernum++)
    if (zoo[playernum] == NULL) break;

  /*
   * don't register more then MAXPLAYERS snakes ! 
   * (gotta remove this limitation - maybe)
   */
  if (playernum >= MAXPLAYERS)
  {
    message ("maximum count of players reached");
    return 0;
  }

  /* set playernum element */
  s->playernum = playernum;

  /* register new player */
  zoo[playernum] = s;

  message ("player %d registered", playernum);

  /* increment players count */
  numplayers++;

  return numplayers;
}

void
pause_game()
{
  game_paused = 1;
}

void
resume_game()
{
  game_paused = 0;
}

static void
sigint (int sig)
{
  finish ();
}

static void
sigsegv (int sig)
{
  fprintf (stderr, "SIGSEGV received, finishing...\n");
  finish ();
}

#ifndef DISABLE_TIMEOUT
static void
sigvtalrm (int sig)
{
  /* reset signal handler (linux needs this, bsd doesn't) */
  signal (sig, sigvtalrm);
  longjmp (playloop, 1);
}
#endif


static int
CMD_quit (char *dummy)
{
  finish ();
  return 1;
}


