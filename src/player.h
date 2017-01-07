/*
 *  player.h
 */

/* kludgy macros, there must be a better way to do this */

#define align(p) p=(void *)(((int)p+3)&-4)
#define GET_BACK_ON_DUTY   " You cannot do that unless you are on duty...\n"
#define CHECK_DUTY(p) if ((p)->flags & BLOCK_SU && !((p)->residency & ADMIN)) \
                            { tell_player(p, GET_BACK_ON_DUTY); return; }
#define FORMAT(ck,s)    if (!(*(ck)))  { tell_player(current_player, (s)); return; }



/* modes that players can be in */
#define NONE 0
#define PASSWORD 1
#define CONV 2
#define ROOMEDIT 4
#define MAILEDIT 8
#define NEWSEDIT 16
#define SUNEWSEDIT 32
/* gender types */

#define MALE 1
#define FEMALE 2
#define PLURAL 3
#define OTHER 0
#define VOID_GENDER -1

/* residency types */

#define STANDARD_ROOMS -2
#define BANISHED -1
#define NON_RESIDENT 0
#define BASE 1
#define NO_SYNC (1<<1)
#define ECHO_PRIV (1<<2)
#define NO_TIMEOUT (1<<3)
#define BANISHD (1<<4)
#define SYSTEM_ROOM (1<<5)
#define MAIL (1<<6)
#define LIST (1<<7)
#define BUILD (1<<8)
#define SESSION (1<<9)
#define SPOD (1<<10)
#define FOREST_STYLE_CHAN (1<<11)
#define SPARR (1<<11)
#define REGULAR_STYLE_CHAN (1<<12)
#define MINISTEROLD (1<<12)
#define NONCLERGY (1<<13)
#define PSU (1<<14)
#define WARN (1<<15)
#define TESTCHAR (1<<16)
#define HOUSE (1<<17)
#define ASU (1<<18)
#define SPARE10 (1<<19)
#define DUMB (1<<20)
#define SPARE11 (1<<21)
#define SCRIPT (1<<22)
#define TRACE (1<<23)
#define CODER (1<<24)
#define SPARE15 (1<<25)
#define LOWER_ADMIN (1<<26)
#define HCADMIN (1<<27)
#define PROTECT (1<<28)
#define SU (1<<29)
#define ADMIN (1<<30)
#define GIT (1<<31)

#define NONSU (BASE + ECHO_PRIV + NO_TIMEOUT + MAIL + LIST + BUILD + SESSION \
               + SCRIPT + TRACE + DUMB + HOUSE + SPOD + REGULAR_STYLE_CHAN \
		+ WARN + FOREST_STYLE_CHAN)

#define HCADMIN_INIT (HCADMIN + ADMIN + LOWER_ADMIN + SU + SPOD \
                     + TRACE + SCRIPT + DUMB + WARN + PSU + SESSION \
                     + BUILD + LIST + MAIL + NO_TIMEOUT + ECHO_PRIV + BASE )

/* #define lengths */

#define MAX_HISTORY_LINES 8
#define MAX_NAME 20
#define MAX_INET_ADDR 40
#define IBUFFER_LENGTH 512
#define MAX_REVIEW 1000
#define MAX_PROMPT 15
#define MAX_ID 15
#define MAX_EMAIL 60
#define MAX_PASSWORD 20
#define MAX_TITLE 65
#define MAX_DESC 300
#define MAX_ALIAS 300
#define MAX_PLAN 300
#define MAX_PRETITLE 19 
#define MAX_ENTER_MSG 65
#define MAX_IGNOREMSG 65
#define MAX_SESSION 60
#define MAX_COMMENT 59
/* an un-subtle size, but... */
#define MAX_REPLY 200
#define MAX_ROOM_CONNECT 35
#define MAX_SPODCLASS 45
#define MAX_ROOM_NAME 50
#define MAX_AUTOMESSAGE 300
#define MAX_ROOM_SIZE 1500
#define MAX_ARTICLE_SIZE 5000
#define MAX_SNEWS 200

/* system flag definitiosn */

