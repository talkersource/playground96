/*
 * config.h
 */

/* define this for ULTRIX */

#undef  ULTRIX
#define ULTRIX_PLAYER_LIM 200

/*
#if !defined(linux)
 #define ULTRIX
#endif
*/

/* define this for Solaris 2.2 */

#undef SOLARIS

/* the system equivalent of the timelocal command */

#if !defined(linux)
 /* this for SunOS  */
 #define TIMELOCAL(x) timelocal(x)
#else
 /* This for Linux */
 #define TIMELOCAL(x) mktime(x)
#endif /* LINUX */


/* this for ULTRIX and Solaris */
#ifdef ULTRIX 
 #define TIMELOCAL(x) mktime(x)
#endif /* ULTIRX */
#ifdef SOLARIS
 #define TIMELOCAL(x) mktime(x)
#endif /* SOLARIS */

/* some stuff that should remain fairly constant... */
#define ONE_SECOND 1
#define ONE_MINUTE 60
#define ONE_HOUR 3600
#define ONE_DAY 86400
#define ONE_WEEK 604800
/* ok, so I defined a month as 28 days .. sue me */
#define ONE_MONTH 2419200
#define ONE_YEAR 31536000

#define SPODLIST_MINIMUM 0  /* minimum time, in seconds, it takes to be
			       considered for the spodlist: set this so 
			       that spodsort takes in between 50 and 400 
			       people for consideration (watch the time on
			       the last person on the list for when to change)
			    */	

#define BACKUP_TIME 40000

/* default port no */
#define DEFAULT_PORT 9669
/* Root directory -- NB: the trailing / *is* important!!! */
#define ROOT "/home/username/talker/"
/* and, what is this talker called? =) */
#define TALKER_NAME "The Great Unnamed Talker"
/* and the emergency HCADMIN password */
#define HC_PASSWORD "silly+rabbit"

/* path for the test alive socket */

#define SOCKET_PATH "junk/alive_socket"

/*
 * this is the room where people get chucked to when they enter the program
 */

#define ENTRANCE_ROOM "main.room"

/* this is the size of the stack for normal functions */

#define STACK_SIZE 500001

/* largest permitted log size */

#define MAX_LOG_SIZE 5000

/* saved hash table size */

#define HASH_SIZE 64

/* note hash table size */

#define NOTE_HASH_SIZE 40

/* speed of the prog (number of clicks a second) */

#define TIMER_CLICK 5

/* speed of the virtual timer */

#define VIRTUAL_CLICK 10000

/* time in seconds between every player file sync */

#define SYNC_TIME 60

/* time in seconds between every full note sync */

#define NOTE_SYNC_TIME 1800

/* defines how many lines EW-three thinks a terminal has */

#define TERM_LINES  16

/* enable or disable malloc debugging */

#undef MALLOC_DEBUG

/* timeout on news articles */

#define NEWS_TIMEOUT (21 * 60 * 60 * 24)

/* timeout on a mail article */

#define MAIL_TIMEOUT (21 * 60 * 60 * 24)

/* timeout on players */

#define PLAYER_TIMEOUT (120 * 60 * 60 * 24)


/* how many names can be included in a pipe */

#define NAME_MAX_IN_PIPE 10

/* maximum number of you and yours and stuff in a pipe */

#define YOU_MAX_IN_PIPE 3


/* which malloc routines to use */

#define MALLOC malloc

#define FREE free

/* maximum resident memory size of the program */

#define MAX_RES 1048576




/* this stuff for testing on a PC */

#undef PC

#ifdef PC
#undef tolower
#define tolower(x) mytolower(x)
#ifndef PC_FILE
extern char     mytolower();
#endif
#endif
