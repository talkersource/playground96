/*
 * validate.c
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

#include "fix.h"
#include "file.h"
#include "player.h"

/* External functions */

/* Prototypes for func's in this file */

int pfiles_to_process(void);
int test_write_directory(char *);
int test_read_file(char *);
void do_tests(void);

int test_write_directory(char *dname)
{
  int fd, length;
  char *oldstack;
   
  oldstack = stack;
  sprintf(stack, "%s.open_test", dname);
  fd = open(oldstack, O_CREAT | O_WRONLY | O_SYNC | O_TRUNC, S_IRUSR | S_IWUSR);
  if (fd < 0)
    return 0;
  if (write(fd, oldstack, 1) < 0)
    return 0;
  close(fd);
  stack = oldstack;
  return 1;
}

int test_read_file(char *file)
{
  int fd, opened = 1;

  fd = open(file, O_RDONLY | O_NDELAY);
  if (fd <0)
    opened = 0;
  close(fd);
  return(opened);
}

int pfiles_to_process()
{
  char c;
  int fd, opened = 0;
  char *oldstack;
  
  for (c = 'a'; c<= 'z'; c++)
  {
    oldstack = stack;
    sprintf(stack, "%s%c", rc_options->pfile_path, c);
    fd = open(oldstack, O_RDONLY | O_NDELAY);
    if (fd >= 0)
      opened++;
    close(fd);
    stack = oldstack;
  }
  return(opened);
}

void do_tests()
{
  int x;
  char *oldstack;
  
  printf(" - Update will now check files and directories -\n");
  /* First check to see if any player files exist */
  printf(" Checking player files exist ....");
  if (!(x = pfiles_to_process()))
  {
    printf(" No.\n");
    printf("  There are no player files to process, aborting update!\n"
	   "  Bye bye, please call again when you have some player files for me :)\n");
    exit(0);
  }else
    printf(" Yes - %d file/(s) found to update\n", x);
  /* Check we can write out to the new_pfile directory */
  printf(" Checking update can write to: \n [%s] ....", rc_options->new_pfile_path);
  if (!(test_write_directory(rc_options->new_pfile_path)))
  {
    printf(" No.\n");
    printf("  Check to make sure directory exists and that write permissions have been set :)\n");
    exit(0);
  }else
    printf(" Yes.\n");
  /* Check we can read the room data needed */
  printf(" Checking files (data & keys) exist ....\n");
  printf("  data ....");
  oldstack = stack;
  sprintf(stack, "%sdata",rc_options->prooms_path);
  if (!(test_read_file(oldstack)))
  {
    printf(" No.\n");
    printf("  Failed to find %sdata - Bye !!\n", rc_options->prooms_path);
    exit(0);
  }else
    printf(" Yes.\n");
  stack = oldstack;
  printf("  keys ....");
  sprintf(stack, "%skeys",rc_options->prooms_path);
  if (!(test_read_file(oldstack)))
  {
    printf(" No.\n");
    printf("  Failed to find %skeys - Bye !!\n", rc_options->prooms_path);
    exit(0);
  }else
    printf(" Yes.\n");
  stack = oldstack;
}
