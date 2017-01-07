/*  
 *    channel.c
 */

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "player.h"
#include "channel.h"

/* externs */
extern void LOGF(), TELLPLAYER();
extern player *find_player_global(char *);
extern sub_commands(player *,char *,struct command *);
extern char *end_string(char *);
extern void su_wall(char *);
extern void tell_player(player *,char *);
extern char *next_space(char *), *convert2time(time_t);
extern char *numbertime(int); 
extern char *get_gender_string(player *);
extern int check_privs(int, int), emote_no_break(char);
extern void view_sub_commands(player *,struct command *);

/* interns */
struct c_struct channel[NUM_CHANNELS];
void   founder_quitting();


void initchannels(void)
{
	int i;

	for(i=0;i<NUM_CHANNELS;i++) {
		channel[i].inuse=0;
		strcpy(channel[i].topic, "Not yet implemented.");
	}
}

/* seemed like the best way to do it at the time... */
void cwall(char * pname, char * str, int chan, int action, int pfz)
{
	char *oldstack;
	int i;
	player *s;
	int chat=0;

	oldstack=stack;
	i = chan;

	if(i < 0 || i > NUM_CHANNELS || !channel[i].inuse) {
		LOGF("channel", "Invalid channel %d.", chan);
		return;
	}
        if (pfz & FROGGED)
	    action = FROGGED_PERSON;
	switch(action) {
		case SAY:
			sprintf(stack," (%d:%s) %s says '%s^N'\n",i,channel[i].name,pname,str);
			break;
		case SAY_ASK:
			sprintf(stack," (%d:%s) %s asks '%s^N'\n",i,channel[i].name,pname,str);
			break;
		case SAY_EXC:
			sprintf(stack," (%d:%s) %s exclaims '%s^N'\n",i,channel[i].name,pname,str);
			break;
		case EMOTE:
			if (emote_no_break(*str))
			  sprintf(stack," (%d:%s) %s%s^N\n",i,channel[i].name,pname,str);
			else
			  sprintf(stack," (%d:%s) %s %s^N\n",i,channel[i].name,pname,str);
			break;
		case PEMOTE:
			sprintf(stack," (%d:%s) %s's %s^N\n",i,channel[i].name,pname,str);
			break;
		case THINK:
			sprintf(stack," (%d:%s) %s thinks . o O ( %s ^N)\n",i,channel[i].name,pname,str);
			break;
		case SING:
			sprintf(stack," (%d:%s) %s sings o/~ %s ^No/~\n",i,channel[i].name,pname,str);
			break; 
		case MESSAGE:
			sprintf(stack," (%d:%s) %s %s^N\n",i, channel[i].name, pname, str);
			break;
		case FROGGED_PERSON:
			sprintf(stack," (%d:%s) %s is a worthless piece of pond scum.\n",i, channel[i].name, pname);
			break;
		default:
			LOGF("channel", "Invalid action %d in cwall", action);
			stack=oldstack;
			return;
	}
	stack=end_string(stack);
        if (!i || !(i%2))
		sys_color_atm = UCEsc;
	else
		sys_color_atm = UCOsc;

	for(s=flatlist_start;s;s=s->flat_next) {
		if(s->chanflags&(1<<i)) {
			if(s->misc_flags & CHAN_HI) {
				command_type|=HIGHLIGHT;
			}
			tell_player(s,oldstack);
			if(s->misc_flags & CHAN_HI) {
				command_type&=~HIGHLIGHT;
			}
		}
	}
	sys_color_atm = SYSsc;
	stack=oldstack;
}


void cmsg(char * str,int chan) 
{
	cwall(" -> ",str,chan,MESSAGE, 0);
}



void channelcreate(player * p, char *str)
{
	int i;
	int found=0;
	char *oldstack;
	player *s;

	if(!*str) {
		tell_player(p, "Format: chan create <name>\n");
		return;
	}
	if(p->flags&CHANNEL_FOUNDER) {
		tell_player(p, "No, you can only have ONE channel of your own.\n");
		return;
	}
	for(i=0;i<NUM_CHANNELS;i++) {
		if(!channel[i].inuse) {
			channel[i].inuse=1;
			strncpy(channel[i].name,str,MAX_NAME - 3);
			channel[i].starttime=time(0);
			channel[i].idle=0;
                        channel[i].cflags=0;
			channel[i].numppl=1;
			strcpy(channel[i].topic, "Not set yet.");
			p->chanflags|=(1<<i);
			p->opflags|=(1<<i);
			p->flags|=CHANNEL_FOUNDER;
			found=1;
			break;
		}
	}
	if(!found) {
		tell_player(p,"Sorry, all channels are in use. Try again later.\n");
	} else {
		oldstack=stack;
		sprintf(stack,"%s creates channel #%d (%s)",p->name,i,channel[i].name);
		stack=end_string(stack);
		log("channel",oldstack);
		stack=oldstack;
		sprintf(stack," A new channel has been created with ID #%d and name %s\n",i,channel[i].name);
		stack=end_string(stack);
		tell_player(p,oldstack);
		stack=oldstack;
		sprintf(stack," %% %s creates a new channel (%s) with ID %d\n",p->name,channel[i].name,i);
		stack=end_string(stack);
		for (s=flatlist_start; s; s=s->flat_next) {
			if (!(s->tag_flags & BLOCKCHANS))	
				tell_player(s, oldstack);
			}
		stack=oldstack;
	}
}


