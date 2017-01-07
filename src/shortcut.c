/*
 * shortcut.c  -- includes all our fun shortcut commands 
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "player.h"

/* and those joyous externs, dont you love em? */

extern char 	*end_string();
extern void 	log(char *, char *), tell_player(player *, char *), 
	        su_wall(char *), au_wall(char *), su_wall_but(player *, char *), 
                au_wall_but(player *, char *), tell_room(room *, char *),
		tell_room_but2(player *, room *, char *);


/* experimental printf clone for log -- if it works, we'll add it for
   tell_player and su_wall too -- cuz its easier to debug one error than
   100000 */

void LOGF(char *file, char *format, ...)
{
   va_list argum;
   char *oldstack;

   oldstack = stack;
   va_start(argum, format);
   vsprintf(stack, format, argum);
   va_end(argum);
   stack = end_string(stack);
   log(file, oldstack);
   stack = oldstack;
}

void TELLPLAYER(player *pl, char *format, ...)
{
   va_list argum;
   char *oldstack;

   oldstack = stack;
   va_start(argum, format);
   vsprintf(stack, format, argum);
   va_end(argum);
   stack = end_string(stack);
   tell_player(pl, oldstack);
   stack = oldstack;
}

void SUWALL(char *format, ...)
{
   va_list argum;
   char *oldstack;

   oldstack = stack;
   va_start(argum, format);
   vsprintf(stack, format, argum);
   va_end(argum);
   stack = end_string(stack);
   su_wall(oldstack);
   stack = oldstack;
}

void AUWALL(char *format, ...)
{
   va_list argum;
   char *oldstack;

   oldstack = stack;
   va_start(argum, format);
   vsprintf(stack, format, argum);
   va_end(argum);
   stack = end_string(stack);
   au_wall(oldstack);
   stack = oldstack;
}

void SW_BUT(player *but, char *format, ...)
{
   va_list argum;
   char *oldstack;

   oldstack = stack;
   va_start(argum, format);
   vsprintf(stack, format, argum);
   va_end(argum);
   stack = end_string(stack);
   su_wall_but(but, oldstack);
   stack = oldstack;
}

void AW_BUT(player *but, char *format, ...)
{
   va_list argum;
   char *oldstack;

   oldstack = stack;
   va_start(argum, format);
   vsprintf(stack, format, argum);
   va_end(argum);
   stack = end_string(stack);
   au_wall_but(but, oldstack);
   stack = oldstack;
}
void TELLROOM(room *here, char *format, ...)
{
   va_list argum;
   char *oldstack;

   oldstack = stack;
   va_start(argum, format);
   vsprintf(stack, format, argum);
   va_end(argum);
   stack = end_string(stack);
   tell_room(here, oldstack);
   stack = oldstack;
}
void TELLROOM_BUT(player * p, room *here, char *format, ...)
{
   va_list argum;
   char *oldstack;

   oldstack = stack;
   va_start(argum, format);
   vsprintf(stack, format, argum);
   va_end(argum);
   stack = end_string(stack);
   tell_room_but2(p, here, oldstack);
   stack = oldstack;
}
/* these two are a bit different, no? Well, um.. yeah, kinda... */

void ADDSTACK(char *format, ...)
{
   va_list argum;

   va_start(argum, format);
   vsprintf(stack, format, argum);
   va_end(argum);
   stack = strchr(stack, 0);
}

void ENDSTACK(char *format, ...)
{
   va_list argum;

   va_start(argum, format);
   vsprintf(stack, format, argum);
   va_end(argum);
   stack = end_string(stack);
}


