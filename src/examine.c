/*
 * examine.c
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

extern int      check_password(char *, char *, player *);
extern char    *crypt(char *, char *);
extern void     check_list_resident(player *);
extern void     check_list_newbie(char *);
extern void     destroy_player(), save_player(), password_mode_on(),
                password_mode_off(), sub_command(), extract_pipe_global(), tell_room(),
                extract_pipe_local(), pstack_mid(), prs_record_display();
extern player  *find_player_global(), *find_player_absolute_quiet(),
               *find_player_global_quiet();
extern char    *end_string(), *tag_string(), *next_space(), *do_pipe(), *full_name(),
               *caps(), *sys_time();
extern int      global_tag(), emote_no_break();
extern file     idle_string_list[];
extern saved_player *find_saved_player();
extern char    *convert_time(time_t);
extern char    *gstring_possessive(player *);
extern char    *gstring(player *);
extern void     su_wall(char *), check_clothing(player *);
extern char    *number2string(int);
extern char    *get_gender_string(player *);
extern char    *havehas(player *);
extern char    *isare(player *);
extern char    *waswere(player *);
extern char    *word_time(int);
extern char    *time_diff(int), *time_diff_sec(time_t, int);
extern char     sess_name[];
extern int      session_reset;
extern list_ent *fle_from_save();
extern void	ADDSTACK(), ENDSTACK(), TELLPLAYER();
#ifdef TRACK
extern int addfunction(char *);
#endif


/* show what somone can do */