void channellist(player *p, char *str)
{
	char *oldstack;
	player *f,*s;
	int i;
	int count=0;

	oldstack=stack;
	strcpy(stack,"=[ID]==[CHANNEL NAME]========[CHANNEL OPERATOR]==="
		"=======[PPL]=============\n");
	stack=strchr(stack,0);
	for(i=0;i<NUM_CHANNELS;i++) {
		if(channel[i].inuse) {

			for(s=flatlist_start;s;s=s->flat_next) {
				if(s->opflags&(1<<i)) {
					f=s;
				}
			}
	if(p->chanflags & (1<<i))
  		sprintf(stack," %-3d+  %-20s  %-20s        %-2d\n       Topic: %s \n",i,channel[i].name,f->name,channel[i].numppl,channel[i].topic);
	else if (!(p->c_invites & (1<<i)) && (channel[i].cflags & INVITE_ONLY || 
          (channel[i].cflags & NO_NEWBIES && p->residency == NON_RESIDENT)))	
		sprintf(stack," %-3d*  %-20s  %-20s        %-2d\n       Topic: %s \n",i,channel[i].name,f->name,channel[i].numppl,channel[i].topic);
	else 
  		sprintf(stack," %-3d   %-20s  %-20s        %-2d\n       Topic: %s \n",i,channel[i].name,f->name,channel[i].numppl,channel[i].topic);
	stack=strchr(stack,0);
	count++;
		}
	}
	strcpy(stack,"========================================"
		     "===================================\n");
	stack=end_string(stack);
	if(!count) {
		tell_player(p," Currently there are no channels being used.\n");
	} else {
		tell_player(p,oldstack);
	}
	stack=oldstack;
}


void channeldestroy(int chan, char *str)
{
	player *s;

	cmsg(str, chan);
	channel[chan].inuse=0;
	for(s=flatlist_start;s;s=s->flat_next) {
		if(s->chanflags&(1<<chan)) {
			s->chanflags&=~(1<<chan);
			if(s->opflags&(1<<chan)) {
				s->flags&=~CHANNEL_FOUNDER;
				s->opflags&=~(1<<chan);
			}
		}
		if(s->c_invites&(1<<chan)) { /* kill old invites */
			s->c_invites &=~(1<<chan);
		}
	}
}

