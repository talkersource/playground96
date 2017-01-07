/*
 * talking.c
 */
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#include "config.h"
#include "player.h"
#include "fix.h"

/* externs */

extern void     destroy_player(), save_player(), password_mode_on(),
                password_mode_off(), sub_command(), extract_pipe_global(), tell_room(),
                extract_pipe_local(), pstack_mid(), suwall();
extern player  *find_player_global(), *find_player_absolute_quiet(),
               *find_player_global_quiet();
extern char    *end_string(), *tag_string(), *next_space(), *do_pipe(), *full_name(),
               *caps(), *sys_time();
extern int      global_tag();
extern saved_player *find_saved_player();
extern char    *idlingstring();
extern struct command check_list[];
extern void     channelsay(), channelemote(), channelsing(), channelthink();
extern list_ent *fle_from_save();
extern room     *colony;
#ifdef TRACK
extern int addfunction(char *);
#endif
/* fuckin interns too */
void emote_shout(), echo_shout(), sing_shout(), newsetpw1(), emote(), remote_cmd();
/* if you like don't like em, just disable em :P */
#include "social.h"  

/* this fxn determines if there should be a space between p->name and str
   -- default characters to be like this are ' (39) and ,   -- adding others
   that you deem appropriate should be very simple. */

int  emote_no_break(char x) {

	if (x == 39 || x == ',')
		return 1;
	return 0;
}

void do_format(player *p, player *s, char *str, char *mid, char *pipe, int type) {

   char tname[MAX_PRETITLE + MAX_NAME + 3];

	if (s->custom_flags & (NOPREFIX | NOEPREFIX))
  		strcpy(tname, p->name);
 	else
  		strcpy(tname, full_name(p));

   switch (type) {
       case 0:   /* say */
         if (s->custom_flags & NOPREFIX)
            sprintf(stack, "%s %s '%s^N'\n", p->name, mid, pipe);
         else
            sprintf(stack, "%s %s '%s^N'\n", full_name(p), mid, pipe);
	 break;
       case 1:   /* emote, pemote, socials */
  	 if (emote_no_break(*str))
    	    sprintf(stack, "%s%s^N\n", tname, pipe);
  	 else
     	    sprintf(stack, "%s %s^N\n", tname, pipe);
	 break;
       case 2:   /* echo */
    	    sprintf(stack, "%s^N\n", pipe);
	 break;
       case 3:   /* think */
         if (s->custom_flags & NOPREFIX)
            sprintf(stack, "%s think%s . o O ( %s ^N)\n", p->name, single_s(p), pipe);
         else
            sprintf(stack, "%s think%s . o O ( %s ^N)\n", full_name(p), single_s(p), pipe);
         break;
       case 4:  /* sing */
      	 if (s->custom_flags & NOPREFIX)
		sprintf(stack, "%s sing%s o/~ %s ^No/~\n", p->name, single_s(p), pipe);
      	 else
		sprintf(stack, "%s sing%s o/~ %s ^No/~\n", full_name(p), single_s(p), pipe);
	 break;
       default:
	 break;
       }
       stack = end_string(stack);
}

int	send_to_room(player * p, char *str, char *mid, int type) {

   char *prepipe, *pipe, *text;
   player *s;

   for (s = p->location->players_top; s; s = s->room_next)
   {
      if (s != current_player)
      {
         prepipe = stack;
         pipe = do_pipe(s, str);
         if (!pipe)
         {
            cleanup_tag(pipe_list, pipe_matches);
   	    sys_color_atm = SYSsc;
            return 0;
         }
         text = stack;
		do_format(p, s, str, mid, pipe, type);	
         tell_player(s, text);
         stack = prepipe;
      }
   }
   return 1;
}

int send_to_everyone (player *p, char *str, char *mid, int type) {

   player *s;
   char *pipe, *prepipe, *text;

   for (s = flatlist_start; s; s = s->flat_next)
   {
      if (s != current_player)
      {
         prepipe = stack;
         pipe = do_pipe(s, str);
         if (!pipe)
         {
            cleanup_tag(pipe_list, pipe_matches);
   	    sys_color_atm = SYSsc;
	    return 0;
         }
         text = stack;
		do_format(p, s, str, mid, pipe, type);	
         tell_player(s, text);
         stack = prepipe;
      }
   }
   return 1;
}

void     process_review(player *p, char *message, int length)
{
     int i, j;

     for (i=0;i<MAX_HISTORY_LINES;i++) /* count from 0 to 5 */
     {
       if (strlen(p->rev[i].review) < 1) /* if the element is empty */
       {
         strncpy(p->rev[i].review,message, length); /* store the data */
         return;
       }
     }

    for (j=0; j < MAX_REVIEW; j++)
        p->rev[0].review[j] = 0;
    for (i=0;i<(MAX_HISTORY_LINES - 1);i++) {
      strncpy(p->rev[i].review, p->rev[i+1].review, strlen(p->rev[i+1].review));
      for (j=0; j < MAX_REVIEW; j++)
        p->rev[i+1].review[j] = 0;
	}
    strncpy(p->rev[i].review, message, length);
/*
    strcpy(p->rev[0].review,p->rev[1].review); 
    strcpy(p->rev[1].review,p->rev[2].review);
    strcpy(p->rev[2].review,p->rev[3].review);
    strcpy(p->rev[3].review,p->rev[4].review);
    strcpy(p->rev[4].review,message);
*/
}

void     view_review(player *p)
   {
     int i, sanity=0;
     char *oldstack;

     tell_player(p,"\n");
     oldstack = stack;
     for (i=0;i<MAX_HISTORY_LINES;i++)
     {
       if (strlen(p->rev[i].review) > 0)
       {
         sanity++;
         command_type |= HIGHLIGHT;
	 sprintf(stack, " -)%s",p->rev[i].review);
	 stack = end_string(stack);
	 tell_player(p, oldstack);
	 stack = oldstack;
         command_type &= ~HIGHLIGHT;
       }
     }
    if (sanity==0) 
	tell_player(p," There is nothing in your review buffer.\n");
}

void     reportto(player *p, char *str)
   {
     char   *oldstack;
     player *p2;
     int i, sanity=0;

   #ifdef TRACK
     sprintf(functionin, "reportto (%s , SOMETHING)", p->name);
     addfunction(functionin);
   #endif

     if (!*str)
     {
       tell_player(p, " Format : reportto <superuser>\n");
       return;
     }
     p2 = find_player_global(str);
     if (!p2)
       return;

     if (p2 == p)
     {
       tell_player(p, " Sorry, you cant report to yourself silly!\n");
       return;
     }

     if (!(p2->residency & SU))
     {
       tell_player(p, " Sorry, you can only report to superusers..\n");
       return;
     }

     for (i=0;i<MAX_HISTORY_LINES;i++)
     {
       if (strlen(p->rev[i].review) > 0)
        {
          sanity++;
        }
     }

     if (sanity==0)
     {
       tell_player(p," There is nothing in your review buffer to report!\n");
       return;
     }
     else
     {
       oldstack = stack;
       sprintf(stack, " -=*> %s is reporting to %s\n",p->name,p2->name);
       stack = end_string(stack);
       su_wall(oldstack);
       command_type = PERSONAL|NO_HISTORY; 
       stack = oldstack;
       sprintf(stack, "%s reported to %s",p->name,p2->name);
       stack = end_string(stack);
       log("reportto", oldstack);
       stack = oldstack;
       sprintf(stack,"\n -=> %s is reporting the following to you:\n",p->name);
       stack = end_string(stack);
       tell_player(p2, oldstack);
       stack=oldstack;
     }

     for (i=0;i<MAX_HISTORY_LINES;i++)
     {
       if (strlen(p->rev[i].review) > 0)
       {
         sanity++;
         sprintf(stack, " -)%s",p->rev[i].review);
	 stack = strchr(stack, 0);
       }
     }
     stack = end_string(stack);
     tell_player(p2, oldstack);
     stack = oldstack;
     command_type = 0;
     sprintf(stack, " You have reported to %s. Contact them for info.\n", p2->name);
     stack = end_string(stack);
     tell_player(p, oldstack);
     stack = oldstack;
    }


/* emergency command */