void            privs(player * p, char *str)
{

    char           *oldstack = stack, name[MAX_NAME + 2], *first;
    int             priv, who = 0;
    player         *p2;
    player          dummy;
    
#ifdef TRACK
    sprintf(functionin,"privs(%s , SOMETHING)",p->name);
    addfunction(functionin);
#endif
    
    /* assume you are looking at your privs */
    strcpy(name, " You");

    /* convert possible name to lower case */
    lower_case(str);
    /* check if the person executing it is an SU */    
    if (*str && p->residency & (SU | ADMIN))
    {
	/* look for person on program - this will report if they
	   are logged out at the time (ie if it fails) */
	p2 = find_player_global(str);
	if (!p2)
	    /* if player is logged out, try and load them into dummy */
	{
	    /* setup name */
	    strcpy(dummy.lower_name, str);
	    lower_case(dummy.lower_name);
	    /* and an fd for messages */
	    dummy.fd = p->fd;
	    /* actually try loading them */
	    if (!load_player(&dummy))
		/* if we can't load them, report abject failure and exit */
	    {
		tell_player(p, " Couldn't find player in saved files.\n");
		return;
	    }
	    /* lets see if this fixes banished privs shit.. */
	    if (dummy.residency & BANISHD) {
		tell_player(p, " That is a banished name or player.\n");
	        return;
	    }
		/* otherwise set p2 so the gender stuff below works */
	    p2 = &dummy;
	}
	    /* setup name */
	    strcpy(name, p2->name);
	    /* and privs */
	    priv = p2->residency;
	    /* flag it as another person's privs */
	    who = 1;
	    /* print the title to the stack */
	    ADDSTACK(" Permissions for %s.\n", name);
    }
    else
	/* the person wants their own privs */
	/* so get person's own privs :-) */
	priv = p->residency;

	/* capitalise name again */
    name[0] = toupper(name[0]);
    
    if (priv == NON_RESIDENT)
    {
	TELLPLAYER(p, "%s will not be saved... not a resident, you see..\n", name);
	stack = oldstack;
	return;
    }
    if (priv == SYSTEM_ROOM)
    {
	TELLPLAYER(p, "%s is a System Room\n", name);
	stack = oldstack;
	return;
    }
    if (who == 0)
	/* privs for yourself */
    {
	if (priv & BASE)
	    ADDSTACK(" You are a resident.\n");
	else
	    ADDSTACK(" You aren't a resident! EEK! Talk to a superuser!\n");
	
	if (priv & LIST)
	    ADDSTACK(" You have a list.\n");
	else
	    ADDSTACK(" You do not have a list.\n");
	
	if (priv & ECHO_PRIV)
	    ADDSTACK(" You can echo.\n");
	else
	    ADDSTACK(" You cannot echo.\n");
	
	if (priv & BUILD)
	    ADDSTACK(" You can use room commands.\n");
	else
	    ADDSTACK(" You can't use room commands.\n");
	
	if (priv & MAIL)
	    ADDSTACK(" You can send mail.\n");
	else
	    ADDSTACK(" You cannot send any mail.\n");
	
	if (priv & SESSION)
	    ADDSTACK(" You can set sessions.\n");
	else
	    ADDSTACK(" You cannot set sessions.\n");
	
        if (p->system_flags & MARRIED)
            ADDSTACK(" You are happily net-married to %s.\n", p->married_to);
        if (p->system_flags & MINISTER)
            ADDSTACK(" You can perform net-marriages.\n");
        if (p->system_flags & BUILDER)
            ADDSTACK(" You can create elaborate objects.\n");
        if (priv & SPOD)
            ADDSTACK(" You are a spod (but you already KNEW that)\n");
	if (priv & NO_TIMEOUT)
	    ADDSTACK(" You will never time-out.\n");
	if (priv & PSU)
	    ADDSTACK(" You can see the SU channel.\n");
	if (priv & REGULAR_STYLE_CHAN)
	    ADDSTACK(" You are using normal channel commands.\n");
	if (priv & FOREST_STYLE_CHAN)
	    ADDSTACK(" You are using forest style commands...\n");    
	if (priv & WARN)
	    ADDSTACK(" You can warn people.\n");
	if (priv & DUMB)
	    ADDSTACK(" You can dumb people (turn em into tweedles).\n");
	if (priv & SCRIPT)
	    ADDSTACK(" You can use extended scripting.\n");
	if (priv & TRACE)
	    ADDSTACK(" You can trace login sites.\n");
	if (priv & SU && !(priv & (ASU | LOWER_ADMIN | ADMIN | HCADMIN)))
	    ADDSTACK(" You are a superuser-in-training.\n");
	if (priv & ASU && !(priv & (LOWER_ADMIN | ADMIN | HCADMIN)))
	    ADDSTACK(" You are a Super User.\n");
	if (priv & LOWER_ADMIN && !(priv & (ADMIN | HCADMIN)))
	    ADDSTACK(" You are a lower admin.\n");
	
	if (priv & ADMIN && !(priv & HCADMIN))
	    ADDSTACK(" You are an admin/spoon.\n");
	
	if (priv & HCADMIN)
	    ADDSTACK(" You are a hard-coded admin/spoon.\n");

	if (priv & HOUSE)
	    ADDSTACK(" You witnessed the Great OL/DL disaster of '94.\n");
	
    }
    if (who == 1)
	/* privs for someone else */
    {
         {
	if (priv & BASE)
	    ADDSTACK( "%s %s resident.\n",name, isare(p2));
	else
	    ADDSTACK("%s %s not resident! EEK!\n",name, isare(p2));
	
	if (priv & LIST)
	    ADDSTACK( "%s %s a list.\n", name, havehas(p2));
	else
	    ADDSTACK( "%s %s no list.\n", name);
	
	if (priv & ECHO_PRIV)
	    ADDSTACK( "%s can echo.\n", name);
	else
	    ADDSTACK( "%s cannot echo.\n", name);
	
	if (priv & BUILD)
	    ADDSTACK("%s can use room commands.\n", name);
	else
	    ADDSTACK("%s can't use room commands.\n", name);
	
	if (priv & MAIL)
	    ADDSTACK("%s can send mail.\n", name);
	else
	    ADDSTACK("%s cannot send any mail.\n", name);
	
	if (priv & SESSION)
	    ADDSTACK("%s can set sessions.\n", name);
	else
	    ADDSTACK("%s cannot set sessions.\n", name);
	
        if (p2->system_flags & MARRIED)
            ADDSTACK("%s %s happily net-married to %s.\n", name, isare(p2), p2->married_to);
            
        if (priv & SPOD)
            if(p2->gender == PLURAL)  
               ADDSTACK( "%s are sad spods.\n", name);
	    else
               ADDSTACK( "%s is a sad spod.\n", name);

        if (p2->system_flags & MINISTER)
            ADDSTACK("%s can perform net-marriages.\n", name);
        if (p2->system_flags & BUILDER)
            ADDSTACK("%s can create amazing items.\n", name);
        if (priv & GIT)
            ADDSTACK("%s cant do much but sit back and enjoy the view.\n", name);

        if (priv & PROTECT)
            ADDSTACK("%s %s carrying a golden parachute.\n", name, isare(p2));
	if (priv & NO_TIMEOUT)
	    ADDSTACK("%s will never time-out.\n",name);
	if (priv & PSU)
	    ADDSTACK("%s can see the SU channel.\n",name);
	if (priv & FOREST_STYLE_CHAN)
	    ADDSTACK("%s is using forest style channel commands *sigh*\n", name);
	if (priv & REGULAR_STYLE_CHAN)
	    ADDSTACK("%s is using NORMAL channel commands.\n", name);
	if (priv & WARN)
	    ADDSTACK("%s can warn people.\n",name);
	if (priv & DUMB)
	    ADDSTACK("%s can dumb people.\n",name);
	if (priv & SCRIPT)
	    ADDSTACK("%s can use extended scripting.\n",name);
	if (priv & TRACE)
	    ADDSTACK("%s can trace login sites.\n",name);
	if (priv & SU)
	    if (p2->gender == PLURAL)
		ADDSTACK("%s are superusers-in-training.\n",name);
	    else
		ADDSTACK("%s is a superuser-in-training.\n",name);

	if (priv & ASU)
	   if (p2->gender == PLURAL)
		ADDSTACK("%s are full superusers.\n",name);
	   else
		ADDSTACK("%s is a full superuser.\n",name);
	
	if (priv & LOWER_ADMIN)
	   if (p2->gender == PLURAL)
		ADDSTACK("%s are lower admins.\n",name);
	   else
		ADDSTACK("%s is a lower admin.\n",name);
	
	if (priv & ADMIN)
	   if (p2->gender == PLURAL)
		ADDSTACK("%s are admins/spoons.\n",name);
	   else
		ADDSTACK("%s is an admin/spoon.\n",name);
	
	if (priv & HCADMIN)
	   if (p2->gender == PLURAL)
		ADDSTACK("%s are hard-coded admins/spoons.\n",name);
	   else
		ADDSTACK("%s is a hard-coded admin/spoon.\n",name);
	if (priv & HOUSE)
	   if (p2->gender == PLURAL)
		ADDSTACK("%s have been around the block a few times.\n",name);
	   else
		ADDSTACK("%s has been around the block a few times.\n",name);
    }
    }
	
	/* finish off the end of the chunk of data */
    ENDSTACK("-----------------------------------\n");
    tell_player(p, oldstack);
    stack = oldstack;
}


