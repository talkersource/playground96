/*
 * parse.c
 */

#include <ctype.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#include <memory.h>
#include <malloc.h>
#include <stdlib.h>

#include "config.h"
#include "player.h"
#include "fix.h"
#include "clist.h"
#include "channel.h"

/* externs */

extern char    *end_string(), *splice_argument(player *, char *, char *, int);
extern int      nhash_update[], update[], true_count_su(), session_reset, emote_no_break();
extern file     load_file(), load_file_verbose(char *, int);
extern void     sync_notes();
extern void     log(char *, char *);
extern char    *full_name(player *);
extern void     tell_player(player *, char *);
extern void     tell_room(room *, char *);
extern void     save_player(player *);
extern void     do_inform(player *, char *);
extern void     destroy_player(player *);
extern void     do_prompt(player *, char *);
extern void     sync_to_file(char, int);
extern void     handle_error(char *);
extern char    *convert_time(time_t);
extern char    *do_alias_match(player *, char *);
extern void     move_to(player *, char *, int);
extern player  *find_player_global(char *), *find_player_absolute_quiet(char *),
	       *p_sess;
extern char    shutdown_reason[];
extern void    do_backup();
extern alias   *get_alias(player *, char *);
extern void	SUWALL(), LOGF();
/* interns */

struct command *last_com;
char           *stack_check;
int             nsync = 0, synct = 0, sync_counter = 0, note_sync = NOTE_SYNC_TIME;
/* int     mem_use_log_count = 0; */
int             account_wobble = 1;
int             performance_timer = 0;
struct command *help_list = 0;
file            help_file = {0, 0};
time_t          shutdown_count = -1;

/* returns the first char of the input buffer */

char           *first_char(player * p)
{
   char           *scan;
   scan = p->ibuffer;
   while (*scan && isspace(*scan))
      *scan++;
   return scan;
}

/* what happens if bad stack detected */

void            bad_stack()
{
   int             missing;
   missing = (int) stack - (int) stack_check;
   if (last_com)
      sprintf(stack_check, "Bad stack in function %s, missing %d bytes",
         last_com->text, missing);
   else
      sprintf(stack_check, "Bad stack somewhere, missing %d bytes", missing);
   stack = end_string(stack_check);
   log("stack", stack_check);
   stack = stack_check;
}


/* flag changing routines */


/* returns the value of a flag from the flag list */

int             get_flag(flag_list * list, char *str)
{
   for (; list->text; list++)
      if (!strcmp(list->text, str))
    return list->change;
   return 0;
}


/* routine to get the next part of an arg */

char           *next_space(char *str)
{
   while (*str && *str != ' ')
      str++;
   if (*str == ' ')
   {
      while (*str == ' ')
    str++;
      str--;
   }
   return str;
}


/* view command lists */

int command_prescan (player * p, char *str)
{
	char *oldstack;
   
	if (!*str || !strcasecmp(str, "?") || !strcasecmp(str, "help"))
	{
     oldstack = stack;
     sprintf(stack, " Format: commands [all|comm|move|desc|info|soc|sys|item|");
     stack = strchr(stack, 0);
     if (p->residency & LIST) {
	strcpy(stack, "list|");
	stack = strchr(stack, 0); }
     if (p->residency & BUILD) {
	strcpy(stack, "room|");
	stack = strchr(stack, 0); }
     if (p->residency & (PSU|SU)) {
	strcpy(stack, "su|");
	stack = strchr(stack, 0); }
     if (p->residency & (TESTCHAR | LOWER_ADMIN | ADMIN)) {
	strcpy(stack, "ad|");
	stack = strchr(stack, 0); }
     strcpy(stack, "misc]\n");
     stack = end_string(stack);
     tell_player(p, oldstack);
     stack = oldstack;
     return THE_EVIL_Q;
	}

/* ok check for what area was inputted, and return appropriate value. */

	if(!strcasecmp(str, "all"))
	{
	tell_player(p, " Your complete set of commands");
	return 0;   /* do all commands =) */
	}
	if (!strcasecmp(str, "comm") || !strcasecmp(str, "talk"))
	{
	tell_player(p, " Your communication commands");
	return COMMc;
	}
	if (p->residency & LIST && (!strcasecmp(str, "list")))
	{
	tell_player(p, " Your list commands");
	return LISTc;
	}
	if (p->residency & BUILD && (!strcasecmp(str, "room")))
	{
	tell_player(p, " Your room commands");
	return ROOMc;
	}
	if (!strcasecmp(str, "go") || !strcasecmp(str, "move"))
	{
	tell_player(p, " Your movement commands");
	return MOVEc;
	}
	if (!strcasecmp(str, "item"))
	{
	tell_player(p, " Your item commands");
	return ITEMc;
	}
	if (!strcasecmp(str, "info"))
	{
	tell_player(p, " Your information commands");
	return INFOc;
	}
	if (!strcasecmp(str, "sys") || !strcasecmp(str, "toggles"))
	{
	tell_player(p, " Your system toggle commands");
	return SYSc;
	}
	if (!strcasecmp(str, "desc") || !strcasecmp(str, "personalize"))
	{
	tell_player(p, " Your personalization commands");
	return DESCc;
	}
	if (!strcasecmp(str, "soc") || !strcasecmp(str, "social"))
	{
	tell_player(p, " Your available socials");
	return SOCIALc;
	} 
	if (!strcasecmp(str, "misc"))
	{
	tell_player(p, " Your miscellaneous commands");
	return MISCc;
	} 
	if (p->residency & PSU && 
		(!strcasecmp(str, "su") || !strcasecmp(str, "super")))
	{
	tell_player(p, " Your super user level commands");
	return SUPERc;
	}
	if (p->residency & (LOWER_ADMIN | ADMIN | TESTCHAR) &&
		(!strcasecmp(str, "ad") || !strcasecmp(str, "admin")))
	{
	tell_player(p, " Your Administration level commands");
	return ADMINc;
	}
	/* ok the area or priv check was invalid - so report error. */
	tell_player(p, " That area not found. Type commands ? to list valid areas.\n");		
	return THE_EVIL_Q;     /* a stupid return yes, but it'll do the job */
}

