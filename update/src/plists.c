/*
 * Plists.c
 */

#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <memory.h>
#include <signal.h>
#include <setjmp.h>

#include "fix.h"
#include "config.h"
#include "player.h"

/* externs */

extern int      restore_player_title(player *, char *, char *);
extern void     decompress_list(saved_player *);
extern char    *end_string(), *retrieve_room_data(),
               *retrieve_list_data(), *retrieve_mail_data(),
	       *retrieve_alias_data(), *retrieve_item_data();
extern void     construct_room_save(), construct_list_save(), 
		construct_mail_save(), construct_alias_save(), 
		construct_item_save();
extern void     decompress_room(room *);
extern void     decompress_alias(saved_player *);
extern void     decompress_item(saved_player *);
extern void     log(char *, char *);
extern void     handle_error(char *);
extern void     free_room_data(saved_player *);
extern void     players_update_function(player *p);
extern void     initialise_data(player *p);
extern char    *get_int(int *dest, char *source);
extern char    *get_string(char *dest, char *source);
extern char    *store_string(char *dest, char *source);
extern char    *store_int(char *dest, int source);
extern void     extra_save_data(player *p);
extern void     extra_load_data(player *p, char *r);
/* interns */

void            error_on_load();
int             bad_player_load = 0;
char            player_loading[MAX_NAME + 2];
jmp_buf         jmp_env;

saved_player  **saved_hash[26];
int             update[26];

void            save_player();
int             restore_player();


void            do_update()
{
  player         *p;
  saved_player   *scan, **hash;
  int             i, j, fd;
  
  fd = open("/dev/null", O_WRONLY);
  p = (player *) MALLOC(sizeof(player));
  for (j = 0; j < 26; j++)
  {
    hash = saved_hash[j];
    for (i = 0; i < HASH_SIZE; i++, hash++)
    {
      for (scan = *hash; scan; scan = scan->next)
      {
        if (scan->residency != STANDARD_ROOMS
            && scan->residency != SYSTEM_ROOM)
	  /*            &&(scan->residency != BANISHED) && (scan->residency != BANISHD))*/
        {
          memset((char *) p, 0, sizeof(player));
          p->fd = fd;
          p->script = 0;
          p->location = (room *) - 1;
          restore_player(p, scan->lower_name);
          save_player(p);
        }
      }
    }
  }
  close(fd);
}

/* saved player stuff */

/* see if a saved player exists (given lower case name) */

saved_player   *find_saved_player(char *name)
{
  saved_player  **hash, *list;
   int             sum = 0,h;
   char           *c;
   if (!isalpha(*name))
      return 0;
   hash = saved_hash[((int) (tolower(*name)) - (int) 'a')];
   for (c = name; *c; c++)
   {
      if (isalpha(*c))
         sum += (int) (tolower(*c)) - 'a';
      else
    return 0;
   }
   list = *(hash + (sum % HASH_SIZE));
   for (; list; list = list->next)
      if (!strcmp(name, list->lower_name))
         return list;
   return 0;
}


/* hard load and save stuff (ie to disk and back) */

