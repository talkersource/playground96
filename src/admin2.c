/*
 * admin2.c -- added to speed up compiling in general --
 * All non-su / admin routines, and some simple staff routines, have
 * been moved here
 */

#include <string.h>
#include <memory.h>
#include <time.h>
#include <malloc.h>

#include "config.h"
#include "player.h"
 
/* externs */
extern void         swap_list_names(char *, char *);
extern void         lower_case(char *);
extern char         *do_crypt(char *, player *);
extern char         *end_string(), *next_space(), *tag_string(), *bit_string();
extern player       *find_player_global(), *find_player_global_quiet(char *),
                    *create_player();
extern saved_player *find_saved_player();
extern int          remove_player_file(), set_update();
extern int          get_flag();
extern void         hard_load_one_file(), sync_to_file(), remove_entire_list(),
                    destroy_player();
extern player       *find_player_absolute_quiet(char *);
extern int          match_banish(), emote_no_break();
extern void         soft_eject(player *, char *);
extern player       *find_player_absolute_quiet(char *);
extern char         *self_string(player *p), *first_char(player *);
extern char          shutdown_reason[];
extern time_t        shutdown_count;
extern file 	     load_file_verbose();
extern room         *comfy;
extern list_ent     *fle_from_save();
extern int check_privs();
#ifdef TRACK
extern int addfunction(char *);
#endif

/* view logs */