void            emergency(player * p, char *str)
{
   char           *oldstack;
   oldstack = stack;

#ifdef TRACK
   sprintf(functionin,"emergency (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->script)
   {
      if (!strcasecmp(str, "off") || !strcasecmp(str, "stop"))
      {
         sprintf(stack, " Time is now %s.\n"
                        " Scripting stopped at your request.\n",
                convert_time(time(0)));
         stack = end_string(stack);
         tell_player(p, oldstack);
         p->script = 0;
         stack = oldstack;
         return;
      }
      tell_player(p, " You are already scripting ... use 'emergency stop' to "
        "stop.\n");
      return;
   }
   if (!*str)
   {
      tell_player(p, " You must give a reason for starting emergency scripting"
        " as an argument.\n"
        " (And the reason better be good ...)\n");
      return;
   } else if (!strcasecmp(str, "stop"))
   {
      tell_player(p, " OK, stop being silly, you hadn't yet STARTED emergency"
                      " scripting. Read 'help emergency' to learn how to use"
                      " 'emergency' properly.\n");
      return;
   }
   command_type = EMERGENCY;
#ifdef PC
   sprintf(stack, "logs\\emergency\\%s_emergency.log", p->lower_name);
#else
   sprintf(stack, "logs/emergency/%s.emergency", p->lower_name);
#endif
   unlink(stack);
   stack = oldstack;
   p->script = 60;
   sprintf(stack, " Emergency scripting started for 60 seconds.\n"
      " Remember, any previous scripts will be deleted\n"
      " Reason given : %s\n"
      " Time is now %s.\n", str, convert_time(time(0)));
   stack = end_string(stack);
   tell_player(p, oldstack);
   sprintf(oldstack, " -=> %s has started emergency scripting with reason "
                     "\'%s\'.\n",
           p->name, str);
   stack = end_string(oldstack);
   su_wall(oldstack);
   stack = oldstack;
}


/* converse mode on and off */

void            converse_mode_on(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"converse_mode_on (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->custom_flags & CONVERSE)
   {
      tell_player(p, " But you are already in converse mode!\n");
      return;
   }
   p->custom_flags |= CONVERSE;
   tell_player(p, " Entering 'converse' mode. Everything you type will get"
                  " said.\n"
                  " Start the line with '/' to use normal commands, and /end"
                  " to\n"
                  " leave this mode.\n");
   p->mode |= CONV;
   return;
}

void            converse_mode_off(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"converse_mode_off (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!(p->custom_flags & CONVERSE))
   {
      tell_player(p, " But you are not in converse mode !\n");
      return;
   }
   p->custom_flags &= ~CONVERSE;
   p->mode &= ~CONV;
   tell_player(p, " Ending converse mode.\n");
   return;
}




/* say command */

void            say(player * p, char *str)
{
   char           *oldstack, *mid, *scan, *temp, *pipe, *text;
   char 	  msg[]="I want my rattle fixed.";

#ifdef TRACK
   sprintf(functionin,"say (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   command_type = ROOM;
   temp = str;

   if (!*str)
   {
      tell_player(p, " Format: say <msg>\n");
      return;
   }

   if ((p->flags & FROGGED) && (p->location != prison))
      str = (char *)&msg;
   extract_pipe_local(str);

   if (sys_flags & FAILED_COMMAND)
   {
      sys_flags &= ~FAILED_COMMAND;
      str = temp;
      return;
   }
   for (scan = str; *scan; scan++);
   if (p->gender!=PLURAL)
     {
       switch (*(--scan))
	 {
	 case '?':
	   mid = "asks";
	   break;
	 case '!':
	   mid = "exclaims";
	   break;
	 default:
	   mid = "says";
	   break;
	 }
     }
   else
     {
       switch (*(--scan))
	 {
	 case '?':
	   mid = "ask";
	   break;
	 case '!':
	   mid = "exclaim";
	   break;
	 default:
	   mid = "say";
	   break;
	 }
     }
     
   
   sys_color_atm = ROMsc;
	if (!send_to_room(p, str, mid, 0)) {
		temp = str;
		return;
	}		
   sys_color_atm = SYSsc;
   pipe = do_pipe(p, str);
   if (!pipe)
   {
      cleanup_tag(pipe_list, pipe_matches);
      stack = oldstack;
      str = temp;
      return;
   }
   switch (*scan)
   {
      case '?':
         mid = "ask";
         break;
      case '!':
         mid = "exclaim";
         break;
      default:
         mid = "say";
         break;
   }
   text = stack;
   sprintf(stack, " You %s '%s^N'\n", mid, pipe);
   stack = end_string(stack);
   tell_player(p, text);
   cleanup_tag(pipe_list, pipe_matches);
   stack = oldstack;
   sys_flags &= ~PIPE;
   str = temp;
}

int check_sing_ability(player *p, char *str) {

   if (p->no_sing || p->system_flags & SAVE_NO_SING)
   {
      if (p->no_sing > 0)
	 TELLPLAYER(p, " You have been prevented from singing for the next %s.\n", word_time(p->no_sing));
      else
	 TELLPLAYER(p, " You have been prevented from singing for the long haul, friend...\n");
      return 0;
   }
   if (p->location->flags & ANTISING) {
	tell_player(p, " But this is a chambered room !!\n");
	return 0;
   }
  if (p->tag_flags & SINGBLOCK) {
	tell_player(p, " You can't sing when you're ignoring other singers.\n");
	return 0;
	}
   if (p->shout_index > 60 || strstr(str, "SFSU") || strstr(str, "S F S U"))
   {
      tell_player(p, " You seem to have gotten a sore throat by singing too"
                     " much!\n");
      return 0; 
   }
   return 1;
}


/* Count  !!s and caps in a str */
int check_shout_ability(player *p, char *str) {

   if (p->tag_flags & BLOCK_SHOUT)
   {
      tell_player(p, " You can't shout whilst ignoring shouts yourself.\n");
      return 0;
   }
   if (p->location->flags & SOUNDPROOF) {
	tell_player(p, " But this is a soundproof room !!\n");
	return 0;
   }
   if (p->location == colony)
   {
      tell_player(p, " You cannot shout whilst in the bathroom..\n");
      return 0;
   }
   if (p->no_shout || p->system_flags & SAVENOSHOUT)
   {
      if (p->no_shout > 0)
         TELLPLAYER(p, " You have been prevented from shouting for the next %s.\n", word_time(p->no_shout));
      else if (p->no_shout == -1 || p->system_flags & SAVENOSHOUT)
         TELLPLAYER(p, " You have been prevented from shouting for the forseeable future.\n");
      return 0;
   }
   if (p->shout_index > 60 || strstr(str, "SFSU") || strstr(str, "S F S U"))
   {
      tell_player(p, " You seem to have gotten a sore throat by shouting too"
                     " much!\n");
      return 0; 
   }
   return 1;
}

int             count_caps(char *str)
{
   int             count = -2;
   char           *mark;

#ifdef TRACK
   sprintf(functionin,"count_caps (SOMETHING)");
   addfunction(functionin);
#endif

   for (mark = str; *mark; mark++)
      if (isupper(*mark) || *mark == '!')
         count++;
   return (count > 0 ? count : 0);
}

/* shout command */

void shout(player * p, char *str)
{
   char *oldstack, *text, *mid, *scan, *pipe, *prepipe;
   player *s;
   char *temp, msg[] = "I want my rattle fixed!!!";

#ifdef TRACK
   sprintf(functionin,"shout (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   temp = str;
   command_type = EVERYONE;


   if (!*str)
   {
      tell_player(p, " Format: shout <msg>\n");
      return;
   }
   if ( p->flags & FROGGED )
   {
      str = (char *) &msg;
   }

   if (!check_shout_ability(p, str)) {
	str = temp;
	return;
   }

   if (!(p->residency & PSU))
   {
      p->shout_index += (count_caps(str) * 2) + 20;
   }

   extract_pipe_global(str);
   if (sys_flags & FAILED_COMMAND)
   {
      sys_flags &= ~FAILED_COMMAND;
      stack = oldstack;
      str = temp;
      return;
   }
   for (scan = str; *scan; scan++);
   switch (*(--scan))
   {
      case '?':
         mid = "shouts, asking";
         break;
      case '!':
         mid = "yells";
         break;
      default:
         mid = "shouts";
         break;
   }

   sys_color_atm = SHOsc;
	if (!send_to_everyone(p, str, mid, 0)) {
		temp = str;
		return;
	}		
   sys_color_atm = SYSsc;
   pipe = do_pipe(p, str);
   if (!pipe)
   {
      cleanup_tag(pipe_list, pipe_matches);
      stack = oldstack;
      str = temp;
      return;
   }
   switch (*scan)
   {
      case '?':
         mid = "shout, asking";
         break;
      case '!':
         mid = "yell";
         break;
      default:
         mid = "shout";
         break;
   }
   text = stack;
   sprintf(stack, " You %s '%s^N'\n", mid, pipe);
   stack = end_string(stack);
   tell_player(p, text);
   cleanup_tag(pipe_list, pipe_matches);
   stack = oldstack;
   sys_flags &= ~PIPE;
   str = temp;
}


/* emote command */

void            emote(player * p, char *str)
{
  char *oldstack, *text, *mid, *pipe;
  char tname[MAX_PRETITLE + MAX_NAME + 3];
  char *temp, msg[]="shakes a broken rattle";
  
#ifdef TRACK
  sprintf(functionin,"emote (%s , SOMETHING)",p->name);
  addfunction(functionin);
#endif
  
  temp = str;
  oldstack = stack;
  
  command_type = ROOM;
  
  if (!*str)
    {
      tell_player(p, " Format: emote <msg>\n");
      return;
    }
  if ((p->flags & FROGGED) && (p->location != prison))
    str = (char *)&msg;

  extract_pipe_local(str);
  if (sys_flags & FAILED_COMMAND)
    {
      sys_flags &= ~FAILED_COMMAND;
      str = temp;
      return;
    }
  sys_color_atm = ROMsc;
	if (!send_to_room(p, str, 0, 1)) {
		temp = str;
		return;
	}		
  sys_color_atm = SYSsc;
  pipe = do_pipe(p, str);
  if (!pipe)
    {
      cleanup_tag(pipe_list, pipe_matches);
      stack = oldstack;
      str = temp;
      return;
    }
  text = stack;
  if (p->custom_flags & (NOPREFIX | NOEPREFIX))
    strcpy(tname, p->name);
  else
    strcpy(tname, full_name(p));
  if (emote_no_break(*str))
    sprintf(stack, " You emote : %s%s\n", tname, pipe);
  else
    sprintf(stack, " You emote : %s %s\n", tname, pipe);
  stack = end_string(stack);
  tell_player(p, text);
  cleanup_tag(pipe_list, pipe_matches);
  stack = oldstack;
  sys_flags &= ~PIPE;
  str = temp;
}


/* echo command */

void            echo(player * p, char *str)
{
   char           *oldstack, *text, *mid, *pipe;
   char *temp, msg[] = "I want my rattle fixed.";

#ifdef TRACK
   sprintf(functionin,"echo (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   temp = str;
   command_type = ROOM | ECHO_COM;

   if (!*str)
   {
      tell_player(p, " Format: echo <msg>\n");
      return;
   }

   extract_pipe_local(str);
   if (p->flags & FROGGED)
      str = (char *) &msg;
   if (sys_flags & FAILED_COMMAND)
   {
      sys_flags &= ~FAILED_COMMAND;
      return;
   }

   sys_color_atm = ROMsc;
	if (!send_to_room(p, str, 0, 2)) {
		temp = str;
		return;
	}		
   sys_color_atm = SYSsc;
   pipe = do_pipe(p, str);
   if (!pipe)
   {
      cleanup_tag(pipe_list, pipe_matches);
      stack = oldstack;
      return;
   }
   text = stack;
   sprintf(stack, " You echo : %s\n", pipe);
   stack = end_string(stack);
   tell_player(p, text);
   cleanup_tag(pipe_list, pipe_matches);
   stack = oldstack;
   sys_flags &= ~PIPE;
}


/* the tell command */

void            tell(player * p, char *str)
{
   char           *msg, *pstring, *final, *mid, *scan;
   char           *oldstack;
   player        **list, **step;
   int             i, n;
   char mesg[] = "I want my rattle fixed.";

#ifdef TRACK
   sprintf(functionin,"tell (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   command_type = PERSONAL | SEE_ERROR;
   if (p->tag_flags & BLOCK_TELLS)
   {
      tell_player(p, " You can't tell other people when you yourself "
                     "are blocking tells.\n");
      return;
   }
   if (p->location->flags & ISOLATED_ROOM) {
	tell_player(p, " But this is an isolated room !!\n");
	return;
   }

   oldstack = stack;
   align(stack);
   list = (player **) stack;

   msg = next_space(str);
   if (!*msg)
   {
      tell_player(p, " Format : tell <player(s)> <msg>\n");
      return;
   }
   /* for channels -- ICK! */

   if(isdigit(*str)) {
	/* lets redirect it to the correct channel command */
	command_type = 0;
	channelsay(p, str);
	stack = oldstack;
	return;
	}	


   if (p->flags & FROGGED)
   {
      *msg = 0;
      msg = (char *) &mesg;
   } else
   {
      *msg++ = 0;
   }
   if (!(strcasecmp(str,"everyone")))
   {
      command_type = 0;
      shout(p, msg);
      return;
   }

if (!strcasecmp(str, "friends") && p->tag_flags & BLOCK_FRIENDS)
	{
	tell_player(p, " You dork. You can not friendspam when you're blocking friends \n");
	return;
	}


if (strstr(str, "friends"))
	sys_color_atm = FRTsc;
else if (strcasecmp(str, "room"))
	sys_color_atm = TELsc;
else
	sys_color_atm = ROMsc;

/* for the repeat command ... */
	p->last_remote_command = 1; 
	strncpy(p->last_remote_msg, msg, MAX_REPLY - 3);


   for (scan = msg; *scan; scan++);
   
   if (p->gender!=PLURAL)
     {
       switch (*(--scan))
	 {
	 case '?':
	   mid = "asks of";
	   break;
	 case '!':
	   mid = "exclaims to";
	   break;
	 default:
	   mid = "tells";
	   break;
	 }
     }
   else
     {
       switch (*(--scan))
	 {
	 case '?':
	   mid = "ask of";
	   break;
	 case '!':
	   mid = "exclaim to";
	   break;
	 default:
	   mid = "tell";
	   break;
	 }
     }  
   
   command_type |= SORE;
   n = global_tag(p, str);

   if (!n)
   {
      stack = oldstack;
      sys_color_atm = SYSsc;
      return;
   }
   /* for reply */

   if (strcasecmp(str, "everyone") && strcasecmp(str, "room") &&
       strcasecmp(str, "friends"))
   {
      make_reply_list(p, list, n);
   } else
   {
      p->shout_index += count_caps(str) + 17;
   }

   for (step = list, i = 0; i < n; i++, step++)
   {
      if (*step != p)
      {
         pstring = tag_string(*step, list, n);
         final = stack;
         if ((*step)->custom_flags & NOPREFIX)
            sprintf(stack, "%s %s %s '%s^N'\n", p->name, mid, pstring, msg);
         else
            sprintf(stack, "%s %s %s '%s^N'\n", full_name(p), mid, pstring, msg);
         stack = end_string(stack);
         tell_player(*step, final);
         /* process_review(*step, final); */
         stack = pstring;
      }
   }
sys_color_atm = SYSsc;
   if (sys_flags & EVERYONE_TAG || sys_flags & (FRIEND_TAG|OFRIEND_TAG)
       || sys_flags & ROOM_TAG || !(sys_flags & FAILED_COMMAND))
   {
      switch (*scan)
      {
         case '?':
            mid = "ask of";
            break;
         case '!':
            mid = "exclaim to";
            break;
         default:
            mid = "tell";
            break;
      }
      pstring = tag_string(p, list, n);
      final = stack;
      sprintf(stack, " You %s %s '%s^N'\n", mid, pstring, msg);
      stack = strchr(stack, 0);
      if (idlingstring(p, list, n))
         strcpy(stack, idlingstring(p, list, n));
      stack = end_string(stack);
      tell_player(p, final);
   }
   cleanup_tag(list, n);
   stack = oldstack;
}


/* the wake command */

void            wake(player * p, char *str)
{
   char           *oldstack;
   player         *p2;

#ifdef TRACK
   sprintf(functionin,"wake (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   command_type = PERSONAL | SEE_ERROR;
   oldstack = stack;

   if (p->tag_flags & BLOCK_TELLS)
   {
      tell_player(p, " You can't wake other people when you yourself are "
                     "blocking tells.\n");
      return;
   }
   if (!*str)
   {
      tell_player(p, " Format : wake <player>\n");
      return;
   }
   p2 = find_player_global(str);
   if (!p2)
      return;

   if (p2->tag_flags & NOBEEPS) {
     if (p2->custom_flags & NOPREFIX)
      	sprintf(stack, "!!!!!!!!!! OI !!!!!!!!!!! WAKE UP, %s wants to speak "
                     "to you.\n", p->name);
     else
      	sprintf(stack, "!!!!!!!!!! OI !!!!!!!!!!! WAKE UP, %s wants to speak "
                     "to you.\n", full_name(p));
   } else {
     if (p2->custom_flags & NOPREFIX)
      	sprintf(stack, "!!!!!!!!!! OI !!!!!!!!!!! WAKE UP, %s wants to speak "
                     "to you.\007\n", p->name);
     else
      	sprintf(stack, "!!!!!!!!!! OI !!!!!!!!!!! WAKE UP, %s wants to speak "
                     "to you.\007\n", full_name(p));
   }
   stack = end_string(stack);
   sys_color_atm = TELsc;
   tell_player(p2, oldstack);
   sys_color_atm = SYSsc;

   if (sys_flags & FAILED_COMMAND)
   {
      stack = oldstack;
      return;
   }
   stack = oldstack;
   sprintf(stack, " You scream loudly at %s in an attempt to wake %s up.\n",
      full_name(p2), get_gender_string(p2));
   stack = strchr(stack, 0);
   if (p2->idle_msg[0] != 0)
      sprintf(stack, " Idling> %s %s\n", p2->name, p2->idle_msg);
   stack = end_string(stack);
   tell_player(p, oldstack);

   stack = oldstack;
}

void nopropose (player *p, char *str)
{

if (p->system_flags & MARRIED)
	{
	tell_player (p, " You're married, dont worry so much! :) \n");
	return;
	}
else if (p->tag_flags & NO_PROPOSE)
	{
	tell_player (p, " You are seeing proposals again. \n");
	p->tag_flags &= ~NO_PROPOSE;
	return;
	}
else
	{
	tell_player (p, " You are now blocking proposals. \n");
	p->tag_flags |= NO_PROPOSE;
	return;
	}
}


/* remote command */

void remote_cmd(player * p, char *str, int manual)
{
   char *msg, *pstring, *final;
   char *oldstack;
   player **list, **step;
   int i, n;
   char tname[MAX_NAME + MAX_PRETITLE + 3];
   char mesg[] = "shakes a broken rattle.";

#ifdef TRACK
   sprintf(functionin,"remote (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   command_type = PERSONAL | SEE_ERROR;

   if (p->tag_flags & BLOCK_TELLS)
   {
      tell_player(p, " You can't remote to other people when you yourself are"
                     " blocking tells.\n");
      return;
   }
   if (p->location->flags & ISOLATED_ROOM) {
	tell_player(p, " But this is an isolated room !!\n");
	return;
   }
   oldstack = stack;
   align(stack);
   list = (player **) stack;

   msg = next_space(str);
   if (!*msg)
   {
      tell_player(p, " Format : remote <player(s)> <msg>\n");
      stack = oldstack;
      return;
   }
   if(isdigit(*str)) {
	/* lets redirect it to the correct channel command */
	command_type = 0;
	channelemote(p, str);
	stack = oldstack;
	return;
	}	
   if ( p->flags & FROGGED )
   {
      *msg = 0;
      msg = (char *)&mesg;
   } else
   {
      *msg++ = 0;
   }
   if (!(strcasecmp(str, "everyone")))
   {
	command_type = 0;
	emote_shout(p, msg);
      return;
   }
   command_type |= SORE;
if (!strcasecmp(str, "friends") && p->tag_flags & BLOCK_FRIENDS)
	{
	tell_player(p, " You dork. You can not friendspam when you're blocking friends \n");
	return;
	}

if (strstr(str, "friends"))
	sys_color_atm = FRTsc;
else if (strcasecmp(str, "room"))
	sys_color_atm = TELsc;
else
	sys_color_atm = ROMsc;


/* for the repeat command ... */
	p->last_remote_command = 2; 
	strncpy(p->last_remote_msg, msg, MAX_REPLY - 3);


   n = global_tag(p, str);
   if (!n)
   {
      stack = oldstack;
      sys_color_atm = SYSsc;
      return;
   }
   for (step = list, i = 0; i < n; i++, step++)
   {
      if ((*step)->custom_flags & (NOPREFIX | NOEPREFIX))
         strcpy(tname, p->name);
      else
         strcpy(tname, full_name(p));
      if (*step != p)
      {
         final = stack;
         if (emote_no_break(*msg))
            sprintf(stack, "%s%s^N\n", tname, msg);
         else
            sprintf(stack, "%s %s^N\n", tname, msg);
         stack = end_string(stack);
         tell_player(*step, final);
         /* process_review(*step, final); */
         stack = final;
      }
   }
sys_color_atm = SYSsc;
 if (manual) {
   if (sys_flags & EVERYONE_TAG || sys_flags & (FRIEND_TAG|OFRIEND_TAG)
       || sys_flags & ROOM_TAG || !(sys_flags & FAILED_COMMAND))
   {
      pstring = tag_string(p, list, n);
      final = stack;
      if (p->custom_flags & (NOPREFIX | NOEPREFIX))
         strcpy(tname, p->name);
      else
         strcpy(tname, full_name(p));
      if (emote_no_break(*msg))
         sprintf(stack, " You emote '%s%s^N' to %s.\n", tname, msg, pstring);
      else
         sprintf(stack, " You emote '%s %s^N' to %s.\n", tname, msg, pstring);
      stack = strchr(stack, 0);
      if (idlingstring(p, list, n))
         strcpy(stack, idlingstring(p, list, n));
      stack = end_string(stack);
      tell_player(p, final);
   }
 }
   cleanup_tag(list, n);
   stack = oldstack;
}

/* manual use of remote (socials go through with p, str, 0) */
void remote(player *p, char *str) {
	remote_cmd(p, str, 1);
}
/* recho command */

void            recho(player * p, char *str)
{
   char           *msg, *pstring, *final;
   char           *oldstack;
   player        **list, **step;
   int             i, n;
   char mesg[] = "I want my rattle fixed.";

#ifdef TRACK
   sprintf(functionin,"recho (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   command_type = PERSONAL | ECHO_COM | SEE_ERROR;

   if (p->tag_flags & BLOCK_TELLS)
   {
      tell_player(p, " You can't recho to other people when you yourself are "
                     "blocking tells.\n");
      return;
   }
   if (p->location->flags & ISOLATED_ROOM) {
	tell_player(p, " But this is an isolated room !!\n");
	return;
   }
   oldstack = stack;
   align(stack);
   list = (player **) stack;

   msg = next_space(str);
   if (!*msg)
   {
      tell_player(p, " Format : recho <player(s)> <msg>\n");
      stack = oldstack;
      return;
   }
   if ( p->flags & FROGGED )
   {
      *msg = 0;
      msg = (char *) &mesg;
   } else
      *msg++ = 0;
   command_type |= SORE;
   if (!(strcasecmp(str, "everyone"))) {
	command_type = 0;
	echo_shout(p, msg);
	return;
	}

if (!strcasecmp(str, "friends") && p->tag_flags & BLOCK_FRIENDS)
	{
	tell_player(p, " You dork. You can not friendspam when you're blocking friends \n");
	return;
	}

if (strstr(str, "friends"))
	sys_color_atm = FRTsc;
else if (strcasecmp(str, "room"))
	sys_color_atm = TELsc;
else
	sys_color_atm = ROMsc;


/* for the repeat command ... */
	p->last_remote_command = 3; 
	strncpy(p->last_remote_msg, msg, MAX_REPLY - 3);



   n = global_tag(p, str);
   if (!n)
   {
      stack = oldstack;
      sys_color_atm = SYSsc;
      return;
   }
   for (step = list, i = 0; i < n; i++, step++)
   {
      if (*step != p)
      {
         final = stack;
         sprintf(stack, "%s^N\n", msg); 
         stack = end_string(stack);
         tell_player(*step, final);
         /* process_review(*step, final); */
         stack = final;
      }
   }
	sys_color_atm = SYSsc;
   if (sys_flags & EVERYONE_TAG || !(sys_flags & FAILED_COMMAND))
   {
      pstring = tag_string(p, list, n);
      final = stack;
      sprintf(stack, " You echo '%s^N' to %s\n", msg, pstring);
      while (*stack)
         stack++;
      if (idlingstring(p, list, n))
         strcpy(stack, idlingstring(p, list, n));
      stack = end_string(stack);
      tell_player(p, final);
   }
   cleanup_tag(list, n);
   stack = oldstack;
}

/* whisper command */

void            whisper(player * p, char *str)
{
  char           *oldstack, *msg, *everyone, *text, *pstring, *mid, *s;
  player        **list, *scan;
  int             n;
  char mesg[] = "I want my rattle fixed.";
  
#ifdef TRACK
  sprintf(functionin,"whisper (%s , SOMETHING)",p->name);
  addfunction(functionin);
#endif
  
  if (p->tag_flags & BLOCK_TELLS)
    {
      tell_player(p, " You can't whisper to other people when you yourself "
		  "are blocking tells.\n");
      return;
    }
  command_type = ROOM | SEE_ERROR;
  
  oldstack = stack;
  align(stack);
  list = (player **) stack;
  
  msg = next_space(str);
  if (!*msg)
    {
      tell_player(p, " Format whisper <person(s)> <msg>\n");
      stack = oldstack;
      return;
    }
  if ( p->flags & FROGGED )
    {
      *msg = 0;
      msg = (char *) &mesg;
  } else
    *msg++ = 0;
  for (s = msg; *s; s++);



/* for the repeat command ... */
	p->last_remote_command = 4; 
	strncpy(p->last_remote_msg, msg, MAX_REPLY - 3);



  if (p->gender==PLURAL)
    {
      switch (*(--s))
	{
	case '?':
	  mid = "ask in a whisper";
	  break;
	case '!':
	  mid = "exclaim in a whisper";
	  break;
	default:
	  mid = "whisper";
	  break;
	}
    }
  else
    {
      switch (*(--s))
        {
        case '?':
          mid = "asks in a whisper";
          break;
        case '!':
          mid = "exclaims in a whisper";
          break;
        default:
          mid = "whispers";
          break;
        }
    }

  n = local_tag(p, str);
  if (!n) {
      sys_color_atm = SYSsc;
    return;
	}
  everyone = tag_string(0, list, n);
  for (scan = p->location->players_top; scan; scan = scan->room_next)
    if (p != scan)
      if (scan->flags & TAGGED)
	{
	  pstring = tag_string(scan, list, n);
	  text = stack;
	  if (scan->custom_flags & NOPREFIX)
	    sprintf(stack, "%s %s '%s^N' to %s.\n", p->name, mid, msg, pstring);
	  else
	    sprintf(stack, "%s %s '%s^N' to %s.\n", full_name(p), mid, msg,
		    pstring);
	  stack = end_string(stack);
          sys_color_atm = TELsc;
	  tell_player(scan, text);
          sys_color_atm = SYSsc;
          /* process_review(scan, text); */
	  stack = pstring;
      } 
  if (!(sys_flags & FAILED_COMMAND))
    {
      switch (*s)
	{
	case '?':
	  mid = "ask in a whisper";
	  break;
	case '!':
	  mid = "exclaim in a whisper";
	  break;
	default:
	  mid = "whisper";
	  break;
	}

      pstring = tag_string(p, list, n);
      text = stack;
      sprintf(stack, " You %s '%s^N' to %s.\n", mid, msg, pstring);
      stack = strchr(stack, 0);
      if (idlingstring(p, list, n))
	strcpy(stack, idlingstring(p, list, n));
      stack = end_string(stack);
      tell_player(p, text);
    }
  cleanup_tag(list, n);
  stack = oldstack;
}

/* exclude command */

void            exclude(player * p, char *str)
{
   char           *oldstack, *msg, *everyone, *text, *pstring, *mid, *s;
   player        **list, *scan;
   int             n;
   char mesg[] = "I want my rattle fixed.";

#ifdef TRACK
   sprintf(functionin,"exclude (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   command_type = ROOM | SEE_ERROR | EXCLUDE;
   oldstack = stack;
   align(stack);
   list = (player **) stack;

   msg = next_space(str);
   if (!*msg)
   {
      tell_player(p, " Format exclude <person(s)> <msg>\n");
      stack = oldstack;
      return;
   }
   if ( p->flags & FROGGED )
   {
      *msg = 0;
      msg = (char *) &mesg;
   } else
      *msg++ = 0;
   for (s = msg; *s; s++);
   switch (*(--s))
   {
      case '?':
         mid = "asks";
         break;
      case '!':
         mid = "exclaims to";
         break;
      default:
         mid = "tells";
         break;
   }
   n = local_tag(p, str);
   if (!n)
      return;
   everyone = tag_string(0, list, n);
   sys_color_atm = ROMsc;
   for (scan = p->location->players_top; scan; scan = scan->room_next)
   {
      if (p != scan)
      {
         if ((scan->flags & TAGGED) && !(scan->residency & (SU | ADMIN )))
         {
            pstring = tag_string(scan, list, n);
            text = stack;
            sprintf(stack, "%s tells everyone something about %s\n",
                    full_name(p), pstring);
            stack = end_string(stack);
            tell_player(scan, text);
            stack = pstring;
         } else
         {
            text = stack;
            sprintf(stack, "%s %s everyone but %s '%s^N'\n",
                    full_name(p), mid, everyone, msg);
            stack = end_string(stack);
            tell_player(scan, text);
            stack = text;
         }
      }
   }
   sys_color_atm = SYSsc;
   if (!(sys_flags & FAILED_COMMAND))
   {
      switch (*s)
      {
         case '?':
            mid = "ask";
            break;
         case '!':
            mid = "exclaim to";
            break;
         default:
            mid = "tell";
            break;
      }
      pstring = tag_string(p, list, n);
      text = stack;
      sprintf(stack, " You %s everyone but %s '%s^N'\n", mid, pstring, msg);
      stack = end_string(stack);
      tell_player(p, text);
   }
   cleanup_tag(list, n);
   stack = oldstack;
}


/* pemote command */

void            pemote(player * p, char *str)
{
   char           *oldstack, *scan;
   oldstack = stack;

#ifdef TRACK
   sprintf(functionin,"pemote (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: pemote <msg>\n");
      return;
   }
   if ( !(p->flags & FROGGED) )
   {
      for (scan = p->lower_name; *scan; scan++);
      if (*(scan - 1) == 's')
         *stack++ = 39;
      else
      {
         *stack++ = 39;
         *stack++ = 's';
      }
      *stack++ = ' ';
      while (*str)
         *stack++ = *str++;
      *stack++ = 0;
      emote(p, oldstack);
      stack = oldstack;
   } else
      emote(p, str);
}


/* premote command */

void premote(player * p, char *str)
{
   char *oldstack, *scan;
   oldstack = stack;

#ifdef TRACK
   sprintf(functionin,"premote (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   scan = next_space(str);
   if (!*scan)
   {
      tell_player(p, " Format: pemote <person> <msg>\n");
      return;
   }
   if ( !(p->flags & FROGGED) )
   {
      while (*str && *str != ' ')
         *stack++ = *str++;
      *stack++ = ' ';
      if (*str)
         str++;
      for (scan = p->lower_name; *scan; scan++);
      if (*(scan - 1) == 's')
         *stack++ = 39;
      else
      {
         *stack++ = 39;
         *stack++ = 's';
      }
      *stack++ = ' ';
      while (*str)
         *stack++ = *str++;
      *stack++ = 0;
      remote(p, oldstack);
      stack = oldstack;
   } else
      remote(p, str);
}


/* the 'check' command */

void            check(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"check (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: check <sub command>\n");
      return;
   }
   sub_command(p, str, check_list);
}

/* view check commands */

void            view_check_commands(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"view_check_commands (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   view_sub_commands(p, check_list);
}


/* new think, tryig to fix the NOPREFIX thang by copying from say */


void            newthink(player * p, char *str)
{
   char           *oldstack, *text, *mid, *scan, *pipe;
   char *temp, msg[]="I want my rattle fixed.";

#ifdef TRACK
   sprintf(functionin,"newthink (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   command_type = ROOM;
   temp = str;

   if (!*str)
   {
      tell_player(p, " Format: think <msg>\n");
      return;
   }
   if ( p->flags & FROGGED )
      str = (char *)&msg;
   extract_pipe_local(str);
   if (sys_flags & FAILED_COMMAND)
   {
      sys_flags &= ~FAILED_COMMAND;
      str = temp;
      return;
   }

   sys_color_atm = ROMsc;
	if (!send_to_room(p, str, 0, 3)) {
		temp = str;
		return;
	}		
   sys_color_atm = SYSsc;
   pipe = do_pipe(p, str);
   if (!pipe)
   {
      cleanup_tag(pipe_list, pipe_matches);
      stack = oldstack;
      str = temp;
      return;
   }
   text = stack;
   sprintf(stack, " You think . o O ( %s ^N)\n", pipe);
   stack = end_string(stack);
   tell_player(p, text);
   cleanup_tag(pipe_list, pipe_matches);
   stack = oldstack;
   sys_flags &= ~PIPE;
   str = temp;
}

/* tell to your friends, the short way */

void tell_friends(player *p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"tell_friends (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: tf <message>\n");
      return;
   }
   oldstack = stack;
   sprintf(stack, "friends %s", str);
   stack = end_string(stack);
   tell(p, oldstack);
   stack = oldstack;
}

/* remote to your friends, the short way */

void remote_friends(player *p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"remote_friends (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: rf <remote>\n");
      return;
   }
   oldstack = stack;
   sprintf(stack, "friends %s", str);
   stack = end_string(stack);
   remote(p, oldstack);

   stack = oldstack;
}


/* remote think */

void remote_think(player *p, char *str)
{
   char *oldstack, *msg;

#ifdef TRACK
   sprintf(functionin,"remote_think (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;

   msg = strchr(str, ' ');
   if (!msg)
   {
      tell_player(p, " Format: rt <player(s)> <think>\n");
      return;
   }

   if(isdigit(*str)) {
	/* lets redirect it to the correct channel command */
	command_type = 0;
	channelthink(p, str);
	stack = oldstack;
	return;
	}	
   *msg++ = '\0';
   sprintf(stack, "%s think%s . o O ( %s ^N)", str, single_s(p), msg);
   stack = end_string(stack);
   remote(p, oldstack);
   stack = oldstack;
}

/* the shoutted emote, pemote, think, and echo... */

void emote_shout(player * p, char *str)
{
   char *oldstack, *text, *pipe, *prepipe;
   player *s;
   char *temp, msg[] = "shakes a broken rattle";

#ifdef TRACK
   sprintf(functionin,"emote_shout (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   temp = str;
   command_type = EVERYONE;

   if (!*str)
   {
      tell_player(p, " Format: yemote <msg>\n");
      return;
   }
   if ( p->flags & FROGGED )
   {
      str = (char *) &msg;
   }
   if (!check_shout_ability(p, str)) {
	str = temp;
	return;
   }
   if (!(p->residency & PSU))
   {
      p->shout_index += (count_caps(str) * 2) + 20;
   }

   extract_pipe_global(str);
   if (sys_flags & FAILED_COMMAND)
   {
      sys_flags &= ~FAILED_COMMAND;
      stack = oldstack;
      str = temp;
      return;
   }

   sys_color_atm = SHOsc;
	if (!send_to_everyone(p, str, 0, 1)) {
		temp = str;
		return;
	}		
   sys_color_atm = SYSsc;
   pipe = do_pipe(p, str);
   if (!pipe)
   {
      cleanup_tag(pipe_list, pipe_matches);
      stack = oldstack;
      str = temp;
      return;
   }
   text = stack;
   if (p->custom_flags & (NOPREFIX | NOEPREFIX))
   {  if (emote_no_break(*str)) 
	   sprintf(stack, " You yemote: %s%s\n", p->name, pipe);
      else sprintf(stack, " You yemote: %s %s\n", p->name, pipe); }
   else
   {  if (emote_no_break(*str)) 
	   sprintf(stack, " You yemote: %s%s\n", full_name(p), pipe);
      else sprintf(stack, " You yemote: %s %s\n", full_name(p), pipe); }
   stack = end_string(stack);
   tell_player(p, text);
   cleanup_tag(pipe_list, pipe_matches);
   stack = oldstack;
   sys_flags &= ~PIPE;
   str = temp;
}
void            pemote_shout(player * p, char *str)
{
   char           *oldstack, *scan;
   oldstack = stack;

#ifdef TRACK
   sprintf(functionin,"pemote_shout (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: pyemote <msg>\n");
      return;
   }
   if ( !(p->flags & FROGGED) )
   {
      for (scan = p->lower_name; *scan; scan++);
      if (*(scan - 1) == 's')
	 *stack++ = 39;
      else
      {
	 *stack++ = 39;
	 *stack++ = 's';
      }
      *stack++ = ' ';
      while (*str)
	 *stack++ = *str++;
      *stack++ = 0;
      emote_shout(p, oldstack);
      stack = oldstack;
   } else
      emote_shout(p, str);
}
void think_shout(player * p, char *str)
{
   char *oldstack, *text, *pipe, *prepipe;
   player *s;
   char *temp, msg[] = "I want my rattle fixed";

#ifdef TRACK
   sprintf(functionin,"think_shout (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   temp = str;
   command_type = EVERYONE;

   if (!*str)
   {
      tell_player(p, " Format: ythink <msg>\n");
      return;
   }
   if ( p->flags & FROGGED )
   {
      str = (char *) &msg;
   }
   if (!check_shout_ability(p, str)) {
	str = temp;
	return;
   }
   if (!(p->residency & PSU))
   {
      p->shout_index += (count_caps(str) * 2) + 20;
   }

   extract_pipe_global(str);
   if (sys_flags & FAILED_COMMAND)
   {
      sys_flags &= ~FAILED_COMMAND;
      stack = oldstack;
      str = temp;
      return;
   }

   sys_color_atm = SHOsc;
	if (!send_to_everyone(p, str, 0, 3)) {
		temp = str;
		return;
	}		
   sys_color_atm = SYSsc;
   pipe = do_pipe(p, str);
   if (!pipe)
   {
      cleanup_tag(pipe_list, pipe_matches);
      stack = oldstack;
      str = temp;
      return;
   }
   text = stack;
     
   sprintf(stack, " You ythink . o O ( %s ^N)\n", pipe); 
   stack = end_string(stack);
   tell_player(p, text);
   cleanup_tag(pipe_list, pipe_matches);
   stack = oldstack;
   sys_flags &= ~PIPE;
   str = temp;
}
void echo_shout(player * p, char *str)
{
   char *oldstack, *text, *pipe, *prepipe;
   player *s;
   char *temp, msg[] = "I want my rattle fixed";

#ifdef TRACK
   sprintf(functionin,"echo_shout (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   temp = str;
   command_type = EVERYONE | ECHO_COM;

   if (!*str)
   {
      tell_player(p, " Format: yecho <msg>\n");
      return;
   }
   if ( p->flags & FROGGED )
   {
      str = (char *) &msg;
   }
   if (!check_shout_ability(p, str)) {
	str = temp;
	return;
   }
   if (!(p->residency & PSU))
   {
      p->shout_index += (count_caps(str) * 2) + 20;
   }

   extract_pipe_global(str);
   if (sys_flags & FAILED_COMMAND)
   {
      sys_flags &= ~FAILED_COMMAND;
      stack = oldstack;
      str = temp;
      return;
   }

   sys_color_atm = SHOsc;
	if (!send_to_everyone(p, str, 0, 2)) {
		temp = str;
		return;
	}		
   sys_color_atm = SYSsc;
   pipe = do_pipe(p, str);
   if (!pipe)
   {
      cleanup_tag(pipe_list, pipe_matches);
      stack = oldstack;
      str = temp;
      return;
   }
   text = stack;
   sprintf(stack, " You yecho: %s\n", pipe); 
   stack = end_string(stack);
   tell_player(p, text);
   cleanup_tag(pipe_list, pipe_matches);
   stack = oldstack;
   sys_flags &= ~PIPE;
   str = temp;
}

/* think to your friends, the short way */

void rthink_friends(player *p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"think_friends (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: rtf <message>\n");
      return;
   }
   oldstack = stack;
   sprintf(stack, "friends think%s . o O ( %s ^N)", single_s(p), str);
   stack = end_string(stack);
   remote(p, oldstack);
   stack = oldstack;
}


/* possessive emote to your friends, the short way */

void premote_friends(player *p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"pemote_friends (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: pf <message>\n");
      return;
   }
   oldstack = stack;
   sprintf(stack, "friends %s ", str);
   stack = end_string(stack);
   premote(p, oldstack);
   stack = oldstack;
}



/* echo to your friends, the short way */

void recho_friends(player *p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"think_friends (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: ef <echo>\n");
      return;
   }
   oldstack = stack;
   sprintf(stack, "friends %s", str);
   stack = end_string(stack);
   recho(p, oldstack);
   stack = oldstack;
}



void sing(player * p, char *str)
{
  char           *oldstack, *text, *mid, *scan, *pipe, *prepipe;
  player         *s;
  char *temp, msg[]="I got the broken rattle blueeeeeeeeeees";

  oldstack = stack;
  command_type = ROOM | BAD_MUSIC;
  temp = str;

  if (!*str)
  {
tell_player(p, " Format: sing <song>\n");
    return;
  }

   if (!check_sing_ability(p, str)) {
	str = temp;
	return;
   }
   if (!(p->residency & PSU))
   {
      p->shout_index += (count_caps(str) * 2) + 20;
   }
  if ( p->flags & FROGGED )
    str = (char *)&msg;
  extract_pipe_local(str);
  if (sys_flags & FAILED_COMMAND)
  {
    sys_flags &= ~FAILED_COMMAND;
    str = temp;
    return;
  }

  sys_color_atm = ROMsc; 
	if (!send_to_room(p, str, 0, 4)) {
		temp = str;
		return;
	}		
  sys_color_atm = SYSsc;
  pipe = do_pipe(p, str);
  if (!pipe)
  {
    cleanup_tag(pipe_list, pipe_matches);
    stack = oldstack;
    str = temp;
    return;
  }
  text = stack;
  sprintf(stack, " You sing o/~ %s ^No/~\n", pipe);
  stack = end_string(stack);
  tell_player(p, text);
  cleanup_tag(pipe_list, pipe_matches);
  stack = oldstack;
sys_flags &= ~PIPE;
  str = temp;
}


void rsing(player * p, char *str)
{
  char           *msg, *pstring, *final, *mid, *scan;
  char           *oldstack;
  player        **list, **step;
  int             i, n;
  char mesg[] = "I got the broken rattle blueeeeeeeeeees";
  command_type = PERSONAL | SEE_ERROR | BAD_MUSIC;

  if (p->tag_flags & BLOCK_TELLS)
 {
    tell_player(p, " You can't sing to other people when you yourself "
		"are blocking tells.\n");
    return;
  }
   if (!check_sing_ability(p, str)) {
	return;
   }
   if (p->location->flags & ISOLATED_ROOM) {
	tell_player(p, " But this is an isolated room !!\n");
	return;
   }

  oldstack = stack;
  align(stack);
  list = (player **) stack;

  msg = next_space(str);
  if (!*msg)
  {
    tell_player(p, " Format : rsing <player(s)> <song>\n");
    return;
  }

   if(isdigit(*str)) {
	/* lets redirect it to the correct channel command */
	command_type = 0;
	channelsing(p, str);
	stack = oldstack;
	return;
	}	

  if (p->flags & FROGGED)
  {
    *msg = 0;
    msg = (char *) &mesg;
  }
  else
    *msg++ = 0;
  if (!(strcasecmp(str,"everyone")))
  {
    command_type = 0;
    sing_shout(p, msg);
    return;
  }
  if (!(strcasecmp(str,"room")))
  {
    command_type = 0;
    sing(p, msg);
    return;
  }
  
if (!strcasecmp(str, "friends") && p->tag_flags & BLOCK_FRIENDS)
	{
	tell_player(p, " You dork. You can not friendspam when you're blocking friends \n");
	return;
	}

if (strstr(str, "friends"))
	sys_color_atm = FRTsc;
else if (strcasecmp(str, "room"))
	sys_color_atm = TELsc;
else
	sys_color_atm = ROMsc;

			
/* for the repeat command ... */
	p->last_remote_command = 5; 
	strncpy(p->last_remote_msg, msg, MAX_REPLY - 3);



  for (scan = msg; *scan; scan++);

  if (p->gender!=PLURAL)
    mid = "sings";
  else
      mid = "sing";

  command_type |= SORE;
  n = global_tag(p, str);

  if (!n)
  {
    stack = oldstack;
      sys_color_atm = SYSsc;
    return;
}
  /* for reply */

  if (strcasecmp(str, "everyone") && strcasecmp(str, "room") &&
      strcasecmp(str, "friends"))
    make_reply_list(p, list, n);
  else
    p->shout_index += count_caps(str) + 17;

  for (step = list, i = 0; i < n; i++, step++)
  {
    if (*step != p)
    {
      pstring = tag_string(*step, list, n);
      final = stack;
      if ((*step)->custom_flags & NOPREFIX)
	sprintf(stack, "%s %s o/~ %s ^No/~ to %s.\n", p->name, mid, msg, pstring);
      else
sprintf(stack, "%s %s o/~ %s ^No/~ to %s.\n", full_name(p), mid, msg, pstring);
      stack = end_string(stack);
      tell_player(*step, final);
         /* process_review(*step, final); */
      stack = pstring;
    }
  }
sys_color_atm = SYSsc;
  if (sys_flags & EVERYONE_TAG || sys_flags & (FRIEND_TAG|OFRIEND_TAG)
      || sys_flags & ROOM_TAG || !(sys_flags & FAILED_COMMAND))
  {
    mid = "sing";
    pstring = tag_string(p, list, n);
    final = stack;
    sprintf(stack, " You %s o/~ %s ^No/~ to %s.\n", mid, msg, pstring);
    stack = strchr(stack, 0);
    if (idlingstring(p, list, n))
      strcpy(stack, idlingstring(p, list, n));
    stack = end_string(stack);
    tell_player(p, final);
  }
  cleanup_tag(list, n);
  stack = oldstack;
}

/* sing to your friends - the easy way {B-> */
void sing_friends (player * p, char *str)
{
char *oldstack;

#ifdef TRACK
	sprintf(functionin, "sing_friends(%s, SOMETHING)", p->name);
	addfunction(functionin);
#endif

if(!*str)
	{
	tell_player(p, " Format: sf <song> \n");
	return;
	}
oldstack = stack;
sprintf(stack, "friends %s", str);
stack = end_string(stack);
rsing(p, oldstack);
stack = oldstack;
}


void sing_shout(player * p, char *str)
{
   char *oldstack, *text, *pipe, *prepipe;
   player *s;
   char *temp, msg[] = "I got the broken rattle blueeeeeeeeeees";

#ifdef TRACK
   sprintf(functionin,"sing_shout (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   temp = str;
   command_type = EVERYONE | BAD_MUSIC;

   if (!*str)
   {
      tell_player(p, " Format: ysing <song>\n");
      return;
   }
   if (!check_sing_ability(p, str)) {
	str = temp;
	return;
   }

   if (!check_shout_ability(p, str)) {
	str = temp;
	return;
   }
   if (!(p->residency & PSU))
   {
      p->shout_index += (2*((count_caps(str) * 2) + 20));
   }
   if ( p->flags & FROGGED )
   {
      str = (char *) &msg;
   }
   extract_pipe_global(str);
   if (sys_flags & FAILED_COMMAND)
   {
      sys_flags &= ~FAILED_COMMAND;
      stack = oldstack;
      str = temp;
      return;
   }

   sys_color_atm = SHOsc;
	if (!send_to_everyone(p, str, 0, 4)) {
		temp = str;
		return;
	}		
   sys_color_atm = SYSsc;
   pipe = do_pipe(p, str);
   if (!pipe)
   {
      cleanup_tag(pipe_list, pipe_matches);
      stack = oldstack;
      str = temp;
      return;
   }
   text = stack;

   sprintf(stack, " You ysing o/~ %s ^No/~\n", pipe);
   stack = end_string(stack);
   tell_player(p, text);
   cleanup_tag(pipe_list, pipe_matches);
   stack = oldstack;
   sys_flags &= ~PIPE;
   str = temp;
}

/* a kind of really LOUD tell... */

void            beep_tell(player * p, char *str)
{
   char           *msg, *pstring, *final, *mid, *scan;
   char           *oldstack;
   player        **list, **step;
   int             i, n;
   char mesg[] = "Gimme a new rattle already !!";

   command_type = PERSONAL | SEE_ERROR;

   if (p->tag_flags & BLOCK_TELLS)
   {
      tell_player(p, " You can't yell at other people when you yourself "
		     "are blocking tells.\n");
      return;
   }
   if (p->location->flags & ISOLATED_ROOM) {
	tell_player(p, " But this is an isolated room !!\n");
	return;
   }

   oldstack = stack;
   align(stack);
   list = (player **) stack;

   msg = next_space(str);
   if (!*msg)
   {
      tell_player(p, " Format : yell <player(s)> <msg>\n");
      return;
   }
   if (p->flags & FROGGED)
   {
      *msg = 0;
      msg = (char *) &mesg;
   } else
   {
      *msg++ = 0;
   }

  if (!(strcasecmp(str,"everyone")))
   {
      tell_player(p, " You can't yell at _everyone_!! \n");
      stack=oldstack;
      return;
   }

   if (!(strcasecmp(str,"room")))
   {
      tell_player(p, " You can't yell at everyone in the room! \n");
      stack=oldstack;
      return;
   }


if (!strcasecmp(str, "friends") && p->tag_flags & BLOCK_FRIENDS)
	{
	tell_player(p, " You dork. You can not friendspam when you're blocking friends \n");
	return;
	}
if (strstr(str, "friends"))
	sys_color_atm = FRTsc;
else if (strcasecmp(str, "room"))
	sys_color_atm = TELsc;
else
	sys_color_atm = ROMsc;

/* for the repeat command ... */
	p->last_remote_command = 6; 
	strncpy(p->last_remote_msg, msg, MAX_REPLY - 3);



   for (scan = msg; *scan; scan++);

   if (p->gender!=PLURAL)
     {
       switch (*(--scan))
	 {
	 case '?':
	   mid = "boggles at";
	   break;
	 case '!':
	   mid = "hollers at";
	   break;
	 default:
	   mid = "yells at";
	   break;
	 }
     }
   else
     {
       switch (*(--scan))
	 {
	 case '?':
	   mid = "boggle at";
	   break;
	 case '!':
	   mid = "holler at";
	   break;
	 default:
	   mid = "yell at";
	   break;
	 }
     }

   command_type |= SORE;
   n = global_tag(p, str);

   if (!n)
   {
      stack = oldstack;
      sys_color_atm = SYSsc;
      return;
   }
   /* for reply */

   if (strcasecmp(str, "friends"))
   {
      make_reply_list(p, list, n);
   } else
   {
      p->shout_index += count_caps(str) + 17;
   }

   for (step = list, i = 0; i < n; i++, step++)
   {
      if (*step != p)
      {
	pstring = tag_string(*step, list, n);
	final = stack;
	if ((*step)->tag_flags & NOBEEPS)
	{
	    if ((*step)->custom_flags & NOPREFIX)
	       sprintf(stack, "%s %s %s -> %s ^N<-\n", p->name, mid, pstring, msg);
	    else
	       sprintf(stack, "%s %s %s -> %s ^N<-\n", full_name(p), mid, pstring,
			msg);
	 } else {
	    if ((*step)->custom_flags & NOPREFIX)
		sprintf(stack, "%s %s %s -> %s ^N<-\007\n", p->name, mid, pstring,
			msg);
	    else
		sprintf(stack, "%s %s %s -> %s ^N<-\007\n", full_name(p), mid,
			pstring, msg);
	 }
	 stack = end_string(stack);
	 tell_player(*step, final);
         /* process_review(*step, final); */
	 stack = pstring;
      }
   }
	sys_color_atm = SYSsc;
   if (sys_flags & EVERYONE_TAG || sys_flags & (FRIEND_TAG|OFRIEND_TAG)
       || sys_flags & ROOM_TAG || !(sys_flags & FAILED_COMMAND))
   {
      switch (*scan)
      {
	 case '?':
	    mid = "boggle at";
	    break;
	 case '!':
	    mid = "holler at";
	    break;
	 default:
	    mid = "yell at";
	    break;
      }
      pstring = tag_string(p, list, n);
      final = stack;
      sprintf(stack, " You %s %s -> %s ^N<-\n", mid, pstring, msg);
      stack = strchr(stack, 0);
      if (idlingstring(p, list, n))
	 strcpy(stack, idlingstring(p, list, n));
      stack = end_string(stack);
      tell_player(p, final);
   }
   cleanup_tag(list, n);
   stack = oldstack;
}


void repeat_mistell (player * p, char *str)
{
char *oldstack;
oldstack = stack;
if (!*str)
  {
     tell_player(p, " Format: repeat <name>\n");
     stack = oldstack;
     return;
  }
sprintf(stack, "%s %s", str, p->last_remote_msg);
stack = end_string(stack);
/* hope dis works ... */
switch (p->last_remote_command)
  {
	case 1:
		/* tell */
		tell(p, oldstack);
		break;
	case 2:
		/* remote, rthink, premote */
		remote(p, oldstack);
		break;
	case 3:
		/* recho */
		recho(p, oldstack);
		break;
	case 4:
		/* whisper */
		whisper(p, oldstack);
		break;
	case 5:
		/* rsing */
		rsing(p, oldstack);
		break;
	case 6:
		/* beep_at */
		beep_tell(p, oldstack);
		break;
	default:
	       /* error */
	       tell_player(p, " Error - no previous remote command used.\n");
	       break;
	}

stack = oldstack;
}


void tell_others_friends(player *p, char *str)
{
   char *oldstack, *msg;

#ifdef TRACK
   sprintf(functionin,"tell_friends (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: to <player> <message>\n");
      return;
   }
   msg = next_space(str);
   *msg++ = 0;
   if (!*msg)
   {
      tell_player(p, " Format: to <player> <message>\n");
      return;
   }
   oldstack = stack;
   sprintf(stack, "%s.friends %s", str, msg);
   stack = end_string(stack);
   tell(p, oldstack);
   stack = oldstack;
}

void remote_others_friends(player *p, char *str)
{
   char *oldstack, *msg;

#ifdef TRACK
   sprintf(functionin,"tell_friends (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: ro <player> <message>\n");
      return;
   }
   msg = next_space(str);
   *msg++ = 0;
   if (!*msg)
   {
      tell_player(p, " Format: ro <player> <message>\n");
      return;
   }
   oldstack = stack;
   sprintf(stack, "%s.friends %s", str, msg);
   stack = end_string(stack);
   remote(p, oldstack);
   stack = oldstack;
}