void view_commands(player * p, char *str)
{
   struct command *comlist;
   char *oldstack;
   char *plyr;
   int s;
   int choice;
   player *p2;
 
   plyr = next_space(str);
   *plyr++ = 0;

   oldstack = stack;

   choice = command_prescan(p, str);

   /* why the evil q? Cuz I'm a trekker :P */
   if (choice == THE_EVIL_Q)
	return;

   if (*plyr && p->residency & ADMIN)
   {
      p2 = find_player_absolute_quiet(plyr);
      if (!p2)
      {
	 tell_player(p,"\nThat player is not logged in right now.\n");
         return;
      }
      sprintf(stack, " (for %s) ... \n", p2->name);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
   } else
   {
      strcpy(stack, "... \n");
      stack = strchr(stack, 0);
      p2 = p;
   }

   for (s = 0; s < 27; s++)
   {
      for (comlist = coms[s]; comlist->text; comlist++)
      {
         if ((!comlist->level || ((p2->residency) & comlist->level)) &&
	     (!comlist->andlevel || ((p2->residency) & comlist->andlevel))
&& ((!choice || comlist->section & choice) && !(comlist->section & INVISc)))
         {
		/* got rid of that damn comma here */
            sprintf(stack, "%s ", comlist->text);
            stack = strchr(stack, 0);
         }
      }
   }
   stack -= 1;
   *stack++ = '\n';
   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}


void            view_sub_commands(player * p, struct command * comlist)
{
   char           *oldstack;
   oldstack = stack;

   strcpy(stack, " Available sub commands ...\n");
   stack = strchr(stack, 0);

   for (; comlist->text; comlist++)
      if (((!comlist->level) || ((p->residency) & (comlist->level))) &&
	  ((!comlist->andlevel) || ((p->residency) & (comlist->andlevel))))
      {
    sprintf(stack, "%s, ", comlist->text);
    stack = strchr(stack, 0);
      }
   stack -= 2;
   *stack++ = '\n';
   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}

/* initialise the hash array */

void            init_parser()
{
   int             i;
   struct command *scan;
   scan = complete_list;
   for (i = 0; i < 27; i++)
   {
      coms[i] = scan;
      while (scan->text)
    scan++;
      scan++;
   }
}

/* see if any commands fit the bill */

char           *do_match(char *str, struct command * com_entry)
{
   char           *t;
   for (t = com_entry->text; *t; t++, str++)
      if (tolower(*str) != *t)
    return 0;
   if ((com_entry->space) && (*str) && (!isspace(*str)))
      return 0;
   while (*str && isspace(*str))
      str++;
   return str;
}

/* execute a function from a sub command list */

void            sub_command(player * p, char *str, struct command * comlist)
{
   char           *oldstack, *rol;
   void            (*fn) ();
   oldstack = stack;

   /* Lowercase string here, line in case the person was rm_capped */
   if (p->system_flags & DECAPPED)
      lower_case(str);

   while (comlist->text)
   {
      if (((!comlist->level) || ((p->residency) & (comlist->level))) &&
	  ((!comlist->andlevel) || ((p->residency) & (comlist->andlevel))))
      {
    rol = do_match(str, comlist);
    if (rol)
    {
       last_com = comlist;
       stack_check = stack;
      sys_flags &= ~ROOM_TAG;
      command_type &= ~ROOM;
       fn = comlist->function;
       (*fn) (p, rol);
       if (stack != stack_check)
          bad_stack();
       sys_flags &= ~(FAILED_COMMAND | PIPE | ROOM_TAG | FRIEND_TAG | OFRIEND_TAG | EVERYONE_TAG);
       command_type = 0;
       return;
    }
      }
      comlist++;
   }
   rol = str;
   while (*rol && !isspace(*rol))
      rol++;
   *rol = 0;
   sprintf(oldstack, " Cannot find sub command '%s'\n", str);
   stack = end_string(oldstack);
   tell_player(p, oldstack);
   stack = oldstack;
}


