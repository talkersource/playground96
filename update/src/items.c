/* note, this is still under construction... */

/*
 * items.c
 */

#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdio.h>
#include <fcntl.h>
#include <memory.h>

#include "config.h"
#include "player.h"
#include "fix.h"
#include "items.h"
/* externs */

extern char    *upper_from_saved(saved_player * sp);
extern char    *check_legal_entry(player *, char *, int);
extern char    *store_string(), *store_int();
extern char    *get_string(), *get_int();
extern char    *gstring_possessive(), *gstring(), *end_string(), *next_space();
extern player  *find_player_global_quiet(char *), *find_player_global(char *);
extern player  *find_player_absolute_quiet(char *);
extern saved_player *find_saved_player(char *);
extern void     log(char *, char *);
extern void     tell_player(player *, char *);
extern char    *number2string(int);
extern char    *full_name(player *);
extern char    *get_gender_string(player *);
extern void     save_player(player *);
extern void     pager(player *, char *, int);
extern int      global_tag(player *, char *);
extern void     cleanup_tag(player **, int);
extern void     TELLROOM(), ADDSTACK(), ENDSTACK(), TELLPLAYER(), LOGF(), 
		SUWALL();
extern saved_player **saved_hash[];
extern int	strnomatch(char *, char *, int);

struct s_item	*top_item;
int		item_unique;
void		check_special_effect();

/* delete and entry from someones list */

