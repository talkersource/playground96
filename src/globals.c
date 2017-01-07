/*
 * globals.c
 */

#define GLOBAL_FILE

#include "config.h"
#include "player.h"

/* boot thangs */

int             up_date;
int             logins = 0;
int             backup = 0;
/* sizes */

int             max_players, current_players = 0;
int             max_ppl_on_so_far = 0;
int             in_total = 0, out_total = 0, in_current = 0, out_current = 0, in_average = 0,
                out_average = 0, net_count = 10, in_bps = 0, out_bps = 0, in_pack_total = 0,
                out_pack_total = 0, in_pack_current = 0, out_pack_current = 0, in_pps = 0,
                out_pps = 0, in_pack_average = 0, out_pack_average = 0;


/* One char for splat sites */

int             splat1, splat2;
int             splat_timeout;
int             soft_splat1, soft_splat2, soft_timeout = 0;

/* sessions!  */

char            session[MAX_SESSION];
int             session_reset = 0;
player         *p_sess = 0;
char            sess_name[MAX_NAME] = "";

/* flags */

int             sys_flags = 0;
int             command_type = 0;
int		sys_color_atm = 8;

/* pointers */

char           *action;
char           *stack, *stack_start;
player         *flatlist_start;
player         *hashlist[27];
player         *current_player;
player         *c_player;
room           *current_room;
player         *stdout_player;
player	       *otherfriend;

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
   {"hesitates a lil.\n", 15},
   {"is thinking about what to type next.\n", 25},
   {"appears to be stuck for words.\n", 40},
   {"ponders thoughtfully about what to say.\n", 60},
   {"stares oblivious into space.\n", 200},
   {"is on the road to idledom.\n", 300},
   {"is off to the potty?\n", 600},
   {"appears to be doing something else.\n", 900},
   {"is slipping into a coma.\n", 1200},
   {"is hospitalized and is comatose.\n", 1800},
   {"Is probably not going to recover\n", 2400},
   {"has had the plug pulled.\n", 3000},
   {"is dead Jim.\n", 3600},
   {"has been six feet under for some time\n", 7200},
{0, 0}};

