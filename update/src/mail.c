/*
 * mail.c
 */

#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/time.h>

#include "fix.h"
#include "config.h"
#include "player.h"

/* Our Extern Functions */

extern char *get_int(int *dest, char *source);
extern char *store_int(char *dest, int source);

/* store info for a player save */

void            construct_mail_save(saved_player * sp)
{
  int             count = 0, *scan;
  char           *oldstack;

  stack = store_int(stack, sp->mail_sent);
  if (!(sp->mail_received))
    stack = store_int(stack, 0);
  else
  {
    oldstack = stack;
    stack = store_int(oldstack, 0);
    for (scan = sp->mail_received; *scan; scan++, count++)
      stack = store_int(stack, *scan);
    store_int(oldstack, count);
  }
}


/* get info back from a player save */

char           *retrieve_mail_data(saved_player * sp, char *where)
{
  int             count = 0, *fill;
  
  where = get_int(&sp->mail_sent, where);
  where = get_int(&count, where);
  if (count)
  {
    fill = (int *) MALLOC((count + 1) * sizeof(int));
    sp->mail_received = fill;
    for (; count; count--, fill++)
      where = get_int(fill, where);
    *fill++ = 0;
  } else
    sp->mail_received = 0;
  return where;
}