void channelkillown(player *p, char *str) {
	
	int chan, found = 0;

	if (*str) {
		tell_player(p, " Format: chan nuke\n");
		return;
		}
	if(!(p->flags&CHANNEL_FOUNDER)) {
		tell_player(p,"Only the operator can kill the channel.\n");
		return;
	}
	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->opflags&(1<<chan)) {
			found=1;
			break;
		}
	}
	if(!found) {
		tell_player(p,"The channel that you're operator of was not found!\n");

		return;
		}
	tell_player(p, "OK, NUKING your channel to a crisp...\n");
	channeldestroy(chan, "Channel destroyed by operator");
}	

	
void channelxfercrown(player *p, char *str) {

	int chan;
	int found=0;
	char *oldstack;
	player *p2;

	if(!*str) {
		tell_player(p,"Format: chan xfer <player>\n");
		return;
	}
	if(!(p->flags&CHANNEL_FOUNDER)) {
		tell_player(p,"Only the operator can xfer authority to people.\n");
		return;
	}
	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->opflags&(1<<chan)) {
			found=1;
			break;
		}
	}
	if(!found) {
		tell_player(p,"The channel that you're operator of was not found!\n");
		return;
	}
	p2 = find_player_global(str);
	if(!p2) {
		return;
	}
	if(!(p2->chanflags&(1<<chan))) {
		oldstack=stack;
		sprintf(stack,"But %s is not in this channel!\n",p2->name);
		stack=end_string(stack);
		tell_player(p,oldstack);
		stack=oldstack;
	} else if (p2->flags & CHANNEL_FOUNDER) {
		oldstack = stack;
		sprintf(stack, "But %s is already the operator of another channel.\n", p2->name);
		stack = end_string(stack);
		tell_player(p, oldstack);
		stack=oldstack;
	} else if (p2->residency == NON_RESIDENT) {
		oldstack = stack;
		strcpy(stack, "That player isn't even a resident!\n");
		stack = end_string(stack);
		tell_player(p, oldstack);
		stack=oldstack;
	} else { 
		p2->opflags|=(1<<chan);
		p2->flags|=CHANNEL_FOUNDER;
		p->opflags &= ~(1<<chan);
		p->flags &= ~CHANNEL_FOUNDER;
		channel[chan].idle=0;
		tell_player(p," Tis done...\n");
		oldstack=stack;
		sprintf(stack," >=- You have been given control of channel #%d by %s.\n",chan,p->name);
		stack=end_string(stack);
		tell_player(p2,oldstack);
		stack=oldstack;
		sprintf(stack,"%s has transfered the operator powers on this channel to %s.\n",p->name,p2->name);
		stack=end_string(stack);
		cmsg(oldstack,chan);
		stack=oldstack;
	}
}

	
void channelquitroot(player *p, char *str, int ch)
{
	player *s;
	int chan;
	int count=0;
	char *oldstack;

	if (ch != -1)
	   chan=ch;
	else
	   chan=atoi(str);

	if(!(p->chanflags&(1<<chan))) {
		tell_player(p," You are not a member of that channel...\n");
		return;
	} else {
		p->chanflags&=~(1<<chan);
		if(p->opflags & (1<<chan)) {
			founder_quitting(p, chan);
		}
		channel[chan].idle=0;
		channel[chan].numppl--;
		oldstack=stack;
		sprintf(stack," You have left channel #%d\n",chan);
		stack=end_string(stack);
		tell_player(p,oldstack);
		stack=oldstack;
		sprintf(stack,"%s has left this channel.\n",p->name);
		stack=end_string(stack);
		cmsg(oldstack,chan);
		stack=oldstack;
	}
}

void channelquit(player * p, char *str) {

	channelquitroot(p, str, -1);
}

void channelquitbyint(player * p, int chan) {
	channelquitroot(p, 0, chan);
}


void channelboot(player *p, char *str)
{
	int chan;
	int found=0;
	char *oldstack;
	player *p2;

	if(!*str) {
		tell_player(p,"Format: chan boot <player>\n");
		return;
	}
	if(!(p->flags&CHANNEL_FOUNDER)) {
		tell_player(p," Hey, you can't do THAT! You're not the op.\n");
		return;
	}
	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->opflags&(1<<chan)) {
			found=1;
			break;
		}
	}
	if(!found) {
		tell_player(p," You're an op, but your channel doesn't exist!!\n");
		return;
	}
	if(!(p2=find_player_global(str))) {
		return;
	}
	if(!(p2->chanflags&(1<<chan))) {
		oldstack=stack;
		sprintf(stack," But %s isn't ON the channel to begin with!\n",p2->name);
		stack=end_string(stack);
		tell_player(p,oldstack);
		stack=oldstack;
	} else {
		p2->chanflags&=~(1<<chan);
		channel[chan].idle=0;
		channel[chan].numppl--;
		tell_player(p," Tis done...\n");
		oldstack=stack;
		sprintf(stack," You have been EVICTED from channel #%d by %s...\n",chan,p->name);
		stack=end_string(stack);
		tell_player(p2,oldstack);
		stack=oldstack;
		sprintf(stack,"%s just kicked that dork %s off the channel.\n",p->name,p2->name);
		stack=end_string(stack);
		cmsg(oldstack,chan);
		stack=oldstack;
	}
}


void channeljoin(player *p, char *str)
{
	char *oldstack;
	int chan;

	if(!*str) {
			tell_player(p,"Format: chan join <channel #>\n");
		return;
	}
	chan=atoi(str);
	if(chan<NUM_CHANNELS) {
		if(p->chanflags&(1<<chan)) {
			tell_player(p, " You're ALREADY on that channel silly.\n");
			return;
		} else if(!channel[chan].inuse) {
			tell_player(p, " That channel is not currently in use...\n");
			return;
		} else if(p->flags&CHANNEL_BAN) {
			tell_player(p, "You can't join channels...\n");
			return;
		} else if(channel[chan].cflags & INVITE_ONLY && !(p->c_invites & (1<<chan))) {
			tell_player(p, "That channel is private.\n");
			return;
		} else if(channel[chan].cflags & NO_NEWBIES && p->residency == NON_RESIDENT && !(p->c_invites & (1<<chan))) {
			tell_player(p, "Only resident players may access this channel.\n");
			return;
		} else {
			p->chanflags|=(1<<chan);
			channel[chan].idle=0;
			channel[chan].numppl++;
			oldstack=stack;
			sprintf(stack," You join channel #%d\n",chan);
			stack=end_string(stack);
			tell_player(p,oldstack);
			stack=oldstack;
			if(p->c_invites & (1<<chan)) {
				p->c_invites &= ~(1<<chan);
			}
				sprintf(stack,"%s has joined this channel.\n",p->name);
				stack=end_string(stack);
				cmsg(oldstack,chan);
				stack=oldstack;
		}
	} else {
		oldstack=stack;
		sprintf(stack," %d is not a valid channel id.\n",chan);
		stack=end_string(stack);
		tell_player(p,oldstack);
		stack=oldstack;
	}
}


