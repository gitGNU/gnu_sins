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
#include <ctype.h>
#include <stdlib.h>

#define MAXINQUEUELEN 128
#undef DEBUG_TYPEIN

/*
 * extern functions 
 */
extern void lower_delay (void);
extern void higher_delay (void);
extern char *ui_prompt (char *);

/*
 * public functions 
 */
int parse_input (void);
int fetchtypein (void);
void rewindtypein (void);
void resettypein (void);

/*
 * private data 
 */
static int _typeinqueue[MAXINQUEUELEN];
static int _typeinlen;		/* actual length of typein queue */
static int _typeincursor;	/* cursor for typein queue */
static char *_score_string (void); /* score string */

/*
 * private functions 
 */
static int _puttypein (int ch);
static void _read_command (void);


/*
 * parse user input 
 */
int
parse_input ()
{
  int typein;
  extern int game_paused;

  /* Read all pending characters */
  while ((typein = ui_getkey ()) != -1)
  {

    if ( game_paused )
    {
      if ( typein == ' ' )
      {
        resume_game ();
        /* game_paused = 0; */
        break;
      }
      continue;
    }

    /* Interpret each key */
    switch (typein)
    {

      /* quit on 'q' */
      case 'q':
          message ("Are you sure you wonna quit [N/y] ?");
          while ((typein = ui_getkey ()) == -1) ui_drawarena(&arena);
          if (tolower (typein) != 'y')
          {
            message ("yeah! keep wasting time ...");
            break;
          }

      case 'Q':
        finish ();

      /* higher delay if '-' (lower speed) */
      case '-':
        higher_delay ();
        break;

      /* lower delay if '+' (higher speed) */
      case '=':
      case '+':
        lower_delay ();
        break;

      /* pause game */
      case ' ':
        /* game_paused = 1; */
        pause_game();
        message ("%s", _score_string());
        break;

      /* evaluate command */
      case ':':
        _read_command ();
        break;

      default:
        if (typein >= 48 && typein < MAXPLAYERS + 48) /* reset dead snake */
        {

          typein -= 48;

          if (zoo[typein] && zoo[typein]->died)
          {
            snake_reset (zoo[typein]);
          }
          else
          {
            message ("no such snake dead (%i)", typein);
          }
        }
        else /* Push the typed char into the typein queue */
        {
          _puttypein (typein);
        }
        break;
    }
  }

  return 1;
}


/*
 * Put a character in the typein queue. Print a message if too many keys
 * are pushed on. * ( see MAXINQUEUELEN define above ) 
 */
static int
_puttypein (int ch)
{

  if (_typeinlen >= MAXINQUEUELEN)
    {
      message ("keyboard spam detected (%i keys per turn)!", MAXINQUEUELEN);
      return 0;
    }
  _typeinqueue[_typeinlen++] = ch;

#ifdef DEBUG_TYPEIN
for(;;)
{
  int i;
  fprintf(stderr, "_typeinqueue: ");
  for (i=0; i<_typeinlen; i++) fprintf(stderr, "%c", _typeinqueue[i]);
  fprintf(stderr, "\n");
  break;
}
#endif
  return 1;
}

/*
 * Fetch first/next character from typein queue. * Return -1 when last
 * character on queue is read. 
 */
int
fetchtypein ()
{

#ifdef DEBUG_TYPEIN
  fprintf(stderr, "fetchtypein (len:%i)\n", _typeinlen);
#endif
  /*
   * no more elements on the list 
   */
  if (_typeincursor == _typeinlen)
    return -1;

  return _typeinqueue[_typeincursor++];
}

/*
 * Reset typein cursor to first element of the queue 
 */
void
rewindtypein ()
{
  _typeincursor = 0;
}

/*
 * Reset typein queue
 */
void
resettypein ()
{
  _typeincursor = _typeinlen = 0;
} 

static void
_read_command ()
{
  char *command;

  command = ui_prompt ("command");
  if (command)
    {
      parse_command (command);
      free (command);
    }
}

static char *
_score_string ()
{
  char str[32];
  static char scstr[256];
  int i;

  sprintf (scstr, "Scores: ");
  for (i=0; i<MAXPLAYERS; i++)
  {
    /* empty slot */
    if ( zoo[i] == NULL ) continue;

    if (i) sprintf (str, " %i=%li", i, zoo[i]->score);
    else sprintf (str, "%i=%li", i, zoo[i]->score);
    strcat (scstr, str);
  }

  return scstr;
}