int             load_player(player * p)
{
   saved_player   *sp;
   char           *r;

   lower_case(p->lower_name);
   sp = find_saved_player(p->lower_name);
   p->saved = sp;
   if (!sp)
      return 0;

   p->residency = sp->residency;

   p->saved_residency = p->residency;
      p->system_flags = sp->system_flags;
      p->tag_flags = sp->tag_flags;
      p->custom_flags = sp->custom_flags;
      p->misc_flags = sp->misc_flags;
      p->pennies = sp->pennies;

   if (sp->residency == BANISHED || sp->residency == STANDARD_ROOMS
        || sp->residency == SYSTEM_ROOM
        || sp->residency == BANISHD)
      return 1;

   r = sp->data.where;
   r = get_string(p->name, r);
   r = get_string(p->prompt, r);
   r = get_string(p->converse_prompt, r);
   r = get_string(p->email, r);
   r = get_string(p->password, r);
   r = get_string(p->title, r);
   r = get_string(p->plan, r);
   r = get_string(p->description, r);
   r = get_string(p->enter_msg, r);
   r = get_string(p->pretitle, r);
   r = get_string(p->ignore_msg, r);
   r = get_string(p->room_connect, r);
   r = get_int(&p->term_width, r);
   r = get_int(&p->word_wrap, r);
   r = get_int(&p->max_rooms, r);
   r = get_int(&p->max_exits, r);
   r = get_int(&p->max_autos, r);
   r = get_int(&p->max_list, r);
   r = get_int(&p->max_mail, r);
   r = get_int(&p->gender, r);
   r = get_int(&p->no_shout, r);
   r = get_int(&p->total_login, r);
   r = get_int(&p->term, r);
   r = get_int(&p->birthday, r);
   r = get_int(&p->age, r);
   r = get_int(&p->jetlag, r);
   r = get_int(&p->sneezed, r);
        r = get_string(p->logonmsg, r);
        r = get_string(p->logoffmsg, r);
        r = get_string(p->blockmsg, r);
        r = get_string(p->exitmsg, r);
        r = get_int(&p->time_in_main, r);
        r = get_int(&p->no_sing, r);
        r = get_string(p->married_to, r);
        r = get_string(p->irl_name, r);
        r = get_string(p->alt_email, r);
        r = get_string(p->hometown, r);
        r = get_string(p->spod_class, r);
        r = get_string(p->favorite1, r);
        r = get_string(p->favorite2, r);
        r = get_string(p->favorite3, r);
        r = get_int(&p->total_idle_time, r);
        r = get_int(&p->max_alias, r);
        r = get_string(p->colorset, r);
        r = get_string(p->ressied_by, r);
        r = get_string(p->git_string, r);
        r = get_string(p->git_by, r);
        r = get_int(&p->warn_count, r);
        r = get_int(&p->eject_count, r);
        r = get_int(&p->idled_out_count, r);
        r = get_int(&p->booted_count, r);
        r = get_int(&p->num_ressied, r);
        r = get_int(&p->num_warned, r);
        r = get_int(&p->num_ejected, r);
        r = get_int(&p->num_rmd, r);
        r = get_int(&p->num_booted, r);
        r = get_int(&p->first_login_date, r);
        r = get_int(&p->max_items, r);
        r = get_int(&p->prs_record, r);
        r = get_string(p->ingredients, r);


   extra_load_data(p, r);

   decompress_list(sp);
   decompress_alias(sp);
   decompress_item(sp);
      p->system_flags = sp->system_flags;
      p->tag_flags = sp->tag_flags;
      p->custom_flags = sp->custom_flags;
      p->misc_flags = sp->misc_flags;
      p->pennies = sp->pennies;

   
   return 1;
}