void channelban(player *p, char *str)
{
	char *oldstack;
	char *reason;
	player *p2;
	
	if(p->flags&BLOCK_SU && !(p->residency&HCADMIN)) {
		tell_player(p," Try again later when you're not slacking off.\n");
		return;
	}
	if(!*str) {
		tell_player(p," Format chan ban <player>\n");
		return;
	}
	if(!(p2=find_player_global(str))) {
		return;
	} else {
		if(!check_privs(p->residency, p2->residency)) {
			tell_player(p," Uh oh.. shouldn't have tried that...\n");
			oldstack=stack;
			sprintf(stack," Oi! %s tried to ban you from channels!\n",p->name);
			stack=end_string(stack);
			tell_player(p2,oldstack);
			stack=oldstack;
		} else {
			if (p2->flags & CHANNEL_BAN)
			{
			p2->flags &= ~CHANNEL_BAN;
			oldstack=stack;
			sprintf(stack," %s has unbanned you from joining channels\n",p->name);
			stack=end_string(stack);
			tell_player(p2,oldstack);
			stack=oldstack;
			sprintf(stack," -=*> %s unbans %s from using channels.\n",p->name,p2->name);
			stack=end_string(stack);
			su_wall(oldstack);
			log("channel",oldstack);
			stack=oldstack;
			} else {			
			p2->flags|=CHANNEL_BAN;
			oldstack=stack;
			sprintf(stack," %s has banned you from joining any channels\n",p->name);
			stack=end_string(stack);
			tell_player(p2,oldstack);
			stack=oldstack;
			sprintf(stack," -=*> %s bans %s from using channels.\n",p->name,p2->name);
			stack=end_string(stack);
			su_wall(oldstack);
			log("channel",oldstack);
			stack=oldstack;
			}
		}
	}
}


void channelkill(player *p, char *str)
{
	int chan;
	char *oldstack;

	if(p->flags&BLOCK_SU && !(p->residency&HCADMIN)) {
		tell_player(p," Only when you're on duty chief!\n");
		return;
	}
	if(!*str) {
		tell_player(p," Format: chan kill <channel #>\n");
		return;
	}
	chan=atoi(str);
	if(chan<NUM_CHANNELS) {
		if(channel[chan].inuse) {
			oldstack=stack;
			sprintf(stack," -=*> %s destroys channel #%d.\n",p->name,chan);
			stack=end_string(stack);
			su_wall(oldstack);
			stack=oldstack;
			sprintf(stack,"%s kills channel %s with topic %s",p->name,channel[chan].name,channel[chan].topic);
			stack=end_string(stack);
			log("channel",oldstack);
			stack=oldstack;
			channeldestroy(chan,"This channel has been destroyed by the staff.\n");
		} else {
			tell_player(p," That channel isn't in use...\n");
		}
	} else {
		oldstack=stack;
		sprintf(stack," %d is an invalid channel ID...\n",chan);
		stack=end_string(stack);
		tell_player(p,oldstack);
		stack=oldstack;
	}
}


void viewchannelcmds(player *p, char *str)
{
	view_sub_commands(p,chan_list);
}


void channelcmd(player *p, char *str)
{
	if(!*str) {
		tell_player(p," Format: chan <subcommand>\n");
		return;
	}
	sub_command(p,str,chan_list);
}


void channelsay(player *p, char *str)
{
	int chan;
	char *msg, *scan;
	int count=0;

	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->chanflags&(1<<chan)) {
			count++;
		}
	}
	if(count!=1 || !*str) {
		msg=next_space(str);
		*msg++=0;
		if(!*str || !*msg) {
				tell_player(p,"Format: ch <chan #> <message>\n");
			return;
		}
		if(isdigit(*str)) {
			chan=atoi(str); 
		} else {
				tell_player(p,"Format: ch <chan #> <message>\n");
				return;
		}
	} else {
		for(chan=0;chan<NUM_CHANNELS;chan++) {
			if(p->chanflags&(1<<chan)) {
				break;
			}
		}
		if (isdigit(*str)) {
			msg=next_space(str);
			*msg++ = 0;
			if(!*str || !*msg) {
				tell_player(p,"Format: ch <message>\n");
				return; }
			chan = atoi(str); /* prevent channel mistells */	
		} else {
			if (!*str) {
				tell_player(p,"Format: ch <message>\n");
				return; }
			msg=str;		
		}
	}
	if(chan<NUM_CHANNELS) {
		if(p->chanflags&(1<<chan)) {
			channel[chan].idle=0;
			for (scan = msg; *scan; scan++); 
				
				switch(*(--scan)) {
				   case '?':
					cwall(p->name, msg,chan,SAY_ASK,p->flags);
					break;
				   case '!':
					cwall(p->name,msg,chan,SAY_EXC,p->flags);
					break;
				   default:
					cwall(p->name,msg,chan,SAY,p->flags);
					break;
				}
		} else {
			tell_player(p," You're not a member of THAT channel...\n");
		}
	}
}