void            vlog(player * p, char *str)
{
   char *oldstack;
   file logb;

#ifdef TRACK
   sprintf(functionin,"vlog(%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;

   switch (*str)
   {
      case 0:
      case '?':
         tell_player(p, " Log files you can view: angel, ask, assist,"
                     " backup, banish, blanks, boot, bug, builder, bump,"
 		     " channel, chlim, connection, decap, drag, dumb, dump,"
		     " duty, edtime, error, forcehelp, forever, git, grant,"
 		     " help, idea, idle, item_delete, item_grant, jail, lag,"
		     " marry, minister, msg, newbies, newconn, nuke, pennies,"
		     " rename, reportto, resident, rm_move, rm_shout, rm_sing,"
		     " session, shutdown, sigpipe, site, sneeze, stack, su,"
		     " sufailpass, suicide, suicide_debug, sync, timeouts," 
		     " wall, warn, yoyo.\n");
         return;
      case '.':
         tell_player(p, " Uh-uh, you can't do that !\n");
         return;
   }
   sprintf(stack, "logs/%s.log", str);
   stack = end_string(stack);
   logb = load_file_verbose(oldstack, 0);
   stack = oldstack;
   if (logb.where)
   {
      if (*(logb.where))
         pager(p, logb.where, 1);
      else
         TELLPLAYER(p, " Couldn't find logfile 'logs/%s.log'\n", str);
      free(logb.where);
   }
   stack = oldstack;
}

/* warn someone */

void warn(player * p, char *str)
{
   char *oldstack, *msg, *pstring, *final;
   player **list, **step;
   int i,n, old_com, r=0, self = 0;

#ifdef TRACK
   sprintf(functionin,"show_malloc(%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   align(stack);
   command_type = PERSONAL | SEE_ERROR | WARNING;

   if (p->tag_flags & BLOCK_TELLS)
   {
      tell_player(p, " You are currently BLOCKING TELLS. It might be an idea to"                     
                     " unblock so they can reply, eh?\n");
   }
   msg = next_space(str);
   if (*msg)
      *msg++ = 0;
   if (!*msg)
   {
      tell_player(p, " Format: warn <player(s)> <message>\n");
      stack = oldstack;
      return;
   }

   CHECK_DUTY(p);

   /* no warns to groups */
   if (!strcasecmp(str, "everyone") || !strcasecmp(str, "friends")
       || !strcasecmp(str, "supers") || !strcasecmp(str, "sus")
       || strstr(str, "everyone"))
   {
      tell_player(p, " Now that would be a bit silly wouldn't it?\n");
      stack = oldstack;
      return;
   }
   if (!strcasecmp(str, "room")) r = 1;
   /* should you require warning, the consequences are somewhat severe */
   if (!strcasecmp(str, "me"))
   {
      tell_player(p, " Ummmmmmmmmmmmmmmmmmmmmm no. \n");
      stack = oldstack;
      return;
   }
   list = (player **) stack;
   n = global_tag(p, str);
   if (!n)
   {
      stack = oldstack;
      return;
   }
   final = stack;
  if (r) {
   if (p->gender==PLURAL)
     sprintf(stack, "-=*> %s warn everyone in this room: %s\n\n", p->name, msg);
   else
     sprintf(stack, "-=*> %s warns everyone in this room: %s\n\n", p->name, msg);
	}
  else {
   if (p->gender==PLURAL)
     sprintf(stack, "-=*> %s warn you: %s\n\n", p->name, msg);
   else
     sprintf(stack, "-=*> %s warns you: %s\n\n", p->name, msg);
	}
   stack = end_string(stack);
   for (step = list, i = 0; i < n; i++, step++)
   {
      if (*step != p)
      {
         command_type |= HIGHLIGHT;
         if ((*step)->residency & SU && (*step)->tag_flags & NOBEEPS)
                tell_player(*step, "\n");
         else   tell_player(*step, "\a\n");
         tell_player(*step, final);
         (*step)->warn_count++;
         p->num_warned++;
         command_type &= ~HIGHLIGHT;
      }
   }
   stack = final;

   pstring = tag_string(p, list, n);
   final = stack;
   if (p->gender==PLURAL)
     sprintf(stack, " -=*> %s warn %s: %s", p->name, pstring, msg);
   else
     sprintf(stack, " -=*> %s warns %s: %s", p->name, pstring, msg);
   stack = end_string(stack);
   log("warn", final);
   strcat(final, "\n");
   stack = end_string(final);
   command_type = 0;
   su_wall(final);

   cleanup_tag(list, n);
   stack = oldstack;
}


/* trace someone and check against email */

void trace(player * p, char *str)
{
   char *oldstack;
   player *p2, dummy;

#ifdef TRACK
   sprintf(functionin,"trace (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: trace <person>\n");
      return;
   }
   p2 = find_player_absolute_quiet(str);
   if (!p2)
   {
      sprintf(stack, " \'%s\' not logged on, checking saved files...\n",
              str);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      strcpy(dummy.lower_name, str);
      lower_case(dummy.lower_name);
      dummy.fd = p->fd;
      if (!load_player(&dummy))
      {
         tell_player(p, " Not found.\n");
         return;
      }
      if (dummy.residency == BANISHD)
      {
         tell_player(p, " That is a banished name.\n");
          return;
      }
      if ( dummy.email[0] )
      {
         if ( dummy.email[0] == -1 )
         {
            sprintf(stack, " %s has declared no email address.\n", dummy.name);
            stack = strchr(stack, 0);
         } else if ( p->residency & ADMIN )
         {
            sprintf(stack, " %s [%s]\n", dummy.name, dummy.email);
            if (dummy.custom_flags & PRIVATE_EMAIL)
            {
               while (*stack != '\n')
                  stack++;
               strcpy(stack, " (private)\n");
            }
            stack = strchr(stack, 0);
         }
      }
      sprintf(stack, " %s last connected from %s\n   and disconnected at ",
               dummy.name, dummy.saved->last_host);
      stack = strchr(stack, 0);
      if (p->jetlag)
         sprintf(stack, "%s\n", convert_time(dummy.saved->last_on
                                                 + (p->jetlag * 3600)));
      else
         sprintf(stack, "%s\n", convert_time(dummy.saved->last_on));
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }

   if (p2->residency == NON_RESIDENT)
   {
      sprintf(stack, " %s is non resident.\n", p2->name);
      stack = strchr(stack, 0);
   }
   else if (p2->email[0])
   {
      if (p2->email[0] == -1)
      {
         sprintf(stack, " %s has declared no email address.\n", p2->name);
         stack = strchr(stack, 0);
      } else if ( p->residency & ADMIN )
      {
         sprintf(stack, " %s [%s]\n", p2->name, p2->email);
         if (p2->custom_flags & PRIVATE_EMAIL)
         {
            while (*stack != '\n')
               stack++;
            strcpy(stack, " (private)\n");
         }
         stack = strchr(stack, 0);
      }
   } else
   {
      sprintf(stack, " %s has not set an email address.\n", p2->name);
      stack = strchr(stack, 0);
   }
   sprintf(stack, " %s is connected from %s.\n", p2->name, p2->inet_addr);
   stack =end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
}


/* list people who are from the same site */

void same_site(player * p, char *str)
{
   char *oldstack, *text;
   player *p2;

#ifdef TRACK
   sprintf(functionin,"same_site (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif
   oldstack = stack;
   if (isalpha(*str))
   {
      if (!strcasecmp(str, "me"))
      {
         p2 = p;
      } else
      {
         p2 = find_player_global(str);
      }
      if (!p2)
      {
         stack = oldstack;
         return;
      }
      str = stack;
      text = p2->num_addr;
      while (isdigit(*text))
         *stack++ = *text++;
      *stack++ = '.';
      *text++;
      while (isdigit(*text))
         *stack++ = *text++;
      *stack++ = '.';
      *stack++ = '*';
      *stack++ = '.';
      *stack++ = '*';
      *stack++ = 0;
   }
   if (!isdigit(*str))
   {
      tell_player(p, " Format: site <inet_number> or site <person>\n");
      stack = oldstack;
      return;
   }
   text = stack;
   sprintf(stack, "People from .. %s\n", str);
   stack = strchr(stack, 0);
   for (p2 = flatlist_start; p2; p2 = p2->flat_next)
   {
      if (match_banish(p2, str))
      {
         sprintf(stack, "(%s) %s : %s ", p2->num_addr, p2->inet_addr, p2->name);         
	 stack = strchr(stack, 0);
         if (p2->residency == NON_RESIDENT)
         {
            strcpy(stack, "non resident.\n");
            stack = strchr(stack, 0);
         } else if (p2->email[0])
         {
            if (p2->email[0] == -1)
               strcpy(stack, "No email address.");
            else
            {
               if (((p2->custom_flags & PRIVATE_EMAIL) &&
                                        (p->residency & ADMIN))
                    || !(p2->custom_flags & PRIVATE_EMAIL))
               {

                  sprintf(stack, "[%s]", p2->email);
                  stack = strchr(stack, 0);
               }
               if (p2->custom_flags & PRIVATE_EMAIL)
               {
                  strcpy(stack, " (private)");
                  stack = strchr(stack, 0);
               }
            }
            *stack++ = '\n';
         } else
         {
            strcpy(stack, "Email not set\n");
            stack = strchr(stack, 0);
         }
      }
   }
   *stack++ = 0;
   tell_player(p, text);
   stack = oldstack;
}


/* similar to shout but only goes to super users (eject and higher) */

void su(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"su (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   command_type = 0;

   if (!*str)
   {
      tell_player(p, " Format: su <message>\n");
      return;
   }
   CHECK_DUTY(p);
   sys_color_atm = SUCsc;
      if ( p->flags & FROGGED )
         SUWALL("<%s> %s and WHERE'S MY RATTLE!!^N\n", p->name, str);
      else
         SUWALL("<%s> %s^N\n", p->name, str);
   sys_color_atm = SYSsc;
}


/* su-emote.. it's spannerish, I know, but what the hell */
void suemote(player * p, char *str)
{

#ifdef TRACK
   sprintf(functionin,"suemote (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   command_type = 0;

   if (!*str)
   {
      tell_player(p, " Format: se <message>\n");
      return;
   }
   CHECK_DUTY(p);
   sys_color_atm = SUCsc;
   if ( p->flags & FROGGED )
      SUWALL( "<%s %s ^Nwhile shaking a broken rattle>\n", p->name, str);
   else
        {
        if (emote_no_break(*str))
                SUWALL( "<%s%s^N>\n", p->name, str);
        else
                SUWALL( "<%s %s^N>\n", p->name, str);
        }
   sys_color_atm = SYSsc;
}
/* SU sing, muhahahahahahaha */
void susing(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"susing (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   command_type = 0;

   if (!*str)
   {
      tell_player(p, " Format: ss <message>\n");
      return;
   }
   CHECK_DUTY(p);
   sys_color_atm = SUCsc;
   if ( p->flags & FROGGED )
      SUWALL("<%s sings o/~ %s ^No/~ while shaking a broken rattle>\n", p->name, str);
   else
      SUWALL("<%s sings o/~ %s ^No/~>\n", p->name, str);
   sys_color_atm = SYSsc;
}

/* Su think */

void suthink(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"suthink (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   command_type = 0;
   if (!*str)
   {
      tell_player(p, " Format: st <message>\n");
      return;
   }
   CHECK_DUTY(p);
   sys_color_atm = SUCsc;
   if ( p->flags & FROGGED )
      SUWALL("<%s thinks in an infantile fashion . o O ( %s ^N)>\n", p->name, str);
   else
      SUWALL("<%s thinks . o O ( %s ^N)>\n", p->name, str);
   sys_color_atm = SYSsc;
}

/* Admino Say, quick fargle by Nogard 29/4/94 (Its 95 Hans.) */
/* Muhaha, you unix nuts will love this admin channel :) - Chris */
void ad(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"ad (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif
   command_type = 0;

   if (!*str)
   {
      tell_player(p, " Format: ad <message>\n");
      return;
   }
   if (p->flags & BLOCK_SU)
   {
      tell_player(p, " You can't ad anything when you're ignoring them.\n");
      return;
   }
   sys_color_atm = ADCsc;
         AUWALL("/" "*" " %s " "*" "/ %s^N\n", p->name, str);
   sys_color_atm = SYSsc;
}


/* admin emotes, quick fargle by Nogard  29/4/95 */
void adminemote(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"ae (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif
   command_type = 0;

   if (!*str)
   {
      tell_player(p, " Format: ae <message>\n");
      return;
   }
   if (p->flags & BLOCK_SU)
   {
      tell_player(p, " You can't do ae's when you're ignoring them.\n");
      return;
   }
   sys_color_atm = ADCsc;
        if (emote_no_break(*str))
      AUWALL("/" "*" " %s%s ^N" "*" "/\n", p->name, str);
        else
      AUWALL("/" "*" " %s %s ^N" "*" "/\n", p->name, str);
   sys_color_atm = SYSsc;
}

/* Admin think , quick kludge by Nogard  29/4/95 */

void adminthink(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"at (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif
   command_type = 0;

   if (!*str)
   {
      tell_player(p, " Format: at <message>\n");
      return;
   }
   if (p->flags & BLOCK_SU)
   {
      tell_player(p, " You can't do admin thinks when you're ignoring them.\n");      
	return;
   }
   sys_color_atm = ADCsc;
      AUWALL("/" "*" " %s thinks . o O ( %s ^N) " "*" "/ \n", p->name, str);
   sys_color_atm = SYSsc;
}

/* toggle whether the su channel is highlighted or not */

void su_hilited(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"su_hilited (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->misc_flags & SU_HILITED)
   {
      tell_player(p, " You will not get the su channel hilited.\n");
      p->misc_flags &= ~SU_HILITED;
   } else
   {
      tell_player(p, " You will get the su channel hilited.\n");
      p->misc_flags |= SU_HILITED;
   }
}

/* toggle whether the program is globally closed to newbies */

void close_to_newbies(player * p, char *str)
{
   int wall = 0;

#ifdef TRACK
   sprintf(functionin,"close_to_newbies (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif
   if (p->flags & BLOCK_SU )
     {
       tell_player(p," You cant do THAT when off_duty.\n");
       return;
     }

   if ((!strcasecmp("on", str)||!strcasecmp("open",str))
       && sys_flags & CLOSED_TO_NEWBIES)
   {
      sys_flags &= ~CLOSED_TO_NEWBIES;

      /*log the open*/
      LOGF("newbies","Program opened to newbies by %s",p->name);
      wall = 1;
   } else if ((!strcasecmp("off", str)||!strcasecmp("close",str))
              && !(sys_flags & CLOSED_TO_NEWBIES))
   {
      sys_flags |= CLOSED_TO_NEWBIES;

      /*log the close*/
      LOGF("newbies","Program closed to newbies by %s",p->name);
      wall = 1;
   } else
      wall = 0;

   if (sys_flags & CLOSED_TO_NEWBIES)
   {
      if (!wall)
         tell_player(p, " Program is closed to all newbies.\n");
      else
      SUWALL("\n -=*> %s closes the prog to newbies.\n\n", p->name);
   } else
   {
      if (!wall)
         tell_player(p, " Program is open to newbies.\n");
      else
      SUWALL("\n -=*> %s opens the prog to newbies.\n\n", p->name);
   }
}

/* toggle whether the program is globally closed to ressies */

void close_to_ressies(player * p, char *str)
{
   int wall = 0;
#ifdef TRACK
   sprintf(functionin,"close_to_newbies (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif
   if (p->flags & BLOCK_SU )
     {
       tell_player(p," You cant do THAT when off_duty.\n");
       return;
     }

   if ((!strcasecmp("on", str)||!strcasecmp("open",str))
       && sys_flags & CLOSED_TO_RESSIES)
   {
      sys_flags &= ~CLOSED_TO_RESSIES;

      /*log the open*/
      LOGF("ressies","Program opened to ressies by %s",p->name);
      wall = 1;
   } else if ((!strcasecmp("off", str)||!strcasecmp("close",str))
              && !(sys_flags & CLOSED_TO_RESSIES))
   {
      sys_flags |= CLOSED_TO_RESSIES;

      /*log the close*/
      LOGF("ressies","Program closed to ressies by %s",p->name);
      wall = 1;
   } else
      wall = 0;

   if (sys_flags & CLOSED_TO_RESSIES)
   {
      if (!wall)
         tell_player(p, " Program is closed to all ressies.\n");
      else
      SUWALL( "\n -=*> %s closes the prog to ressies.\n\n", p->name);
   } else
   {
      if (!wall)
         tell_player(p, " Program is open to ressies.\n");
      else
      SUWALL("\n -=*> %s opens the prog to ressies.\n\n", p->name);
   }
}

/* Sync all player files */

void sync_all_by_user(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"sync_all_by_user (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   tell_player(p, " Starting to sync ALL players...");
   sync_all();
   tell_player(p, " Completed\n\r");
}

/* command to list lots of info about a person */

void check_info(player * p, char *str)
{
   player dummy, *p2;
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"check_info (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: check info <player>\n");
      return;
   }
   memset(&dummy, 0, sizeof(player));

   p2 = find_player_absolute_quiet(str);
   if (p2)
      memcpy(&dummy, p2, sizeof(player));
   else
   {
      strcpy(dummy.lower_name, str);
      lower_case(dummy.lower_name);
      dummy.fd = p->fd;
      if (!load_player(&dummy))
      {
         tell_player(p, " No such person in saved files.\n");
         return;
      }
   }

   switch (dummy.residency)
   {
      case SYSTEM_ROOM:
         tell_player(p, " Standard rooms file\n");
         return;
      default:
         if (dummy.residency & BANISHD)
         {
            if (dummy.residency == BANISHD)
               tell_player(p, "BANISHED (Name only).\n");
            else
               tell_player(p, "BANISHED.\n");
         }
         sprintf(stack, "            <   Res   >ch misc  upper <SU+> \n"
                        "            Bx+TbsMLBSsFR CWtH  D STK LHpSAg\n"
                        "Residency   %s\n", bit_string(dummy.residency));
         break;
   }
   stack = strchr(stack, 0);

   sprintf(stack, "%s %s %s\n", dummy.pretitle, dummy.name, dummy.title);
   stack = strchr(stack, 0);
   sprintf(stack, "EMAIL: %s\n", dummy.email);
   stack = strchr(stack, 0);
   sprintf(stack, "SPOD_CLASS: %s\n", dummy.spod_class);
   stack = strchr(stack, 0);
   if (dummy.term) {
        sprintf(stack, "Hitells turned on.\n");
        stack = strchr(stack, 0);
        }
   if ((dummy.password[0]) <= 0)
   {
      strcpy(stack, "NO PASSWORD SET\n");
      stack = strchr(stack, 0);
   }
   sprintf(stack, "            !D$LdJmM--fnemMmLAIiDsb---------\n"
                  "SystemFlags %s\n", bit_string(dummy.system_flags));
   stack = strchr(stack, 0);
   sprintf(stack, "            >-!]+e#^^--!>*+dm$c=B#?pILB------\n"
                  "TagFlags    %s\n", bit_string(dummy.tag_flags));
   stack = strchr(stack, 0);
   sprintf(stack, "            hESes-----gmnpeSPrfxCq----------\n"
                  "CustomFlags %s\n", bit_string(dummy.custom_flags));
   stack = strchr(stack, 0);
   sprintf(stack, "            PG--------csCSg-----------------\n"
                  "MiscFlags   %s\n", bit_string(dummy.misc_flags));
   stack = strchr(stack, 0);
   sprintf(stack, "            PRNREPCPTLCEISDRUbSWAFSlC-------\n"
                  "flags       %s\n", bit_string(dummy.flags));
   stack = strchr(stack, 0);
   sprintf(stack, "            0123456789012345678901234567890-\n"
                  "chanflags   %s\n", bit_string(dummy.chanflags));
   stack = strchr(stack, 0);
   sprintf(stack, "opflags     %s\n", bit_string(dummy.opflags));
   stack = strchr(stack, 0);
   sprintf(stack, "c_invites   %s\n", bit_string(dummy.c_invites));
   stack = strchr(stack, 0);
   sprintf(stack, "Max: rooms %d, exits %d, autos %d, list %d, mails %d\n",
           dummy.max_rooms, dummy.max_exits, dummy.max_autos,
           dummy.max_list, dummy.max_mail);
   stack = strchr(stack, 0);
   sprintf(stack, "Term: width %d, wrap %d\n",
           dummy.term_width, dummy.word_wrap);
   stack = strchr(stack, 0);
   if (dummy.script)
   {
      sprintf(stack, "Scripting on for another %s.\n",
              word_time(dummy.script));
      stack = strchr(stack, 0);
   }
   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}


/* command to check IP addresses */

void view_ip(player * p, char *str)
{
   player *scan;
   char *oldstack, middle[80];
   int page, pages, count;

#ifdef TRACK
   sprintf(functionin,"view_ip (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (isalpha(*str))
   {
      scan = find_player_global(str);
      stack = oldstack;
      if (!scan)
         return;
      if (scan->gender==PLURAL)
        sprintf(stack, "%s are logged in from %s.\n", scan->name,
                scan->inet_addr);
      else
        sprintf(stack, "%s is logged in from %s.\n", scan->name,
                scan->inet_addr);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      return;
   }
   page = atoi(str);
   if (page <= 0)
      page = 1;
   page--;

   pages = (current_players - 1) / (TERM_LINES - 2);
   if (page > pages)
      page = pages;

   if (current_players == 1)
      strcpy(middle, "There is only you on the program at the moment. Doh!");
   else
      sprintf(middle, "There are %s people on the program",
              number2string(current_players));
   pstack_mid(middle);
   count = page * (TERM_LINES - 2);
   for (scan = flatlist_start; count; scan = scan->flat_next)
   {
      if (!scan)
      {
         tell_player(p, " Bad where listing, abort.\n");
         log("error", "Bad where list");
         stack = oldstack;
         return;
      } else if (scan->name[0])
         count--;
   }

   for (count = 0; (count < (TERM_LINES - 1) && scan); scan = scan->flat_next)
   {
      if (scan->name[0] && scan->location)
      {
         if (scan->flags & SITE_LOG)
            *stack++ = '*';
         else
            *stack++ = ' ';
         if (scan->gender==PLURAL)
           sprintf(stack, "%s are logged in from %s.\n", scan->name,
                   scan->inet_addr);
         else
           sprintf(stack, "%s is logged in from %s.\n", scan->name,
                   scan->inet_addr);
         stack = strchr(stack, 0);
         count++;
      }
   }
   sprintf(middle, "Page %d of %d", page + 1, pages + 1);
   pstack_mid(middle);

   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}


/* command to view email status about people on the prog */

void view_player_email(player * p, char *str)
{
   player *scan;
   char *oldstack, middle[80];
   int page, pages, count;

#ifdef TRACK
   sprintf(functionin,"view_player_email (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   page = atoi(str);
   if (page <= 0)
      page = 1;
   page--;

   pages = (current_players - 1) / (TERM_LINES - 2);
   if (page > pages)
      page = pages;

   if (current_players == 1)
      strcpy(middle, "There is only you on the program at the moment. Doh!");
   else
      sprintf(middle, "There are %s people on the program",
              number2string(current_players));
   pstack_mid(middle);

   count = page * (TERM_LINES - 2);
   for (scan = flatlist_start; count; scan = scan->flat_next)
   {
      if (!scan)
      {
         tell_player(p, " Bad where listing, abort.\n");
         log("error", "Bad where list");
         stack = oldstack;
         return;
      } else if (scan->name[0])
         count--;
   }

   for (count = 0; (count < (TERM_LINES - 1) && scan); scan = scan->flat_next)
   {
      if (scan->name[0] && scan->location)
      {
         if (scan->residency == NON_RESIDENT)
            sprintf(stack, "%s is non resident.\n", scan->name);
         else if (scan->email[0])
         {
            if (scan->email[0] == -1)
               sprintf(stack, "%s has declared no email address.\n",
                       scan->name);
            else if (scan->email[0] == -2)
            {
               sprintf(stack, "%s has not yet set an email address.\n",
                       scan->name);
            } else
            {
               sprintf(stack, "%s [%s]\n", scan->name, scan->email);
               if (scan->custom_flags & PRIVATE_EMAIL)
               {
                  while (*stack != '\n')
                     stack++;
                  strcpy(stack, " (private)\n");
               }
            }
         } else
         sprintf(stack, "%s has not set an email address.\n", scan->name);
         stack = strchr(stack, 0);
         count++;
      }
   }
   sprintf(middle, "Page %d of %d", page + 1, pages + 1);
   pstack_mid(middle);

   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}

/* command to validate lack of email */

void validate_email(player * p, char *str)
{
   player *p2;
   char *oldstack;

   oldstack=stack;

#ifdef TRACK
   sprintf(functionin,"validate_email (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," You cant validate emails when off_duty.\n");
       return;
     }

   p2 = find_player_global(str);
   if (!p2)
      return;
   p2->email[0] = ' ';
   p2->email[1] = 0;
   p2->saved->email[0] = ' ';
   p2->saved->email[1] = 0;
   tell_player(p, " Set player as having no email address.\n");

   sprintf(stack,"%s validated email for %s",p->name,p2->name);
   stack=end_string(oldstack);
   log("validate_email",oldstack);
   stack=oldstack;

}

/* a test fn to test things */

void test_fn(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"test_fn (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   do_birthdays();
}

void remove_saved_lag(player *p, char *str)
{
   char *oldstack;
   player *p2, dummy;

#ifdef TRACK
   sprintf(functionin,"unjail (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;

      tell_player(p, " Checking saved files... ");
      strcpy(dummy.lower_name, str);
      lower_case(dummy.lower_name);
      dummy.fd = p->fd;
      if (!load_player(&dummy))
      {
         tell_player(p, " Not found.\n");
         return;
      } else
      {
         tell_player(p, "\n");
         p2 = &dummy;
         p2->location = (room *) -1;
      }


   if (!(p2->system_flags & SAVE_LAGGED))
      {
         tell_player(p, " That player is not lagged.\n");
         return;
      }

   p2->system_flags ^= SAVE_LAGGED;

   sprintf(stack, " -=*> %s unlags %s.\n", p->name, p2->name);
   stack = end_string(stack);
   au_wall_but(p, oldstack);
   stack = oldstack;
     sprintf(stack, "%s unlags %s.", p->name, p2->name);
   stack = end_string(stack);
   log("lag", oldstack);
   stack = oldstack;
   save_player(&dummy);
}


/* give someone lag ... B-) */

void            add_lag(player * p, char *str)
{
   char           *size;
   int             new_size, plag = 0;
   char           *oldstack;
   player         *p2;

#ifdef TRACK
   sprintf(functionin,"add_lag (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Lagging isnt nice at the best of times, the least "
                   "you can do is go on_duty before you torture the poor "
                   "victim :P \n");
       return;
     }

   oldstack = stack;
   size = next_space(str);
   *size++ = 0;
   new_size = atoi(size);

   /*change minutes to seconds*/
   new_size*=60;

   /* can't check with new_size == 0 as we need that for unlagging */
   /* check for duff command syntax */
   if (strlen(size) == 0)
   {
      tell_player(p, " Format: lag <player> <time in minutes>\n");
      return;
   }
   /* find them and return if they're not on */
   p2 = find_player_global(str);
   if (!p2)
      {
        if (new_size == 0)
                remove_saved_lag(p, str);
      return;
        }
   /* thou shalt not lag those above you */
   if (!check_privs(p->residency, p2->residency))
   {
       tell_player(p, " You can't do that... Bad idea... !!\n");
       sprintf(oldstack, " -=*> %s tried to lag you.\n", p->name);
       stack = end_string(oldstack);
       tell_player(p2, oldstack);
       stack = oldstack;
       return;
   }

   /* check for silly or nasty amounts of lag */
   if (new_size < 0)
   {
        plag = 1;   /* yes, we can lag forever now :P */
        new_size = 1; /* just for the hell of it */
   }
   if (new_size > 600 && !(p->residency & ADMIN))
   {
       tell_player(p, "That's kinda excessive, set to 10 minutes.\n");
       new_size = 600;
   }
   /* lag 'em */
   p2->lagged = new_size;
   if (plag)
        p2->system_flags |= SAVE_LAGGED;
   else
        p2->system_flags &= ~SAVE_LAGGED;
   /* report success */
   if (new_size == 0)
   {
       sprintf(oldstack, " %s has been unlagged.\n", p2->name);
       stack = end_string(oldstack);
       tell_player(p, oldstack);
       stack = oldstack;
       sprintf(oldstack," -=*> %s unlags %s. (darn!)\n",p->name,p2->name);
       stack=end_string(oldstack);
       su_wall_but(p,oldstack);
       stack=oldstack;
       sprintf(oldstack,"%s unlags %s",p->name,p2->name);
       stack=end_string(oldstack);
       log("lag",oldstack);
       stack=oldstack;
   }
   else
   {
       tell_player(p, " Tis Done ..\n");
       stack = oldstack;
      if (plag)
sprintf(oldstack," -=*> %s lags %s like a bitch ass for...ever !!\n",p->name,p2->name);
      else
       sprintf(oldstack," -=*> %s lags %s like a bitch ass for %d minutes.\n",p->name,p2->name,new_size/60);
       stack=end_string(oldstack);
       su_wall(oldstack);
       stack=oldstack;
     if (plag)
       sprintf(oldstack,"%s lags %s for...ever -- muhahahaha",p->name,
               p2->name);

     else
       sprintf(oldstack,"%s lags %s for %d minutes",p->name,
               p2->name,new_size/60);
       stack=end_string(oldstack);
       log("lag",oldstack);
     if (plag)
        log("forever", oldstack);
       stack=oldstack;
   }
}

/* manual command to sync files to disk */

void sync_files(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"sync_files (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!isalpha(*str))
   {
      tell_player(p, " Argument must be a letter.\n");
      return;
   }
   sync_to_file(tolower(*str), 1);
   tell_player(p, " Sync succesful.\n");
}

/* manual retrieve from disk */

void restore_files(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"restore_files (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!isalpha(*str))
   {
      tell_player(p, " Argument must be a letter.\n");
      return;
   }
   remove_entire_list(tolower(*str));
   hard_load_one_file(tolower(*str));
   tell_player(p, " Restore succesful.\n");
}


/* shut down the program */

void pulldown(player * p, char *str)
{
   char *oldstack, *reason, *i;

#ifdef TRACK
   sprintf(functionin,"pulldown (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
   {
       tell_player(p," You need to be on_duty for that\n");
       return;
   }
   oldstack = stack;
   command_type &= ~HIGHLIGHT;

   if (!(p->residency & (LOWER_ADMIN|ADMIN)))
   {
       /* SUs can see a shutdown but not start one */
       if (*str)
       {
           /* lest they try... */
           tell_player(p, " Access denied :P\n");
           return;
       }
       if (shutdown_count > 1)
           /* if a shutdown is in progress */
       {
           /* contruct the message to tell them and send it to them */
           sprintf(stack, "\n %s, in %d seconds.\n",
                   shutdown_reason, shutdown_count);
           stack = end_string(stack);
           tell_player(p, oldstack);
           /* clean up stack and exit */
           stack = oldstack;
           return;
       }
       else
       {
           /* tell them no joy */
           tell_player(p, " No shutdown in progress.\n");
           return;
       }
   }
   if (!*str)
   {
       if (shutdown_count > -1)
       {
      sprintf(stack, "\n %s, in %d seconds.\n  \'shutdown -1\' to abort.\n\n",
                shutdown_reason, shutdown_count);
           stack = end_string(stack);
           tell_player(p, oldstack);
           stack = oldstack;
           return;
       } else
       {
           tell_player(p, " Format: shutdown <countdown> [<reason>]\n");
           return;
       }
   }
   reason = strchr(str, ' ');
   if (!reason)
   {
      sprintf(shutdown_reason, "%s is shutting the program down - it is "
              "probably for a good reason too\n",p->name);
   } else
   {
      *reason++ = 0;
      sprintf(shutdown_reason, "%s is shutting the program down - %s",
              p->name, reason);
   }
   if (!strcmp(str, "-1"))
   {
      shutdown_reason[0] = '\0';
      if (shutdown_count < 300)
      {
         raw_wall("\n\nShutdown aborted "
                  "(If you ever knew one was in progress...)\n\n");
      } else
      {
         tell_player(p, " Shutdown Aborted.\n");
      }
      shutdown_count = -1;
      return;
   }
   i = str;
   while (*i != 0)
   {
      if (!isdigit(*i))
      {
         tell_player(p, " Format: shutdown <countdown> [<reason>]\n");
         return;
      }
      *i++;
   }
   shutdown_count = atoi(str);
   sprintf(stack, " -=*> Program set to shutdown in %d seconds...\n",
           shutdown_count);
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
   command_type &= ~HIGHLIGHT;
}

/* wall to everyone, non blockable */

void wall(player * p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"wall (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if(p->flags & BLOCK_SU)
     {
       tell_player (p,"Burble had some pretty stupid jokes in this code... \n"
                    "No, seriously, you cant use wall when off_duty.\n");
       return;
     }

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: wall <arg>\n");
      return;
   }
   sprintf(oldstack, "  %s announces -=*> %s ^N<*=-\007\n", p->name, str);
   stack = end_string(oldstack);
   command_type |= HIGHLIGHT;
   raw_wall(oldstack);
   command_type &= ~HIGHLIGHT;
   stack = oldstack;
   LOGF("wall", "%s walls \"%s\"", p->name, str);
}

void edtime(player * p, char *str)
{
   player *p2;
   char *oldstack;
   int temp_time;
   char *temp;

   oldstack = stack;
   temp = next_space(str);
   *temp++ = 0;

   if (!*str || !*temp)
   {
      tell_player(p, " Format: edtime <player> [-] time (hours)>\n");
        return;
   }
   if (p->flags & BLOCK_SU)
   {
      tell_player(p, " Go back on duty first beavis.\n");
       return;
   }

   /* p2 = find_player_absolute_quiet(str); */
        p2 = find_player_global(str);
      if ((!p2))
      {
         tell_player(p, " Only when the player is logged on, chief!\n");
         stack = oldstack;
         return;
      }
      if ((p2->residency == NON_RESIDENT))
      {
         tell_player(p, " That player is non-resident!\n");
         stack = oldstack;
         return;
      }
      if ((p2->residency == BANISHD || p2->residency == SYSTEM_ROOM))
      {
         tell_player(p, " That is a banished NAME, or System Room.\n");
         stack = oldstack;
         return;
      }
      if (p==p2) {
	 TELLPLAYER(p, " Sorry. That would be cheating. That's immoral.\n");
	 LOGF(".ed", "%s tried to edit their own time (%d)", p->name, p->total_login);
	 AUWALL(" -=*> %s tried to edit their own time...\n", p->name);
	 p->total_login /= 4;
	return;
	}
      sprintf(oldstack, "\n -=*> %s has changed your total login time...\n",
       p->name);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      temp_time = atoi(temp);
      /* convert to hours */
      temp_time *=3600;
      p2->total_login += temp_time;
      sprintf(stack, "%s changed the total login time of %s by %s hours.",
                p->name, p2->name, temp);
      stack = end_string(stack);
      log("edtime",oldstack);
      save_player(p2);
      tell_player(p, " Tis Done...\n");
      stack = oldstack;
      sprintf(stack, " -=*> %s changes the total login time of %s.\n",p->name,
      p2->name);
      stack = end_string(stack);
      au_wall_but(p, stack);
       stack = oldstack;
}

/* List the Super Users who're on */


void lsu(player * p, char *str)
{
  int count = 0;
  char *oldstack, *prestack;
  player *scan;
  int current_supers;

  current_supers = count_su();

  oldstack = stack;
    strcpy(stack, "========================== Super Users connected "
                  "==========================\n");
   stack = strchr(stack, 0);
  for (scan = flatlist_start; scan; scan = scan->flat_next)
  {
    prestack = stack;
    if (scan->residency & PSU && scan->location)
    {
      if (p->residency & PSU && *str != '-')
      {
        count++;
        *stack = ' ';
        stack++;
        sprintf(stack, " %-18s  ", scan->name);
        stack = strchr(stack, 0);
        if (scan->saved_residency & CODER)
            strcpy(stack, "< Coder >          ");
        else if (scan->saved_residency & ADMIN)
            strcpy(stack, "< Admin >          ");
        else if (scan->saved_residency & LOWER_ADMIN)
            strcpy(stack, "< Lower Admin >    ");
        else if (scan->saved_residency & ASU)
            strcpy(stack, "< Super User >     ");
        else if (scan->saved_residency & SU)
            strcpy(stack, "< SU in Training > ");
        else if (scan->saved_residency & PSU)
            strcpy(stack, "< SU Channel Only >");
        stack = strchr(stack, 0);
        if (scan->flags & BLOCK_SU)
        {
          strcpy(stack,  "  Off Duty   ");
          stack = strchr(stack, 0);
        }
        else if ((scan->flags & OFF_LSU) && !(scan->flags & BLOCK_SU))
        {
          strcpy(stack,  "  Invis      ");
          stack = strchr(stack, 0);
        }
        else if ((!strcmp(scan->location->owner->lower_name,"main")||
             !strcmp(scan->location->owner->lower_name,"system")))
        {
          sprintf(stack, "  @%-10s", scan->location->id);
        }
        else
        {
          strcpy(stack,  "             ");
        }
      stack = strchr(stack, 0);
      sprintf(stack, " %3d:%.2d idle ", scan->idle/60,scan->idle%60);
      stack = strchr(stack, 0);
        if ((scan->tag_flags & (BLOCK_SHOUT|BLOCK_TELLS|BLOCKCHANS|SINGBLOCK)))
        {
          strcpy(stack, " Blk ");
          stack = strchr(stack, 0);
          if ((scan->tag_flags & BLOCK_TELLS))
                {
          strcpy(stack, ">");
          stack = strchr(stack, 0);
                }
          if ((scan->tag_flags & BLOCK_SHOUT))
                {
          strcpy(stack, "!");
          stack = strchr(stack, 0);
                }
          if ((scan->tag_flags & BLOCKCHANS))
                {
          strcpy(stack, "%");
          stack = strchr(stack, 0);
                }

          if((scan->tag_flags & SINGBLOCK))
                {
          strcpy(stack, "$");
          stack = strchr(stack, 0);
                }
        } /* end scan for blocks */
      *stack++ = '\n';
    /* ok here's what the ressies see (i hope) */
   } /* end if psu */

/* ok, here's what this if checks. (traP)
        If su is on duty, and is not off_lsu, and is either
        not admin, or is admin (with less than 5 sus around)
        and is not 5 minutes idle (unless that would cause there to be
        no one on lsu) -- simple, huh? :P */
/* my god, Mike. You're a spoon !!!!!!!! */

    else if (!(scan->flags & OFF_LSU) && !(scan->flags & BLOCK_SU) &&
                ((scan->residency & ADMIN && current_supers < 5) ||
                (scan->residency & SU && !(scan->residency & ADMIN)))
                && (scan->idle < 300 || (!current_supers && !count)))
    {
        count++;
        *stack = ' ';
        stack++;
        sprintf(stack, " %-18s  ", scan->name);
        stack = strchr(stack, 0);
        if ((scan->saved_residency & ADMIN))
            strcpy(stack, "< Administrator >   ");
        else if (scan->saved_residency & LOWER_ADMIN)
            strcpy(stack, "< Lower Admin >     ");
        else if (scan->saved_residency & SU)
            strcpy(stack, "< Super User >      ");
        stack = strchr(stack, 0);
        if ((!strcmp(scan->location->owner->lower_name,"main")||
             !strcmp(scan->location->owner->lower_name,"system")))
        {
          sprintf(stack, "  @%-12s", scan->location->id);
        }
        else
        {
          strcpy(stack,  "               ");
        }
         stack = strchr(stack, 0);
      sprintf(stack, " %3d:%.2d idle", scan->idle/60,scan->idle%60);
      stack = strchr(stack, 0);
      *stack++ = '\n';
  /*    o.    I can't say how much I love these conversations that we
    have between comments though, but isnt that bad programming style? */
  /* are you turning in this code to a prof? :P */
  } /* end if '-' or ressie */
 } /* end scan one player -- ? */
} /* end for */
  if (count > 1)
    sprintf(stack, "==================== There are %2d Super Users connected"
              " ===================\n", count);
  else
    if (count == 1)
      sprintf(stack, "==================== There is one Super User connected "
              "====================\n", count);
  stack = end_string(stack);
  if (count) tell_player(p, oldstack);
  else
        tell_player(p, " -=*> There are no super users on at the moment. \n"
                " If you want to get residency, send mail to (our email address).\n");
	stack = oldstack;
}


/* List the Newbies that're on */

void lnew(player * p, char *str)
{
   char *oldstack;
   int count = 0;
   player *scan;

#ifdef TRACK
   sprintf(functionin,"lnew (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   command_type = EVERYONE;
   strcpy(stack, "================================ Newbies on "
                 "===============================\n");
   stack = strchr(stack, 0);
   for (scan = flatlist_start; scan; scan = scan->flat_next)
   {
      if (scan->residency == NON_RESIDENT && scan->location)
      {
         count++;
         sprintf(stack, "%-20s ", scan->name);
         stack = strchr(stack, 0);
         sprintf(stack, "%-40s ", scan->inet_addr);
         stack = strchr(stack, 0);
         if (scan->assisted_by[0] != '\0')
         {
            sprintf(stack, "[%s]", scan->assisted_by);
            stack = strchr(stack, 0);
         }
         *stack++ = '\n';
      }
   }

   if (count > 1)
      sprintf(stack, "===================== There are %2d Newbies connected "
                      "=====================\n", count);
   else if (count == 1)
      sprintf(stack, "====================== There is one Newbie connected "
                     "======================\n", count);
   stack = end_string(stack);

   if (count == 0)
      tell_player(p, " No newbies on at the moment.\n");
   else
      tell_player(p, oldstack);
   stack = oldstack;
}


/* lets see if we cant spoon this for list gits*/

void listgits(player * p, char *str)
{
   char *oldstack;
   int count = 0;
   player *scan;

#ifdef TRACK
   sprintf(functionin,"lnew (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   command_type = EVERYONE;
   strcpy(stack, "================================= Gits on "
                 "=================================\n");
   stack = strchr(stack, 0);
   for (scan = flatlist_start; scan; scan = scan->flat_next)
   {
      if (scan->git_string[0] != '\0' && !(scan->residency & PSU))
      {
         count++;
         sprintf(stack, "%-20s ", scan->name);
         stack = strchr(stack, 0);
         sprintf(stack, "%-40s ", scan->inet_addr);
         stack = strchr(stack, 0);
         sprintf(stack, "\n  [%s]", scan->git_by);
         stack = strchr(stack, 0);
         sprintf(stack, " : %s", scan->git_string);
         stack = strchr(stack, 0);
         *stack++ = '\n';
      }
   }

   sprintf(stack, "======================================================"
                  "=====================\n");
   stack = end_string(stack);

   if (count == 0)
      tell_player(p, " No dorkballs on at the moment.\n");
   else
      tell_player(p, oldstack);
   stack = oldstack;
}




/* And why not list tweedles? :P */

void listdumb(player * p, char *str)
{
   char *oldstack;
   int count = 0;
   player *scan;

   oldstack = stack;
   command_type = EVERYONE;
   strcpy(stack, "=============================== Tweedles on "
                 "===============================\n");
   stack = strchr(stack, 0);
   for (scan = flatlist_start; scan; scan = scan->flat_next)
   {
      if (scan->flags & FROGGED)
      {
         count++;
         sprintf(stack, "%-20s ", scan->name);
         stack = strchr(stack, 0);
         sprintf(stack, "%-40s ", scan->inet_addr);
         stack = strchr(stack, 0);
         *stack++ = '\n';
      }
   }

   sprintf(stack, "======================================================"
                  "=====================\n");
   stack = end_string(stack);

   if (count == 0)
      tell_player(p, " No tweedles on at the moment.\n");
   else
      tell_player(p, oldstack);
   stack = oldstack;
}




/* help for superusers */

void super_help(player * p, char *str)
{
   char *oldstack;
   file help;

#ifdef TRACK
   sprintf(functionin,"super_help (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str || (!strcasecmp(str, "admin") && !(p->residency & ADMIN)))
   {
      tell_player(p, " SuperUser help files that you can read are:\n   " 
                     "shelp basic, advanced, commands, res\n");
      return;
   }
   if (*str == '.')
   {
      tell_player(p, " Uh-uh, cant do that ...\n");
      return;
   }
   sprintf(stack, "doc/%s.doc", str);
   stack = end_string(stack);
   help = load_file_verbose(oldstack, 0);
   if (help.where)
   {
      if (*(help.where))
      {
         if (p->custom_flags & NO_PAGER)
            tell_player(p, help.where);
         else
            pager(p, help.where, 1);
      } else
      {
         tell_player(p, " Couldn't find that help file ...\n");
      }
      FREE(help.where);
   }
   stack = oldstack;
}

/* assist command */

void assist_player(player * p, char *str)
{
   char *oldstack, *comment;
   player *p2, *p3;

#ifdef TRACK
   sprintf(functionin,"assist_player (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go on_duty first.\n");
       return;
     }

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: assist <person>\n");
      return;
   }
   if (!strcasecmp(str, "me"))
   {
      p2 = p;
   } else
   {
      p2 = find_player_global(str);
      if (!p2)
         return;
   }
   if (p != p2)
   {
      if (p2->residency != NON_RESIDENT)
      {
         tell_player(p, " That person isn't a newbie though ...\n");
         return;
      }
   }
   if (p2->flags & ASSISTED)
   {
      p3 = find_player_absolute_quiet(p2->assisted_by);
      if (p3)
      {
         if (p != p3)
         {
            sprintf(stack, " That person is already assisted by %s.\n",
                    p2->assisted_by);
         } else
         {
            sprintf(stack, " That person has already been assisted by %s."
                           " Oh! That's you that is! *smirk*\n",
                           p2->assisted_by);
         }
         stack = end_string(stack);
         tell_player(p, oldstack);
         stack = oldstack;
         return;
      }
   }
   if (p!=p2)
   {
      p2->flags |= ASSISTED;
      strcpy(p2->assisted_by, p->name);
   }
   oldstack = stack;
   if (p->gender == PLURAL)
     sprintf(stack, "\n -=*> %s are superusers, and would be "
             "happy to assist you in any problems you may have, including "
             "gaining residency, so that you can have your own rooms, and "
             "have your character saved between visits (type \'help "
             "residency\') for " "more information about that.) "
             "To talk to %s, type 'tell %s <whatever>\', "
             "short forms of names usually work as well.\n\n", p->name,
             get_gender_string(p), p->lower_name);
   else
     sprintf(stack, "\n -=*> %s is a superuser, and would be "
             "happy to assist you in any problems you may have, including "
             "gaining residency, so that you can have your own rooms, and "
             "have your character saved between visits (type \'help "
             "residency\') for more information about that.) "
             "To talk to %s, type 'tell %s <whatever>\', "
             "short forms of names usually work as well.\n\n", p->name,
             get_gender_string(p), p->lower_name);

   stack = end_string(stack);
   tell_player(p2, oldstack);
   stack = oldstack;
   if (p!=p2)
   {
      sprintf(stack, " -=*> %s jumps on %s.\n", p->name, p2->name);
      stack = end_string(stack);
      p->flags |= NO_SU_WALL;
      su_wall(oldstack);
      stack = oldstack;
      sprintf(stack, " You assist %s.\n", p2->name);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      sprintf(stack, "%s assists %s", p->name, p2->name);
      stack = end_string(stack);
      log("assist", oldstack);
      stack = oldstack;
   }
}


/* dibbs command */

void dibbs(player * p, char *str)
{
   char *oldstack, *comment;
   player *p2;

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go on_duty first.\n");
       return;
     }

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: dibbs <person>\n");
      return;
   }
   if (!strcasecmp(str, "me"))
   {
      p2 = p;
   } else
   {
      p2 = find_player_global(str);
      if (!p2)
         return;
   }
   if (p!=p2)
   {
      sprintf(stack, " -=*> %s calls dibbs on %s.\n", p->name, p2->name);
      stack = end_string(stack);
      p->flags |= NO_SU_WALL;
      su_wall(oldstack);
      stack = oldstack;
      sprintf(stack, " You call dibbs on %s.\n", p2->name);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
      sprintf(stack, "%s calls dibbs on %s", p->name, p2->name);
      stack = end_string(stack);
      log("assist", oldstack);
      stack = oldstack;
   }
}

/* Confirm if password and email are set on a resident */

void confirm_password(player * p, char *str)
{
   char *oldstack;
   player *p2;

#ifdef TRACK
   sprintf(functionin,"confirm_password (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (!*str)
   {
      tell_player(p, " Format: confirm <name>\n");
      return;
   }
   p2 = find_player_global(str);
   if (!p2)
      return;

   if (p2->residency == NON_RESIDENT)
   {
      tell_player(p, " That person is not a resident.\n");
      return;
   }
   oldstack = stack;

   p2->residency |= NO_SYNC;
   /* check email */
   if (p2->email[0] == 2)
   {
      strcpy(stack, " Email has not yet been set.");
   } else if (p2->email[0] == ' ')
   {
      strcpy(stack, " Email validated set.");
      p2->residency &= ~NO_SYNC;
   } else if (!strstr(p2->email, "@") || !strstr(p2->email, "."))
   {
      strcpy(stack, " Probably not a correct email.");
      p2->residency &= ~ NO_SYNC;
   } else
   {
      strcpy(stack, " Email set.");
      p2->residency &= ~ NO_SYNC;
   }
   stack = strchr(stack, 0);
   if (p2->email[0] != 2 && p2->email[0] != ' ')
   {
      if (p->residency & ADMIN || !(p2->custom_flags & PRIVATE_EMAIL))
      {
         sprintf(stack, " - %s", p2->email);
         stack = strchr(stack, 0);
         if (p2->custom_flags & PRIVATE_EMAIL)
         {
            strcpy(stack, " (private)\n");
         } else
         {
            strcpy(stack, "\n");
         }
      } else
      {
         strcpy(stack, "\n");
      }
   } else
   {
      strcpy(stack, "\n");
   }
   stack = strchr(stack, 0);

   /* password */
   if (p2->password[0] && p2->password[0] != -1)
   {
      strcpy(stack, " Password set.\n");
   } else
   {
      strcpy(stack, " Password NOT-set.\n");
      p2->residency |= NO_SYNC;
   }
   stack = strchr(stack, 0);

   if (p2->residency & NO_SYNC)
      sprintf(stack, " Character '%s' won't be saved.\n", p2->name);
   else
      sprintf(stack, " Character '%s' will be saved.\n", p2->name);
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
}


/* Now this is just plain silly, probly worst than 'crash' */

void hang(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"hang (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   while (1)
      sleep(1);
}


/* Make someone a minister (changed from a ptriv cause it keeps fucking
up the priv checks and is just pointless as a priv) - asty */

void minister(player *p, char *str)
{
   char *oldstack;
   player *d;

#ifdef TRACK
   sprintf(functionin,"minister (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go on_duty first beavis!\n");
       return;
     }

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: minister <player>\n");
      return;
   }
   d = find_player_global(str);
   if (d)
   {
      if (d->system_flags & MINISTER)
      {
      sprintf(stack, " You take the minister's guide from %s.\n", d->name);
      stack = end_string(oldstack);
      tell_player(p, oldstack);
      stack = oldstack;
      sprintf(stack, " -=*> %s take%s the collar from your neck!\n",
p->name,single_s(p));
      stack = end_string(stack);
      tell_player(d, oldstack);
      stack = oldstack;
      sprintf(oldstack, " -=*> %s take%s the duties of minister from %s\n", p->name, 
              single_s(p), d->name);
      stack = end_string(oldstack);
      su_wall_but(p, oldstack);
      log("minister",oldstack);
      stack = oldstack;
      d->system_flags &= ~MINISTER;
      save_player(d);
     }

   else
      {
      sprintf(stack, " You give the title of minister to %s.\n", d->name);
      stack = end_string(oldstack);
      tell_player(p, oldstack);
      stack = oldstack;
      sprintf(stack, " -=*> %s %s made you a minister. =)\n",
p->name,havehas(p));
      stack = end_string(stack);
      tell_player(d, oldstack);
      stack = oldstack;
      sprintf(oldstack, " -=*> %s make%s %s minister\n", p->name,
              single_s(p), d->name);
      stack = end_string(oldstack);
      su_wall_but(p, oldstack);
      log("minister",oldstack);
      stack = oldstack;
      d->system_flags |= MINISTER;
      save_player(d);
   }}
   else
   {
   tell_player(p, " No such player on the program - tough :P\n");
   stack = oldstack;
   }
}


/* unconverse, get idiots out of converse mode */

void unconverse(player *p, char *str)
{
   player *p2;
   saved_player *sp;
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"unconverse (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go on_duty first... Please..... Thanx.\n");
       return;
     }

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: unconverse <player>\n");
      return;
   }
   lower_case(str);
   p2 = find_player_global_quiet(str);
   if (!p2)
   {
      tell_player(p, " Player not logged on, checking saved player files...\n");      
      sp = find_saved_player(str);
      if (!sp)
      {
         tell_player(p, " Can't find saved player file.\n");
         return;
      }
      if (!(sp->residency & SU) && !(sp->residency & ADMIN))
      {
         if (!(sp->custom_flags & CONVERSE))
         {
            tell_player(p, " They aren't IN converse mode!!!\n");
            return;
         }
         sp->custom_flags &= ~CONVERSE;
         set_update(*str);
         sprintf(stack, " You take \'%s' out of converse mode.\n",
                 sp->lower_name);
         stack = end_string(stack);
         tell_player(p, oldstack);
         stack = oldstack;
      } else
      {
         tell_player(p, " You can't do that to them!\n");
      }
      return;
   }
   if (!(p2->custom_flags & CONVERSE))
   {
      tell_player(p, " But they're not in converse mode!!!\n");
      return;
   }
   if (!(p2->residency & SU) && !(p2->residency & ADMIN))
   {
      p2->custom_flags &= ~CONVERSE;
      p2->mode &= ~CONV;
      if (p->gender == PLURAL)
        sprintf(stack, " -=*> %s have taken you out of converse mode.\n",
                p->name);
      else
        sprintf(stack, " -=*> %s has taken you out of converse mode.\n",
                p->name);
      stack = end_string(stack);
      tell_player(p2, oldstack);
      stack = oldstack;
      do_prompt(p2, p2->prompt);
      sprintf(stack, " You take %s out of converse mode.\n", p2->name);
      stack = end_string(stack);
      tell_player(p, oldstack);
      stack = oldstack;
   } else
   {
      tell_player(p, " You can't do that to them!\n");
      sprintf(stack, " -=*> %s tried to unconverse you!\n", p->name);
      stack = end_string(stack);
      tell_player(p2, oldstack);
      stack = oldstack;
   }
}

