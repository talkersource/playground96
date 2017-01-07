/* 
 * CHANNEL.H
 */
#ifndef __CHANNEL_H
#define __CHANNEL_H

#define MAX_TOPIC		60
#define NUM_CHANNELS		30	

struct c_struct {
	int inuse;
	char topic[MAX_TOPIC];
	int idle;
	time_t starttime;
	int numppl;
        char name[MAX_NAME];
	int cflags;
};
extern struct c_struct channel[];

extern struct command chan_list[];

#define SAY				1
#define EMOTE				2
#define PEMOTE				3
#define THINK				4
#define MESSAGE				5
#define SING				6
#define SAY_ASK				7
#define SAY_EXC				8
#define FROGGED_PERSON			9

#define CHANNEL_TIMEOUT		(15*30)		/* 7.5 minutes idle */

/* cflag defs */
#define NO_NEWBIES (1<<1)
#define INVITE_ONLY (1<<2)
#define NO_SINGING (1<<3)

#endif /* __CHANNEL_H */