void channelemote(player *p, char *str)
{
	int chan;
	char *msg;
	int count=0;

	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->chanflags&(1<<chan)) {
			count++;
		}
	}
	if(count!=1 || !*str) {
		msg=next_space(str);
		*msg++=0;
		if(!*str || !*msg) {
				tell_player(p," Format: ce <chan #> <message>\n");
			return;
		}
		if(isdigit(*str)) {
			chan=atoi(str); 
		} else {
				tell_player(p,"Format: ce <chan #> <message>\n");
				return;
		}
	} else {
		for(chan=0;chan<NUM_CHANNELS;chan++) {
			if(p->chanflags&(1<<chan)) {
				break;
			}
		}
		if (isdigit(*str)) {
			msg=next_space(str);
			*msg++ = 0;
			if(!*str || !*msg) {
				tell_player(p,"Format: ce <message>\n");
				return; }
			chan = atoi(str); /* prevent channel mistells */	
		} else {
			if (!*str) {
				tell_player(p,"Format: ce <message>\n");
				return; }
			msg=str;		
		}
	}
	if(chan<NUM_CHANNELS) {
		if(p->chanflags&(1<<chan)) {
			channel[chan].idle=0;
			cwall(p->name,msg,chan,EMOTE,p->flags);
		} else {
			tell_player(p," You're not a member of THAT channel...\n");
		}
	}
}


void channelpemote(player *p, char *str)
{
	int chan;
	char *msg;
	int count=0;

	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->chanflags&(1<<chan)) {
			count++;
		}
	}
	if(count!=1 || !*str) {
		msg=next_space(str);
		*msg++=0;
		if(!*str || !*msg) {
				tell_player(p," Format: cp <chan #> <message>\n");
			return;
		}
		if(isdigit(*str)) {
			chan=atoi(str); 
		} else {
				tell_player(p,"Format: cp <chan #> <message>\n");
				return;
		}
	} else {
		for(chan=0;chan<NUM_CHANNELS;chan++) {
			if(p->chanflags&(1<<chan)) {
				break;
			}
		}
		if (isdigit(*str)) {
			msg=next_space(str);
			*msg++ = 0;
			if(!*str || !*msg) {
				tell_player(p,"Format: cp <message>\n");
				return; }
			chan = atoi(str); /* prevent channel mistells */	
		} else {
			if (!*str) {
				tell_player(p,"Format: cp <message>\n");
				return; }
			msg=str;		
		}
	}
	if(chan<NUM_CHANNELS) {
		if(p->chanflags&(1<<chan)) {
			channel[chan].idle=0;
			cwall(p->name,msg,chan,PEMOTE, p->flags);
		} else {
			tell_player(p," You're not a member of that channel..\n");
		}
	}
}


void channelthink(player *p, char *str)
{
	int chan;
	char *msg;
	int count=0;

	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->chanflags&(1<<chan)) {
			count++;
		}
	}
	if(count!=1 || !*str) {
		msg=next_space(str);
		*msg++=0;
		if(!*str || !*msg) {
				tell_player(p," Format: ct <chan #> <message>\n");
			return;
		}
		if(isdigit(*str)) {
			chan=atoi(str); 
		} else {
				tell_player(p,"Format: ct <chan #> <message>\n");
				return;
		}
	} else {
		for(chan=0;chan<NUM_CHANNELS;chan++) {
			if(p->chanflags&(1<<chan)) {
				break;
			}
		}
		if (isdigit(*str)) {
			msg=next_space(str);
			*msg++ = 0;
			if(!*str || !*msg) {
				tell_player(p,"Format: ct <message>\n");
				return; }
			chan = atoi(str); /* prevent channel mistells */	
		} else {
			if (!*str) {
				tell_player(p,"Format: ct <message>\n");
				return; }
			msg=str;		
		}
	}
	if(chan<NUM_CHANNELS) {
		if(p->chanflags&(1<<chan)) {
			channel[chan].idle=0;
			cwall(p->name,msg,chan,THINK, p->flags);
		} else {
			tell_player(p," But you're not a member of that channel!\n");
		}
	}
}