void rt_others_friends(player *p, char *str)
{
   char *oldstack, *msg;

#ifdef TRACK
   sprintf(functionin,"tell_friends (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: rto <player> <message>\n");
      return;
   }
   msg = next_space(str);
   *msg++ = 0;
   if (!*msg)
   {
      tell_player(p, " Format: rto <player> <message>\n");
      return;
   }
   oldstack = stack;
   if (p->gender == PLURAL)
   	sprintf(stack, "%s.friends think . o O ( %s )", str, msg);
   else
   	sprintf(stack, "%s.friends thinks . o O ( %s )", str, msg);
   stack = end_string(stack);
   remote(p, oldstack);
   stack = oldstack;
}

void premote_others_friends(player *p, char *str)
{
   char *oldstack, *msg;

#ifdef TRACK
   sprintf(functionin,"tell_friends (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: po <player> <message>\n");
      return;
   }
   msg = next_space(str);
   *msg++ = 0;
   if (!*msg)
   {
      tell_player(p, " Format: po <player> <message>\n");
      return;
   }
   oldstack = stack;
   sprintf(stack, "%s.friends %s", str, msg);
   stack = end_string(stack);
   premote(p, oldstack);
   stack = oldstack;
}

void recho_others_friends(player *p, char *str)
{
   char *oldstack, *msg;

#ifdef TRACK
   sprintf(functionin,"tell_friends (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: eo <player> <message>\n");
      return;
   }
   msg = next_space(str);
   *msg++ = 0;
   if (!*msg)
   {
      tell_player(p, " Format: eo <player> <message>\n");
      return;
   }
   oldstack = stack;
   sprintf(stack, "%s.friends %s", str, msg);
   stack = end_string(stack);
   recho(p, oldstack);
   stack = oldstack;
}

