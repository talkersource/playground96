/*
 * Rooms.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <memory.h>
#include <string.h>

#include "config.h"
#include "player.h"
#include "fix.h"
#include "dynamic.h"


/* Our Extern Functions */

extern void dynamic_free(dfile *df,int key);
extern dfile *dynamic_init(char *file,int granularity);
extern int dynamic_load(dfile *df,int key,char *data);
extern int dynamic_save(dfile *df,char *data,int l,int key);
extern char *end_string(char *str);
extern saved_player *find_saved_player(char *name);
extern char *get_int(int *dest, char *source);
extern char *get_string(char *dest, char *source);
extern file load_file(char *filename);
extern int load_player(player * p);
extern void log(char *file, char *string);
extern int restore_player(player * p, char *name);
extern void save_player(player * p);
extern char *store_int(char *dest, int source);
extern char *store_string(char *dest, char *source);
extern void init_your_rooms(void);

/* Local Function Prototypes */

void delete_room(player * p, char *str); 

/* Local Variables */

dfile *room_df;

void            delete_room(player * p, char *str)
{
   char           *oldstack;
   room           *scan, **previous, *r;
   oldstack = stack;
#ifdef ROOM_TRACK
   printf("delete_room(%s, %s)\n", p->lower_name, str);
#endif
   if (!current_room)
   {
      return;
   }
   if (!strcmp("system", current_room->owner->lower_name) &&
       !strcmp("void", current_room->id))
     return;
   r = current_room;
   while (r->players_top)
   {}
   previous = &(r->owner->rooms);
   scan = *previous;
   for (; scan && scan != r; scan = scan->next)
      previous = &(scan->next);
   if (scan)
   {
      *previous = scan->next;
      if (scan->text.where)
    FREE(scan->text.where);
      if (scan->exits.where)
    FREE(scan->exits.where);
      if (scan->automessage.where)
    FREE(scan->automessage.where);
      dynamic_free(room_df,scan->data_key);
      FREE(scan);
   } else
      *previous = 0;
   stack = oldstack;
}

/* creates a new standard room */

room *create_room(player * p)
{
  room *r, *scan;
  saved_player   *sp;
  int             number, home = 1, unique = 1;
  char            id[MAX_ID];
#ifdef ROOM_TRACK
  printf("create_room(%s)\n", p->lower_name);
#endif
  sp = p->saved;
  if (!sp)
  {
    return 0;
  }
  sprintf(id, "room.%d", unique);
  do
  {
    for (scan = sp->rooms, number = 1; scan; number++, scan = scan->next)
    {
      if (!strcmp(scan->id, id))
      {
	unique++;
	sprintf(id, "room.%d", unique);
	break;
      }
      if (scan->flags & HOME_ROOM)
	home = 0;
    }
  } while (scan);
  
  if (number > p->max_rooms)
    return 0;
  r = (room *) MALLOC(sizeof(room));
  memset(r, 0, sizeof(room));
  strcpy(r->id, id);
  sprintf(r->name, "in somewhere belonging to %s.", p->name);
  sprintf(r->enter_msg, "goes to a room belonging to %s.", p->name);
  r->flags = (home * HOME_ROOM)|ROOM_UPDATED;
  sprintf(stack, "\n A bare room, belonging to %s.\n"
	  " Isn't it time to write a description ?\n", p->name);
  number = strlen(stack) + 1;
  r->text.where = (char *) MALLOC(number);
  memcpy(r->text.where, stack, number);
  r->text.length = number;
  r->exits.length = 0;
  r->exits.where = 0;
  r->auto_base = 30;
  r->automessage.length = 0;
  r->automessage.where = 0;
  r->owner = sp;
  r->players_top = 0;
  r->next = sp->rooms;
  sp->rooms = r;
  r->flags |= ROOM_UPDATED;
  return r;
}


/* change decompress room to get from disk */

