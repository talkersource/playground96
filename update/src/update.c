/*
 * update.c
 */

 /*********************************************************************
 * Update v1.0 - devloped and coded by Goliath                        *
 *               based on EW-too by Simon Marsh                       *
 * The code for update may be freely copied and distributed.          *
 **********************************************************************
 * This file contains all the functions that are used to update       *
 * player files to include new saved info and also funcions to        *
 * initialise the new data.                                           *
 **********************************************************************
 * For information on how to run and use update please read           *
 *  UPDATE.README                                                     *
 **********************************************************************
 * The code in itself is as safe as EW is, so no matter what you do   *
 * always keep backups of your player files prior to updating them :) *
 *********************************************************************/

#include <string.h>
#include "fix.h"
#include "config.h"
#include "player.h"
#include "dynamic.h"
#include "update.h"

/* External functions */

extern void          init_room(char *name, file rf);
extern file          load_file(char *filename);
extern char         *get_int(int *dest, char *source);
extern char         *get_string(char *dest, char *source);
extern char         *store_string(char *dest, char *source);
extern char         *store_int(char *dest, int source);
extern int           restore_player(player *p, char *name);
extern void          save_player(player *p);

/*
 * This will be called on every player save
 * so can be used to produce summary reports
 * and print data from player files etc
 */

void     players_update_function(player * p)
{
  /* Don't change these lines, they are for res counts */
  if (p->residency != STANDARD_ROOMS
      && p->residency != SYSTEM_ROOM)
  {
    if ( (p->residency == BANISHED) ||
	 (p->residency & BANISHD) )
      num_banished++;
    else
      if (!(p->residency & (PSU|SU|LOWER_ADMIN|ADMIN|HCADMIN) ))
	num_residents++;
      else
	if (!(p->residency & (SU|LOWER_ADMIN|ADMIN|HCADMIN) ))
	  num_psu++;
	else
	  if (!(p->residency & (LOWER_ADMIN|ADMIN|HCADMIN) ))
	    num_su++;
	  else
	    if (!(p->residency & (ADMIN|HCADMIN)))
	      num_ladmin++;
	    else
	      num_admin++;
  }
}

/*
 * Initialise any new data you want to save
 * in this function
 */

void     initialise_data(player *p)
{
}

/* dumps the save data onto the stack */

void     extra_save_data(player * p)
{
}

void     extra_load_data(player * p, char *r)
{
}

/*
 * Alter this to function to match that of
 * init_rooms in your own room.c at the bottom of the file.
 * You MUST alter the names of the players system, summink and boot
 * and include the names of you own system rooms.
 * Just copy a chunk and replace the name in the sprintf
 * e.g. sprintf(stack, "%sMYROOM.rooms", rc_options->srooms_path);
 * also change the line init_room("<name>", lf); where <name> is the
 * name of the system room player file associated with that room.
 */
 
void init_your_rooms(void)
{
  file lf;
  char *oldstack;
  /* This is the format for each room file you have

   sprintf(stack, "%sSOMEROOM.rooms", rc_options->srooms_path);
   stack = strchr(stack,0);
   lf = load_file(oldstack);
   init_room("PLAYER", lf);
   stack = oldstack;
   */
  
  oldstack = stack;
  /* Add your room files after this line */
  
}


