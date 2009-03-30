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
 * $Id: VARS.c,v 1.3 2009/03/30 09:47:06 strk Exp $
 ****************************************************************************
 *
 * purpose: manage settings (currently unused)
 * 
 ****************************************************************************/

struct varent_t {
  char *name = NULL;
  char *value = NULL;
  struct varent_t *next = NULL;
} varent;

char *getvar (char *name)
{
}

int putvar (char *name, char *value)
{
  struct varent_t *entp;
  struct varent_t *newent;
  int cmp;
  int done=0;

  /* create a new entry */
  newent = safe_malloc(sizeof(struct varent), "new entry in variable table");

  /* place entry in alphabetical order */
  for ( entp = &varent; entp->next; entp=entp->next )
  {
    cmp = strcmp(entp->name, name);
    if ( ! cmp )
    {
      entp->value = value;
      return 1;
    }
  }

}