void            friend_finger(player * p)
{
   char           *oldstack, *temp;
   list_ent       *l;
   saved_player   *sp;
   player          dummy, *p2;
   int             jettime, friend = 0;

#ifdef TRACK
   sprintf(functionin,"friend_finger (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   memset(&dummy, 0, sizeof(player));
   oldstack = stack;
   if (!p->saved)
   {
      tell_player(p, " You have no save information, and therefore no "
        "friends ...\n");
      return;
   }
   sp = p->saved;
   l = sp->list_top;
   if (!l)
   {
      tell_player(p, " You have no list ...\n");
      return;
   }
   strcpy(stack, "\n Your friends were last seen...\n");
   stack = strchr(stack, 0);
   do
   {
      if (l->flags & FRIEND && strcasecmp(l->name, "everyone"))
      {
         p2 = find_player_absolute_quiet(l->name);
         friend = 1;
         if (p2)
         {
            sprintf(stack, " %s is logged on.\n", p2->name);
            stack = strchr(stack, 0);
         } else
         {
            temp = stack;
            strcpy(temp, l->name);
            lower_case(temp);
            strcpy(dummy.lower_name, temp);
            dummy.fd = p->fd;
            if (load_player(&dummy))
            {
               switch (dummy.residency)
               {
                  case BANISHED:
                     sprintf(stack, " %s is banished (Old Style)\n",
                             dummy.lower_name);
                     stack = strchr(stack, 0);
                     break;
                  case SYSTEM_ROOM:
                     sprintf(stack, " %s is a system room ...\n", dummy.name);
                     stack = strchr(stack, 0);
                     break;
                  default:
                     if (dummy.residency == BANISHD)
                     {
                        sprintf(stack, " %s is banished. (Name Only)\n",
                                dummy.lower_name);
                        stack = strchr(stack, 0);
                     } else if ( dummy.residency & BANISHD)
                     {
                        sprintf(stack, " %s is banished.\n", dummy.lower_name);
                        stack = strchr(stack, 0);
                     } else
                     {
                        if (dummy.saved)
                           jettime = dummy.saved->last_on + (p->jetlag * 3600);
                        else
                           jettime = dummy.saved->last_on;
                        sprintf(stack, " %s was last seen at %s.\n", dummy.name,
                                convert_time(jettime));
                        stack = strchr(stack, 0);
                    }
                    break;
               }
            } else
            {
               sprintf(stack, " %s doesn't exist.\n", l->name);
               stack = strchr(stack, 0);
            }
         }
      }
      l = l->next;
   } while (l);
   if (!friend)
   {
      tell_player(p, " But you have no friends !!\n");
      stack = oldstack;
      return;
   }
   stack = strchr(stack, 0);
   *stack++ = '\n';
   *stack++ = 0;
   pager(p, oldstack, 0);
   stack = oldstack;
   return;
}


/* command to list pinfo about a saved person */
 
void pinfo_saved_player(player * p, char *str)
{
   player dummy, *p2;
   char *oldstack;
 
#ifdef TRACK
    sprintf(functionin,"check_info (%s , SOMETHING)",p->name);
    addfunction(functionin);
#endif
 
   oldstack = stack;
   memset(&dummy, 0, sizeof(player));
 
      strcpy(dummy.lower_name, str);
      lower_case(dummy.lower_name);
      dummy.fd = p->fd;
      if (!load_player(&dummy))
      {
	 tell_player(p, " No such person in saved files.\n");
	 return;
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
	    return;
 	 }
 	 break;
   }
   stack = strchr(stack, 0);

   oldstack = stack;
   sprintf(stack, " Listing pinfo for %s (logged out atm).\n"
   "======================================================\n"
   "ENTERMSG :%s %s\nIGNOREMSG: %s  \nLOGONMSG :%s %s\n"
   "LOGOFFMSG:%s %s\nBLOCKMSG :%s %s\nEXITMSG  :%s %s\n"
   "======================================================\n",
   dummy.name, dummy.name, dummy.enter_msg, dummy.ignore_msg, dummy.name, 
   dummy.logonmsg, dummy.name, dummy.logoffmsg, dummy.name, dummy.blockmsg,
   dummy.name, dummy.exitmsg);
   stack = strchr(stack, 0);
   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}