/* match commands to the main command lists */

void            old_match_commands(player * p, char *str)
{
   struct command *comlist;
   char           *rol, *oldstack, *space;
   void            (*fn) ();
   oldstack = stack;

   while (*str && *str == ' ')
      str++;
   space = strchr(str, 0);
   space--;
   while (*space == ' ')
      *space-- = 0;
   if (!*str)
      return;

   /* Lowercase string here, line in case the person was rm_capped */
   if (p->system_flags & DECAPPED)
      lower_case(str);

   if (isalpha(*str))
      comlist = coms[((int) (tolower(*str)) - (int) 'a' + 1)];
   else
      comlist = coms[0];

   while (comlist->text)
   {
      if (((!comlist->level) || ((p->residency) & (comlist->level))) &&
	  ((!comlist->andlevel) || ((p->residency) & (comlist->andlevel))))
      {
    rol = do_match(str, comlist);
    if (rol)
    {
       last_com = comlist;
       stack_check = stack;
      sys_flags &= ~ROOM_TAG;
      command_type &= ~ROOM;
       fn = comlist->function;
       (*fn) (p, rol);
       if (stack != stack_check)
          bad_stack();
       sys_flags &= ~(FAILED_COMMAND|PIPE|ROOM_TAG|FRIEND_TAG|OFRIEND_TAG|EVERYONE_TAG);
       command_type = 0;
       return;
    }
      }
      comlist++;
   }

   p->antipipe++;

/*
   if (p->antipipe > 30)
   {
      quit(p, 0);
      return;
   }
*/
   rol = str;
   while (*rol && !isspace(*rol))
      rol++;
   *rol = 0;
   sprintf(oldstack, " Cannot find command '%s'\n", str);
   stack = end_string(oldstack);
   tell_player(p, oldstack);
   stack = oldstack;
}

void	match_commands(player * p, char *str) {

   char *rol, holder[1000];
   alias *al;

   if (!(p->saved)) {
	old_match_commands(p, str);
	return;
	}

   /* lets try an alias match first */
   rol = do_alias_match(p, str);
   if (strcmp(rol, "\n")) {
	al = get_alias(p, str);
        if (!al)
		{
		tell_player(p, " Alias matching error\n");
		return;
		}
	/* NEED TO LOOP THROUGH HERE */
	strcpy(holder,splice_argument(p, al->sub, rol, 0));	
	while (holder[0]) {
        if (p->system_flags & DECAPPED)
               lower_case(holder);
        old_match_commands(p, holder);
	strcpy(holder,splice_argument(p, al->sub, rol, 1));	
		}			
	    }	
   else 
	old_match_commands(p, str);
}

/* handle input from one player */

void            input_for_one(player * p)
{
   char           *pick;
   void            (*fn) ();

   if (p->input_to_fn)
   {
      p->idle = 0;
      p->idle_index = 0;
      p->idle_msg[0] = 0;
      last_com = &input_to;
      stack_check = stack;
      fn = p->input_to_fn;
      sys_flags &= ~ROOM_TAG;
      command_type &= ~ROOM;
      (*fn) (p, p->ibuffer);
      if (stack != stack_check)
         bad_stack();
      sys_flags &= ~(FAILED_COMMAND|PIPE|ROOM_TAG|FRIEND_TAG|OFRIEND_TAG|EVERYONE_TAG);
      command_type = 0;
      return;
   }
   if (!p->ibuffer[0])
      return;
   p->idle = 0;
   p->idle_index = 0; 
   p->idle_msg[0] = 0;
   action = "doing command";
   if (p->ibuffer[0] != '#')
   {
      if (p->custom_flags & CONVERSE)
      {
         pick = p->ibuffer;
         while (*pick && isspace(*pick))
            pick++;
         if (*pick)
            if (*pick == '/' || *pick == '.')
               if (current_room == prison && !(p->residency & (ADMIN | SU)))
                  sub_command(p, pick + 1, restricted_list);
               else
                  match_commands(p, pick + 1);
	    else if (!isalpha(*pick))
               if (current_room == prison && !(p->residency & (ADMIN | SU)))
                  sub_command(p, pick, restricted_list);
               else
                  match_commands(p, pick);
            else
               say(p, pick);
      } else if (current_room == prison && !(p->residency & (ADMIN | SU)))
         sub_command(p, p->ibuffer, restricted_list);
      else
         match_commands(p, p->ibuffer);
   }
}