void unjail(player *p, char *str)
{
   char *oldstack;
   player *p2, dummy;

#ifdef TRACK
   sprintf(functionin,"unjail (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go on_duty first.\n");
       return;
     }

   if (!*str)
   {
      tell_player(p, " Format: unjail <player>\n");
      return;
   }

   if (!strcasecmp(str, "me"))
      p2 = p;
   else
      p2 = find_player_global(str);
   if (!p2)
   {
      tell_player(p, " Checking saved files... ");
      strcpy(dummy.lower_name, str);
      lower_case(dummy.lower_name);
      dummy.fd = p->fd;
      if (!load_player(&dummy))
      {
         tell_player(p, " Not found.\n");
         return;
      } else
      {
         tell_player(p, "\n");
         p2 = &dummy;
         p2->location = (room *) -1;
      }
   }
   if (p2 == p)
   {
      if (p->location == prison)
      {
         tell_player(p, " You struggle to open the door, but to no avail.\n");
         if (p->gender == PLURAL)
           sprintf(stack, " -=*> %s try to unjail %s. *grin*\n", p->name,
                   self_string(p));
         else
           sprintf(stack, " -=*> %s tries to unjail %s. *grin*\n", p->name,
                   self_string(p));
         stack = end_string(stack);
         su_wall_but(p, oldstack);
         stack = oldstack;
      } else
      {
         tell_player(p, " But you're not in jail!\n");
      }
      return;
   }

   if (p2 == &dummy)
   {
      if (!(p2->system_flags & SAVEDJAIL))
      {
         tell_player(p, " Erm, how can I say this? They're not in jail...\n");
         return;
      }
   } else if (p2->jail_timeout == 0 || p2->location != prison)
   {
      tell_player(p, " Erm, how can I say this? They're not in jail...\n");
      return;
   }

   p2->jail_timeout = 0;
   p2->system_flags &= ~SAVEDJAIL;
   if (p2 != &dummy)
   {
     if (p->gender== PLURAL)
       sprintf(stack, " -=*> The %s release you from prison.\n", p->name);
     else
       sprintf(stack, " -=*> %s releases you from prison.\n", p->name);
      stack = end_string(stack);
      tell_player(p2, oldstack);
      stack = oldstack;
      move_to(p2, ENTRANCE_ROOM, 0);
   }

   if (p->gender== PLURAL)
     sprintf(stack, " -=*> The %s release %s from jail.\n", p->name,
p2->name);
   else
     sprintf(stack, " -=*> %s releases %s from jail.\n", p->name, p2->name);
   stack = end_string(stack);
   su_wall(oldstack);
   stack = oldstack;
   sprintf(stack, "%s releases %s from jail.", p->name, p2->name);
   stack = end_string(stack);
   log("jail", oldstack);
   stack = oldstack;
   if (p2 == &dummy)
   {
      save_player(&dummy);
   }
}