void rsing_others_friends(player *p, char *str)
{
   char *oldstack, *msg;

#ifdef TRACK
   sprintf(functionin,"tell_friends (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: so <player> <message>\n");
      return;
   }
   msg = next_space(str);
   *msg++ = 0;
   if (!*msg)
   {
      tell_player(p, " Format: so <player> <message>\n");
      return;
   }
   oldstack = stack;
   sprintf(stack, "%s.friends %s", str, msg);
   stack = end_string(stack);
   rsing(p, oldstack);
   stack = oldstack;
}

void beepat_friends(player *p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"tell_friends (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: yf <player> <message>\n");
      return;
   }
   oldstack = stack;
   sprintf(stack, "friends %s", str);
   stack = end_string(stack);
   beep_tell(p, oldstack);
   stack = oldstack;
}

void beepat_others_friends(player *p, char *str)
{
   char *oldstack, *msg;

#ifdef TRACK
   sprintf(functionin,"tell_friends (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: yo <player> <message>\n");
      return;
   }
   msg = next_space(str);
   *msg++ = 0;
   if (!*msg)
   {
      tell_player(p, " Format: yo <player> <message>\n");
      return;
   }
   oldstack = stack;
   sprintf(stack, "%s.friends %s", str, msg);
   stack = end_string(stack);
   beep_tell(p, oldstack);
   stack = oldstack;
}