void            pinfo_command(player * p, char *str)
{
   player         dummy, *p2;
   char           *oldstack;

#ifdef TRACK
   sprintf(functionin,"pinfo_command (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if ((*str) && (p->residency & SU))
   {   
      p2 = find_player_absolute_quiet(str);
      if (!p2)
	{
	pinfo_saved_player(p, str);
	return;
	}
      else
	 {
   strcpy(stack, "========================================================"
                 "=====================\n");
	 stack = strchr(stack, 0);
	 sprintf(stack, " Pinfo for %s. \n", p2->name);
	 stack = strchr(stack, 0);
	 }
   }
   else 
      {
      p2 = p;
      strcpy(stack, "====================================================="
	     "======================\n Your Pinfo...\n");
      stack = strchr(stack, 0);
      }
   strcpy(stack, "========================================================"
                 "===================\n");
   stack = strchr(stack, 0);
   if (p2 == p || p->residency & SU)
   {
      if(emote_no_break(*p2->enter_msg))
	 sprintf(stack, "Your entermsg is ...\n  %s%s\n", 
	    p2->name, p2->enter_msg);
      else
	 sprintf(stack, "Your entermsg is ...\n  %s %s\n",
	    p2->name, p2->enter_msg);

      stack = strchr(stack, 0);
   }
   if ((p2 == p || p->residency & LOWER_ADMIN) && p2->residency & BASE)
   {
      if (strlen(p2->ignore_msg) > 0)
	 sprintf(stack, "Your ignoremsg is set to ...\n  %s\n", p2->ignore_msg);
      else
	 strcpy(stack, "You have not set an ignoremsg yet ...\n");
      stack = strchr(stack, 0);
   }
   if ((p2 == p || p->residency & SU) && p2->residency & BASE)

   {
      if (strlen(p2->logonmsg) > 0)
	{ 
	 if(emote_no_break(*p2->logonmsg))
	 sprintf(stack, "Your logonmsg is set to ...\n  %s%s\n", 
				 p2->name, p2->logonmsg);
	 else
	 sprintf(stack, "Your logonmsg is set to ...\n  %s %s\n", 
				 p2->name, p2->logonmsg);
	}
      else
	 strcpy(stack, "You have not set a logonmsg yet ...\n");
      stack = strchr(stack, 0);
   }
   if ((p2 == p || p->residency & SU) && p2->residency & BASE)
   {
      if (strlen(p2->logoffmsg) > 0)
	{ 
	 if(emote_no_break(*p2->logoffmsg))
	 sprintf(stack, "Your logoffmsg is set to ...\n  %s%s\n", 
				 p2->name, p2->logoffmsg);
	 else
	 sprintf(stack, "Your logoffmsg is set to ...\n  %s %s\n", 
				 p2->name, p2->logoffmsg);
	}
      else
	 strcpy(stack, "You have not set a logoffmsg yet ...\n");
      stack = strchr(stack, 0);
   }
   if (0 && (p2 == p || p->residency & SU) && p2->residency & BASE)
   {
      if (strlen(p2->blockmsg) > 0)
	{ 
	 if(emote_no_break(*p2->blockmsg))
	 sprintf(stack, "Your blockmsg is set to ...\n  %s%s\n", 
				 p2->name, p2->blockmsg);
	 else
	 sprintf(stack, "Your blockmsg is set to ...\n  %s %s\n", 
				 p2->name, p2->blockmsg);
	}
      else
	 strcpy(stack, "You have not set a blockmsg yet ...\n");
      stack = strchr(stack, 0);
   }

    if ((p2 == p || p->residency & SU) && p2->residency & BASE)
    {
       if (strlen(p2->exitmsg) > 0)
       {
       if(emote_no_break(*p2->exitmsg))
 	 sprintf(stack, "Your exitmsg is ...\n  %s%s\n", 
 	    p2->name, p2->exitmsg);
       else
 	 sprintf(stack, "Your exitmsg is ...\n  %s %s\n",
 	    p2->name, p2->exitmsg);
       }
       else strcpy(stack, "You have no exitmsg set.\n");
 
       stack = strchr(stack, 0);
    }
   strcpy(stack, "========================================================"
                 "===================\n");
stack = strchr(stack, 0);
   *stack++ = 0;
   tell_player(p, oldstack);
   stack = oldstack;
}

void su_examine(player * p, char *str)
{
   player dummy, *p2;
   char *oldstack;
   float partic;
   int jettime;

#ifdef TRACK
   sprintf(functionin,"finger (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: y <player>\n");
      return;
   }

   if (!strcasecmp(str, "me"))
      p2 = p;
   else
   {
      p2 = find_player_absolute_quiet(str);
      if (!p2)
      {
         strcpy(dummy.lower_name, str);
         lower_case(dummy.lower_name);
         dummy.fd = p->fd;
         if (!load_player(&dummy))
         {
            tell_player(p, " No such person in saved files.\n");
            return;
         }
         p2 = &dummy;
      }
   }
   switch (p2->residency)
   {
      case BANISHED:
         tell_player(p, " That player has been banished from this program.\n");
         return;
      case SYSTEM_ROOM:
         tell_player(p, " That is where some of the standard rooms are stored."
                        "\n");
         return;
      default:
         if (p2->residency == BANISHD)
         {
            tell_player(p, " That name has been banished from this program.\n");
            return;
         } else if (p2->residency & BANISHD)
         {
            tell_player(p, " That player has been banished from"
                           " this program.\n");
            return;
         }
   }

   sprintf(stack, "================================================"
                  "===========================\n"
                  "%s %s \n"
                  "================================================"
                  "===========================\n",
           p2->name, p2->title);
   stack = strchr(stack, 0);

   if (p2->saved)
   {
      jettime = p2->saved->last_on + (p->jetlag * 3600);
   } else
   {
      jettime = 0;
   }
   if (p2 != &dummy)
   {
      sprintf(stack, "%s %s been logged in for %s since\n%s.\n",
              full_name(p2), havehas(p2), word_time(time(0) - (p2->on_since)),
              convert_time(p2->on_since));
   } else if (p2->saved)
   {
      if (p->jetlag)
      {
         sprintf(stack, "%s %s last seen at %s. (Your time)\n",
                 p2->name, waswere(p2), convert_time(jettime));
      } else
      {
         sprintf(stack, "%s %s last seen at %s.\n", p2->name,
                 waswere(p2), convert_time(p2->saved->last_on));
      }
   }
   stack = strchr(stack, 0);

   sprintf(stack, "%s total login time is %s.\n", caps(gstring_possessive(p2)),
      word_time(p2->total_login));
   stack = strchr(stack, 0);
   sprintf(stack, "%s total adjusted spod time is %s.\n", caps(gstring_possessive(p2)),
		word_time(p2->total_login - p2->total_idle_time));
   stack = strchr(stack, 0); 

   if (p2->residency & BASE)
   {
      sprintf(stack, "%s was ressied by %s on %s\n",
         p2->name, p2->ressied_by, convert_time(p2->first_login_date));
   }
   stack = strchr(stack, 0);
   
   if (p2->time_in_main && p2->total_login) {
	partic = ((float) p2->time_in_main / (float) p2->total_login) * 100;
	if (partic >= 100)
	  sprintf(stack, "%s %s spent 100%% of the time in a main room.\n", p2->name, havehas(p2));
	else
	  sprintf(stack, "%s %s spent %.2f%% of the time in a main room.\n", p2->name, havehas(p2), partic);
	}
   else
	sprintf(stack, "%s %s spent no time at all in main rooms.\n", p2->name, havehas(p2));

   stack = strchr(stack, 0);

   if (p2->warn_count)
   {
      sprintf(stack, "%s %s been warned %d times.\n",
         p2->name, havehas(p2), p2->warn_count);
   }
   stack = strchr(stack, 0);
   if (p2->booted_count)
   {
      sprintf(stack, "%s %s been booted or jailed %d times.\n",
         p2->name, havehas(p2), p2->booted_count);
   }
   stack = strchr(stack, 0);
   if (p2->idled_out_count)
   {
      sprintf(stack, "%s %s idled out of the program %d times.\n",
         p2->name, havehas(p2), p2->idled_out_count);
   }
   stack = strchr(stack, 0);
   if (p2->eject_count)
   {
      sprintf(stack, "%s %s been kicked off the program %d times.\n", 
		p2->name, havehas(p2), p2->eject_count);
   }
   stack = strchr(stack, 0);

   if (p2->system_flags & SAVE_NO_SING)
   {
      sprintf(stack, "%s cannot sing -- ever.\n", p2->name);
   }
   else if (p2 != &dummy && p2->no_sing)
   {
      sprintf(stack, "%s cannot sing for %d more seconds.\n", 
		p2->name, p2->no_shout);
   }
   stack = strchr(stack, 0);

   if (p2->system_flags & SAVENOSHOUT)
   {
      sprintf(stack, "%s cannot shout -- ever.\n", p2->name);
   }
   else if (p2 != &dummy && p2->no_shout)
   {
      sprintf(stack, "%s cannot shout for %d more seconds.\n", 
		p2->name, p2->no_shout);
   }
   stack = strchr(stack, 0);

   if (p2->system_flags & SAVEDFROGGED)
   {
      sprintf(stack, "%s is currently \"dum\"med down ;-).\n", p2->name);
   }
   stack = strchr(stack, 0);
	
   if (p2->system_flags & NO_MSGS)
   {
      sprintf(stack, "%s cannot change pinfo, x or f data...\n", p2->name);
   }
   stack = strchr(stack, 0);
   if (p2->system_flags & DECAPPED)
   {
      sprintf(stack, "%s cannot use capital letters.\n", p2->name);
   }
   stack = strchr(stack, 0);
	
   if (p2->git_string[0])
   {
      pstack_mid(p2->git_by);
      sprintf(stack, "%s\n", p2->git_string);
      stack = strchr(stack, 0);
   }
   strcpy(stack, "========================================================"
                 "===================\n");
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
}


