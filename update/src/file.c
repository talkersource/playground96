/*
 * file.c
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <memory.h>

#include "file.h"


/* Local functions */
void process_option(char *option, char *param);

/* Local variables */
int rcline_no = 0, first_space;

/* Extern variables */
extern rc_type *rc_options;

/*
 * Routine to remove white space preceeding
 * some text in a string
 */
char *remove_wspace(char *str)
{
  while (*str && *str == ' ')
    str++;
  return str;
}

/* routine to get the next part of an arg */
char           *next_space(char *str)
{
   while (*str && *str != ' ')
      str++;
   if (*str == ' ')
   {
      while (*str == ' ')
    str++;
      str--;
   }
   return str;
}

void strip(char *str)
{
  while(*str && *str != ' ')
    str++;
  if (*str == ' ') 
    *str = '\0';
}

void nstrip(char *str)
{
  char *t;

  t = str;
  if (*str == ' ')
  {
    while (*str == ' ')
      str++;
    first_space = 1;
    return;
  }
  while(*t && *t != '\n')
    t++;
  if (*t == '\n') 
    *t = '\0';
}  

/* Takes in a line of lowercase text and
 * matches against a list of possible
 * options and associated parameters
 */
void process_line(char *str)
{
  char *option, *parameter;

  parameter = next_space(str);
  *parameter++ = 0;
  remove_wspace(parameter);
  option = str;
  strip(str);
  /* Allow #'s for commenting
   * blanks lines or lines starting with a space are ignored
   */
  if ( (*str && *str == '#') || (!*str) )
  {
    rcline_no++;
    return;
  }
  tolower(*option);
  /* Check to make sure it's a valid option */
  process_option(option, parameter);
}

/* The main function for this file */
void init_updaterc(char *filename)
{
  FILE *rcfile;
  int error, fd;
  char *curr_line;

  curr_line = (char *) malloc(100);
  memset(curr_line, 0, 100);
  
  if ((rcfile = fopen(filename, "r")) == NULL)
  {
    printf("Error - cannot open file '%s'\n", filename);
    printf(" Player file update aborted ..... BYE :)\n");
    exit(1);
  }
  printf("Reading in updaterc file ......\n");
  while ((fgets(curr_line, 100, rcfile)) != NULL)
  {
    first_space = 0;
    rcline_no ++;
    nstrip(curr_line);
    process_line(curr_line);
    memset(curr_line, 0, 100);
  }
  fclose(rcfile);
}
 
void no_param(char *option)
{
  printf("Error in updaterc (Line %d)\n", rcline_no);
  printf("No path specified for %s\n", option);
  return;
}

/* This handles errors for bad option
 * paramaters and gives a suitable message
 */

void found_error(char *str)
{
  printf("Error in updaterc (Line %d)\n", rcline_no);
  printf("Illegal parameter '%s'\n", str);
  return;
}

/* This routine checks for a valid option
 * and sets the neccessary variable in the
 * rc_options struct.
 */

void process_option(char *option, char *param)
{

  if (!(strcmp(option, "system_rooms_path:")))
  {
    if (!*param)
      no_param(option);
    else
    {
      strncpy(rc_options->srooms_path, param, 255);
    }
    return;
  }else
  if (!(strcmp(option, "player_files_path:")))
  {
    if (!*param)
      no_param(option);
    else
    {
      strncpy(rc_options->pfile_path, param, 255);
    }
    return;
  } else
  if (!(strcmp(option, "new_player_files_path:")))
  {
    if (!*param)
      no_param(option);
    else
    {
      strncpy(rc_options->new_pfile_path, param, 255);
    }
    return;
  }else
  if (!(strcmp(option, "player_rooms_path:")))
  {
    if (!*param)
      no_param(option);
    else
    {
      strncpy(rc_options->prooms_path, param, 255);
    }
    return;
  }else
  {
    printf("Error in updaterc (Line %d)\n", rcline_no);
    printf("'%s' not a valid option\n", option);
    exit(1);
  }
}