/* cut down version of lsu() to just return number of SUs on */
/* This shows only sus that RESSIES can see.. not the REAL amount on */
int count_su()
{
  int count=0;
  player *scan;

  for (scan=flatlist_start;scan;scan=scan->flat_next)
    if (scan->residency & SU && scan->location &&
                !(scan->flags &(BLOCK_SU|OFF_LSU))
                && scan->idle < 300)
      count++;

  return count;
}

/* needed for statistics */
int true_count_su()
{
  int count=0;
  player *scan;

  for (scan=flatlist_start;scan;scan=scan->flat_next)
    if (scan->residency & SU && scan->location)
      count++;

  return count;
}

/*
   Now that we know:
   the number of SUs,
   the number of newbies,
   and the number of ppl on the prog (current_players),
   we can output some stats
   */

void player_stats(player *p, char *str)
{
  char *oldstack;
  player *scan;
  int new = 0,sus = 0;

#ifdef TRACK
   sprintf(functionin,"player_stats (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

  oldstack=stack;

  for (scan=flatlist_start;scan;scan=scan->flat_next)
        {
    if (scan->residency==NON_RESIDENT && scan->location)
      new++;
    if (scan->residency & PSU && scan->location)
        sus++;
        }

  tell_player(p,"Current Program/Player stats:\n");
  sprintf(oldstack," Players on program: %3d\n"
          "      Newbies on   : %3d\n"
          "      Supers on    : %3d\n"
          "      Normal res.  : %3d\n\n",
          current_players,
          new, sus, (current_players-(sus+new)));
  stack=strchr(stack,0);
  *stack++=0;
  tell_player(p,oldstack);
  stack=oldstack;
}

/* Go to the SUs study */

void go_comfy(player *p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"go_comfy (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   command_type |= ADMIN_BARGE;
   if (p->location == comfy)
   {
      tell_player(p, " You're already in the study!\n");
      return;
   }
   if (p->no_move)
   {
      tell_player(p, " You seem to be stuck to the ground.\n");
      return;
   }
   move_to(p, "main.comfy", 0);
}

/* Tell you what mode someone is in */

void mode(player *p, char *str)
{
   player *p2;
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"mode(%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: mode <player>\n");
      return;
   }

   p2 = find_player_global(str);
   if (!p2)
      return;

   if (p2->mode == NONE)
   {
      sprintf(stack, " %s is in no particular mode.\n", p2->name);
   } else if (p2->mode & PASSWORD)
   {
      sprintf(stack, " %s is in Password Mode.\n", p2->name);
   } else if (p2->mode & ROOMEDIT)
   {
      sprintf(stack, " %s is in Room Mode.\n", p2->name);
   } else if (p2->mode & MAILEDIT)
   {
      sprintf(stack, " %s is in Mail Mode.\n", p2->name);
   } else if (p2->mode & NEWSEDIT)
   {
      sprintf(stack, " %s is in News Mode.\n", p2->name);
   } else if (p2->mode & CONV)
   {
      sprintf(stack, " %s is in Converse Mode.\n", p2->name);
   } else
   {
      sprintf(stack, " Ermmm, %s doesn't appear to be in any mode at all.\n",
                 p2->name);
   }
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
}
void abort_shutdown(player *p,char *str)
{
  pulldown(p,"-1");
  return;
}