file            construct_save_data(player * p)
{
   file            d;
   d.where = stack;

   /* Initialise any new variables */
   initialise_data(p); 
   players_update_function(p);
   stack = store_string(stack, p->name);
   stack = store_string(stack, p->prompt);
   stack = store_string(stack, p->converse_prompt);
   stack = store_string(stack, p->email);
   if (p->password[0] == -1)
      p->password[0] = 0;
   stack = store_string(stack, p->password);
   stack = store_string(stack, p->title);
   stack = store_string(stack, p->plan);
   stack = store_string(stack, p->description);
   stack = store_string(stack, p->enter_msg);
   stack = store_string(stack, p->pretitle);
   stack = store_string(stack, p->ignore_msg);
   stack = store_string(stack, p->room_connect);
   stack = store_int(stack, p->term_width);
   stack = store_int(stack, p->word_wrap);
   stack = store_int(stack, p->max_rooms);
   stack = store_int(stack, p->max_exits);
   stack = store_int(stack, p->max_autos);
   stack = store_int(stack, p->max_list);
   stack = store_int(stack, p->max_mail);
   stack = store_int(stack, p->gender);
   stack = store_int(stack, p->no_shout);
   stack = store_int(stack, p->total_login);
   stack = store_int(stack, p->term);
   stack = store_int(stack, p->birthday);
   stack = store_int(stack, p->age);
   stack = store_int(stack, p->jetlag);
   stack = store_int(stack, p->sneezed);
        stack = store_string(stack, p->logonmsg);
        stack = store_string(stack, p->logoffmsg);
        stack = store_string(stack, p->blockmsg);
        stack = store_string(stack, p->exitmsg);
        stack = store_int(stack, p->time_in_main);
        stack = store_int(stack, p->no_sing);
        stack = store_string(stack, p->married_to);
        stack = store_string(stack, p->irl_name);
        stack = store_string(stack, p->alt_email);
        stack = store_string(stack, p->hometown);
        stack = store_string(stack, p->spod_class);
        stack = store_string(stack, p->favorite1);
        stack = store_string(stack, p->favorite2);
        stack = store_string(stack, p->favorite3);
        stack = store_int(stack, p->total_idle_time);
        stack = store_int(stack, p->max_alias);
        stack = store_string(stack, p->colorset);
        stack = store_string(stack, p->ressied_by);
        stack = store_string(stack, p->git_string);
        stack = store_string(stack, p->git_by);
        stack = store_int(stack, p->warn_count);
        stack = store_int(stack, p->eject_count);
        stack = store_int(stack, p->idled_out_count);
        stack = store_int(stack, p->booted_count);
        stack = store_int(stack, p->num_ressied);
        stack = store_int(stack, p->num_warned);
        stack = store_int(stack, p->num_ejected);
        stack = store_int(stack, p->num_rmd);
        stack = store_int(stack, p->num_booted);
        stack = store_int(stack, p->first_login_date);
        stack = store_int(stack, p->max_items);
        stack = store_int(stack, p->prs_record);
        stack = store_string(stack, p->ingredients);

   
   extra_save_data(p);
   
   d.length = (int) stack - (int) d.where;
   stack = d.where;
   return d;
}
/* extract one player */

void           extract_player(char *where, int length)
{
   int             len, sum;
   char           *oldstack, *c;
   saved_player   *old, *sp, **hash;

   oldstack = stack;
   where = get_int(&len, where);
   where = get_string(oldstack, where);
   stack = end_string(oldstack);
   old = find_saved_player(oldstack);
   sp = old;
   if (!old)
   {
      sp = (saved_player *) MALLOC(sizeof(saved_player));
      memset((char *) sp, 0, sizeof(saved_player));
      strncpy(sp->lower_name, oldstack, MAX_NAME);
      strncpy(player_loading, sp->lower_name, MAX_NAME);
      sp->rooms = 0;
      sp->mail_sent = 0;
      sp->mail_received = 0;
      sp->list_top = 0;
      hash = saved_hash[((int) sp->lower_name[0] - (int) 'a')];
      for (sum = 0, c = sp->lower_name; *c; c++)
         sum += (int) (*c) - 'a';
      hash = (hash + (sum % HASH_SIZE));
      sp->next = *hash;
      *hash = sp;
   }
   where = get_int(&sp->last_on, where);
   where = get_int(&sp->system_flags, where);
   where = get_int(&sp->tag_flags, where);
   where = get_int(&sp->custom_flags, where);
   where = get_int(&sp->misc_flags, where);
   where = get_int(&sp->pennies, where);
   where = get_int(&sp->residency, where);
 
   if (sp->residency == BANISHED)
      sp->residency = BANISHD;
   if (sp->residency == BANISHD)
   {
      sp->last_host[0] = 0;
      sp->data.where = 0;
      sp->data.length = 0;
      stack = oldstack;
      return;
   }
/* PUT ANYTHING TO CHANGE RESIDENCY OR OTHER FLAGS HERE */
   where = get_string(sp->last_host, where);
   where = get_string(sp->email, where);
   where = get_int(&sp->data.length, where);
   sp->data.where = (char *) MALLOC(sp->data.length);
   memcpy(sp->data.where, where, sp->data.length);
   where += sp->data.length;
   where = retrieve_room_data(sp, where);
   where = retrieve_list_data(sp, where);
   where = retrieve_alias_data(sp, where);
   where = retrieve_item_data(sp, where);
   where = retrieve_mail_data(sp, where);
   stack = oldstack;
}

