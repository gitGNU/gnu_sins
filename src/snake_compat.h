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
 * $Id: snake_compat.h,v 1.3 2009/03/30 09:47:07 strk Exp $
 ****************************************************************************/

#ifndef SNAKE_COMPAT_H
#define SNAKE_COMPAT_H

/* Compatibility header */

#ifdef STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#include <unistd.h>		/* getuid, stat, getopt...   */
#else
int usleep (unsigned long);
#endif


#endif