void echoroomall(player *p,char *str)
{
  char *oldstack;
  oldstack=stack;
  if (strlen(str)<1)
    {
      tell_player(p,"Usage: becho <message>\n");
      return;
    }
  sprintf(oldstack,"%s\n",str);
  stack=end_string(oldstack);
  raw_room_wall(p->location, oldstack);
  stack=oldstack;
  return;
}
void echoall(player *p,char *str)
{
  char *oldstack;
  oldstack=stack;
  if (strlen(str)<1)
    {
      tell_player(p,"Usage: aecho <message>\n");
      return;
    }
  sprintf(oldstack,"%s\n",str);
  stack=end_string(oldstack);
  raw_wall(oldstack);
  stack=oldstack;
  return;
}


/* chlim command, combines all 5 chlims.  */
/* but now there are 6 !! */
/* soon to be 7 !! */

void change_player_limits(player * p, char *str)
{
   char *size;
   int new_size;
   char *oldstack;
   player *p2;
   player dummy;
   char *player;

#ifdef TRACK
   sprintf(functionin,"change_player_limits (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   player = next_space(str);
   *player++ = 0;
   size = next_space(player);
   *size++ = 0;
   new_size = atoi(size);

   if (!str || !player || !size) {
        tell_player(p, " Format: chlim <area> <player> <new limit>\n"
                " Valid areas are: mail, list, room, auto, exit, alias, item\n");
        stack = oldstack;
        return;
        }

   if (new_size < 0)
   {
      tell_player(p, " Now try a _positive_ limit...\n");
      stack = oldstack;
      return;
   }
   p2 = find_player_absolute_quiet(player);
   if (!p2)
   {
     /* load them if they're not logged in */
      strcpy(dummy.lower_name, player);
      lower_case(dummy.lower_name);
      dummy.fd = p->fd;
      /* if they don't exist, say so and return */
      if (!load_player(&dummy))
      {
         tell_player(p, " That player does not exist.\n");
         stack = oldstack;
         return;
      }
      p2 = &dummy;
   }

   /* ok lets get the valid areas */
   if (strcasecmp(str,"room") && strcasecmp(str,"list") &&
        strcasecmp(str,"alias") && strcasecmp(str,"item") && 
        (!(p->residency & ADMIN) || (strcasecmp(str,"exit") &&
        strcasecmp(str,"auto") && strcasecmp(str,"mail")))) {

        tell_player(p, " You can't change THAT limit !! \n");
        stack = oldstack;
        return; }

   if (!check_privs(p->residency, p2->residency))
   {
     /* now now, no messing with your superiors */
      tell_player(p, " You can't do that !!\n");
      sprintf(oldstack, " -=*> %s tried to change your %s limit.\n",
                        p->name, str);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      return;
   }
   /* otherwise change the limit */
   if (!strcasecmp(str,"list")) p2->max_list = new_size;
   else if (!strcasecmp(str,"room")) p2->max_rooms = new_size;
   else if (!strcasecmp(str,"alias")) p2->max_alias = new_size;
   else if (!strcasecmp(str,"mail")) p2->max_mail = new_size;
   else if (!strcasecmp(str,"item")) p2->max_items = new_size; 
   else if (!strcasecmp(str,"exit")) p2->max_exits = new_size;
   else if (!strcasecmp(str,"auto")) p2->max_autos = new_size;

   /* and if they are logged in, tell them */
   if (p2 != &dummy)
   {
      sprintf(oldstack, " -=*> %s has changed your %s limit to %d.\n",
              p->name,str,new_size);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
   } else
   {
      save_player(&dummy);
   }
   /* now log that change please? */
   stack = oldstack;
   if (p2 != &dummy)
     sprintf(oldstack, "%s changed %s's %s limit to %d",
                        p->name,p2->name,str,new_size);
   else
     sprintf(oldstack, "%s changed %s's %s limit to %d - (logged out)",
                        p->name,p2->name,str,new_size);
     stack = end_string(oldstack);
     log("chlim", oldstack);

   tell_player(p, " Tis Done ..\n");
   stack = oldstack;
}

void extend(player * p, char *str) {
   char *size;
   char *oldstack;
   player *p2;
   player dummy;

   oldstack = stack;
   p2 = find_player_global(str);

   if (!str || !p2) {
        tell_player(p, " Format: extend <player>\n");
        stack = oldstack;
        return;
        }
   if (!check_privs(p->residency, p2->residency) && (p != p2))
   {
     /* now now, no messing with your superiors */
      tell_player(p, " You can't do that !!\n");
      sprintf(oldstack, " -=*> %s tried to extend your list limit.\n",
                        p->name, str);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      return;
   }
   if (!(p2->residency & LIST)) {
        /* whats the point? */
        tell_player(p, " That person has no list to extend.\n");
        stack = oldstack;
        return;
        }
   /* otherwise change the limit */
   if (p2->max_list >= 50 && !(p->residency & ADMIN))
tell_player(p, "Sorry, their list is too full, ask an admin to extend it.\n");
   else if (p2->max_list >= 45 && !(p->residency & ADMIN)) {
      p2->max_list = 50;
      sprintf(oldstack, " -=*> %s has increased your list limit to 40.\n",
        p->name);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      tell_player(p, " Tis done ...\n");
      stack = oldstack;
      sprintf(oldstack, "%s extended %s's list to 50", p->name,p2->name);
      stack = end_string(oldstack);
      log("chlim", oldstack);
   } else {
      p2->max_list += 5;
      sprintf(oldstack, " -=*> %s has increased your list limit to %d.\n",
                p->name, p2->max_list);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      tell_player(p, " Tis done ...\n");
      stack = oldstack;
      sprintf(oldstack, "%s extended %s's list to %d", p->name,p2->name,
        p2->max_list);
      stack = end_string(oldstack);
      log("chlim", oldstack);
      }
   stack = oldstack;
}


void blank_something(player *p, char *str)
{
   char           *oldstack;
   player         *p2, dummy;
   char           *person;

#ifdef TRACK
   sprintf(functionin,"blank_something (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," Go on_duty first.\n");
       return;
     }

   oldstack = stack;
   person = next_space(str);
   *person++ = 0;

   if (*person)
     {
       p2 = find_player_absolute_quiet(person);
       if (p2)
         {
   if (!check_privs(p->residency, p2->residency))
             {
               tell_player(p, " You can't do that to THAT person.\n");
               sprintf(stack, " -=*> %s tried to blank %s\'s %s!\n",
                       p->name, p2->name, str);
               stack = end_string(stack);
               su_wall_but(p, oldstack);
               stack = oldstack;
               return;
             }
        if (!strcasecmp(str,"prefix")) p2->pretitle[0] = 0;
        else if (!strcasecmp(str,"desc")) p2->description[0] = 0;
        else if (!strcasecmp(str,"plan")) p2->plan[0] = 0;
        else if (!strcasecmp(str,"title")) p2->title[0] = 0;
        else if (!strcasecmp(str,"comment")) p2->comment[0] = 0;
        else if (!strcasecmp(str, "entermsg")) p2->enter_msg[0] = 0;
        else if (!strcasecmp(str, "url")) p2->alt_email[0] = 0;
        else if (!strcasecmp(str, "logonmsg")) p2->logonmsg[0] = 0;
        else if (!strcasecmp(str, "logoffmsg")) p2->logoffmsg[0] = 0;
        else if (!strcasecmp(str, "irl_name")) p2->irl_name[0] = 0;
        else if (!strcasecmp(str, "exitmsg")) p2->exitmsg[0] = 0;
        else if (!strcasecmp(str, "hometown")) p2->hometown[0] = 0;
        else if (!strcasecmp(str, "madefrom")) p2->ingredients[0] = 0;
        else if (!strcasecmp(str, "fav1")) p2->favorite1[0] = 0;
        else if (!strcasecmp(str, "fav2")) p2->favorite2[0] = 0;
        else if (!strcasecmp(str, "fav3")) p2->favorite3[0] = 0;

         else {
        /* too many sprintf's on the brain... sorry */
                tell_player(p, "Wanna tell me what a ");
                tell_player(p, str);
                tell_player(p, " is exactly? =)\n");
                return; }

         sprintf(stack, " -=*> %s has blanked your %s.\n", p->name, str);
           stack = end_string(stack);
           tell_player(p2, oldstack);
           stack = oldstack;
           sprintf(stack, "%s blanked %s's %s.", p->name, p2->name, str);
           stack = end_string(stack);
           log("blanks", oldstack);
           stack = oldstack;
           sprintf(stack, " -=*> %s has blanked %s's %s.\n", p->name, p2->name, str);
           stack = end_string(stack);
           su_wall(oldstack);
           tell_player(p, " Blanked.\n");
           stack = oldstack;
           return;
         }
       strcpy(dummy.lower_name, person);
       dummy.fd = p->fd;
       if (load_player(&dummy))
         {
   if (!check_privs(p->residency, dummy.residency))
             {
               tell_player(p, " You can't do that to THAT person.\n");
               sprintf(stack, " -=*> %s tried to blank %s\'s %s!\n",
                       p->name, dummy.name, str);
               stack = end_string(stack);
               su_wall_but(p, oldstack);
              stack = oldstack;
               return;
             }
        if (!strcasecmp(str,"prefix")) dummy.pretitle[0] = 0;
        else if (!strcasecmp(str,"desc")) dummy.description[0] = 0;
        else if (!strcasecmp(str,"plan")) dummy.plan[0] = 0;
        else if (!strcasecmp(str,"title")) dummy.title[0] = 0;
        else if (!strcasecmp(str, "entermsg")) dummy.enter_msg[0] = 0;
        else if (!strcasecmp(str, "url")) dummy.alt_email[0] = 0;
        else if (!strcasecmp(str, "logonmsg")) dummy.logonmsg[0] = 0;
        else if (!strcasecmp(str, "logoffmsg")) dummy.logoffmsg[0] = 0;
        else if (!strcasecmp(str, "irl_name")) dummy.irl_name[0] = 0;
        else if (!strcasecmp(str, "exitmsg")) dummy.exitmsg[0] = 0;
        else if (!strcasecmp(str, "hometown")) dummy.hometown[0] = 0;
        else if (!strcasecmp(str, "madefrom")) dummy.ingredients[0] = 0;
        else if (!strcasecmp(str, "fav1")) dummy.favorite1[0] = 0;
        else if (!strcasecmp(str, "fav2")) dummy.favorite2[0] = 0;
        else if (!strcasecmp(str, "fav3")) dummy.favorite3[0] = 0;
         else {
                tell_player(p, "Wanna tell me what a ");
                tell_player(p, str);
                tell_player(p, " is exactly? =)\n");
                return; }

           sprintf(stack, "%s blanked %s's %s.", p->name, dummy.name, str);
           stack = end_string(stack);
           log("blanks", oldstack);
           stack = oldstack;
           sprintf(stack, " -=*> %s has blanked %s's %s (logged out)\n", p->name, dummy.name, str);
           stack = end_string(stack);
           su_wall(oldstack);
           stack = oldstack;
           dummy.location = (room *) -1;
           save_player(&dummy);
           tell_player(p, " Blanked in saved file.\n");
           return;
         }
       else
           tell_player(p, " Can't find that person on the program or in the "
                       "files.\n");
     }
   else
       tell_player(p, " Format: blank <something> <player>\n"
                " You can blank: title, plan, desc, comment, prefix, exitmsg, "
                "url, entermsg, logonmsg, logoffmsg, hometown, irl_name, "
		"madefrom, fav1, fav2, fav3.\n");
 }


/* nice little bump command - might have to make this more nasty... */
void bump_off(player * p, char *str)
{
   char *oldstack, *text;
   player *e;

#ifdef TRACK
   sprintf(functionin,"bump_off (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: bump <person>\n");
      return;
   }

   if ((p->flags & BLOCK_SU) && !(p->residency & HCADMIN))
     {
       tell_player(p," Its generally best to be on_duty to do that...\n");
       return;
     }
   e = find_player_global(str);
   if (e)
   {
      text = stack;
   if (!check_privs(p->residency, e->residency))
      {
         tell_player(p, " Sorry, you can't...\n");
         tell_player(e, " Someone just tried to bump you...\n");
         sprintf(stack, "%s tried to pull %s's away", p->name, e->name);
         stack = end_string(stack);
         log("bump", text);
         stack = text;
      } else
      {
/* asty again gets moral - shouldnt have let me touch the code */
      if (p->gender==PLURAL)
    sprintf(stack, " -=*> The %s all bump %s off the program - naughty!!\n",
        p->name, e->name);
      else
    sprintf(stack, " -=*> %s bumps %s off the program - naughty!!.\n",
        p->name, e->name);
         stack = end_string(stack);
         au_wall_but (p, oldstack);
         stack = oldstack;
         sprintf(stack, " -=*> %s pulled %s off the prog",
                   p->name, e->name);
         stack = end_string(stack);
         quit(e, 0);
         log("bump", text);
         stack = text;
      }
   }
   stack = oldstack;
}