#define PANIC (1<<0)
#define VERBOSE (1<<1)
#define SHUTDOWN (1<<2)
/* missing flag 8 */
#define EVERYONE_TAG (1<<3)
#define FAILED_COMMAND (1<<4)
#define CLOSED_TO_NEWBIES (1<<5)
#define PIPE (1<<6)
#define ROOM_TAG (1<<7)
#define FRIEND_TAG (1<<8)
#define DO_TIMER (1<<9)
#define UPDATE (1<<10)
#define NO_PRINT_LOG (1<<11)
#define NO_PRETITLES (1<<12)
#define UPDATEROOMS (1<<13)
#define UPDATEFLAGS (1<<14)
#define NEWBIE_TAG (1<<15)
#define REPLY_TAG (1<<16)
#define SECURE_DYNAMIC (1<<17)
#define UPDATE_SPODLIST (1<<18)
#define ITEM_TAG (1<<19)
#define OFRIEND_TAG (1<<20)
#define UPDATE_URLS (1<<21)
#define CLOSED_TO_RESSIES (1<<22)
#define UPDATE_INT_DATA (1<<23)
/* player flag defs */

/* keep PANIC as 1 */
#define INPUT_READY (1<<1)
#define LAST_CHAR_WAS_N (1<<2)
#define LAST_CHAR_WAS_R (1<<3)
#define DO_LOCAL_ECHO (1<<4)
#define PASSWORD_MODE (1<<5)
/* keep closed to newbies at 64 */
#define PROMPT (1<<7)
#define TAGGED (1<<8)
#define LOGIN (1<<9)
#define CHUCKOUT (1<<10)
#define EOR_ON (1<<11)
#define IAC_GA_DO (1<<12)
#define SITE_LOG (1<<13)
#define DONT_CHECK_PIPE (1<<14)
#define RECONNECTION (1<<15)
#define NO_UPDATE_L_ON (1<<16)
#define BLOCK_SU (1<<17)
#define NO_SAVE_LAST_ON (1<<18)
#define NO_SU_WALL (1<<19)
#define ASSISTED (1<<20)
#define FROGGED (1<<21)
#define SCRIPTING (1<<22)
#define OFF_LSU (1<<23)
#define CHANNEL_BAN (1<<24)
#define CHANNEL_FOUNDER (1<<25)
#define WAITING_ENGAGE (1<<26)
#define BAN18 (1<<27)

/* ones that get saved */
/* lower block, system flags */
#define SAVENOSHOUT (1<<0)
#define SAVEDFROGGED (1<<1)
#define SAVE_NO_SING (1<<2)
#define SAVE_LAGGED (1<<3)
#define DECAPPED (1<<4)
#define SAVEDJAIL (1<<5)
#define NO_MSGS (1<<6)
/* how'd this get lost? */
#define SAVED_RM_MOVE (1<<7)   
/* upper block, system flags */
#define FLIRT_BACHELOR (1<<10)
#define BACHELOR_HIDE (1<<11)
#define ENGAGED (1<<12)
#define MARRIED (1<<13)
#define MINISTER (1<<14)
#define NEW_MAIL (1<<15)
#define COMPRESSED_LIST (1<<16)
#define COMPRESSED_ALIAS (1<<17)
#define COMPRESSED_ITEMS (1<<18) 
#define IAC_GA_ON (1<<19)
#define AGREED_DISCLAIMER (1<<20)
#define NEW_SITE (1<<21)
#define BUILDER (1<<22)

/* lower tag flags */
#define TAG_PERSONAL (1<<0)
#define TAG_ROOM (1<<1)
#define TAG_SHOUT (1<<2)
#define TAG_LOGINS (1<<3)
#define TAG_ECHO (1<<4)
#define SEEECHO (1<<5)
#define TAG_AUTOS (1<<6)
#define TAG_ITEMS (1<<7)
/* upper tag flags */
#define BLOCK_SHOUT (1<<10)
#define BLOCK_TELLS (1<<11)
#define BLOCK_FRIENDS (1<<12)
#define BLOCK_ECHOS (1<<13)
#define BLOCK_ROOM_DESC (1<<14)
#define BLOCK_FRIEND_MAIL (1<<15)
#define SINGBLOCK (1<<16)
#define BLOCKCHANS (1<<17)
#define NO_FACCESS (1<<18)
#define NOBEEPS (1<<19)
#define BLOCK_AUTOS (1<<20) 
#define NO_ANONYMOUS (1<<21)
#define NO_PROPOSE (1<<22)
#define BLOCK_ITEMS (1<<23)
#define BLOCK_LOGINS (1<<24)
#define BLOCK_BOPS (1<<25)

