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

/* externs */

extern char    *end_string();
extern void     log(char *, char *);

char           *stack_check;


void help(player * p, char *str)
{
}