void emoted_wall(player * p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"emoted_wall (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if((p->flags & BLOCK_SU) && !(p->residency & HCADMIN))
     {
       tell_player (p,"Permissions changed...\nOnly kidding {:-) \n"
                    "No, seriously, you cant use ewall when off_duty.\n");
       return;
     }

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: ewall <emote>\n");
      return;
   }
   if (emote_no_break(*str))
     sprintf(oldstack, "  -=*> %s%s ^N<*=- \007\n", p->name, str);
   else
     sprintf(oldstack, "  -=*> %s %s ^N<*=- \007\n", p->name, str);

   stack = end_string(oldstack);
   command_type |= HIGHLIGHT;
   raw_wall(oldstack);
   command_type &= ~HIGHLIGHT;
   stack = oldstack;
   if (emote_no_break(*str))
   LOGF("wall", "%s%s", p->name, str);
   else
   LOGF("wall", "%s %s", p->name, str);
}

/* Ah hell, why the fuck not? ][ */
void thinkin_wall(player * p, char *str)
{
   char *oldstack;

#ifdef TRACK
   sprintf(functionin,"thinkin_wall (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if((p->flags & BLOCK_SU) && !(p->residency & HCADMIN))
     {
       tell_player (p,"Permissions changed...\nOnly kidding {:-) \n"
                    "No, seriously, you cant use twall when off_duty.\n");
       return;
     }

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: twall <think>\n");
      return;
   }
     sprintf(oldstack, "  -=*> %s think%s . o O ( %s ^N) <*=- \007\n",
                        p->name, single_s(p), str);

   stack = end_string(oldstack);
   command_type |= HIGHLIGHT;
   raw_wall(oldstack);
   command_type &= ~HIGHLIGHT;
   stack = oldstack;
   LOGF("wall", "%s thinks . o O ( %s )", p->name, str);
}