/* custom flags, lower */
#define HIDING (1<<0)
#define PRIVATE_EMAIL (1<<1)
#define PUBLIC_SITE (1<<2)
#define FRIEND_SITE (1<<3)
#define FRIEND_EMAIL (1<<4)
/* upper custom flags */
#define TRANS_TO_HOME (1<<10)
#define MAIL_INFORM (1<<11)
#define NEWS_INFORM (1<<12)
#define NOPREFIX (1<<13)
#define NOEPREFIX (1<<14)
#define YES_SESSION (1<<15)
#define NO_PAGER (1<<16)
#define ROOM_ENTER (1<<17)
#define YES_QWHO_LOGIN (1<<18)
#define SHOW_EXITS (1<<19)
#define CONVERSE (1<<20)
#define QUIET_EDIT (1<<21)

/* misc flags, lower */
#define NO_PRS (1<<0)
#define NO_GIFT (1<<1)
/* misc flags, upper */
#define CHAN_HI (1<<10)
#define SU_HILITED (1<<11)
#define NOCOLOR (1<<12)
#define SYSTEM_COLOR (1<<13)
#define GAME_HI (1<<14)
#define STOP_BAD_COLORS (1<<15)

/* list flags */

#define NOISY 1
#define IGNORE 2
#define INFORM 4
#define GRAB 8
#define FRIEND 16
#define BAR 32
#define INVITE 64
#define BEEP 128
#define BLOCK 256
#define KEY 512
#define FIND 1024
#define FRIENDBLOCK 2048
#define MAILBLOCK 4096
#define SHARE_ROOM 8192
#define NO_FACCESS_LIST 16384

/* command types */

#define VOID 0
#define SEE_ERROR (1<<0)
#define PERSONAL (1<<1)
#define ROOM (1<<2)
#define EVERYONE (1<<3)
#define ECHO_COM (1<<4)
#define EMERGENCY (1<<5)
#define AUTO (1<<6)
#define HIGHLIGHT (1<<7)
#define NO_P_MATCH (1<<8)
#define TAG_INFORM (1<<9)
#define LIST_EVERYONE (1<<10)
#define ADMIN_BARGE (1<<11)
#define SORE (1<<12)
#define WARNING (1<<13)
#define EXCLUDE (1<<14)
#define BAD_MUSIC (1<<15)
#define NO_HISTORY (1<<16)
#define LOGIN_TAG (1<<17)
#define LOGOUT_TAG (1<<18)
#define RECON_TAG (1<<19)

/* color modes */
#define TELsc 0
#define SUCsc 1
#define ADCsc 2
#define FRTsc 3
#define ROMsc 4
#define SHOsc 5
#define UCEsc 6
#define UCOsc 7
#define SYSsc 8

/* for Mantis' reportto and history shit */
struct rev_struct 
{
	char review[MAX_REVIEW];
};

/* files'n'things */

typedef struct
{
   char           *where;
   int             length;
}               file;

/* just simple struct to hold the super help pages */

struct super_n
{
   char            text[MAX_SNEWS];
   int             ident;
   struct super_n *next;
};

typedef struct super_n snews;

/* room definitions */

#define HOME_ROOM 1
#define COMPRESSED 2
#define AUTO_MESSAGE 4
#define AUTOS_TAG 8
#define LOCKABLE 16
#define LOCKED 32
#define OPEN 64
#define LINKABLE 128
#define KEYLOCKED 256
#define CONFERENCE 512
#define ROOM_UPDATED 1024
/* to be implemented at a later date... */
#define EXITMSGS_OK 2048 
#define SOUNDPROOF 4096 
#define ISOLATED_ROOM 8192 
#define ANTISING 16384

struct r_struct
{
   char            name[MAX_ROOM_NAME];
   char            id[MAX_ID];
   int             flags;
   int             data_key;
   int             auto_count;
   int             auto_base;
   file            text;
   file            exits;
   file            automessage;
   struct s_struct *owner;
   struct p_struct *players_top;
   struct r_struct *next;
   char            enter_msg[MAX_ENTER_MSG];
};

typedef struct r_struct room;

/* note defs */

#define NEWS_ARTICLE 1
#define ANONYMOUS 2
#define NOT_READY 4
#define SUPRESS_NAME 8
/* for later on... could be useful */
#define NEWS_NO_TIMEOUT 16
#define NOTE_FRIEND_TAG 32
#define SUNEWS_ARTICLE 64