void channelhitell(player *p, char *str)
{
	if(p->misc_flags&CHAN_HI) {
		p->misc_flags&=~CHAN_HI;
		tell_player(p," Channels will nolonger be hilited.\n");
	} else {
		p->misc_flags|=CHAN_HI;
		tell_player(p, " Channels will now be hilited.\n");
	}
}


void channelwho(player *p, char *str)
{
	char *oldstack;
	int chan;
	player *s;
	int i=0;
	int count=0;
	char junk[MAX_NAME+2];

	if(!*str) {
		tell_player(p,"Format: chan who <channel #>\n");
		return;
	}
	chan=atoi(str);
	if(chan<NUM_CHANNELS) {
		if(!channel[chan].inuse) {
			tell_player(p," That channel is not in use.\n");
			return;
		}
	} else {
		oldstack=stack;
		sprintf(stack," %d is an invalid channel number...\n",chan);
		stack=end_string(stack);
		tell_player(p,oldstack);
		stack=oldstack;
		return;
	}
	oldstack=stack;
	sprintf(stack,"====[People on channel #%d]================================================\n",chan);
	stack=strchr(stack,0);
	for(s=flatlist_start;s;s=s->flat_next) {
		if(s->chanflags&(1<<chan)) {
			if(s->opflags&(1<<chan)) {
				sprintf(junk,"<*%s*>",s->name);
				sprintf(stack," %-24s",junk);
			} else {
				sprintf(stack," %-24s",s->name);
			}
			stack=strchr(stack,0);
			count++;
			if(++i==3) {
				strcpy(stack,"\n");
				stack=strchr(stack,0);
				i=0;
			}
		}
	}
	if(i) {
		strcpy(stack,"\n");
		stack=strchr(stack,0);
	}
	strcpy(stack,"======================================"
		     "====================================\n");
	stack=end_string(stack);
	tell_player(p,oldstack);
	stack=oldstack;
	channel[chan].numppl = count;
}

void channelsettopic(player *p, char *str)
{
	int chan;
	int found=0;
	char *oldstack;

	if(!*str) {
		tell_player(p,"Format: chan topic <message>\n");
		return;
	}
	if(!(p->flags&CHANNEL_FOUNDER)) {
		tell_player(p," Hey, you can't do THAT! You're not the op.\n");
		return;
	}
	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->opflags&(1<<chan)) {
			found=1;
			break;
		}
	}
	if(!found) {
		tell_player(p," You're an op, but your channel doesn't exist!!\n");
		return;
	}
		channel[chan].idle=0;
		tell_player(p," Tis done...\n");
		oldstack=stack;
		sprintf(stack,"%s changes the topic to %s\n",p->name, str);
		stack=end_string(stack);
		cmsg(oldstack, chan);
		stack=oldstack;
		strncpy(channel[chan].topic, str, MAX_TOPIC - 3);
}

void channelsing(player *p, char *str)
{
	int chan;
	char *msg;
	int count=0;

	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->chanflags&(1<<chan)) {
			count++;
		}
	}
	if(count!=1 || !*str) {
		msg=next_space(str);
		*msg++=0;
		if(!*str || !*msg) {
				tell_player(p," Format: cs <chan #> <message>\n");
			return;
		}
		if(isdigit(*str)) {
			chan=atoi(str); 
		} else {
				tell_player(p,"Format: cs <chan #> <message>\n");
				return;
		}
	} else {
		for(chan=0;chan<NUM_CHANNELS;chan++) {
			if(p->chanflags&(1<<chan)) {
				break;
			}
		}
		if (isdigit(*str)) {
			msg=next_space(str);
			*msg++ = 0;
			if(!*str || !*msg) {
				tell_player(p,"Format: cs <message>\n");
				return; }
			chan = atoi(str); /* prevent channel mistells */	
		} else {
			if (!*str) {
				tell_player(p,"Format: cs <message>\n");
				return; }
			msg=str;		
		}
	}
	if(chan<NUM_CHANNELS) {
		if(channel[chan].cflags & NO_SINGING) {
			tell_player(p, "This channel does not allow singing\n");
			return;
			}
		if(p->chanflags&(1<<chan)) {
			channel[chan].idle=0;
			cwall(p->name,msg,chan,SING, p->flags);
		} else {
			tell_player(p," You're not a member of THAT channel...\n");
		}
	}
}

player *find_new_channel_founder(int chan) {

	player *s, *r;
	int found_one = 0;	
		for(s=flatlist_start;s;s=s->flat_next) {
			if(s->residency && s->chanflags&(1<<chan) && !(s->flags & CHANNEL_FOUNDER)) {
				found_one = 1;
				r = s;
				}
			}

		if (found_one)		
			return r; 
		else 
			return 0;
}