int decompress_room(room *r)
{
  int length;
  char *oldstack,*tmp;
#ifdef ROOM_TRACK
  printf("decompress_room(%s)\n", r->id);
#endif
  oldstack=stack;
  if (!(r->flags&COMPRESSED)) return 1;
  
  length=dynamic_load(room_df,r->data_key,stack);
  if (length<=0) {
    current_room=r;
    delete_room(current_player,0);
    return 0;
  }
  tmp=stack;
  stack+=length;
  
  tmp=get_string(stack,tmp);
  length=strlen(stack)+1;
  r->text.length=length;
  if (r->text.where) FREE(r->text.where);
  r->text.where=(char *)MALLOC(length);
  strcpy(r->text.where,stack);
  
  tmp=get_string(stack,tmp);
  length=strlen(stack)+1;
  if (length==1) {
    r->exits.length=0;
    r->exits.where=0;
  }
  else {
    r->exits.length=length;
    if (r->exits.where) FREE(r->exits.where);
    r->exits.where=(char *)MALLOC(length);
    strcpy(r->exits.where,stack);
  }
  tmp=get_string(stack,tmp);
  length=strlen(stack)+1;
  if (length==1) {
    r->automessage.length=0;
    r->automessage.where=0;
  }
  else {
    r->automessage.length=length;
    if (r->automessage.where) FREE(r->automessage.where);
    r->automessage.where=(char *)MALLOC(length);
    strcpy(r->automessage.where,stack);
  }
  r->flags &= ~(COMPRESSED|ROOM_UPDATED);
  stack=oldstack;
  return 1;
}


/* and the compress room chugs to the disk */

void compress_room(room *r)
{
  int length;
  char *oldstack;
  oldstack=stack;

#ifdef ROOM_TRACK
  printf("compress_room(%s)\n", r->id);
#endif
  if (r->flags&COMPRESSED) return;
  if (r->owner && r->owner->residency==STANDARD_ROOMS) return;
/*  printf("compress key = %d, id = %s\n",r->data_key,r->id); */
  if (r->flags&ROOM_UPDATED) {
    if (r->text.where) stack=store_string(stack,r->text.where);
    else stack=store_string(stack,"");
    if (r->exits.where) stack=store_string(stack,r->exits.where);
    else stack=store_string(stack,"");
    if (r->automessage.where) stack=store_string(stack,r->automessage.where);
    else stack=store_string(stack,"");
    length=(int)stack-(int)oldstack;
    r->data_key=dynamic_save(room_df,oldstack,length,r->data_key);
  }
  if (r->text.where) FREE(r->text.where);
  r->text.where=0;
  r->text.length=0;
  if (r->exits.where) FREE(r->exits.where);
  r->exits.where=0;
  r->exits.length=0;
  if (r->automessage.where) FREE(r->automessage.where);
  r->automessage.where=0;
  r->automessage.length=0;
  r->flags |= COMPRESSED;
  r->flags &= ~ROOM_UPDATED;
  stack=oldstack;
 }






 /* destroy all the room data */

void free_room_data(saved_player *sp)
{
  room *room_list,*next;
#ifdef ROOM_TRACK
  printf("free_room_data(%s)\n", sp->lower_name);
#endif
  if (!sp->rooms) return;
   room_list=sp->rooms;
   while(room_list) {
     next=room_list->next;
     if (room_list->text.where) FREE(room_list->text.where);
     if (room_list->exits.where)  FREE(room_list->exits.where);
     if (room_list->automessage.where)  FREE(room_list->automessage.where);
     dynamic_free(room_df,room_list->data_key);
     FREE(room_list);
     room_list=next;
   }
   sp->rooms=0;
}

/* collect all the room data ready for saving */