void su_quit_log(player * p) {

	char *oldstack;
	int csu;

	csu = true_count_su();
	oldstack = stack;
	sprintf(stack, "%s leaves -- %d sus left", p->name, csu - 1);
	stack = end_string(stack);
	log ("su", oldstack);
	stack = oldstack;
}
/* scan through the players and see if anything needs doing */

void            process_players()
{
   player         *scan, *sparky;
   char           *oldstack, *hasta;
   int            chan;         


   if (current_players > max_ppl_on_so_far)
	max_ppl_on_so_far = current_players;
   for (scan = flatlist_start; scan; scan = sparky)
   {
     sparky = scan->flat_next;
     if (scan->flat_next)
       if (((player *)scan->flat_next)->flat_previous != scan)
	 {
	   raw_wall("\n\n   -=> Non-terminated flatlist <=-\n\n");
	   raw_wall("\n\n   -=> Dumping end off of list <=-\n\n");
	   scan->flat_next=NULL;
	 }

      if ((scan->fd < 0) || (scan->flags & PANIC) ||
          (scan->flags & CHUCKOUT))
      {

         oldstack = stack;
         current_player = scan;

         if (scan->location && scan->name[0] && !(scan->flags & RECONNECTION))
         {

	    if (scan->residency & SU && true_count_su() <= 1)
		su_quit_log(scan);

            hasta = do_alias_match(scan, "_logoff");
	    if (strcmp(hasta, "\n")) {
		match_commands(scan, "_logoff");
	    }
	    if (scan == p_sess) {
		session_reset = 0;
	    }
	    for (chan=0; chan<NUM_CHANNELS; chan++) {
		if(scan->chanflags & (1<<chan)) {
			channelquitbyint(scan, chan);
			}
		}

	    if (strlen(scan->logoffmsg) < 1)
            sprintf(stack, "%s hear%s %s parents calling and trot%s away.\n",
                           scan->name, single_s(scan), gstring_possessive(scan), single_s(scan));
	    else {
		if (emote_no_break(*scan->logoffmsg))
		   sprintf(stack, "%s%s\n", scan->name, scan->logoffmsg);
		else
		   sprintf(stack, "%s %s\n", scan->name, scan->logoffmsg); }

            stack = end_string(stack);
            command_type |= LOGOUT_TAG;
            tell_room(scan->location, oldstack);
	    command_type &= ~LOGOUT_TAG;
            stack = oldstack;
            save_player(scan);
         }
         if (!(scan->flags & RECONNECTION))
         {
            command_type = 0;
	    if (scan->gender==PLURAL)
	      do_inform(scan, "[%s have left] %s");
	    else
	      do_inform(scan, "[%s has left] %s");
            if (scan->saved && !(scan->flags & NO_SAVE_LAST_ON))
               scan->saved->last_on = time(0);
         }
         if (sys_flags & VERBOSE || scan->residency == 0)
         {
            if (scan->name[0])
               sprintf(oldstack, "%s has disconnected from %s", scan->name,
                       scan->inet_addr);
            else
               sprintf(oldstack, "Disconnect from login. [%s]",
                       scan->inet_addr);
            stack = end_string(oldstack);
            log("newconn", oldstack);
         }
         destroy_player(scan);
         current_player = 0;
         stack = oldstack;
      } else if (scan->flags & INPUT_READY)
      {
/* there used to be this here...
            if (!(scan->lagged) && !(scan->flags & PERM_LAG))
   for reference... */

         if (!(scan->lagged) && !(scan->system_flags & SAVE_LAGGED))
         {
            current_player = scan;
            current_room = scan->location;
            input_for_one(scan);
            action = "processing players";
            current_player = 0;
            current_room = 0;
	    sys_color_atm = SYSsc;

#ifdef PC
            if (scan->flags & PROMPT && scan == input_player)
#else
            if (scan->flags & PROMPT)
#endif
            {
               if (scan->custom_flags & CONVERSE)
                  do_prompt(scan, scan->converse_prompt);
               else
                  do_prompt(scan, scan->prompt);
            }
         }
         memset(scan->ibuffer, 0, IBUFFER_LENGTH);
         scan->flags &= ~INPUT_READY;
      }
   }
}




/* timer things */


/* automessages */

void            do_automessage(room * r)
{
   int             count = 0, type;
   char           *scan, *oldstack;
   oldstack = stack;
   scan = r->automessage.where;
   if (!scan)
   {
      r->flags &= ~AUTO_MESSAGE;
      return;
   }
   for (; *scan; scan++)
      if (*scan == '\n')
    count++;
   if (!count)
   {
      FREE(r->automessage.where);
      r->automessage.where = 0;
      r->automessage.length = 0;
      r->flags &= ~AUTO_MESSAGE;
      stack = oldstack;
      return;
   }
   count = rand() % count;
   for (scan = r->automessage.where; count; count--, scan++)
      while (*scan != '\n')
    scan++;
   while (*scan != '\n')
      *stack++ = *scan++;
   *stack++ = '\n';
   *stack++ = 0;
   type = command_type;
   command_type = AUTO;
   tell_room(r, oldstack);
   command_type = type;
   r->auto_count = r->auto_base + (rand() & 63);
   stack = oldstack;
}