/* hard load in on player file */

void            hard_load_one_file(char c)
{
   char           *oldstack, *where, *scan;
   int             fd, length, len2, i, fromjmp;

   oldstack = stack;
   if (sys_flags & VERBOSE)
   {
      sprintf(oldstack, "Loading player file '%c'.", c);
      stack = end_string(oldstack);
      log("boot", oldstack);
      stack = oldstack;
   }
#ifdef PC
   sprintf(oldstack, "files\\players\\%c", c);
   fd = open(oldstack, O_RDONLY | O_BINARY);
#else
   sprintf(oldstack, "%s%c", rc_options->pfile_path,c);
   fd = open(oldstack, O_RDONLY | O_NDELAY);
#endif
   if (fd < 0)
   {
      sprintf(oldstack, "Failed to load player file '%c'", c);
      stack = end_string(oldstack);
      log("error", oldstack);
   } else
   {
      length = lseek(fd, 0, SEEK_END);
      lseek(fd, 0, SEEK_SET);
      if (length)
      {
         where = (char *) MALLOC(length);
         if (read(fd, where, length) < 0)
            handle_error("Can't read player file.");
         for (i = 0, scan = where; i < length;)
         {
            get_int(&len2, scan);
            fromjmp = setjmp(jmp_env);
            if (!fromjmp && !bad_player_load)
            {
               extract_player(scan, len2);
            } else
            {
               sprintf(oldstack, "Bad Player \'%s\' deleted on load.",
                       player_loading);
               stack = end_string(oldstack);
               log("boot", oldstack);
               stack = oldstack;
               remove_player_file(player_loading);
               bad_player_load = 0;
            }
            i += len2;
            scan += len2;
         }
         FREE(where);
      }
      close(fd);
   }
   stack = oldstack;
}


/* load in all the player files */

void            hard_load_files()
{
  char            c;
  int             i, hash_length;
  char           *oldstack;
  
  oldstack = stack;
  hash_length = HASH_SIZE * sizeof(saved_player *);
  for (i = 0; i < 26; i++)
  {
    saved_hash[i] = (saved_player **) MALLOC(hash_length);
    memset((void *) saved_hash[i], 0, hash_length);
  }
  for (c = 'a'; c <= 'z'; c++)
    hard_load_one_file(c);
}


/* write one player file out */

void            write_to_file(saved_player * sp)
{
   char           *oldstack;
   int             length;
#ifdef PLIST_TRACK
   printf("write_to_file:%s\n", sp->lower_name);
#endif
   oldstack = stack;
   if (sys_flags & VERBOSE && sys_flags & PANIC)
   {
      sprintf(oldstack, "Attempting to write player '%s'.", sp->lower_name);
      stack = end_string(oldstack);
      log("sync", oldstack);
      stack = oldstack;
   }
   stack += 4;
   stack = store_string(stack, sp->lower_name);
   stack = store_int(stack, sp->last_on);
   stack = store_int(stack, sp->system_flags);
   stack = store_int(stack, sp->tag_flags);
   stack = store_int(stack, sp->custom_flags);
   stack = store_int(stack, sp->misc_flags);
   stack = store_int(stack, sp->pennies);
   stack = store_int(stack, sp->residency);

   if ((sp->residency != BANISHED) )
   {
      stack = store_string(stack, sp->last_host);
      stack = store_string(stack, sp->email);
      stack = store_int(stack, sp->data.length);
      memcpy(stack, sp->data.where, sp->data.length);
      stack += sp->data.length;
      construct_room_save(sp);
      construct_list_save(sp);
      construct_alias_save(sp);
      construct_item_save(sp);
      construct_mail_save(sp);
    }
   length = (int) stack - (int) oldstack;
   (void) store_int(oldstack, length);

}

/* sync player files corresponding to one letter */