void            delete_entry_item(saved_player * sp, item * l)
{
   item    *scan;
   if (!sp)
      return;
   scan = sp->item_top;
   if (scan == l)
   {
      sp->item_top = l->next;
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
   log("error", "Tried to delete item that wasn't there.\n");
}


/* compress list */

void            tmp_comp_item(saved_player * sp)
{
   char           *oldstack;
   item          *l, *next, *z, *znext;

   l = sp->item_top;

   oldstack = stack;
   stack = store_int(stack, 0);

   while (l)
   {
      next = l->next;
      if (l->id < 0)
      {
    log("error", "Bad item entry on compress .. auto deleted.\n");
    delete_entry_item(sp, l);
      } else {
    stack = store_int(stack, l->id);
    stack = store_int(stack, l->number);
    stack = store_int(stack, l->flags);
      }
      l = next;
   }
   store_int(oldstack, ((int) stack - (int) oldstack));
}


void            compress_item(saved_player * sp)
{
   char           *oldstack;
   int             length;
   item           *new, *l, *next;
   if (sp->system_flags & COMPRESSED_ITEMS)
      return;
   sp->system_flags |= COMPRESSED_ITEMS;
   oldstack = stack;
   tmp_comp_item(sp);
   length = (int) stack - (int) oldstack;
   if (length == 4)
   {
      sp->item_top = 0;
      stack = oldstack;
      return;
   }
   new = (item *) MALLOC(length);
   memcpy(new, oldstack, length);

   l = sp->item_top;
   while (l)
   {
      next = l->next;
      FREE(l);
      l = next;
   }
   sp->item_top = new;
   stack = oldstack;
}

/* decompress list */

void            decompress_item(saved_player * sp)
{
   item          *l;
   char           *old, *end, *start;
   int             length;

   if (!(sp->system_flags & COMPRESSED_ITEMS))
      return;
   sp->system_flags &= ~COMPRESSED_ITEMS;

   old = (char *) sp->item_top;
   start = old;
   if (!old)
      return;
   old = get_int(&length, old);
   end = old + length - 4;
   sp->item_top = 0;
   while (old < end)
   {
      l = (item *) MALLOC(sizeof(item));
      old = get_int(&(l->id), old);
      old = get_int(&(l->number), old);
      old = get_int(&(l->flags), old);
      l->next = sp->item_top;
      l->loc_next = 0;
      sp->item_top = l;
      l->owner = sp;
   }
   FREE(start);
}

/* save list */

void            construct_item_save(saved_player * sp)
{
   int             length;
   char           *where;

   if (!(sp->system_flags & COMPRESSED_ITEMS) &&
       (!find_player_absolute_quiet(sp->lower_name)))
      compress_item(sp);

   if (sp->system_flags & COMPRESSED_ITEMS)
   {
      if (sp->item_top)
      {
    where = (char *) sp->item_top;
    (void) get_int(&length, where);
    memcpy(stack, where, length);
    stack += length;
      } else
    stack = store_int(stack, 4);
   } else
      tmp_comp_item(sp);
}

/* retrieve list */

char           *retrieve_item_data(saved_player * sp, char *where)
{
   int             length;
   (void) get_int(&length, where);
   if (length == 4)
      sp->item_top = 0;
   else
   {
      sp->system_flags |= COMPRESSED_ITEMS;
      sp->item_top = (item *) MALLOC(length);
      memcpy(sp->item_top, where, length);
   }
   where += length;
   return where;
}

/* count list entries */

int             count_items(player * p)
{
   item	       *l, *s;
   int          count = 0;
   if (!p->saved)
      return 0;
   if (!p->saved->item_top)
      return 0;
   for (l = p->saved->item_top; l; l = l->next) {
      count++;
   }
   return count;
}

/* s_item stuff starts here! */


int             count_sitem()
{
   struct s_item	       *l;
   int          count = 0;
   if (!top_item)
      return 0;
   for (l = top_item; l; l = l->next) 
      count++;
   return count;
}

char *extract_sitem(char *stack2){

	struct s_item *d;

	d = (struct s_item *) MALLOC(sizeof(struct s_item));
	memset(d, 0, sizeof(struct s_item));
	stack2 = get_string(d->desc, stack2);
	stack2 = get_string(d->name, stack2);
	stack2 = get_string(d->author, stack2);
	stack2 = get_int(&d->id, stack2);
	stack2 = get_int(&d->sflags, stack2);
	stack2 = get_int(&d->value, stack2);

	d->next = top_item;
	top_item = d;
	return stack2;	
}

void	save_sitem(struct s_item *s) {

	stack = store_string(stack, s->desc);
	stack = store_string(stack, s->name);
	stack = store_string(stack, s->author);
	stack = store_int(stack, s->id);
	stack = store_int(stack, s->sflags);
	stack = store_int(stack, s->value);
}

/* throw all the saved items to disk */

void            sync_sitems(int background)
{
   int                n, fd, count, len;
   struct itimerval   new;
   char              *oldstack;
   struct s_item     *z;

   oldstack = stack;

   if (background && fork())
      return;

#ifdef PC
   fd = open("files\\notes\\track", O_CREAT | O_WRONLY | O_TRUNC | O_BINARY);
#else
   fd = open("files/items/saved.items", O_CREAT | O_WRONLY | O_SYNC | O_TRUNC, S_IRUSR | S_IWUSR);
#endif
   if (fd < 0)
      handle_error("Failed to open items file.");
   count = count_sitem();
   stack = store_int(stack, count);
   stack = store_int(stack, item_unique);
  if (top_item)
   for (z = top_item; z; z = z->next) {
	save_sitem(z);
   }

   len = (int) stack - (int) oldstack;

   if (write(fd, oldstack, len) < 0)
	handle_error("Ack! Can't save the saved items!!");
   close(fd);

   stack = oldstack;

   if (background)
      exit(0);
}

/*
 * load all notes from disk this should be changed for arbitary hashes
 */
	

void            init_sitems()
{
   int             length, fd, count;
   char           *oldstack;
   oldstack = stack;

   if (sys_flags & VERBOSE || sys_flags & PANIC)
      log("boot", "Loading notes from disk");

   fd = open("files/items/saved.items", O_RDONLY | O_NDELAY);
   if (fd < 0)
   {
      sprintf(oldstack, "Failed to load items file");
      stack = end_string(oldstack);
      log("error", oldstack);
      top_item = 0;
      item_unique = 0;
      stack = oldstack;
   } else {
   length = lseek(fd, 0, SEEK_END);
   lseek(fd, 0, SEEK_SET);
   if (length) { 
	if (read(fd, oldstack, length) < 0)
		handle_error("Damnit, s_items aren't loading");
	stack = get_int(&count, stack);
	stack = get_int(&item_unique, stack);
	for (; count; count--) 
		stack = extract_sitem(stack);
	       }
      close(fd);
          }
      stack = oldstack;
}


struct s_item *find_item_before(struct s_item *scan) {

	struct s_item *s;

	for (s = top_item; s; s = s->next) {
		if (s->next == scan)
			return s;
		}
	return 0;
}

struct s_item *find_item(char *name, int id) {
	
	struct s_item *s;

	for (s = top_item; s; s = s->next) 
		if (((name) && !strcasecmp(s->name, name)) || s->id == id)
			return s;

	return 0;
}

item *find_pitem(saved_player *sp, int id) {
	
	item *i;

	if (sp->system_flags & COMPRESSED_ITEMS)
		decompress_item(sp);	
	if (!(sp) || !(sp->item_top))
		return 0;
	for (i = sp->item_top; i; i = i->next) 
		if (i->id == id)
			return i;

	return 0;
}