/* file syncing */

void            do_sync()
{
   int             origin;
   action = "doing sync";
   sync_counter = SYNC_TIME;
   origin = synct;
   while (!update[synct])
   {
      synct = (synct + 1) % 26;
      if (synct == origin)
    break;
   }
   if (update[synct])
   {
      sync_to_file(synct + 'a', 0);
      synct = (synct + 1) % 26;
   }
}

/* this is the actual timer pling */

void            actual_timer()
{
   static int      pling = TIMER_CLICK;
   player         *scan, *wibble;
   time_t t;
   int i;

   if (sys_flags & PANIC)
      return;

#if !defined(hpux) && !defined(linux)
   if ((int) signal(SIGALRM, actual_timer) < 0)
      handle_error("Can't set timer signal.");
#endif /* hpux */

   t = time(0);
   if ((splat_timeout - t) <= 0)
      splat1 = splat2 = 0;
   pling--;
   if (pling)
      return;

   pling = TIMER_CLICK;

   sys_flags |= DO_TIMER;

   /* if (mem_use_log_count > 0)
      mem_use_log_count--; */
   if (shutdown_count > 0)
      shutdown_count--;

   for (i=0; i < NUM_CHANNELS; i++) {
	if (channel[i].inuse) {
		channel[i].idle++;
		if (!channel[i].numppl)
		  channeldestroy(i, "No one left on channel !!\n");
		else if (channel[i].numppl <= 1 && (time(0) - channel[i].starttime) > CHANNEL_TIMEOUT)
		  channeldestroy(i, "Channel destroyed: There are too few users.\n");
		else if (channel[i].idle > CHANNEL_TIMEOUT)
		  channeldestroy(i, "Channel destroyed due to lack of use -- too idle.\n");
		}
	}


   for (scan = flatlist_start; scan; scan = scan->flat_next)
      if (!(scan->flags & PANIC))
      {
    scan->idle++;
    scan->idle_index++; 
    scan->total_login++;
    if (scan->total_login % ONE_HOUR == 0) 
		scan->pennies += 10;
    if (scan->pennies > 100000)
		scan->pennies = 100000;
    if (scan->residency && !(scan->residency & NO_SYNC) && scan->total_login % 1200 == 800 )
	save_player(scan);
    if (scan->location && !strcmp(scan->location->owner->lower_name, "main"))
	scan->time_in_main++;
    if (scan->script && scan->script > 1)
       scan->script--;
    if (scan->timer_fn && scan->timer_count > 0)
       scan->timer_count--;
    if (scan->no_shout > 0)
       scan->no_shout--;
    if (scan->no_move > 0 && !(scan->system_flags & SAVED_RM_MOVE))
       scan->no_move--;
    if (scan->lagged > 0)
       scan->lagged--;
    if (scan->shout_index > 0)
       scan->shout_index--;
/* for sing */
    if (scan->no_sing > 0)
	scan->no_sing--;

/* for asty's idle thang */
    if (scan->idle_index > 300 && !(scan->residency & ADMIN)) 
	{
		scan->total_idle_time += (290 + (rand() % 40)); 
		scan->idle_index = 0;
  	} 
    if (scan->jail_timeout > 0)
       scan->jail_timeout--;
    /* unidle admins */
    if(scan->idle > 3000 && scan->residency & ADMIN) 
	scan->idle = 0; 
    /* timing out REALLY idle gits... */
    if(scan->idle == 3000 || scan->idle == 3300 || 
	(!(scan->residency) && (scan->idle == 1500 || scan->idle == 1620))) {
	scan->total_idle_time +=(1500 + (rand() % 500)); 
	TELLPLAYER(scan, " -=*> Warning - you are now %d minutes idle.\007\n",
			scan->idle/ONE_MINUTE);
	}
    if(scan->idle == 3540 || (!(scan->residency) && scan->idle == 1740)) {
	scan->total_idle_time +=(1500 + (rand() % 500)); 
	TELLPLAYER(scan, " -=*> You're %d minutes idle. 1 minute till auto-disconnect.\007\n",
			scan->idle/ONE_MINUTE);
	}
    if(scan->idle >= ONE_HOUR || (!(scan->residency) && scan->idle >= (ONE_HOUR/2)))  {
 		TELLPLAYER(scan, " Thank you for visiting.. but next time, when you decide to leave, "
			" you ought quit for yourself... Bye bye.\n");
		scan->total_idle_time +=(3000 + (rand() % 1000)); 
	        scan->idled_out_count++;
		log("idle", scan->lower_name);
		quit(scan, 0);
		}	
      }
   net_count--;
   if (!net_count)
   {
      net_count = 10;
      in_total += in_current;
      out_total += out_current;
      in_pack_total += in_pack_current;
      out_pack_total += out_pack_current;
      in_bps = in_current / 10;
      out_bps = out_current / 10;
      in_pps = in_pack_current / 10;
      out_pps = out_pack_current / 10;
      in_average = (in_average + in_bps) >> 1;
      out_average = (out_average + out_bps) >> 1;
      in_pack_average = (in_pack_average + in_pps) >> 1;
      out_pack_average = (out_pack_average + out_pps) >> 1;
      in_current = 0;
      out_current = 0;
      in_pack_current = 0;
      out_pack_current = 0;
   }
}