struct n_struct
{
   int             id;
   int             flags;
   int             date;
   file            text;
   int             next_sent;
   int             read_count;
   struct n_struct *hash_next;
   char            header[MAX_TITLE];
   char            name[MAX_NAME];
};

typedef struct n_struct note;

/* list defs */

struct l_struct
{
   char            name[MAX_NAME];
   int             flags;
   struct l_struct *next;
};

typedef struct l_struct list_ent;

struct al_struct
{
	char 	cmd[MAX_NAME];
	char    sub[MAX_DESC];
	struct al_struct *next;
};

typedef struct al_struct alias;

struct library_alias
{
	char *command;
	char *alias_string;
	char *description;
	char *author;
	int privs;
};

typedef struct library_alias alias_library;
/* saved player defs */

struct s_struct
{
   char            lower_name[MAX_NAME];
   char            last_host[MAX_INET_ADDR];
   char		   email[MAX_EMAIL];
   int             last_on;
   int             residency;
   int		   system_flags;
   int		   misc_flags;
   int		   custom_flags;
   int		   tag_flags;
   int 		   pennies;

   file            data;
   struct l_struct *list_top;
   struct r_struct *rooms;
   int             mail_sent;
   int            *mail_received;
   struct al_struct *alias_top;
   struct p_item  *item_top; 
   struct s_struct *next;
};

typedef struct s_struct saved_player;

/* editor info structure */

typedef struct
{
   char           *buffer;
   char           *current;
   int             max_size;
   int             size;
   void           *finish_func;
   void           *quit_func;
   int             flag_copy;
   int             sflag_copy;
   int             tflag_copy;
   int             cflag_copy;
   int             mflag_copy;
   void           *input_copy;
   void           *misc;
}               ed_info;


/* terminal defs */

struct terminal
{
   char           *name;
   char           *bold;
   char           *off;
   char           *cls;
};


/* the player structure */

struct p_struct
{
   int             fd;
   int             performance;
   int             hash_top;
   int             flags;
   int             term;
   int             anticrash;
   int             antipipe;
   int             residency;
   int             saved_residency;
   int             term_width;
   int             column;
   int             word_wrap;
   int             idle;
   int             gender;
   int             no_shout;
   int             shout_index;
   int             jail_timeout;
   int             no_move;
   int             lagged;
   int             script;
   int             jetlag;	/* This has just become time zone difference */
   int             sneezed;	/* wibble! */
   int             birthday;
   int             age;
   int             last_newsb;
   struct p_struct *hash_next;
   struct p_struct *flat_next;
   struct p_struct *flat_previous;
   struct p_struct *room_next;
   saved_player   *saved;
   room           *location;
   int             max_rooms;
   int             max_exits;
   int             max_autos;
   int             max_list;
   int             max_mail;
   int             on_since;
   int             total_login;
   ed_info        *edit_info;
   char            inet_addr[MAX_INET_ADDR];
   char            num_addr[MAX_INET_ADDR];
   char            name[MAX_NAME];
   char            title[MAX_TITLE];
   char            pretitle[MAX_PRETITLE];
   char            description[MAX_DESC];
   char            plan[MAX_PLAN];
   char            lower_name[MAX_NAME];
   char            idle_msg[MAX_TITLE];

   char            ignore_msg[MAX_IGNOREMSG];
   char            comment[MAX_COMMENT];
   char            reply[MAX_REPLY];
   char            room_connect[MAX_ROOM_CONNECT];
   int             reply_time;

