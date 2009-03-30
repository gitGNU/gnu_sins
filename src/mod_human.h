/****************************************************************************
 *
 *    sins - a videogame derived from Q-Basic nibbles
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
 * $Id: mod_human.h,v 1.3 2009/03/30 09:47:07 strk Exp $
 ****************************************************************************/

/* local definitions */
#define MAXQUEUELEN 2

/* Queue element */
typedef struct t_qelem
{
  int key;
  struct t_qelem *next;
}
qelem;

/* Queue */
typedef struct t_queue
{
  qelem *first;
  qelem *last;
  int len;
}
queue;

/* Snake brain */
typedef struct t_brain
{
  int kl;			/* left key */
  int ku;			/* up key */
  int kr;			/* right key */
  int kd;			/* down key */
  int ks;			/* stop key */
  int kg;			/* grow key */
  queue *kqueue;		/* key queue */
}
brain;
