/*
 * file.h
 */

#ifndef FILE_H
 #define FILE_H


struct rc_struct
{
  char   srooms_path[256];
  char   prooms_path[256];
  char   notes_path[256];
  char   pfile_path[256];
  char   new_pfile_path[256];
};

typedef struct rc_struct rc_type;

#endif /* FILE_H */


