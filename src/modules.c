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
 * $Id: modules.c,v 1.2 2004/07/28 13:33:13 strk Exp $
 ****************************************************************************/


/*
 * modules init functions 
 */
extern int ai_module_init (void);
extern int human_module_init (void);
extern int long_module_init (void); 
extern int wall_module_init (void); 

/*
 * extern int your_module_init (void); 
 */

void
init_modules ()
{
  human_module_init ();
  ai_module_init ();
  wall_module_init ();

  /*
   * this is a test for the execution limit of play functions 
   */
  /*
   * long_module_init();  
   */

  /*
   * your_module_init(); 
   */
}