/* the timer function */

void            timer_function()
{
   player         *scan, *old_current;
   void            (*fn) ();
   room           *r, **list;
   char           *oldstack, *text;
   int             count = 0, pcount = 0;
   char           *action_cpy;
   struct tm      *ts;
   time_t          t;
#if !defined(linux)
   struct mallinfo minfo;
#else
   /* struct mstats memstats; */
#endif /* LINUX */

   if (!(sys_flags & DO_TIMER))
      return;
   sys_flags &= ~DO_TIMER;

   waitpid((pid_t) - 1, (int *) 0, WNOHANG);
   /* wait3(0,WNOHANG,0); */

   old_current = current_player;
   action_cpy = action;

   oldstack = stack;
/*
   if (mem_use_log_count == 0)
   {
#if !defined(linux)
      minfo = mallinfo();
      sprintf(stack, "Total arena space - %d", minfo.arena);
#else
      memstats = mstats();
      sprintf(stack, "Total heap size - %d", memstats.bytes_total); 
#endif 
      stack = end_string(stack);
      log("mem", oldstack);
      stack = oldstack;
      mem_use_log_count = ONE_MINUTE;
   }
*/
   if (shutdown_count > -1)
   {
      command_type |= HIGHLIGHT;
      switch (shutdown_count)
      {
      case ONE_YEAR:
	raw_wall("\n\n -=*>           Your attention please.           <*=-\n"
		     " -=*>   We'll be rebooting in exactly one year   <*=-\n"
		     " -=*> Anyone still here at that time needs help! <*=-\n\n");
	break;
      case ONE_DAY:
	raw_wall("\n -=*> We'll shutdown this time tomorrow -- Where will you be? Eh? <*=-\n\n");
	break;
      case (15 * ONE_MINUTE):
	raw_wall("\n -=*> Reboot in 15 mins, most people can have sex in less time... <*=-\n\n");
	break;
      case (10 * ONE_MINUTE):
	raw_wall("\n -=*> Reboot in 10 mins, Speedy Gonzalez waves to you. Arriva ;-) <*=-\n\n");
	break;
      case (5 * ONE_MINUTE):
	raw_wall("\n -=*> Reboot in 5 mins, there's plenty of time to edit your list. <*=-\n\n");
	break;
      case (3 * ONE_MINUTE):
	raw_wall("\n -=*> Reboot in 3 mins, last chance to talk silly admins outta it <*=-\n\n");
	break;
      case (2 * ONE_MINUTE):
	raw_wall("\n -=*> Reboot in 2 mins, so the three ring circus will be closing. <*=-\n\n");
	break;
      case ONE_MINUTE:
	raw_wall("\n -=*> Reboot in 1 minute, well be right back up.. (traP hopes...) <*=-\n\n");
	break;
      case 45:
	raw_wall("\n -=*> Reboot in 45 secs, (this space for rent - call 18009110000) <*=-\n\n");
	break;
      case 30:
	raw_wall("\n -=*> Reboot in 30 secs, Watch all the people start logging off.. <*=-\n\n");
	break;
      case 15:
	raw_wall("\n -=*> Reboot in 15 secs, We should be back up in 30 seconds or so <*=-\n\n");
	break;
      case 10:
	raw_wall("\n -=*> Reboot in 10 secs, Initiating final liftoff sequence now... <*=-\n");
	break;
      case 9:
	raw_wall("\n -=*> Reboot in 9\n");
	break;
      case 8:
	raw_wall("\n                8\n");
	break;
      case 7:
	raw_wall("\n                7\n");
	break;
      case 6:
	raw_wall("\n                6\n");
	break;
      case 5:
	raw_wall("\n                5\n");
	break;
      case 4:
	raw_wall("\n                4\n");
	break;
      case 3:
	raw_wall("\n                3\n");
	break;
      case 2:
	raw_wall("\n                2\n");
	break;
      case 1:
	raw_wall("\n                1\n");
	break;
      case 0:
	log("shutdown", shutdown_reason);
	sys_flags |= SHUTDOWN;
	stack = oldstack;
	return;
      }
      command_type &= ~HIGHLIGHT;
    }

   if (sync_counter)
      sync_counter--;
   else
      do_sync();

   if (note_sync)
      note_sync--;
   else
   {
      note_sync = NOTE_SYNC_TIME;
      sync_notes(1);
   }

   align(stack);
   list = (room **) stack;

   for (scan = flatlist_start; scan; scan = scan->flat_next)
   {
      if (!(scan->flags & PANIC))
      {
         if (scan->script && scan->script == 1)
         {
            text = stack;
            sprintf(text, " Time is now %s.\n"
                          " Scripting stopped ...\n", convert_time(time(0)));
            stack = end_string(text);
            tell_player(scan, text);
            stack = text;
            scan->script = 0;
         }
         if (scan->timer_fn && !scan->timer_count)
         {
            current_player = scan;
            fn = scan->timer_fn;
	    sys_flags &= ~ROOM_TAG;
      	    command_type &= ~ROOM;
            (*fn) (scan);
            scan->timer_fn = 0;
            scan->timer_count = -1;
         }
         current_player = old_current;
         action = "processing autos";
         r = scan->location;
         if (r)
         {
            pcount++;
            if (r->flags & AUTO_MESSAGE && !(r->flags & AUTOS_TAG))
            {
               if (!r->auto_count)
                  do_automessage(r);
               else
                  r->auto_count--;
               *(room **) stack = r;
               stack += sizeof(room *);
               count++;
               r->flags |= AUTOS_TAG;
            }
         }
/* Jail timeout thang */
         if (scan->jail_timeout == 0 && scan->location == prison)
         {
            command_type |= HIGHLIGHT;
            tell_player(scan, " After serving your sentence you are flung out"
                              " to society again.\n");
            command_type &= ~HIGHLIGHT;
            move_to(scan, ENTRANCE_ROOM, 0);
         }
      }
   }
   for (; count; count--, list++)
      (*list)->flags &= ~AUTOS_TAG;
   stack = oldstack;
   action = action_cpy;
   current_players = pcount;

   t = time(0);
   ts = localtime(&t);

   if (t%ONE_DAY == BACKUP_TIME) {  
	do_backup();
	}	
   /*
    * if (!account_wobble && (ts->tm_hour)==0) { account_wobble=1;
    * do_birthdays(); }
    */
   if (account_wobble == 1 && ((ts->tm_hour) == 3))
   {
      account_wobble = 2;
      /* system("bin/account &"); */
   }
   if (account_wobble == 2 && ((ts->tm_hour) > 3))
      account_wobble = 1;
}

