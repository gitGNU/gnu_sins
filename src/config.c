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
#include <stdio.h>		/* fopen, fclose, ...   */
#include <sys/types.h>		/* getpwuid, ...   */
#include <sys/stat.h>		/* stat, ...   */

#define DEBUG_CONFIG

/* public_functions */
extern int source_rcfile ();
extern int CMD_source (char *);

/* private data */

/* public data */


/* Evaluate commands from a file */
int
CMD_source (char *arg)
{
  char buf[MAXCMDLINELEN];
  FILE *stream;
  char *fname;

  if (!arg) return -1;
  
  /* Find file in PATH */
  if ( arg[0] != '/' )
  {
          if ( (fname=find_file(arg)) == NULL )
          {
            message ("Can't find file %s in path %s", arg, path);
            return 0;
          }
  }
  else
  {
    fname = arg;
  }

  /*
   * Open file for reading or complain
   */
  stream = fopen (fname, "r");
  if (!stream)
  {
      message ("Can't open %s: %s", fname, strerror (errno));
      return 0;
  }

  /*
   * Scan the whole file for commands
   */
  while (fgets (buf, MAXCMDLINELEN, stream))
  {
      parse_command (buf);
  }

  fclose (stream);

  return 1;
}

int
source_rcfile ()
{
  char rcpath[256]; /* to be removed */
  char *homedir;

  homedir = getenv("HOME");
  if ( ! homedir ) return 0;

  sprintf(rcpath, "%s/%s", homedir, ".sinsrc");

  /* Should check for existance before going */

  /*message("Sourcing %s", rcpath);*/
  return CMD_source (rcpath);
}