void construct_room_save(saved_player *sp)
{
   room *room_list;
   char *tmpstack;
   int length;
#ifdef ROOM_TRACK
   printf("construct_room_save(%s)\n", sp->lower_name);
#endif
   for(room_list=sp->rooms;room_list;room_list=room_list->next) {
     if (!room_list->players_top || (sys_flags&(SHUTDOWN|PANIC)))
       compress_room(room_list);
     tmpstack=stack;
     stack+=4;
     stack=store_string(stack,room_list->name);
     stack=store_string(stack,room_list->id);
     stack=store_string(stack,room_list->enter_msg);
     stack=store_int(stack,room_list->auto_base);
     stack=store_int(stack,room_list->flags);
     stack=store_int(stack,room_list->data_key);
     length=(int)stack-(int)tmpstack;
     (void) store_int(tmpstack,length);
   }
   stack=store_int(stack,0);
}

/* retrieve room data from a save file */

char *retrieve_room_data(saved_player *sp,char *where)
{
  int length;
  room *r,**last;
#ifdef ROOM_TRACK
  printf("retrieve_room_data(%s, %s)\n", sp->lower_name, where);
#endif
  free_room_data(sp);
  last=&sp->rooms;
  where=get_int(&length,where);
  for(;length;where=get_int(&length,where))
  {
    r=(room *)MALLOC(sizeof(room));
    memset(r,0,sizeof(room));
    where=get_string(r->name,where);
    where=get_string(r->id,where);
    where=get_string(r->enter_msg,where);
    where=get_int(&r->auto_base,where);
    where=get_int(&r->flags,where);
    where=get_int(&r->data_key,where);
    r->flags|=COMPRESSED;
    *last=r;
    last=&r->next;
    r->owner=sp;
    r->players_top=0;
    r->next=0;
  }
  *last=0;
  return where;
}
 
/* convert a room string into an actual room */

room *convert_room_verbose(player *p,char *str,int verbose)
{
  char *scan,*oldstack;
  saved_player *sp;
  room *r;
#ifdef ROOM_TRACK
  printf("convert_room_verbose(%s, %s, v:%d)\n",
	 p->lower_name, str, verbose);
#endif
  oldstack=stack;
  scan=str;
  while(*scan && *scan!='.')
    *stack++=*scan++;
  if (!*scan)
  {
    stack=oldstack;
    return 0;
  }
  *stack++=0;
  scan++;
  if (!*oldstack)
    strcpy(oldstack,p->name);
  lower_case(oldstack);
  sp=find_saved_player(oldstack);
  if (!sp)
  {
    stack=oldstack;
    return 0;
  }
  strcpy(oldstack,scan);
  lower_case(oldstack);
  stack=end_string(oldstack);
  r=sp->rooms;
  while(r)
  {
    strcpy(stack,r->id);
    lower_case(stack);
    if (!strcmp(stack,oldstack))
    {
      stack=oldstack;
      current_room=r;
      if (!decompress_room(r))
	return 0;
      return r;
    }
    if (!sp)
    {
      stack=oldstack;
      return 0;
    }
    strcpy(oldstack,scan);
    lower_case(oldstack);
    stack=end_string(oldstack);
    r=sp->rooms;
    while(r)
    {
      strcpy(stack,r->id);
      lower_case(stack);
      if (!strcmp(stack,oldstack))
      {
	stack=oldstack;
	current_room=r;
	if (!decompress_room(r))
	  return 0;
	return r;
      }
      r=r->next;
    }
    stack=oldstack;
    return 0;
  }
  return r;
}

room           *convert_room(player *p, char *str)
{
   return convert_room_verbose(p, str, 1);
}

/* load in the standard rooms and initialise */

char           *get_line(file * f)
{
   char           *start;
#ifdef ROOM_TRACK
   printf("get_line(%s)\n", f->where);
#endif
   while (*(f->where) == '#')
   {
      while ((f->length > 0) && *(f->where) != '\n')
      {
    f->where++;
    f->length--;
      }
      if (f->length == 0)
    return 0;
      f->where++;
      f->length--;
   }
   start = f->where;
   while (*(f->where) && *(f->where) != '\n' && *(f->where) != '\r')
   {
      f->where++;
      f->length--;
   }
   if (*(f->where))
   {
      *(f->where)++ = 0;
      f->length--;
   }
   if (*(f->where) == '\r' || *(f->where) == '\n')
   {
      *(f->where)++ = 0;
      f->length--;
   }
   return start;
}