void su_stats(player * p, char *str)
{
   player dummy, *p2;
   char *oldstack;
   int jettime;

#ifdef TRACK
   sprintf(functionin,"finger (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: z <player>\n");
      return;
   }

   if (!strcasecmp(str, "me"))
      p2 = p;
   else
   {
      p2 = find_player_absolute_quiet(str);
      if (!p2)
      {
         strcpy(dummy.lower_name, str);
         lower_case(dummy.lower_name);
         dummy.fd = p->fd;
         if (!load_player(&dummy))
         {
            tell_player(p, " No such person in saved files.\n");
            return;
         }
         p2 = &dummy;
      }
   }
   switch (p2->residency)
   {
      case BANISHED:
         tell_player(p, " That player has been banished from this program.\n");
         return;
      case SYSTEM_ROOM:
         tell_player(p, " That is where some of the standard rooms are stored."
                        "\n");
         return;
      default:
         if (p2->residency == BANISHD)
         {
            tell_player(p, " That name has been banished from this program.\n");
            return;
         } else if (p2->residency & BANISHD)
         {
            tell_player(p, " That player has been banished from"
                           " this program.\n");
            return;
         }
   }
   if (!(p2->residency & PSU))
	{
		tell_player(p, " That person isn't an su..\n");
		return;
	}

   sprintf(stack, "================================================"
                  "===========================\n"
                  "%s %s \n"
                  "================================================"
                  "===========================\n",
           p2->name, p2->title);
   stack = strchr(stack, 0);

   if (p2->saved)
   {
      jettime = p2->saved->last_on + (p->jetlag * 3600);
   } else
   {
      jettime = 0;
   }
   if (p2 != &dummy)
   {
      sprintf(stack, "%s %s been logged in for %s since\n%s.\n",
              full_name(p2), havehas(p2), word_time(time(0) - (p2->on_since)),
              convert_time(p2->on_since));
   } else if (p2->saved)
   {
      if (p->jetlag)
      {
         sprintf(stack, "%s %s last seen at %s. (Your time)\n",
                 p2->name, waswere(p2), convert_time(jettime));
      } else
      {
         sprintf(stack, "%s %s last seen at %s.\n", p2->name,
                 waswere(p2), convert_time(p2->saved->last_on));
      }
   }
   stack = strchr(stack, 0);

   sprintf(stack, "%s total login time is %s.\n", caps(gstring_possessive(p2)),
      word_time(p2->total_login));
   stack = strchr(stack, 0);
   sprintf(stack, "%s total adjusted spod time is %s.\n", caps(gstring_possessive(p2)),
		word_time(p2->total_login - p2->total_idle_time));
   stack = strchr(stack, 0); 

   if (p2->num_ressied)
   {
      sprintf(stack, "%s %s granted residency to %d people.\n",
         p2->name, havehas(p2), p2->num_ressied);
   }
   stack = strchr(stack, 0);
   if (p2->num_warned)
   {
      sprintf(stack, "%s %s warned %d people.\n",
         p2->name, havehas(p2), p2->num_warned);
   }
   stack = strchr(stack, 0);
   if (p2->idled_out_count)
   {
      sprintf(stack, "%s %s idled out of the program %d times.\n",
         p2->name, havehas(p2), p2->idled_out_count);
   }
   stack = strchr(stack, 0);
   if (p2->num_ejected)
   {
      sprintf(stack, "%s %s kicked %d gits off the program.\n", 
		p2->name, havehas(p2), p2->num_ejected);
   }
   stack = strchr(stack, 0);

   if (p2->num_rmd)
   {
      sprintf(stack, "%s removed %d shouts, sings and moves.\n", 
		p2->name, p2->num_rmd);
   }
   stack = strchr(stack, 0);

   if (p2->num_booted)
   {
      sprintf(stack, "%s booted or jailed %d morons.\n", 
		p2->name, p2->num_booted);
   }
   stack = strchr(stack, 0);

   strcpy(stack, "========================================================"
                 "===================\n");
   stack = end_string(stack);
   tell_player(p, oldstack);
   stack = oldstack;
}