void            superview(player * p, char *str)
{
   char *oldstack;
   file logb;

   oldstack = stack;

   if (*str)
   {
      tell_player(p, " Format: snews\n");
      return;
   }
   sprintf(stack, "logs/sunews.log", str);
   stack = end_string(stack);
   logb = load_file_verbose(oldstack, 0);
   if (logb.where)
   {
      if (*(logb.where))
         pager(p, logb.where, 1);
         free(logb.where);
   }
   stack = oldstack;
   }

void            new(player * p, char *str)
{
   char *oldstack;
   file logb;

   oldstack = stack;
   sprintf(stack, "files/version.msg", str);
   stack = end_string(stack);
   logb = load_file_verbose(oldstack, 0);
   if (logb.where)
   {
      if (*(logb.where))
         pager(p, logb.where, 1);
         free(logb.where);
   }
   stack = oldstack;
   }

void            adminview(player * p, char *str)
{
   char *oldstack;
   file logb;

   oldstack = stack;

   if (*str)
   {
      tell_player(p, " Format: anews\n");
      return;
   }
   sprintf(stack, "logs/adnews.log", str);
   stack = end_string(stack);
   logb = load_file_verbose(oldstack, 0);
   if (logb.where)
   {
      if (*(logb.where))
         pager(p, logb.where, 1);
         free(logb.where);
   }
   stack = oldstack;
   }

/* Post to the superuser memo log */

void superpost(player * p, char *str)
{
   char *oldstack;
   if (!*str)
   {
      tell_player(p, " Format: spost <message>\n");
      return;
   }
   if (strlen(str) > 500)
   {
      tell_player(p, " spam the logs? Not likely...\n");
      return;
   }
   oldstack = stack;
   sprintf(stack, "%s :- %s", p->name, str);
   stack = end_string(oldstack);
   log("sunews", oldstack);
   /* echo it to the sus now */
   stack = oldstack;
   sprintf(oldstack, " -(SNEWS)=*> %s :- %s\n", p->name, str);
   stack = end_string(oldstack);
   su_wall(oldstack);
   stack = oldstack;
}

void adminpost(player * p, char *str)
{
   char *oldstack;
   if (!*str)
   {
      tell_player(p, " Format: apost <message>\n");
      return;
   }
   if (strlen(str) > 500)
   {
      tell_player(p, " spam the logs? Not likely...\n");
      return;
   }
   tell_player(p, " Administrator memo logged ...\n");
   oldstack = stack;
   sprintf(stack, "%s :- %s", p->name, str);
   stack = end_string(stack);
   log("adnews", oldstack);
   stack = oldstack;
}

void list_ministers(player * p, char *str)
{
  int count = 0;
  char *oldstack, *prestack;
  player *scan;

  oldstack = stack;
  strcpy(stack, "============================ Ministers on "
"=================================\n");
  stack = strchr(stack, 0);
  for (scan = flatlist_start; scan; scan = scan->flat_next)
  {
    prestack = stack;
    /* hey asty -- you forgot to update this.. I think this is right now.. */
    if (scan->system_flags & MINISTER && scan->location)
    {
      {
        count++;
        *stack = ' ';
        stack++;
        sprintf(stack, "%-20s ", scan->name);
        stack = strchr(stack, 0);
        sprintf(stack, "   ");
        stack = strchr(stack, 0);
        if (count%3 == 0)       *stack++ = '\n';
      }
    }
  }
  if (count%3 != 0)   *stack++ = '\n';
  if (count > 1)
    sprintf(stack, "================== There are %2d Ministers connected"
                  " =======================\n", count);
  else
   if (count == 1)
    sprintf(stack, "================ There is only one Minister connected "
                   "=====================\n", count);
  stack = end_string(stack);
  if (count) tell_player(p, oldstack);
  else  tell_player(p, " There are no ministers on at the moment.\n");
  stack = oldstack;
}
/* list spods =P */

void list_spods(player * p, char *str)
{
  int count = 0;
  char *oldstack, *prestack;
  player *scan;

  oldstack = stack;
  strcpy(stack, "================================ Spods on "
         "=================================\n");
  stack = strchr(stack, 0);
  for (scan = flatlist_start; scan; scan = scan->flat_next)
  {
    prestack = stack;
    if (scan->residency & SPOD && scan->location)
    {
           if (scan->residency & SPOD)
      {
        count++;
        *stack = ' ';
        stack++;
        sprintf(stack, "%-20s ", scan->name);
        stack = strchr(stack, 0);
        if (strlen(scan->spod_class) > 0)
        sprintf(stack, " { %s }                                               ",
                                scan->spod_class);
        else  sprintf(stack, " { No title set }                                     ");
        stack += 50;
        *stack = '\0';
        *stack++ = '\n';
      }
    }
  }
  if (count > 1)
    sprintf(stack, "========================== There are %2d Spods connected"
                  " ===================\n", count);
  else
   if (count == 1)
    sprintf(stack, "======================== There is only one spod connected "
                   "=================\n", count);
  stack = end_string(stack);
  if (count) tell_player(p, oldstack);
  else tell_player(p, " There are no spods on at the moment.\n");
  stack = oldstack;
}

void switch_channel_style(player * p, char *str)
{
        if (p->residency & FOREST_STYLE_CHAN)
        {
                p->residency &= ~FOREST_STYLE_CHAN;
                p->residency |= REGULAR_STYLE_CHAN;
tell_player(p, "You are now using NORMAL su channel commands.\n");
        }
        else if (p->residency & REGULAR_STYLE_CHAN)
        {
                p->residency &= ~REGULAR_STYLE_CHAN;
                p->residency |= FOREST_STYLE_CHAN;
tell_player(p, "You declare yourself a clanlander, and use forest cmds.\n");
        }
        else
        {
                p->residency |= REGULAR_STYLE_CHAN;
                tell_player(p, "No style found, defaulting to normal.\n");
        }
}
void see_player_whois(player * p, char *str)
{
   char *oldstack;
   file help;

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: whois <player>\n" );
      return;
   }
   lower_case(str);
   if (*str == '.')
   {
      tell_player(p, " Uh-uh, cant do that ...\n");
      return;
   }
   sprintf(stack, "files/whois/%s.who", str);
   stack = end_string(stack);
   help = load_file_verbose(oldstack, 0);
   if (help.where)
   {
      if (*(help.where))
      {
         if (p->custom_flags & NO_PAGER)
            tell_player(p, help.where);
         else
            pager(p, help.where, 1);
      } else
      {
         tell_player(p, " Couldn't find a whois for that player ...\n");
      }
      FREE(help.where);
   }
   stack = oldstack;
}


void wall_to_supers(player * p, char *str)
{
   char *oldstack;

   oldstack = stack;
   command_type = 0;

   if (!*str)
   {
      tell_player(p, " Format: suwall <message>\n");
      return;
   }
   if (p->flags & BLOCK_SU)
   {
      tell_player(p, " You can't do su walls when you're ignoring them.\n");
      return;
   }
   sprintf(stack, " -=*> %s\n", str);
   stack = end_string(stack);
   su_wall(oldstack);
   stack = oldstack;
}

/* your lsp is so cool I stole it for lsa */
void list_admins(player * p, char *str)
{
  int count = 0;
  char *oldstack, *prestack;
  player *scan;

  oldstack = stack;
  strcpy(stack, "=============================== Admins on "
         "=================================\n");
  stack = strchr(stack, 0);
  for (scan = flatlist_start; scan; scan = scan->flat_next)
  {
    prestack = stack;
   if (scan->residency & (LOWER_ADMIN|ADMIN|TESTCHAR))
      {
        count++;
         if ((count == 1) || (count ==2))
            sprintf(stack, " Name : ");
         else
            sprintf(stack, "        ");
        stack = strchr(stack, 0);
       /*         if (count%2 ==1)
           sprintf(stack, "%-15s ", scan->name);
        else */
           sprintf(stack, "%-15s ", scan->name);
        stack = strchr(stack, 0);
        if ((count ==1) || (count ==2))
           sprintf(stack, " Idle : ");
        else
           sprintf(stack, "        ");
        stack = strchr(stack, 0);
        if (scan->idle%60 < 10)
        sprintf(stack, "%d:0%d  ", scan->idle/60,scan->idle%60);
        else
        sprintf(stack, "%d:%2d  ", scan->idle/60,scan->idle%60);
        stack = strchr(stack, 0);
        if (count%2 == 0)
           *stack++ = '\n';
        else
           *stack++ = '|';
      }
  }
  if (count%2 == 1)  *stack++ = '\n';
  if (count >= 1)
    sprintf(stack, "========================================================"
              "===================\n");
  stack = end_string(stack);
  if (count) tell_player(p, oldstack);
  else
        tell_player (p, " *wibble* no admins on?!\n");
  stack = oldstack;
}