void init_room(char *name, file rf)
{
   char *line, *oldstack, *file_start;
   saved_player *sp;
   room *r;
   player np;
#ifdef ROOM_TRACK
   printf("init_room(%s, %s)\n", name, rf.where);
#endif
   oldstack = stack;

   /* if (!malloc_verify()) printf("wrong HERE 1\n"); */

   file_start = rf.where;

   sp = find_saved_player(name);
   if (!sp)
   {
      memset(&np, 0, sizeof(player));
      np.fd = 0;
      np.location = (room *) - 1;
      restore_player(&np, name);
      np.residency = SYSTEM_ROOM;
      np.saved_residency = SYSTEM_ROOM;
      np.email[0] = -1;
      np.password[0] = -1;
      np.max_exits=30;
      np.max_rooms=30;
      np.max_autos=30;
      np.max_list=1;
      save_player(&np);
      sp = find_saved_player(name);
      sp->list_top = 0;
   }

   sp->residency=SYSTEM_ROOM;
   sp->rooms = 0;

   /* if (!malloc_verify()) printf("wrong HERE 2\n"); */

   while (rf.length > 0)
   {
      line = get_line(&rf);
      if (rf.length <= 0 || !(*line))
    break;
      r = (room *) MALLOC(sizeof(room));
      memset(r, 0, sizeof(room));
      r->owner = sp;
      r->players_top = 0;
      r->next = sp->rooms;
      sp->rooms = r;
      strncpy(r->id, line, MAX_ID - 2);
      line = get_line(&rf);
      strncpy(r->name, line, MAX_ROOM_NAME - 2);
      line = get_line(&rf);
      strncpy(r->enter_msg, line, MAX_ENTER_MSG - 2);
      r->flags = OPEN | ROOM_UPDATED;

      while (*(rf.where) != '#')
    if (*(rf.where) != '\r')
    {
       *stack++ = *rf.where++;
       rf.length--;
    } else
    {
       rf.where++;
       rf.length--;
    }
      *stack++ = 0;
      r->text.length = strlen(oldstack) + 1;
      r->text.where = (char *) MALLOC(r->text.length);
      strcpy(r->text.where, oldstack);
      stack = oldstack;
      line = get_line(&rf);
      while (strcasecmp(line, "end"))
      {
    while (*line)
       *stack++ = *line++;
    *stack++ = '\n';
    line = get_line(&rf);
      }
      *stack++ = 0;
      r->exits.length = strlen(oldstack) + 1;
      if (r->exits.length == 1)
      {
    r->exits.where = 0;
    r->exits.length = 0;
      } else
      {
    r->exits.where = (char *) MALLOC(r->exits.length);
    strcpy(r->exits.where, oldstack);
      }
      stack = oldstack;

      line = get_line(&rf);
      while (strcasecmp(line, "end"))
      {
    while (*line)
       *stack++ = *line++;
    *stack++ = '\n';
    line = get_line(&rf);
      }
      *stack++ = 0;
      r->automessage.length = strlen(oldstack) + 1;
      if (r->automessage.length == 1)
      {
    r->automessage.where = 0;
    r->automessage.length = 0;
      } else
      {
    r->automessage.where = (char *) MALLOC(r->automessage.length);
    strcpy(r->automessage.where, oldstack);
    r->flags |= AUTO_MESSAGE;
      }
      stack = oldstack;
   }
   
   /* if (!malloc_verify()) printf("wrong HERE 3\n"); */
   FREE(file_start);
   stack = oldstack;
}

void init_rooms(void)
{
  room_df=dynamic_init("rooms", 256);
  init_your_rooms();
}