   void           *input_to_fn;
   void           *timer_fn;
   int             timer_count;
   char            ibuffer[IBUFFER_LENGTH];
   int             ibuff_pointer;
   char            prompt[MAX_PROMPT];
   char            converse_prompt[MAX_PROMPT];
   char            email[MAX_EMAIL];
   char            password[MAX_PASSWORD];
   char            password_cpy[MAX_PASSWORD];
   char            enter_msg[MAX_ENTER_MSG];

/* start of any adding by Athanasius */
   char            script_file[MAX_NAME + 16];
   char            assisted_by[MAX_NAME];
   int             logged_in;
   int             mode;
/* stuff trap wants to add... */
	char logonmsg[MAX_ENTER_MSG];
	char logoffmsg[MAX_ENTER_MSG];
	char blockmsg[MAX_IGNOREMSG];
	char exitmsg[MAX_ENTER_MSG];
	int time_in_main;   /* used to be int last_motd, but thats useless */   
	int no_sing;     /*not saved*/
	char married_to[MAX_NAME];
	char irl_name[MAX_NAME];
	char alt_email[MAX_EMAIL];
	char hometown[MAX_SPODCLASS];   /*yes a spoon thing so what?*/
	char spod_class[MAX_SPODCLASS];
	char favorite1[MAX_SPODCLASS];
	char favorite2[MAX_SPODCLASS];
	char favorite3[MAX_SPODCLASS];
        int total_idle_time;       /* 
     Hopefully I can incorporate this correctly - I want spodsort to 
     only cound ACTIVE time on the program - total time - total_idle_time
     Just a spoony thing to try but Im a spoon so sue me  -- asty */
	int last_remote_command;	/*for repeat*/
	char last_remote_msg[MAX_REPLY];
	int idle_index; /* traP - for counting idleness checks */
        char slock_pw[MAX_PASSWORD];  /* for screenlock */
        int chanflags;
        int opflags;
        int c_invites;
/* hopefully we can get these in rather painlessly... */
	int max_alias;
	char colorset[10];
/* is this pstruct big enough? I guess not! :P */
	char ressied_by[MAX_NAME];
	char git_string[MAX_DESC];
	char git_by[MAX_NAME];
	/* bad ressie stats */
	int warn_count;
	int eject_count;
	int idled_out_count;
	int booted_count;
	/* su stats */
	int num_ressied;
	int num_warned;
	int num_ejected;
	int num_rmd;
	int num_booted;
	int first_login_date;
	struct rev_struct rev[MAX_HISTORY_LINES];
	unsigned int prs;
	struct gag_struct *gag_top;
	int max_items; 
	int prs_record;
   int		   system_flags;
   int		   misc_flags;
   int		   custom_flags;
   int		   tag_flags;
   char		   ingredients[MAX_SPODCLASS];
   int		   pennies;
};

typedef struct p_struct player;

/* flag list def */

typedef struct
{
   char           *text;
   int             change;
}               flag_list;

/* gag definition */
struct gag_struct 
{
   player            *gagged;
   struct gag_struct *next;
};

typedef struct gag_struct gag_entry;

struct s_item 
{
	int	id;
	int	sflags;
	int	value;
	char	desc[MAX_TITLE];
	char	name[MAX_NAME];
	char	author[MAX_NAME];
	struct s_item *next;
};

struct p_item
{
	int	id;
	int 	number;
	int 	flags;
	saved_player  *owner;
	struct p_item *loc_next;
	struct p_item *next;
	struct s_item *root;
};	

typedef struct p_item item;

/* flags for the commands section.. */

#define THE_EVIL_Q (1<<1)
#define COMMc (1<<2)
#define LISTc (1<<3)
#define ROOMc (1<<4)
#define MOVEc (1<<5)
#define INFOc (1<<6)
#define SYSc (1<<7)
#define DESCc (1<<8)
#define SOCIALc (1<<9)
#define MISCc (1<<10)
#define SUPERc (1<<11)
#define ADMINc (1<<12)
#define ITEMc (1<<13)
/* last one - determines if it's shown at all on list or not */
#define INVISc (1<<14)

/* structure for commands */

struct command
{
   char           *text;
   void           *function;
   int             level;
   int             andlevel;
   int             space;
   char           *help;
/* for the "smart" commandslist */
   int	     section; 

};

/* global definitions */

#ifndef GLOBAL_FILE

int            backup;
char           *action;
extern char    *stack, *stack_start;
extern int      sys_flags, max_players, current_players, command_type, up_time,
                up_date, logins, sys_color_atm, max_ppl_on_so_far;
extern player  *flatlist_start, *hashlist[], *current_player, *c_player, *stdout_player, *input_player;
extern room    *current_room, *entrance_room, *prison;
extern player **pipe_list;
extern int      pipe_matches;
extern int      splat1, splat2, splat_timeout;
extern int      soft_splat1, soft_splat2, soft_timeout;
extern player  *otherfriend;
#ifdef TRACK
extern char     functionin[100];
extern char     functionhist[20][100];
extern int      funcposition;
#endif
#endif


extern int      in_total, out_total, in_current, out_current, in_average,
                out_average, net_count, in_bps, out_bps, in_pack_total,
                out_pack_total, in_pack_current, out_pack_current, in_pps,
                out_pps, in_pack_average, out_pack_average;
