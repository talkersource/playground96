/*
 * aliases.c
 */

#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <fcntl.h>
#include <memory.h>

#include "config.h"
#include "player.h"
#include "fix.h"

/* externs */

extern char    *upper_from_saved(saved_player * sp);
extern char    *check_legal_entry(player *, char *, int);
extern char    *store_string(), *store_int();
extern char    *get_string(), *get_int();
extern char    *gstring_possessive(), *end_string(), *next_space();
extern player  *find_player_global_quiet(char *), *find_player_global(char *);
extern player  *find_player_absolute_quiet(char *);
extern saved_player *find_saved_player(char *);
extern void     log(char *, char *);
extern char    *number2string(int);
extern char    *full_name(player *);
extern char    *get_gender_string(player *);
extern void     save_player(player *);
extern void     pager(player *, char *, int);
extern int      global_tag(player *, char *);
extern void     cleanup_tag(player **, int);

extern saved_player **saved_hash[];
int	strnomatch(char *, char *, int);


/* delete and entry from someones list */

void            delete_entry_alias(saved_player * sp, alias * l)
{
   alias       *scan;
   if (!sp)
      return;
   scan = sp->alias_top;
   if (scan == l)
   {
      sp->alias_top = l->next;
      FREE(l);
      return;
   }
   while (scan)
      if (scan->next == l)
      {
    scan->next = l->next;
    FREE(l);
    return;
      } else
    scan = scan->next;
   log("error", "Tried to delete alias that wasn't there.\n");
}


/* compress list */

void            tmp_comp_alias(saved_player * sp)
{
   char           *oldstack;
   alias          *l, *next;

   l = sp->alias_top;

   oldstack = stack;
   stack = store_int(stack, 0);

   while (l)
   {
      next = l->next;
      if (!l->cmd[0])
      {
    log("error", "Bad list entry on compress .. auto deleted.\n");
    delete_entry_alias(sp, l);
      } else
      {
    stack = store_string(stack, l->cmd);
    stack = store_string(stack, l->sub);
      }
      l = next;
   }
   store_int(oldstack, ((int) stack - (int) oldstack));
}
/* */
void            compress_alias(saved_player * sp)
{
   char           *oldstack;
   int             length;
   alias       *new, *l, *next;
   if (sp->system_flags & COMPRESSED_ALIAS)
      return;
   sp->system_flags |= COMPRESSED_ALIAS;
   oldstack = stack;
   tmp_comp_alias(sp);
   length = (int) stack - (int) oldstack;
   if (length == 4)
   {
      sp->alias_top = 0;
      stack = oldstack;
      return;
   }
   new = (alias *) MALLOC(length);
   memcpy(new, oldstack, length);

   l = sp->alias_top;
   while (l)
   {
      next = l->next;
      FREE(l);
      l = next;
   }
   sp->alias_top = new;
   stack = oldstack;
}

/* decompress list */

void            decompress_alias(saved_player * sp)
{
   alias          *l;
   char           *old, *end, *start;
   int             length;

   if (!(sp->system_flags & COMPRESSED_ALIAS))
      return;
   sp->system_flags &= ~COMPRESSED_ALIAS;

   old = (char *) sp->alias_top;
   start = old;
   if (!old)
      return;
   old = get_int(&length, old);
   end = old + length - 4;
   sp->alias_top = 0;
   while (old < end)
   {
      l = (alias *) MALLOC(sizeof(alias));
      old = get_string(stack, old);
      strncpy(l->cmd, stack, MAX_NAME - 3);
      old = get_string(stack, old);
      strncpy(l->sub, stack, MAX_DESC - 3);
      l->next = sp->alias_top;
      sp->alias_top = l;
   }
   FREE(start);
}



/* save list */

void            construct_alias_save(saved_player * sp)
{
   int             length;
   char           *where;

   if (!(sp->system_flags & COMPRESSED_ALIAS) &&
       (!find_player_absolute_quiet(sp->lower_name)))
      compress_alias(sp);

   if (sp->system_flags & COMPRESSED_ALIAS)
   {
      if (sp->alias_top)
      {
    where = (char *) sp->alias_top;
    (void) get_int(&length, where);
    memcpy(stack, where, length);
    stack += length;
      } else
    stack = store_int(stack, 4);
   } else
      tmp_comp_alias(sp);
}

/* retrieve list */

char           *retrieve_alias_data(saved_player * sp, char *where)
{
   int             length;
   (void) get_int(&length, where);
   if (length == 4)
      sp->alias_top = 0;
   else
   {
      sp->system_flags |= COMPRESSED_ALIAS;
      sp->alias_top = (alias *) MALLOC(length);
      memcpy(sp->alias_top, where, length);
   }
   where += length;
   return where;
}

/* count list entries */

int             count_alias(player * p)
{
   alias       *l;
   int             count = 0;
   if (!p->saved)
      return 0;
   if (!p->saved->alias_top)
      return 0;
   for (l = p->saved->alias_top; l; l = l->next)
      count++;
   return count;
}

/* find list entry for a person */

alias       *find_alias_entry(player * p, char *name)
{
   alias       *l;

   if (!p->saved)
      return 0;
   decompress_alias(p->saved);
   l = p->saved->alias_top;
   while (l)
      if (!strcasecmp(l->cmd, name))
         return l;
      else
         l = l->next;
   return 0;
}


char           *do_alias_match(player * p, char *str)
{
   char           *t;
   alias          *scan;
   int            g;

   if (!p->saved)
	return "\n";
   scan = p->saved->alias_top;
   while (scan) {
      g = 1;
      if (strnomatch(scan->cmd, str, 0))
		g=0;
      if (g) {
         while (*str && *str != ' ')
            str++;
	 while (*str && isspace(*str))	
	    str++;
	 return str;
	 }
      scan = scan->next;
   }
   return "\n";
}

alias           *get_alias(player * p, char *str)
{
   char           *t;
   alias          *scan;
   int            g;

   if (!p->saved)
	return 0;
   scan = p->saved->alias_top;
   while (scan) {
      g = 1;
      if (strnomatch(scan->cmd, str, 0))
		g=0;
      if (g) {
         return scan;
	 }
      scan = scan->next;
   }
   return 0;
}

int strnomatch(char *str1, char *str2, int unanal) {

	char *s1p, *s2p;

	s1p = str1;
	s2p = str2;

	for (;*s1p;s1p++,s2p++) {
		if (unanal && *s1p != *s2p)
			return 1;
		else
		   if (tolower(*s1p) != tolower(*s2p))
			return 1;
		}
	if (!unanal && *s2p && (!isspace(*s2p)))
			return 1;
	return 0;
}	