/* the help system (aargh argh argh) */


/* look through all possible places to find a bit of help */

struct command *find_help(char *str)
{
   struct command *comlist;
   if (isalpha(*str))
      comlist = coms[((int) (tolower(*str)) - (int) 'a' + 1)];
   else
      comlist = coms[0];

   for (; comlist->text; comlist++)
      if (do_match(str, comlist))
    return comlist;
   comlist = help_list;
   if (!comlist)
      return 0;
   for (; comlist->text; comlist++)
      if (do_match(str, comlist))
    return comlist;

   return 0;
}


void            next_line(file * hf)
{
   while (hf->length > 0 && *(hf->where) != '\n')
   {
      hf->where++;
      hf->length--;
   }
   if (hf->length > 0)
   {
      hf->where++;
      hf->length--;
   }
}

void            init_help()
{
   file            hf;
   struct command *found, *hstart;
   char           *oldstack, *start, *scan;
   int             length;
   oldstack = stack;


   if (sys_flags & VERBOSE)
      log("boot", "Loading help pages");


   if (help_list)
      free(help_list);
   help_list = 0;

   if (help_file.where)
      free(help_file.where);
   help_file = load_file("doc/help");
   hf = help_file;

   align(stack);
   hstart = (struct command *) stack;

   while (hf.length > 0)
   {
      while (hf.length > 0 && *(hf.where) != ':')
    next_line(&hf);
      if (hf.length > 0)
      {
    scan = hf.where;
    next_line(&hf);
    *scan++ = 0;
    while (scan != hf.where)
    {
       start = scan;
       while (*scan != ',' && *scan != '\n')
	  scan++;
       *scan++ = 0;
       found = find_help(start);
       if (!found)
       {
	  found = (struct command *) stack;
	  stack += sizeof(struct command);
	  found->text = start;
	  found->function = 0;
	  found->level = 0;
	  found->andlevel = 0;
	  found->space = 1;
	  found->section = 0;
       }
       found->help = hf.where;
    }
      }
   }
   *(hf.where - 1) = 0;
   found = (struct command *) stack;
   stack += sizeof(struct command);
   found->text = 0;
   found->function = 0;
   found->level = 0;
   found->andlevel = 0;
   found->space = 0;
   found->help = 0;
   found->section = 0;
   length = (int) stack - (int) hstart;
   help_list = (struct command *) malloc(length);
   memcpy(help_list, hstart, length);
   stack = oldstack;
}