void            sync_to_file(char c, int background)
{
  saved_player   *scan, **hash;
   char           *oldstack;
   int             fd, i, length;
#ifdef PLIST_TRACK_TEST
  printf("sync_to_file(%c)\n", c);
#endif
   if (background && fork())
      return;

   oldstack = stack;
   if (sys_flags & VERBOSE)
   {
      sprintf(oldstack, "Syncing File '%c'.", c);
      stack = end_string(oldstack);
      log("sync", oldstack);
      stack = oldstack;
   }
   hash = saved_hash[((int) c - (int) 'a')];
   for (i = 0; i < HASH_SIZE; i++, hash++)
      for (scan = *hash; scan; scan = scan->next)
    if (scan->residency != STANDARD_ROOMS
        && scan->residency != SYSTEM_ROOM)
       write_to_file(scan);
   length = (int) stack - (int) oldstack;


   /* test that you can write out a file ok */

   sprintf(stack, "%sbackup_write", rc_options->new_pfile_path);
   fd = open(stack, O_CREAT | O_WRONLY | O_SYNC | O_TRUNC, S_IRUSR | S_IWUSR);
   if (fd < 0)
      handle_error("Primary open failed (player back)");
   if (write(fd, oldstack, length) < 0)
      handle_error("Primary write failed "
         "(playerback)");
   close(fd);
   sprintf(stack, "%s%c", rc_options->new_pfile_path, c);
   fd = open(stack, O_CREAT | O_WRONLY | O_SYNC | O_TRUNC, S_IRUSR | S_IWUSR);
   if (fd < 0)
      handle_error("Failed to open player file.");
   if (write(fd, oldstack, length) < 0)
      handle_error("Failed to write player file.");
   close(fd);
  update[(int) c - (int) 'a'] = 0;
   stack = oldstack;

   if (background)
      exit(0);

}

/* sync everything to disk */

void            sync_all()
{
   char            c, *oldstack;
   oldstack = stack;
   for (c = 'a'; c <= 'z'; c++)
      sync_to_file(c, 0);
}

/* flicks on the update flag for a particular player hash */

void            set_update(char c)
{
   update[(int) c - (int) 'a'] = 1;
}


/* removes an entry from the saved player lists */

int             remove_player_file(char *name)
{
  saved_player   *previous = 0, **hash, *list;
   char           *c;
   int             sum = 0;

   if (!isalpha(*name))
   {
      log("error", "Tried to remove non-player from save files.");
      return 0;
   }
   strcpy(stack, name);
   lower_case(stack);

   hash = saved_hash[((int) (*stack) - (int) 'a')];
   for (c = stack; *c; c++)
   {
      if (isalpha(*c))
         sum += (int) (*c) - 'a';
      else
      {
         log("error", "Remove bad name from save files");
         return 0;
      }
   }

   hash += (sum % HASH_SIZE);
   list = *hash;
   for (; list; previous = list, list = list->next)
   {
      if (!strcmp(stack, list->lower_name))
      {
         if (previous)
            previous->next = list->next;
         else
            *hash = list->next;
         if (list->data.where)
            FREE(list->data.where);
         if (list->mail_received)
            FREE(list->mail_received);
         free_room_data(list);
         FREE((void *) list);
         set_update(*stack);
         return 1;
      }
   }
   return 0;
}

/* the routine that sets everything up for the save */