void newfinger(player * p, char *str)
{
   player dummy, *p2;
   char *oldstack, datastring[15];
   int jettime, overtime;
   float partic;
   list_ent *l;

#ifdef TRACK
   sprintf(functionin,"finger (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: finger <player>\n");
      return;
   }
   if (!strcasecmp(str, "friends"))
   {
      friend_finger(p);
      return;
   }

   if (!strcasecmp(str, "me"))
      p2 = p;
   else
   {
      p2 = find_player_absolute_quiet(str);
      if (!p2)
      {
         strcpy(dummy.lower_name, str);
         lower_case(dummy.lower_name);
         dummy.fd = p->fd;
         if (!load_player(&dummy))
         {
            tell_player(p, " No such person in saved files.\n");
            return;
         }
         p2 = &dummy;
      }
   }
   if (p2->residency & LIST && !(p2->residency & NO_SYNC))
         l = fle_from_save(p2->saved, p->lower_name);
   switch (p2->residency)
   {
      case BANISHED:
         tell_player(p, " That player has been banished from this program.\n");
         return;
      case SYSTEM_ROOM:
         tell_player(p, " That is where some of the standard rooms are stored."
                        "\n");
         return;
      default:
         if (p2->residency == BANISHD)
         {
            tell_player(p, " That name has been banished from this program.\n");
            return;
         } else if (p2->residency & BANISHD)
         {
            tell_player(p, " That player has been banished from"
                           " this program.\n");
            return;
         }
   }

      if (emote_no_break(*p2->title))
   ADDSTACK("================================================"
                  "===========================\n%s %s%s \n"
                  "================================================="
                  "==========================\n",
           	  p2->pretitle, p2->name, p2->title);
      else
   ADDSTACK("================================================"
                  "===========================\n%s %s %s \n"
                  "================================================="
                  "==========================\n",
           	  p2->pretitle, p2->name, p2->title);

   if (p->jetlag) {
	overtime = p2->on_since + (p->jetlag * 3600);
 strcpy(datastring, "(Your time)"); 
   } else {
	overtime = p2->on_since;
 strcpy(datastring, "(PG time)"); 
  }

   if (p2->saved)
   {
      jettime = p2->saved->last_on + (p->jetlag * 3600);
   } else
   {
      jettime = 0;
   }
   if (p2 != &dummy)
   {
      ADDSTACK("Time on PG so far   : %s\nLogged onto PG at   : %s %s\n",
              word_time(time(0) - (p2->on_since)), convert_time(overtime), datastring);
   } else if (p2->saved)
   {
      if (p->jetlag)
      {
         ADDSTACK("Date last logged on : %s %s\n",
                 convert_time(jettime), datastring);
      } else
      {
         ADDSTACK("Date last logged on : %s\n",
                 convert_time(p2->saved->last_on), datastring);
      }
   }

   ADDSTACK("Total login time    : %s.\n", word_time(p2->total_login));
   ADDSTACK("Truespod login time : %s.\n", word_time(p2->total_login - p2->total_idle_time));

   if (p2->total_login && p2->first_login_date) {
	partic = ((float) p2->total_login / (float) (time(0) - p2->first_login_date)) * 100;
	if (partic < 100)
	  ADDSTACK("Chronic spod factor : %.2f%%\n", partic);
	else
	  ADDSTACK("Chronic spod factor : 100%\n");
   }
   if (p2->age)
      ADDSTACK("Years of age        : %d\n", p2->age);
   if (p2->birthday)
      ADDSTACK("Date of birth       : %s\n", birthday_string(p2->birthday));
   if (p2->system_flags & NEW_MAIL)
      ADDSTACK("Mailbox status      : New mail recieved\n");
   if (p2->residency && (p==p2 || p->residency & (LOWER_ADMIN|ADMIN) 
        || !(p2->custom_flags & PRIVATE_EMAIL) || 
	(l && l->flags & FRIEND && p2->custom_flags & FRIEND_EMAIL))) {
	 
      if (!(p2->email[0])) 
	ADDSTACK("Email address       : Not set.\n"); 
      else if (p2->email[0] == ' ')
        ADDSTACK("Email address       : Set as validated.\n");
      else {
        ADDSTACK("Email address       : %s", p2->email);
      if (p2->custom_flags & FRIEND_EMAIL) 
           ADDSTACK(" (friends)\n");
      else if (!(p2->custom_flags & PRIVATE_EMAIL))
           ADDSTACK(" \n");
      else
	   ADDSTACK(" (private)\n");
      }
   }
   /* alt_email stolen for URL -- you probably knew that tho :P */
   if (p2->alt_email[0])
	ADDSTACK("WWW homepage URL    : %s \n", p2->alt_email);
   if (p2->hometown[0])
	ADDSTACK("Place of residency  : %s \n", p2->hometown);
   if (p2->system_flags & (BUILDER|MINISTER) || p2->residency & (SU|ADMIN))
        ADDSTACK("Online Positions    : ");
   if (p2->system_flags & MINISTER) ADDSTACK("Minister ");
   if (p2->system_flags & BUILDER)  ADDSTACK("Builder ");
   if (p2->residency & SU)	    ADDSTACK("Staff "); 
   if (p2->residency & ADMIN)	    ADDSTACK("Administrator "); 
   if (p2->system_flags & (BUILDER|MINISTER) || p2->residency & (SU|ADMIN))
	ADDSTACK("\n");

      if (p2->system_flags & (MARRIED|FLIRT_BACHELOR|ENGAGED) || !(p2->system_flags & BACHELOR_HIDE))
	{
	ADDSTACK("Marital status      : ");
	stack = strchr(stack, 0);
	if (p2->system_flags & MARRIED) 
		ADDSTACK("Happily net.married to %s\n", p2->married_to);
	else if (p2->system_flags & ENGAGED)
		ADDSTACK("Net.engaged to %s\n", p2->married_to);
	else if (p2->system_flags & FLIRT_BACHELOR)
		ADDSTACK("Horrible net.flirt =)\n");
	else if (p2->gender == FEMALE)
		ADDSTACK("Swinging net.bachelorette\n");
	else
		ADDSTACK("Swinging net.bachelor\n");
	}	
		
   if (p2->ingredients[0]) {
      switch (p2->gender) {
	case FEMALE:
	  ADDSTACK("She is made from    : %s \n", p2->ingredients);
	  break;
 	case MALE:
	  ADDSTACK("He is made from     : %s \n", p2->ingredients);
	  break;
	case PLURAL:
	  ADDSTACK("They are made from  : %s \n", p2->ingredients);
	  break;
	default:
	  ADDSTACK("It is made from     : %s \n", p2->ingredients);
	  break;
	}
   }
   else {
      switch (p2->gender) {
	case FEMALE:
	  ADDSTACK("She is made from    : Sugar and spice and everything nice.\n");
	  break;
 	case MALE:
	  ADDSTACK("He is made from     : Snakes and snails and puppy dog tails.\n");
	  break;
	case PLURAL:
	  ADDSTACK("They are made from  : Um.. everything but the kitchen sink...\n");
	  break;
	default:
	  ADDSTACK("It is made from     : If you knew it would have to kill you....\n");
	  break;
	}
   }
   if (p2->plan[0])
   {
      pstack_mid("plan");
      ADDSTACK("%s \n", p2->plan);
   }
   ENDSTACK("========================================================"
                 "===================\n");
   tell_player(p, oldstack);
   stack = oldstack;
}

