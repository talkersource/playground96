/*
    dynamic.c   dynamic files (in theory)
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <unistd.h>
#include <fcntl.h>

/* just in case youre on a dippy system */

#ifndef S_IRUSR
 #define S_IRUSR 00400
#endif
#ifndef S_IWUSR
 #define S_IWUSR 00200
#endif


#include "config.h"
#include "player.h"
#include "dynamic.h"

/* External functions */

extern char *end_string(char *);
extern char *store_int(char *,int);
extern char *get_int(int *,char *);
extern dfile *room_df;
extern saved_player **saved_hash[];

/* throw the keylist to disk */

void dynamic_key_sync(dfile *df)
{
  int fd,length;
  char *oldstack,*to;
  oldstack=stack;

  store_int((char *)df->keylist,df->first_free_block);
  store_int((char *)(df->keylist+1),df->first_free_key);

/* doing a straight open could mean losing the key data
   dont risk it by moving the file first
   this is grossly slow, so provide a means of escaping it */

  sprintf(oldstack,"files/%s/keys",df->fname);
  if (!(sys_flags&SECURE_DYNAMIC)) {
    stack=end_string(oldstack);
    to=stack;
    sprintf(to,"files/%s/keys.b",df->fname);
    rename(oldstack,to);
  }
  fd=open(oldstack,O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR);
  if (fd<0) handle_error("Failed to open key file");
  length=(df->nkeys+1)*8;
  if (write(fd,df->keylist,length)!=length)
    handle_error("Failed to write key data");
  close(fd);    
  
  stack=oldstack;  
}


/* set up a dfile structure */

dfile *dynamic_init(char *file,int granularity)
{
  dfile *df;
  char *oldstack;
  int fd,length,i,*fill;
  oldstack=stack;

  if (sys_flags&VERBOSE) {
    sprintf(oldstack,"Loading dynamic file '%s'",file);
    stack=end_string(oldstack);
    log("sync",oldstack);
    stack=oldstack;
  }
  
  df=(dfile *)MALLOC(sizeof(dfile));
  memset(df,0,sizeof(dfile));
  df->granularity=granularity;
  sprintf(oldstack,"%skeys",rc_options->prooms_path);
  fd=open(oldstack,O_RDONLY|O_NDELAY);
  /* just in case this is the first time round */
  
  if (fd<0) {
    printf("in if fd<0\n");
    sprintf(oldstack,"Failed to load room keys");
    stack=end_string(oldstack);
    log("error",oldstack);
    stack=oldstack;
    df->first_free_block=0;
    df->first_free_key=0;
    df->nkeys=0;
    df->keylist=0;
  }
  else {
    length=lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);
    if ((length%8)!=0) handle_error("Corrupt key data");
    df->nkeys=(length/8)-1;
    df->keylist=(int *)MALLOC(length);
    if (read(fd,df->keylist,length)!=length)
      handle_error("Failed to read keys");
    close(fd);
    (void)get_int(&(df->first_free_block),(char *)df->keylist);
    (void)get_int(&(df->first_free_key),(char *)(df->keylist+1));
  }
  /* keep the data file open all the time */
  
  sprintf(oldstack,"%sdata",rc_options->prooms_path);
  df->data_fd=open(oldstack,O_CREAT|O_RDWR,S_IRUSR|S_IWUSR);
  if (df->data_fd<0) handle_error("Failed to open dynamic data file");
  lseek(df->data_fd,0,SEEK_SET);
  stack=oldstack;
  return df; 
}

/* free up a dfile structure */

void dynamic_close(dfile *df)
{
  dynamic_key_sync(df);
  close(df->data_fd);
  free(df->keylist);
  free(df);
}

/* get the block and length info from the keylist */

int convert_key(dfile *df,int key,int *block,int *length)
{
  if ((key<=0) || (key>(df->nkeys-1))) return 0;
  key*=2;
  (void) get_int(block,(char *)(df->keylist+key));
  if (*block<=0) return 0;
  (void) get_int(length,(char *)(df->keylist+key+1));
  if (*length<=0) return 0;
  return 1;
}

/* grab a block from the data file */

int load_block(dfile *df,int *block,char *data)
{
  if (*block<=0) return 0;
  dynamic_seek_block(df,*block);
  if (read(df->data_fd,data,4)!=4) handle_error("Failed to read next block 1");
  get_int(block,data);
  if (read(df->data_fd,data,df->granularity-4)!=(df->granularity-4))
    handle_error("Failed to read block");
  return 1;
}

/* load an entire entry from the data file */

int dynamic_load(dfile *df,int key,char *data)
{
  int block,length,l;
/*  printf("Dynamic load key=%d\n",key);  */
  if (!convert_key(df,key,&block,&length)) return -1;
  l=length;
/*    printf("DL block %d\n",block);  */
  while(l>0) {
    if (!load_block(df,&block,data))
      handle_error("Failed to load block of data");
/*    printf("DL block %d\n",block);  */
    l-=(df->granularity-4);
    data+=(df->granularity-4);
  }
  return length;
}