void            save_player(player * p)
{
  saved_player   *old, **hash, *sp;
  int             sum;
  file            data;
  char           *c, *oldstack;
  int verb = 1;
  oldstack = stack;
  
  if (!(p->location) || !(p->name[0])
      || p->residency == NON_RESIDENT)
    return;
  
  if (sys_flags & PANIC)
  {
    c = stack;
    sprintf(c, "Attempting to save player %s.", p->name);
    stack = end_string(c);
    log("boot", c);
    stack = c;
  }
  if (!(isalpha(p->lower_name[0])))
  {
    log("error", "Tried to save non-player.");
    return;
  }
  if (p->residency & SYSTEM_ROOM)
    verb = 0;
  if (verb)
  {
    if (!(p->password[0] && p->password[0] != -1))
    {
      p->residency |= NO_SYNC;
      stack = oldstack;
      return;
    }
    if (p->email[0] == 2)
    {
      p->residency |= NO_SYNC;
      stack = oldstack;
      return;
    }
  }
  p->residency &= ~NO_SYNC;
  p->saved_residency = p->residency;
  old = p->saved;
  sp = old;
  if (!old)
  {
    sp = (saved_player *) MALLOC(sizeof(saved_player));
    memset((char *) sp, 0, sizeof(saved_player));
    strncpy(sp->lower_name, p->lower_name, MAX_NAME);
    sp->rooms = 0;
    sp->mail_sent = 0;
    sp->mail_received = 0;
    sp->list_top = 0;
    hash = saved_hash[((int) p->lower_name[0] - (int) 'a')];
    for (sum = 0, c = p->lower_name; *c; c++)
    {
      if (isalpha(*c))
	sum += (int) (*c) - 'a';
      else
      {
	FREE(sp);
	return;
      }
    }
    hash = (hash + (sum % HASH_SIZE));
    sp->next = *hash;
    *hash = sp;
    p->saved = sp;
      sp->system_flags = p->system_flags;
      sp->tag_flags = p->tag_flags;
      sp->custom_flags = p->custom_flags;
      sp->misc_flags = p->misc_flags;
      sp->pennies = p->pennies;
    create_room(p);
  }
  data = construct_save_data(p);
  if (!data.length)
  {
    log("error", "Bad construct save.");
    return;
  }
  if (old && sp->data.where)
    FREE((void *) sp->data.where);
  sp->data.where = (char *) MALLOC(data.length);
  sp->data.length = data.length;
  memcpy(sp->data.where, data.where, data.length);
  sp->residency = p->saved_residency;
      sp->system_flags = p->system_flags;
      sp->tag_flags = p->tag_flags;
      sp->custom_flags = p->custom_flags;
      sp->misc_flags = p->misc_flags;
      sp->pennies = p->pennies;

  set_update(*(sp->lower_name));
  p->saved = sp;
}


/* load from a saved player into a current player */


/* load and do linking */

int             restore_player(player * p, char *name)
{
   return restore_player_title(p, name, 0);
}

int             restore_player_title(player * p, char *name, char *title)
{
   int             did_load;
   int             found_lower;
   char           *n;

   strncpy(p->name, name, MAX_NAME - 2);
   strncpy(p->lower_name, name, MAX_NAME - 2);
   lower_case(p->lower_name);
   if (!strcmp(p->name, p->lower_name))
     p->name[0] = toupper(p->name[0]);
   found_lower = 0;
   n = p->name;
   while (*n)
   {
     if (*n >= 'a' && *n <= 'z')
     {
       found_lower = 1;
     }
     *n++;
   }
   if (!found_lower)
   {
     n = p->name;
     *n++;
     while (*n)
     {
       *n = *n - ('A' - 'a');
       *n++;
     }
   }
   did_load = load_player(p);
   strcpy(p->assisted_by, "");
   if (title && *title)
   {
      strncpy(p->title, title, MAX_TITLE);
      p->title[MAX_TITLE] = 0;
   }
   if ((p->system_flags & IAC_GA_ON) && (!(p->flags & EOR_ON)))
      p->flags |= IAC_GA_DO;
   else
      p->flags &= ~IAC_GA_DO;
   if (p->residency == 0 && did_load == 1)
      p->residency = SYSTEM_ROOM;
   if (p->system_flags & SAVEDFROGGED)
      p->flags |= FROGGED;
   if (p->residency & PSU)
      p->no_shout = 0;
   p->saved_residency = p->residency;
   if ((p->word_wrap) > ((p->term_width) >> 1) || p->word_wrap < 0)
      p->word_wrap = (p->term_width) >> 1;
   if (p->term > 9)
      p->term = 0;
   return did_load;
}

/* init everything needed for the plist file */

void            init_plist()
{
   char           *oldstack;
   int             i;

   oldstack = stack;
   hard_load_files();
   for (i = 0; i < 26; i++)
      update[i] = 1;
   stack = oldstack;
}
/* Catch SEGV's and BUS's on load of players, hopefully... */

void error_on_load()
{
   bad_player_load = 1;
   longjmp(jmp_env, 0);
   longjmp(jmp_env, 0);
}