void reset_total_idle(player * p, char *str)
{
        player *p2, dummy;
        char *oldstack, *temp;
        int temp_time;

   oldstack = stack;
   temp = next_space(str);
   *temp++ = 0;

   if (!*str)
   {
      tell_player(p, " Format: reset_idle <player>\n");
        return;
   }
   if (p->flags & BLOCK_SU)
   {
      tell_player(p, " Go back on duty first beavis.\n");
       return;
   }

   /* p2 = find_player_absolute_quiet(str); */
        p2 = find_player_global(str);
     if(p2) {
      if ((p2->residency == NON_RESIDENT))
      {
         tell_player(p, " That player is non-resident!\n");
         stack = oldstack;
         return;
      }
      if ((p2->residency == BANISHD || p2->residency == SYSTEM_ROOM))
      {
         tell_player(p, " That is a banished NAME, or System Room.\n");
         stack = oldstack;
         return;
      }
      sprintf(oldstack, "\n -=*> %s has reset your idle time...\n",
       p->name);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      p2->total_idle_time = 0;
      sprintf(stack, "%s reset the idle time on %s.", p->name, p2->name);
      stack = end_string(stack);
      log("edtime",oldstack);
      save_player(p2);
      tell_player(p, " Tis Done...\n");
      stack = oldstack;
      sprintf(stack, " -=*> %s resets %s's idle time.\n",p->name,
      p2->name);
      stack = end_string(stack);
      au_wall_but(p, stack);
       stack = oldstack;
        return;
        }
        /* else, load the dummy */
        strcpy(dummy.lower_name, str);
        dummy.fd = p->fd;
        if(load_player(&dummy)) {
     if ((dummy.residency == BANISHD || dummy.residency == SYSTEM_ROOM))
                {
                tell_player(p, " That is a banished NAME, or System Room.\n");
                stack = oldstack;
                return;
                }
      dummy.total_idle_time = 0;
      sprintf(stack, "%s reset the idle time on %s.", p->name, dummy.name);
      stack = end_string(stack);
      log("edtime",oldstack);
      dummy.location = (room *) -1;
      save_player(&dummy);
      tell_player(p, " Tis Done...\n");
      stack = oldstack;
      sprintf(stack, " -=*> %s resets %s's idle time.\n",p->name,
      dummy.name);
      stack = end_string(stack);
      au_wall_but(p, stack);
      stack = oldstack;
      return;
        }

}

/* use this to reset the disclaimer bit for email residency */
void disclaim(player * p, char *str) {

        player *p2;

        if (!*str) {
                tell_player(p, " Format: disclaim <player>\n");
                return;
                }
        p2 = find_player_global(str);
        if (!p2) return;

        p2->system_flags &= ~AGREED_DISCLAIMER;
}
void make_git(player * p, char *str) {

        char *gitstr;
        player *git;

        if (!*str) {
                tell_player(p, " Format: git <player> <gitstring>\n");
                return;
                }
        gitstr = next_space(str);
        if (*gitstr)
                *gitstr++ = 0;
        else {
                tell_player(p, " Format: git <player> <gitstring>\n");
                return;
                }
        git = find_player_global(str);
        if (!git)
                return;

        if (!check_privs(p->residency, git->residency)) {
                tell_player(p, " No way, Jose!\n");
                return;
                }
        /* else, dooooooooooooooooooooooo eeeeeeeeeeeeeeeeeeeet */

        strncpy(git->git_string, gitstr, MAX_DESC - 3);
        strncpy(git->git_by, p->name, MAX_NAME - 3);
        tell_player(p, " All done =) \n");
/*        oldstack = stack; */
        SUWALL(" -=*> %s declares %s to be a git -- %s\n",
                p->name, git->name, gitstr);
	LOGF("git", "%s gits %s - %s", p->name, git->name, gitstr);
}

void clear_git(player * p, char *str) {

        player *p2;

        if (!*str) {
                tell_player(p, " Format: ungit <player>\n");
                return;
                }
        p2 = find_player_global(str);

        if (!p2)        return;

        if (!check_privs(p->residency, p2->residency))
                {       tell_player(p, " No way, Jose! \n");
                        return;
                }
        p2->git_string[0] = 0;
        p2->git_by[0] = 0;

        tell_player(p, " All done.\n");
	SUWALL(" -=*> %s ungits %s.\n", p->name, p2->name);
	LOGF("git", "%s ungits %s.", p->name, p2->name);
}


/* A "lesser warn" -- idea from kw, but code is spoon of old warn command */

void lesser_warn(player * p, char *str)
{
   char *oldstack, *msg, *pstring, *final;
   player **list, **step;
   int i,n, old_com, r=0, self = 0;

#ifdef TRACK
   sprintf(functionin,"show_malloc(%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   align(stack);
   command_type = PERSONAL | SEE_ERROR | WARNING;

   if (p->tag_flags & BLOCK_TELLS)
   {
    tell_player(p, " You are currently BLOCKING TELLS. It might be an idea to"
                     " unblock so they can reply, eh?\n");
   }
   msg = next_space(str);
   if (*msg)
      *msg++ = 0;
   if (!*msg)
   {
      tell_player(p, " Format: ask <player(s)> <message>\n");
      stack = oldstack;
      return;
   }

   if (p->flags & BLOCK_SU)
     {
       tell_player(p," You cannot ask when off_duty\n");
       stack=oldstack;
       return;
     }

   /* no warns to groups */
   if (!strcasecmp(str, "everyone") || !strcasecmp(str, "friends")
       || !strcasecmp(str, "supers") || !strcasecmp(str, "sus")
       || strstr(str, "everyone"))
   {
      tell_player(p, " Now that would be a bit silly wouldn't it?\n");
      stack = oldstack;
      return;
   }
   if (!strcasecmp(str, "room")) r=1;
   /* should you require warning, the consequences are somewhat (less) severe */   
   if (!strcasecmp(str, "me"))
   {
      tell_player(p, " Ummmmmmmmmmmmmmmmmmmmmm no. \n");
      stack = oldstack;
      return;
   }
   list = (player **) stack;
   n = global_tag(p, str);
   if (!n)
   {
      stack = oldstack;
      return;
   }
   final = stack;
   if (r) {
   if (p->gender==PLURAL)
     sprintf(stack, " %s caution everyone in the room '%s'\n", p->name, msg);
   else
     sprintf(stack, " %s cautions everyone in the room '%s'\n", p->name, msg);
   } else {
   if (p->gender==PLURAL)
     sprintf(stack, " %s caution you '%s'\n", p->name, msg);
   else
     sprintf(stack, " %s cautions you '%s'\n", p->name, msg);
   }
   stack = end_string(stack);
   for (step = list, i = 0; i < n; i++, step++)
   {
      if (*step != p)
      {
         command_type |= HIGHLIGHT;
         tell_player(*step, final);
         (*step)->warn_count++;
         p->num_warned++;
         command_type &= ~HIGHLIGHT;
      }
   }
   stack = final;

   pstring = tag_string(p, list, n);
   final = stack;
   if (p->gender==PLURAL)
     sprintf(stack, " -=*> %s caution %s '%s'", p->name, pstring, msg);
   else
     sprintf(stack, " -=*> %s cautions %s '%s'", p->name, pstring, msg);
   stack = end_string(stack);
   log("ask", final);
   strcat(final, "\n");
   stack = end_string(final);
   command_type = 0;
   su_wall(final);

   cleanup_tag(list, n);
   stack = oldstack;
}


void edfirst(player * p, char *str)
{
   player *p2;
   char *oldstack;
   int temp_time;
   char *temp;

   oldstack = stack;
   temp = next_space(str);
   *temp++ = 0;

   if (!*str || !*temp)
   {
      tell_player(p, " Format: edfirst <player> [-] time (days)>\n");
        return;
   }
   if (p->flags & BLOCK_SU)
   {
      tell_player(p, " Go back on duty first beavis.\n");
       return;
   }
   /* p2 = find_player_absolute_quiet(str); */
        p2 = find_player_global(str);
      if ((!p2))
      {
         tell_player(p, " Only when the player is logged on, chief!\n");
         stack = oldstack;
         return;
      }
      if ((p2->residency == NON_RESIDENT))
      {
         tell_player(p, " That player is non-resident!\n");
         stack = oldstack;
         return;
      }
      if ((p2->residency == BANISHD || p2->residency == SYSTEM_ROOM))
      {
         tell_player(p, " That is a banished NAME, or System Room.\n");
         stack = oldstack;
         return;
      }
      sprintf(oldstack, "\n -=*> %s has changed your date of first login...\n",
       p->name);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      temp_time = atoi(temp);
      /* convert to hours */
      temp_time *=3600;
      /* and to days */
      temp_time *= 24;
      p2->first_login_date += temp_time;
      sprintf(stack, "%s changed the initial login date of %s by %s days.", 
	p->name, p2->name, temp);
      stack = end_string(stack);
      log("edtime",oldstack);
      save_player(p2);
      tell_player(p, " Tis Done...\n");
      stack = oldstack;
      sprintf(stack, " -=*> %s changes the initial login date of %s.\n",p->name,      
	p2->name);
      stack = end_string(stack);
      au_wall_but(p, stack);
       stack = oldstack;
}



void edidle(player * p, char *str)
{
   player *p2;
   char *oldstack;
   int temp_time;
   char *temp;

   oldstack = stack;
   temp = next_space(str);
   *temp++ = 0;

   if (!*str || !*temp)
   {
      tell_player(p, " Format: edidle <player> [-] time (hours)>\n");
        return;
   }
   if (p->flags & BLOCK_SU)
   {
      tell_player(p, " Go back on duty first beavis.\n");
       return;
   }

   /* p2 = find_player_absolute_quiet(str); */
        p2 = find_player_global(str);
      if ((!p2))
      {
         tell_player(p, " Only when the player is logged on, chief!\n");
         stack = oldstack;
         return;
      }
      if ((p2->residency == NON_RESIDENT))
      {
         tell_player(p, " That player is non-resident!\n");
         stack = oldstack;
         return;
      }
      if ((p2->residency == BANISHD || p2->residency == SYSTEM_ROOM))
      {
         tell_player(p, " That is a banished NAME, or System Room.\n");
         stack = oldstack;
         return;
      }
      sprintf(oldstack, "\n -=*> %s has changed your total login time...\n",
       p->name);
      stack = end_string(oldstack);
      tell_player(p2, oldstack);
      stack = oldstack;
      temp_time = atoi(temp);
      /* convert to hours */
      temp_time *=3600;
      p2->total_idle_time += temp_time;
      sprintf(stack, "%s changed the idle time of %s by %s hours.",
                p->name, p2->name, temp);
      stack = end_string(stack);
      log("edtime",oldstack);
      save_player(p2);
      tell_player(p, " Tis Done...\n");
      stack = oldstack;
      sprintf(stack, " -=*> %s changes the truespod login time of %s.\n",p->name,
      p2->name);
      stack = end_string(stack);
      au_wall_but(p, stack);
       stack = oldstack;
}


void marry_edit(player * p, char *str) {

        char *newmn;
        player *p2;

        if (!*str) {
                tell_player(p, " Format: medit <player> <new spouce>\n");
                return;
                }
        newmn = next_space(str);
        if (*newmn)
                *newmn++ = 0;
        else {
                tell_player(p, " Format: medit <player> <new spouce>\n");
                return;
                }
        p2 = find_player_global(str);
        if (!p2)
                return;

        strncpy(p2->married_to, newmn, MAX_NAME - 3);
	p2->system_flags |= MARRIED;
        tell_player(p, " All done =) \n");
	TELLPLAYER(p2, " -=*> %s has married you to %s.\n", p->name, p2->married_to);
	LOGF("marry", "%s medits %s's spouses name to %s", p->name, p2->name, p2->married_to);
}

void list_builders(player * p, char *str)
{
  int count = 0;
  char *oldstack, *prestack;
  player *scan;

  oldstack = stack;
  strcpy(stack, "============================ Builders on "
"==================================\n");
  stack = strchr(stack, 0);
  for (scan = flatlist_start; scan; scan = scan->flat_next)
  {
    prestack = stack;
    /* hey asty -- you forgot to update this.. I think this is right now.. */
    if (scan->system_flags & BUILDER && scan->location)
    {
      {
        count++;
        *stack = ' ';
        stack++;
        sprintf(stack, "%-20s ", scan->name);
        stack = strchr(stack, 0);
        sprintf(stack, "   ");
        stack = strchr(stack, 0);
        if (count%3 == 0)       *stack++ = '\n';
      }
    }
  }
  if (count%3 != 0)   *stack++ = '\n';
  if (count > 1)
    sprintf(stack, "================= There are %2d Builders connected"
                  " =========================\n", count);
  else
   if (count == 1)
    sprintf(stack, "=============== There is only one Builder connected "
                   "=======================\n", count);
  stack = end_string(stack);
  if (count) tell_player(p, oldstack);
  else  tell_player(p, " There are no builders on at the moment.\n");
  stack = oldstack;
}
