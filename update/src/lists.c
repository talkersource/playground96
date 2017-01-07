/*
 * lists.c
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

extern char    *store_string(), *store_int();
extern char    *get_string(), *get_int();
extern void     log(char *, char *);


player         *find_player_absolute_quiet(char *name)
{
   player         *scan;

   if (!isalpha(*name))
      return 0;

   strcpy(stack, name);
   lower_case(stack);

   scan = hashlist[(int) (*stack) - (int) 'a' + 1];
   for (; scan; scan = scan->hash_next)
      if (!strcmp(stack, scan->lower_name))
    return scan;

   return 0;
}

/* delete and entry from someones list */

void            delete_entry(saved_player * sp, list_ent * l)
{
   list_ent       *scan;
   if (!sp)
      return;
   scan = sp->list_top;
   if (scan == l)
   {
      sp->list_top = l->next;
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
   log("error", "Tried to delete list entry that wasn't there.\n");
}

/* compress list */

void            tmp_comp_list(saved_player * sp)
{
  char           *oldstack;
  list_ent       *l, *next;
  
  l = sp->list_top;
  
  oldstack = stack;
  stack = store_int(stack, 0);
  
  while (l)
  {
    next = l->next;
    if (!l->name[0])
    {
      log("error", "Bad list entry on compress .. auto deleted.\n");
      delete_entry(sp, l);
    } else
    {
      stack = store_string(stack, l->name);
      stack = store_int(stack, l->flags);
    }
    l = next;
  }
  store_int(oldstack, ((int) stack - (int) oldstack));
}

void            compress_list(saved_player * sp)
{
   char           *oldstack;
   int             length;
   list_ent       *new, *l, *next;
   if (sp->system_flags & COMPRESSED_LIST)
      return;
   sp->system_flags |= COMPRESSED_LIST;
   oldstack = stack;
   tmp_comp_list(sp);
   length = (int) stack - (int) oldstack;
   if (length == 4)
   {
      sp->list_top = 0;
      stack = oldstack;
      return;
   }
   new = (list_ent *) MALLOC(length);
   memcpy(new, oldstack, length);

   l = sp->list_top;
   while (l)
   {
      next = l->next;
      FREE(l);
      l = next;
   }
   sp->list_top = new;
   stack = oldstack;
}

/* decompress list */

void            decompress_list(saved_player * sp)
{
   list_ent       *l;
   char           *old, *end, *start;
   int             length;

   if (!(sp->system_flags & COMPRESSED_LIST))
      return;
   sp->system_flags &= ~COMPRESSED_LIST;

   old = (char *) sp->list_top;
   start = old;
   if (!old)
      return;
   old = get_int(&length, old);
   end = old + length - 4;
   sp->list_top = 0;
   while (old < end)
   {
      l = (list_ent *) MALLOC(sizeof(list_ent));
      old = get_string(stack, old);
      strncpy(l->name, stack, MAX_NAME - 2);
      old = get_int(&(l->flags), old);
      l->next = sp->list_top;
      sp->list_top = l;
   }
   FREE(start);
}

/* save list */

void            construct_list_save(saved_player * sp)
{
   int             length;
   char           *where;

   if (!(sp->system_flags & COMPRESSED_LIST) &&
       (!find_player_absolute_quiet(sp->lower_name)))
      compress_list(sp);

   if (sp->system_flags & COMPRESSED_LIST)
   {
      if (sp->list_top)
      {
    where = (char *) sp->list_top;
    (void) get_int(&length, where);
    memcpy(stack, where, length);
    stack += length;
      } else
    stack = store_int(stack, 4);
   } else
      tmp_comp_list(sp);
}

/* retrieve list */

char           *retrieve_list_data(saved_player * sp, char *where)
{
   int             length;
   (void) get_int(&length, where);
   if (length == 4)
      sp->list_top = 0;
   else
   {
      sp->system_flags |= COMPRESSED_LIST;
      sp->list_top = (list_ent *) MALLOC(length);
      memcpy(sp->list_top, where, length);
   }
   where += length;
   return where;
}


