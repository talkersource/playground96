/*
 * glue.c
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <memory.h>
#include <malloc.h>

#include "config.h"
#include "player.h"
#include "fix.h"
#include "file.h"
#define ABS(x) (((x)>0)?(x):-(x))

/* extern definitions */
extern unsigned int sleep(unsigned int seconds);
extern void     lower_case(char *);
extern char    *sys_errlist[];
extern void     init_rooms(), init_plist(),
                save_player(), sync_all(), do_update();
extern void     do_update();
extern void     do_tests();
void            close_down();

/* External variables */

extern int num_residents, num_psu, num_su, num_ladmin, num_admin,
           num_banished;

/* Internal func's definitions */


/* return a string of the system time */

char           *sys_time()
{
   time_t          t;
   static char     time_string[25];
   t = time(0);
   strftime(time_string, 25, "%H:%M:%S - %d/%m/%y", localtime(&t));
   return time_string;
}

char           *end_string(char *str)
{
   str = strchr(str, 0);
   str++;
   return str;
}

void            log(char *file, char *string)
{
   int             fd, length;

   sprintf(stack, "logs/%s.log", file);
   fd = open(stack, O_CREAT | O_WRONLY | O_SYNC, S_IRUSR | S_IWUSR);
   length = lseek(fd, 0, SEEK_END);
   if (length > MAX_LOG_SIZE)
   {
      close(fd);
      fd = open(stack, O_CREAT | O_WRONLY | O_SYNC | O_TRUNC, S_IRUSR | S_IWUSR);
   }
   sprintf(stack, "%s - %s\n", sys_time(), string);
   if (!(sys_flags & NO_PRINT_LOG))
      printf(stack);
   write(fd, stack, strlen(stack));
   close(fd);
}

/* what happens when *shriek* an error occurs */

void            handle_error(char *error_msg)
{
   char            dump[80];

   if (sys_flags & PANIC)
   {
      stack = stack_start;
      log("error", "Immediate PANIC shutdown.");
      exit(-1);
   }
   sys_flags |= PANIC;
   stack = stack_start;

   if ( sys_flags & UPDATE )
      sys_flags &= ~NO_PRINT_LOG;

   log("error", error_msg);
   log("boot", "Abnormal exit from error handler");

   /* dump possible useful info */

   log("dump", "------------ Starting dump");

   sprintf(stack_start, "Errno set to %d, %s", errno, sys_errlist[errno]);
   stack = end_string(stack_start);
   log("dump", stack_start);

   if (current_player)
   {
      log("dump", current_player->name);
      if (current_player->location)
      {
    sprintf(stack_start, "player %s.%s",
       current_player->location->owner->lower_name,
       current_player->location->id);
    stack = end_string(stack_start);
    log("dump", stack_start);
      } else
    log("dump", "No room of current player");

      sprintf(stack_start, "flags %d saved %d residency %d", current_player->flags,
         current_player->system_flags, current_player->residency);
      stack = end_string(stack_start);
      log("dump", stack_start);
      log("dump", current_player->ibuffer);
   } else
      log("dump", "No current player !");
   if (current_room)
   {
      sprintf(stack_start, "current %s.%s", current_room->owner->lower_name,
         current_room->id);
      stack = end_string(stack_start);
      log("dump", stack_start);
   } else
      log("dump", "No current room");

   sprintf(stack_start, "global flags %d, players %d", sys_flags, current_players);
   stack = end_string(stack_start);
   log("dump", stack_start);

   sprintf(stack_start, "action Updating player files");
   stack = end_string(stack_start);
   log("dump", stack_start);

   log("dump", "---------- End of dump info");


   close_down();
   exit(-1);
}

/* load a file into memory */

file            load_file_verbose(char *filename, int verbose)
{
   file            f;
   int             d;
   char           *oldstack;

   oldstack = stack;

   d = open(filename, O_RDONLY);
   if (d < 0)
   {
      sprintf(oldstack, "Can't find file:%s", filename);
      stack = end_string(oldstack);
      if (verbose)
    log("error", oldstack);
      f.where = (char *) MALLOC(1);
      *(char *) f.where = 0;
      f.length = 0;
      stack = oldstack;
      return f;
   }
   f.length = lseek(d, 0, SEEK_END);
   lseek(d, 0, SEEK_SET);
   f.where = (char *) MALLOC(f.length + 1);
   memset(f.where, 0, f.length + 1);
   if (read(d, f.where, f.length) < 0)
   {
      sprintf(oldstack, "Error reading file:%s", filename);
      stack = end_string(oldstack);
      log("error", oldstack);
      f.where = (char *) MALLOC(1);
      *(char *) f.where = 0;
      f.length = 0;
      stack = oldstack;
      return f;
   }
   close(d);
   if (sys_flags & VERBOSE)
   {
      sprintf(oldstack, "Loaded file:%s", filename);
      stack = end_string(oldstack);
      log("boot", oldstack);
      stack = oldstack;
   }
   stack = oldstack;
   *(f.where + f.length) = 0;
   return f;
}

file            load_file(char *filename)
{
   return load_file_verbose(filename, 1);
}

/* convert a string to lower case */

void            lower_case(char *str)
{
   while (*str)
      *str++ = tolower(*str);
}

/* close down sequence */

void            close_down()
{
   player         *scan, *old_current;

   for (scan = flatlist_start; scan; scan = scan->flat_next)
      save_player(scan);
   sync_all();
   old_current = current_player;
   current_player = 0;
   current_player = old_current;

   for (scan = flatlist_start; scan; scan = scan->flat_next)
      close(scan->fd);

}


/* the boot sequence */

void init_update()
{
   char *oldstack;
   int i;

   oldstack = stack;
   flatlist_start = 0;
   for (i = 0; i < 27; i++)
      hashlist[i] = 0;
   stdout_player = (player *) MALLOC(sizeof(player));
   memset(stdout_player, 0, sizeof(player));
   srand(time(0));
   init_plist();
   init_rooms();
   current_players = 0;
   stack = oldstack;
}



void main(int argc, char *argv[])
{
  int mem_used;

  /* Initialise rc_options struct */
  rc_options = (rc_type *) malloc(sizeof(rc_type));
  memset(rc_options, 0, sizeof(rc_type));
  stack_start = (char *) MALLOC(STACK_SIZE);
  memset(stack_start, 0, STACK_SIZE);
  stack = stack_start;
  /* Initialise ressie counters */
  num_residents = 0;
  num_psu = 0;
  num_su = 0;
  num_ladmin = 0;
  num_admin = 0;
  num_banished = 0;
  /* Read in .rc file and process */
  init_updaterc(".updaterc");
  /* Make sure update will be successful */
  do_tests();
  printf(" Starting update .....\n");
  sys_flags |= SHUTDOWN | UPDATE;
  /* Do actual update */
  init_update();
  do_update();
  close_down();
  free(rc_options);
  printf(" Finished !!\n\n");
  printf(" ===========================\n");
  printf("     Resident Statistics\n"
	 " ===========================\n");
  printf("  Normal Residents  (%d)\n", num_residents);
  printf("  Pseudo Su's       (%d)\n", num_psu);
  printf("  Super Users       (%d)\n", num_su);
  printf("  Lower Admins      (%d)\n", num_ladmin);
  printf("  Admins            (%d)\n", num_admin);
  printf(" ===========================\n");
  printf("  Total             [%d]\n", (num_residents+num_psu+
					num_su+num_ladmin+num_admin));
  printf("  Banished Pfiles   [%d]\n", num_banished);
  printf(" ===========================\n");
  printf(" Update for PG96 Run Completed.\n");
}