/* load that help file in  */

int             get_help(player * p, char *str)
{
   int             fail = 0;
   file            text;
   char           *oldstack;

   oldstack = stack;
   if (*str == '.')
      return 0;

   sprintf(stack, "doc/%s.help", str);
   stack = end_string(stack);
   text = load_file_verbose(oldstack, 0);
   if (text.where)
   {
      if (*(text.where))
      {
    stack = oldstack;
    sprintf(stack, "-----| Online Help System |-----------------------"
       "-------------------------\n%s\n---------------------------------"
       "------------------------------------------\n", text.where);
    stack = end_string(stack);
    pager(p, oldstack, 1);
    fail = 1;
      } else
    fail = 0;
      free(text.where);
   }
   stack = oldstack;
   return fail;
}



int             get_victim(player * p, char *text)
{
   return 0;
}


/* the help command */

void help(player * p, char *str)
{
   char *oldstack;
   struct command *fn, *comlist;
   oldstack = stack;

   if (!*str)
   {
      if (p->residency)
	 str = "general";
      else
	 str = "newbie";
   }
   /* so ressies can see "help su" */
   if (!strcasecmp(str, "su")) {
	str = "superuser";
	}
   if (isalpha(*str))
      comlist = coms[((int) (tolower(*str)) - (int) 'a' + 1)];
   else
      comlist = coms[0];
   
   /* Here it is - check person's privs before helping them =) */
   for(; comlist->text; comlist++)
      if(!strcmp(comlist->text, str))
	 if((!(p->residency & comlist->level)) && comlist->level != 0) 
		str = " ";
   fn = find_help(str);
   if (!fn || !(fn->help))
   {
      if (get_help(p, str))
	 return;
      sprintf(stack, " Cannot find any help on that subject. \n");
      stack = end_string(stack);
      if (p->custom_flags & NO_PAGER)
	 tell_player(p, oldstack);
      else
	 pager(p, oldstack, 0);
      stack = oldstack;
      return;
   }
   if (!strcasecmp(str, "newbie"))
      if (get_victim(p, fn->help))
      {
	 stack = oldstack;
	 return;
      }
    sprintf(stack, "-----| Online Help System |-----------------------"
       "-------------------------\n%s\n---------------------------------"
       "------------------------------------------\n", fn->help);
   stack = end_string(stack);
   if (p->custom_flags & NO_PAGER)
      tell_player(p, oldstack);
   else
      pager(p, oldstack, 0);
   stack = oldstack;
}


void forcehelp(player * p, char *str)
{
player *p2;
char *temp;

temp = next_space(str);
*temp++ = 0;

if (!*str)
  {
   tell_player(p, " Format: forcehelp <player> <help file>\n");
   return;
  }

p2 = find_player_global(str);
if (!p2) return;

tell_player(p2, " -=> I think you need to read this ... \n");
help (p2, temp);

SUWALL(" -=*> %s shows help '%s' to %s\n", p->name, temp, p2->name);
LOGF("forcehelp", "%s forcehelpped '%s' to %s.", p->name, temp, p2->name);
}

void redtape(player * p, char *str)
{
player *p2;
char *temp, *oldstack;

temp = next_space(str);
*temp++ = 0;

oldstack = stack;
if ((!*str) || (!*temp))
  {
   tell_player(p, " Format: redtape <git> <SuperUser/Minister/etc>\n");
   return;
  }

p2 = find_player_global(str);
if (!p2) return;

sprintf(stack, "\n -=*> %s may be able to help you better. \n\n", temp);
stack = end_string(oldstack);
command_type |= HIGHLIGHT;
tell_player(p2, oldstack);
command_type &= ~HIGHLIGHT;
stack = oldstack;

sprintf(stack, " -=*> %s pushes %s towards %s\n", p->name, p2->name, temp);
stack = end_string(oldstack);
su_wall (oldstack);
stack = oldstack;
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!   WARNING   !!!!!!!!!!!!!!!!!!!
   Modifying this function in ANY way without the permission of at least
   2 of the following (trap, astyanax, vallie, nogard) is a blatent 
   violation of the licence under which you obtained this program.
 */


void pg_version(player *p, char *str) {

TELLPLAYER(p, "\n"
	      "   This talker is based on Playground 96: ver. 5.0    \n"
	      "        by traP, astyanax, vallie, and Nogard.        \n" 
	      "     Playground 96 was itself based on Summink by     \n" 
              " Athanasius, which was based on EW-too by Simon Marsh \n"  
	      "\n");
}