void founder_quitting(player * p, int chan) {

	player *newf;
	char *oldstack;

	tell_player(p, "Scanning for a new leader...\n");
	newf = find_new_channel_founder(chan); 
	tell_player(p, "Scan done.\n");

		if (!newf) {
		channeldestroy(chan, "Channel destroyed due to lack of operator.");
		p->flags &= ~CHANNEL_FOUNDER;
		p->opflags &= ~(1<<chan);
		return;
		}	
		oldstack = stack;
		sprintf(stack, " You have inherited the operator powers on channel #%d\n",chan);
		stack = end_string(stack);
		tell_player(newf, oldstack);
		stack = oldstack;
		/* why doesn't this work?! */
		sprintf(stack, "%s is the new operator of this channel.\n", newf->name);
		stack = end_string(stack);
		cmsg(oldstack, chan);
		stack = oldstack;
		newf->opflags |= (1<<chan);
		newf->flags |= CHANNEL_FOUNDER;	
		p->opflags &= ~(1<<chan);
		p->flags &= ~CHANNEL_FOUNDER;
		channel[chan].idle = 0;
}

void channelnosing(player * p, char *str) {

	int chan;
	int found=0;
	char *oldstack;

	if(*str) {
		tell_player(p,"Format: chan no_sing\n");
		return;
	}
	if(!(p->flags&CHANNEL_FOUNDER)) {
		tell_player(p," Hey, you can't do THAT! You're not the op.\n");
		return;
	}
	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->opflags&(1<<chan)) {
			found=1;
			break;
		}
	}
	if(!found) {
		tell_player(p," You're an operator, but your channel doesn't exist!!\n");
		return;
	}
		channel[chan].idle=0;
		tell_player(p," Tis done...\n");
		oldstack=stack;
		if (channel[chan].cflags & NO_SINGING) {
			sprintf(stack,"%s unbans singing on this channel\n",p->name);
			channel[chan].cflags &= ~NO_SINGING;
			}
		else {
			sprintf(stack,"%s bans singing on this channel\n",p->name);
			channel[chan].cflags |= NO_SINGING;
			}	
		stack=end_string(stack);
		cmsg(oldstack, chan);
		stack=oldstack;
}
void channelnonewbies(player * p, char *str) {

	int chan;
	int found=0;
	char *oldstack;

	if(*str) {
		tell_player(p,"Format: chan no_newbies\n");
		return;
	}
	if(!(p->flags&CHANNEL_FOUNDER)) {
		tell_player(p," Hey, you can't do THAT! You're not the op.\n");
		return;
	}
	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->opflags&(1<<chan)) {
			found=1;
			break;
		}
	}
	if(!found) {
		tell_player(p," You're an op, but your channel doesn't exist!!\n");
		return;
	}
		channel[chan].idle=0;
		tell_player(p," Tis done...\n");
		oldstack=stack;
		if (channel[chan].cflags & NO_NEWBIES) {
			sprintf(stack,"%s allows newbies onto the channel\n",p->name);
			channel[chan].cflags &= ~NO_NEWBIES;
			}
		else {
			sprintf(stack,"%s bans newbies from this channel\n",p->name);
			channel[chan].cflags |= NO_NEWBIES;
			}	
		stack=end_string(stack);
		cmsg(oldstack, chan);
		stack=oldstack;
}

void channelprivate(player * p, char *str) {

	int chan;
	int found=0;
	char *oldstack;

	if(*str) {
		tell_player(p,"Format: chan private\n");
		return;
	}
	if(!(p->flags&CHANNEL_FOUNDER)) {
		tell_player(p," Hey, you can't do THAT! You're not the op.\n");
		return;
	}
	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->opflags&(1<<chan)) {
			found=1;
			break;
		}
	}
	if(!found) {
		tell_player(p," You're an op, but your channel doesn't exist!!\n");
		return;
	}
		channel[chan].idle=0;
		tell_player(p," Tis done...\n");
		oldstack=stack;
		if (channel[chan].cflags & INVITE_ONLY) {
			sprintf(stack,"%s opens the channel to the pubilc\n",p->name);
			channel[chan].cflags &= ~INVITE_ONLY;
			}
		else {
			sprintf(stack,"%s makes the channel private\n",p->name);
			channel[chan].cflags |= INVITE_ONLY; 
			}	
		stack=end_string(stack);
		cmsg(oldstack, chan);
		stack=oldstack;
}

void toggleblockchans(player * p, char *str) {

	if (p->tag_flags & BLOCKCHANS) {
		tell_player(p, " You resume listening to new channel announcements\n");
		p->tag_flags &= ~BLOCKCHANS;
	} else {
		tell_player(p, " You ignore new channel announcements.\n");
		p->tag_flags |= BLOCKCHANS;
		}
}

