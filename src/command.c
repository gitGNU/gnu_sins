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
#include <fcntl.h>
#include <sys/types.h>		/* getpwuid, ...   */
#include <sys/stat.h>		/* stat, ...   */

/* public functions */
int register_command (char *, int (*)(char *), char *);
int parse_command (char *);
int CMD_help (char *);
void list_commands ();
#ifndef DISABLE_CMDFIFO
int CMD_cmdfifo (char *);
void read_cmdfifo ();
#endif

/* public data */
#ifndef DISABLE_CMDFIFO
FILE *cmdfifo;
char *cmdfifofile;
#endif

/*
 * local data 
 */
static struct t_command_table
{
  char label[MAXCMDLBLLEN];
  char help[MAXCMDHLPLEN];
  int (*handler) (char *);
  struct t_command_table *next;
}
*command_table;


/*
 * register a command handler in the command table 
 */
int
register_command (char *label, int (*handler) (char *), char *help)
{
  struct t_command_table *ptr, *lptr;

  /*
   * scan the command table 
   */
  for (lptr = ptr = command_table; ptr; lptr = ptr, ptr = ptr->next)
  {
    /* command already registered */
    if (!strcmp (ptr->label, label)) return 0;
  }

  /* append the new command if not already registered */
  ptr = safe_malloc (sizeof (struct t_command_table), "new command table item");
  strncpy (ptr->label, label, MAXCMDLBLLEN - 1);
  strncpy (ptr->help, help, MAXCMDHLPLEN - 1);
  ptr->handler = handler;
  ptr->next = NULL;

  if (lptr) lptr->next = ptr;
  else command_table = ptr;

#ifdef DEBUG_CONFIG
  message ("command '%s' registered", label);
#endif

  return 1;
}


/* parse a single command line */
int
parse_command (char *s)
{
  char *label, *arg;
  struct t_command_table *ptr;
  int ret;

  /* remove leading spaces */
  while (*s == ' ' || *s == '\t') s++;

  /* this is a comment */
  if (*s == '#') return 0;

  label = strtok (s, " \t\n");
  if (!label) return 0;

  arg = strtok (NULL, "\n");

  /* scan the command table */
  for (ptr = command_table; ptr; ptr = ptr->next)
  {
    if (!strcmp (ptr->label, label))
    {
	    ret = ptr->handler (arg);
	    if (ret == -1)
	    {
	      message ("Usage: %s", ptr->help);	/* syntax error */
	      return -1;
	    }
	    return 1;
    }
  }

  message ("Unknown command: %s", label);
  return 0;
}

int
CMD_help (char *label)
{
  struct t_command_table *ptr;

  if (!label)
    return -1;

  /*
   * scan the command table 
   */
  for (ptr = command_table; ptr; ptr = ptr->next)
    if (!strcmp (ptr->label, label))
      {
	message (ptr->help);
	return 1;
      }

  /*
   * unknown command 
   */
  message ("Unknown command: %s", label);
  return 1;
}

#ifndef DISABLE_CMDFIFO
void
read_cmdfifo ()
{
  char buf[256];

  if (fgets (buf, 256, cmdfifo))
    parse_command (buf);
}

int
CMD_cmdfifo (char *arg)
{
  FILE *newcmdfifo;

  if (!arg) return -1;

  /* should we create it ? Only if not there ... */
  if ( -1 == mkfifo(arg, 0644) )
  {
    message ("Creating cmdfifo file: %s", strerror(errno));
    if ( errno != EEXIST ) return 0;
  }

  newcmdfifo = fopen (arg, "r+");
  if (!newcmdfifo)
  {
    message ("Error opening command fifo %s: %s", arg, strerror (errno));
    return 0;
  }

  /* set nonblocking read mode */
  if (fcntl (fileno (newcmdfifo), F_SETFL, O_NONBLOCK) == -1)
  {
    message ("Can't set nonblocking mode on fifo: %s", strerror(errno));
    return 0;
  }

  /* should we destroy old fifo ? */

  if (cmdfifo) fclose (cmdfifo);
  cmdfifo = newcmdfifo;
  cmdfifofile = arg;

  message ("Command fifo: %s", cmdfifofile);

  return 1;
}
#endif /* !DISABLE_CMDFIFO */

void
list_commands()
{
  struct t_command_table *ptr;

  for (ptr = command_table; ptr; ptr = ptr->next)
  {
    printf("%s\n", ptr->help);
  }
}