/* returns a free key, if necessary it will go and create
   some new key space (in chunks of 100 keys at a time) */

int dynamic_find_free_key(dfile *df)
{
  int *newlist,oldnkeys,i,key;
  if (!df->first_free_key) {
    oldnkeys=df->nkeys;
    df->nkeys+=100;
    newlist=(int *)MALLOC((df->nkeys+1)*8);
    if (df->keylist) {
      memcpy(newlist+2,df->keylist+2,(oldnkeys+1)*8);
      FREE(df->keylist);
    }
    df->keylist=newlist;
    df->first_free_key=oldnkeys+1;
    newlist+=df->first_free_key*2;
    for(i=-df->first_free_key-1;i>=-df->nkeys;i--) {
      newlist=(int *)store_int((char *)newlist,i);
      newlist=(int *)store_int((char *)newlist,0);
    }
    newlist=(int *)store_int((char *)newlist,0);
    newlist=(int *)store_int((char *)newlist,0);
  }
  key=df->first_free_key;
  (void) get_int(&(df->first_free_key),(char *)(df->keylist+key*2));
  df->first_free_key=-df->first_free_key;
  return key;
}

/* read in the index from the start of a block */

int dynamic_get_next_block(dfile *df,int block)
{
  int next_block,ret;
  if (block<=0) return;
  dynamic_seek_block(df,block);
  ret=read(df->data_fd,stack,4);
  if (!ret) return 0;
  if (ret!=4) handle_error("Failed to read next block 2");
  get_int(&next_block,stack);
  return next_block;
}

/* go through all the blocks in a file and free them up */

void dynamic_free(dfile *df,int key)
{
  char *oldstack;
  int block,length,next_block;

  if (!convert_key(df,key,&block,&length))
     return;
  (void) store_int((char *)(df->keylist+key*2),-df->first_free_key);
  (void) store_int((char *)(df->keylist+key*2+1),0);
  df->first_free_key=key;

/* simply keep adding blocks onto the top of the free list */
  
   while(length>0)
   {
      if (block<=0)
         return;
      next_block = dynamic_get_next_block(df,block);
      dynamic_seek_block(df,block);
      store_int(stack,-df->first_free_block);
      if (write(df->data_fd,stack,4)!=4)
         handle_error("Failed to write next block");
      df->first_free_block=block;
      block = next_block;
      length -= (df->granularity-4);
   }
   if (!(sys_flags&SECURE_DYNAMIC))
      dynamic_key_sync(df);
}

/* go hunting for a free block, if necessary, create enough
   space on the end of the file */

int dynamic_find_free_block(dfile *df)
{
  int length,new_block;
/*  printf("Called find_free_block ffb=%d\n",df->first_free_block); */
  if (df->first_free_block) {
    new_block=df->first_free_block;
    df->first_free_block=-dynamic_get_next_block(df,new_block);
  }
  else {
    length=lseek(df->data_fd,0,SEEK_END);
    memset(stack,0,df->granularity);
    if (!length) {
      if (write(df->data_fd,stack,df->granularity)!=df->granularity)
        handle_error("Failed to make data block");
      length=df->granularity;
    }
    if (write(df->data_fd,stack,df->granularity)!=df->granularity)
      handle_error("Failed to make data block");
    new_block=(length/df->granularity);
  }
  return new_block;
}

/* save an entry to the data file */

int dynamic_save(dfile *df,char *data,int l,int key)
{
  int block,length,next_block,blength,free_block;

/* key==0 means this is a new entry */
  
  if (!key) {
    key=dynamic_find_free_key(df);
    block=dynamic_find_free_block(df);
  }
  else if (!convert_key(df,key,&block,&length)) return;
  (void) store_int((char *)(df->keylist+key*2),block);
  (void) store_int((char *)(df->keylist+key*2+1),l);  
  blength=df->granularity-4;
/*  printf("Dynamic Save key=%d\n",key); */
  while(l>0) {
    next_block=dynamic_get_next_block(df,block);
/*    printf("DS block %d / next block %d\n",block,next_block);  */

/* do we need more space ? */

    if (l>blength) {
      if (next_block<=0) next_block=dynamic_find_free_block(df);
    }
    else {

/* can we free up any blocks at the end ? */
    
      free_block=next_block;
      while(free_block>0) {
        next_block=dynamic_get_next_block(df,free_block);
        dynamic_seek_block(df,free_block);
        store_int(stack,-df->first_free_block);
        if (write(df->data_fd,stack,4)!=4)
          handle_error("Failed to write next block");
        df->first_free_block=free_block;
        free_block=next_block;
      }
      next_block=0;
    }
    store_int(stack,next_block);
    dynamic_seek_block(df,block);
    if (write(df->data_fd,stack,4)!=4)
      handle_error("Failed to write next block");
    if (write(df->data_fd,data,blength)!=blength)
      handle_error("Failed to write block data");
    block=next_block;
    l-=blength;
    data+=blength;
  }

  /* make sure the keylist is up to date */
  
  if (!(sys_flags&SECURE_DYNAMIC)) dynamic_key_sync(df);
  return key;
}