/* Go to the nudist colony, where you can be as offensive as you like */
/* Well, nearly */
void go_colony(player *p, char *str)
{
   player *p2;
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"go_colony (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if ((p->residency & SU) && (*str))
       /* Move someone else to the colony */
   {
       /* find them */
       p2 = find_player_global(str);
       if (!p2)
           return;
       else
       {
           /* no colonising your superiors... */
           if (p2->residency >= p->residency)
           {
               tell_player(p, " You try and try, but you're just to weak to"
                           " budge them!\n");
               return;
           }
           /* and they might already be there */
           if (!strcmp(p2->location->owner->lower_name, "main")
               && !strcmp(p2->location->id, "potty"))
           {
               tell_player(p, " They're already in the potty!!\n");
               return;
           }
           /* tell the SU channel */
           command_type |= ADMIN_BARGE;
           sprintf(stack, " -=> %s puts %s in the potty.\n", p->name,
                   p2->name);
           stack = end_string(stack);
           su_wall(oldstack);
           stack = oldstack;
           /* tell the player */
           tell_player(p2, " -=> You suddenly find yourself picked up out of"
                       " the room, and you fly through the air, landing in...\n");
           /* move them */
           move_to(p2, "main.potty", 0);
           /* stick them in place for 60 secs */
           p2->no_move=60;
           return;
       }
       return;
   }

   /* no argument specified or player is not an SU */
   /* check if they're in the can already */
   if (!strcmp(p->location->owner->lower_name, "main")
       && !strcmp(p->location->id, "potty"))
   {
       tell_player(p, " You're already in the potty!\n");
       return;
   }
   /* or if they're stuck */
   if (p->no_move)
   {
      tell_player(p, " You seem to be stuck to the ground.\n");
      return;
   }
   /* otherwise move them */
   move_to(p, "main.potty", 0);
}
 
