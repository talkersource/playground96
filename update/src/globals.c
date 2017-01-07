/*
 * globals.c
 */

#define GLOBAL_FILE

#include "config.h"
#include "player.h"
#include "file.h"

/* boot thangs */

int             max_players, current_players = 0;


int             sys_flags = 0;
int             command_type = 0;

/* pointers */

char           *action;
char           *stack, *stack_start;
player         *flatlist_start;
player         *hashlist[27];
player         *current_player;
player         *c_player;
room           *current_room;
player         *stdout_player;

/*Debugging globals*/
#ifdef TRACK
char           functionin[100];
char           functionhist[20][100];
int            funcposition;
#endif

player        **pipe_list;
int             pipe_matches;

room           *entrance_room, *prison, *colony, *comfy, *boot_room;

/*
 * lists for use with idle times its here for want of a better place to put it
 */

file            idle_string_list[] = {
   {"has just hit return.\n", 0},
   {"is typing merrily away.\n", 10},
   {"hesitates slightly.\n", 15},
   {"is thinking about what to type next.\n", 25},
   {"appears to be stuck for words.\n", 40},
   {"ponders thoughtfully about what to say.\n", 60},
   {"stares oblivious into space.\n", 200},
   {"is on the road to idledom.\n", 300},
   {"is off to the loo ?\n", 600},
   {"appears to be doing something else.\n", 900},
   {"is slipping into unconsciousness.\n", 1200},
   {"has fallen asleep at the keyboard.\n", 1800},
   {"snores loudly.\n", 2400},
   {"moved !! .... no sorry, false alarm.\n", 3000},
   {"seems to have passed away.\n", 3600},
   {"is dead and buried.\n", 5400},
   {"passed away a long time ago.\n", 7200},
{0, 0}};

/* New globals for update */

rc_type rc_options;
int num_residents, num_psu, num_su, num_admin, num_ladmin, num_banished;