void channelinvite(player * p, char *str) {

	int chan;
	int found=0;
	char *oldstack;
	player *p2;

	if(!*str) {
		tell_player(p,"Format: chan invite <player>\n");
		return;
	}
	if(!(p->flags&CHANNEL_FOUNDER)) {
		tell_player(p," Hey, you can't do THAT! You're not the op.\n");
		return;
	}
	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->opflags&(1<<chan)) {
			found=1;
			break;
		}
	}
	if(!found) {
		tell_player(p," You're an op, but your channel doesn't exist!!\n");
		return;
	}
	p2 = find_player_global(str);
	if (!p2)
		return;
	if (p2->c_invites & (1<<chan)) {
		tell_player(p, " But that person has already been invited.\n");
		return;
        } else if (p2->chanflags & (1<<chan)) {
		tell_player(p, " That player is already on the channel.\n");
		return;
	} else  {
		oldstack=stack;
		sprintf(stack, " %s has invited you to join channel %d. Type \"chan join %d\" to join.\n", p->name, chan, chan);
		stack = end_string(stack);
		tell_player(p2, oldstack);
		p2->c_invites |= (1<<chan);
		tell_player(p, " Tis done...\n");
		stack=oldstack;
		}				
}

void channeluninvite(player * p, char *str) {

	int chan;
	int found=0;
	char *oldstack;
	player *p2;

	if(!*str) {
		tell_player(p,"Format: chan uninvite <player>\n");
		return;
	}
	if(!(p->flags&CHANNEL_FOUNDER)) {
		tell_player(p," Hey, you can't do THAT! You're not the op.\n");
		return;
	}
	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->opflags&(1<<chan)) {
			found=1;
			break;
		}
	}
	if(!found) {
		tell_player(p," You're an op, but your channel doesn't exist!!\n");
		return;
	}
	p2 = find_player_global(str);
	if (!p2)
		return;
	if (!(p2->c_invites & (1<<chan))) {
		tell_player(p, " But that person hasn't BEEN invited !!\n");
		return;
        } else if (p2->chanflags & (1<<chan)) {
		tell_player(p, " That player is already on the channel.\n");
		return;
	} else  {
		oldstack=stack;
		sprintf(stack, " %s revokes your invitation to join channel %d\n", p->name, chan);
		stack = end_string(stack);
		tell_player(p2, oldstack);
		p2->c_invites &= ~(1<<chan);
		tell_player(p, " Tis done...\n");
		stack=oldstack;
		}				
}
void channelrename(player *p, char *str)
{
	int chan;
	int found=0;
	char *oldstack;

	if(!*str) {
		tell_player(p,"Format: chan rename <message>\n");
		return;
	}
	if(!(p->flags&CHANNEL_FOUNDER)) {
		tell_player(p," Hey, you can't do THAT! You're not the op.\n");
		return;
	}
	for(chan=0;chan<NUM_CHANNELS;chan++) {
		if(p->opflags&(1<<chan)) {
			found=1;
			break;
		}
	}
	if(!found) {
		tell_player(p," You're an op, but your channel doesn't exist!!\n");
		return;
	}
		channel[chan].idle=0;
		tell_player(p," Tis done...\n");
		oldstack=stack;
		sprintf(stack,"%s changes the name of the channel to %s\n",p->name, str);
		stack=end_string(stack);
		cmsg(oldstack, chan);
		stack=oldstack;
		strncpy(channel[chan].name, str, MAX_NAME - 3);
}

void channelbarge(player *p, char *str)
{
	char *oldstack;
	int chan;

	if(!*str) {
		tell_player(p,"Format: chan barge <channel #>\n");
		return;
	}
	chan=atoi(str);
	if(chan<NUM_CHANNELS) {
		if(p->chanflags&(1<<chan)) {
			tell_player(p, " You're ALREADY on that channel silly.\n");
			return;
		} else if(!channel[chan].inuse) {
			tell_player(p, " That channel is not currently in use...\n");
			return;
		} else {
			p->chanflags|=(1<<chan);
			channel[chan].idle=0;
			channel[chan].numppl++;
			oldstack=stack;
			sprintf(stack," You barge onto channel #%d\n",chan);
			stack=end_string(stack);
			tell_player(p,oldstack);
			stack=oldstack;
			if(p->c_invites & (1<<chan)) {
				p->c_invites &= ~(1<<chan);
			}
				sprintf(stack,"%s has joined this channel.\n",p->name);
				stack=end_string(stack);
				cmsg(oldstack,chan);
				stack=oldstack;
		}
	} else {
		oldstack=stack;
		sprintf(stack," %d is not a valid channel id.\n",chan);
		stack=end_string(stack);
		tell_player(p,oldstack);
		stack=oldstack;
	}
}