/* the examine command */

void            newexamine(player * p, char *str)
{
   player         *p2;
   char           *oldstack;
   char           first[MAX_SPODCLASS], *second;
   list_ent       *l = 0;

#ifdef TRACK
   sprintf(functionin,"examine (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   oldstack = stack;
   if (!*str)
   {
      tell_player(p, " Format: examine <player>\n");
      return;
   }
   if (!strcasecmp("me", str))
      p2 = p;
   else
      p2 = find_player_global(str);
   if (!p2)
      return;
   if (p2->saved && p2->residency & LIST && !(p2->residency & NO_SYNC))
        l = fle_from_save(p2->saved, p->lower_name);
   else
	l = 0;
   if (p2->description[0])
   {
	if (emote_no_break(*p2->title))
   ADDSTACK("===========================================================================\n%s %s%s \n" , p2->pretitle, p2->name, p2->title);
	else
   ADDSTACK("===========================================================================\n%s %s %s \n" , p2->pretitle, p2->name, p2->title);
   pstack_mid("desc");
   ADDSTACK("%s \n" 
   	          "======================================================"
 		  "=====================\n", p2->description);
   }
   else
   {
	if (emote_no_break(*p2->title))
   ADDSTACK("===========================================================================\n"
	"%s %s%s \n===========================================================================\n", p2->pretitle, p2->name, p2->title);
	else
   ADDSTACK("===========================================================================\n" 
	"%s %s %s \n===========================================================================\n", p2->pretitle, p2->name, p2->title);
   }
   if (p==p2 || p->residency & SU || p2->custom_flags & PUBLIC_SITE || 
	(l && l->flags & FRIEND && p2->custom_flags & FRIEND_SITE)) {
      ADDSTACK("Site logged on from   : %s (%s)", p2->inet_addr, p2->num_addr);
      if (p2->custom_flags & FRIEND_SITE) 
           ADDSTACK( " (friends)\n");
      else if (p2->custom_flags & PUBLIC_SITE)
           ADDSTACK( " \n");
      else
	   ADDSTACK(" (private)\n");
      } 
   if (p->jetlag)
      	    ADDSTACK("Time logged on so far : %s\n"
                     "Time logged in        : %s (Your time)\n",
              word_time(time(0) - (p2->on_since)),
              convert_time((p2->on_since + (p->jetlag * 3600))));
   else
   	 ADDSTACK("Time logged on so far : %s\n"
                  "Time logged in        : %s (PG time)\n",
         word_time(time(0) - (p2->on_since)), convert_time(p2->on_since));

   ADDSTACK("Total login time      : %s\n", word_time(p2->total_login));
   ADDSTACK("Truespod login time   : %s\n", word_time(p2->total_login - p2->total_idle_time));

   if (p2->tag_flags & (BLOCK_TELLS|BLOCK_SHOUT|BLOCK_FRIENDS|SINGBLOCK))
   {
   ADDSTACK("Block modes active    : ");
   if (p2->tag_flags & BLOCK_SHOUT)
	 ADDSTACK("shouts, ");
   if (p2->tag_flags & BLOCK_TELLS)
	 ADDSTACK("tells, ");
   if (p2->tag_flags & BLOCK_FRIENDS && !(p2->tag_flags & BLOCK_TELLS))
 	 ADDSTACK("friend tells, ");
   if (p2->tag_flags & SINGBLOCK)
	 ADDSTACK("singing, ");
   stack -=2;
   *stack++ = '.';
   *stack++ = '\n';
   }
   if (p2->age) {
   ADDSTACK("Years of age          : %d\n", p2->age);
   } if (p2->birthday) {
   ADDSTACK("Date of birth         : %s\n", birthday_string(p2->birthday));
   }

   if (p2->residency && p2->saved && (p==p2 || p->residency & (LOWER_ADMIN|ADMIN) 
        || !(p2->custom_flags & PRIVATE_EMAIL) || 
	(l && l->flags & FRIEND && p2->custom_flags & FRIEND_EMAIL))) {
	 
      if (!(p2->email[0])) 
        ADDSTACK("Email address         : None set.\n"); 
      else if (p2->email[0] == ' ')
        ADDSTACK("Email address         : Validated.\n");
      else {
        ADDSTACK("Email address         : %s", p2->email);
      if (p2->custom_flags & FRIEND_EMAIL) 
           ADDSTACK(" (friends)\n");
      else if (!(p2->custom_flags & PRIVATE_EMAIL))
           ADDSTACK(" \n");
      else
	   ADDSTACK(" (private)\n");
      }
   }
   if (p2->irl_name[0])
      ADDSTACK("Also known as (irl)   : %s \n", p2->irl_name);
   if (p2->alt_email[0])
      ADDSTACK("WWW homepage URL      : %s \n", p2->alt_email);
   if (p2->prs_record) {
      ADDSTACK("Paper Rock Scissors   : ");
      prs_record_display(p2);
      }
   if (p2->system_flags & (BUILDER|MINISTER) || p2->residency & (SU|ADMIN))
      ADDSTACK("Online Positions      : ");
   if (p2->system_flags & MINISTER) ADDSTACK("Minister ");
   if (p2->system_flags & BUILDER)  ADDSTACK("Builder ");
   if (p2->residency & SU)	    ADDSTACK("Staff "); 
   if (p2->residency & ADMIN)	    ADDSTACK("Administrator "); 
   if (p2->system_flags & (BUILDER|MINISTER) || p2->residency & (SU|ADMIN))
	ADDSTACK("\n");

      if (p2->system_flags & (MARRIED|FLIRT_BACHELOR|ENGAGED) 
		|| !(p2->system_flags & BACHELOR_HIDE))
	{
	ADDSTACK("Marital status        : ");
	if (p2->system_flags & MARRIED) 
		ADDSTACK("Happily net.married to %s\n", p2->married_to);
	else if (p2->system_flags & ENGAGED)
		ADDSTACK("Net.engaged to %s\n", p2->married_to);
	else if (p2->system_flags & FLIRT_BACHELOR)
		ADDSTACK("Horrible net.flirt =)\n");
	else if (p2->gender == FEMALE)
		ADDSTACK("Swinging net.bachelorette\n");
	else
		ADDSTACK("Swinging net.bachelor\n");
	}	
 strcpy(first, "");
 second=0;
   if(p2->favorite1[0])
	{
	 strcpy(first,p2->favorite1);
	 second = next_space(first);
	 *second++ = 0;
	 ADDSTACK("Favorite %-12.12s : %s \n", first, second);
	}
 strcpy(first, "");
 second=0;
   if(p2->favorite2[0])
	{
	 strcpy(first,p2->favorite2);
	 second = next_space(first);
	 *second++ = 0;
	ADDSTACK("Favorite %-12.12s : %s \n", first, second);
	}
 strcpy(first, "");
 second=0;
   if(p2->favorite3[0])
	{
	 strcpy(first,p2->favorite3);
	 second = next_space(first);
	 *second++ = 0;
	 ADDSTACK("Favorite %-12.12s : %s \n", first, second);
	}
		
   if(p2->hometown[0])
	ADDSTACK("Place of residence    : %s \n", p2->hometown);

   if (p2->ingredients[0]) {
      switch (p2->gender) {
	case FEMALE:
	  ADDSTACK("She is made from      : %s \n", p2->ingredients);
	  break;
 	case MALE:
	  ADDSTACK("He is made from       : %s \n", p2->ingredients);
	  break;
	case PLURAL:
	  ADDSTACK("They are made from    : %s \n", p2->ingredients);
	  break;
	default:
	  ADDSTACK("It is made from       : %s \n", p2->ingredients);
	  break;
	}
   }
   else {
      switch (p2->gender) {
	case FEMALE:
	  ADDSTACK("She is made from      : Sugar and spice and everything nice.\n");
	  break;
 	case MALE:
	  ADDSTACK("He is made from       : Snakes and snails and puppy dog tails.\n");
	  break;
	case PLURAL:
	  ADDSTACK("They are made from    : Um.. everything but the kitchen sink...\n");
	  break;
	default:
	  ADDSTACK("It is made from       : If you knew it would have to kill you....\n");
	  break;
	}
   }
   check_clothing(p2);
   ENDSTACK("========================================================"
                 "===================\n");
   tell_player(p, oldstack);
   stack = oldstack;
}

