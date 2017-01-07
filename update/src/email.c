/*
 * email.c
 */

#include <string.h>
#include "fix.h"
#include "config.h"
#include "player.h"

/* Email types:
 * dartie@dds.nl                 - use dds     1
 * wzd3ark@cardiff.ac.uk         - use cardiff 1
 * dwilson@cs.strath.ac.uk       - use strath  2
 * styka2b@stn1.nott.agric.ac.uk - use nott    2
 * max email length 60
 */

/* This checks to see if the format of the players
 * email address is a valid one
 * returns 1(true) if valid format, 0(false) otherwise.
 */

int valid_email(char *email)
{
  char *p;
  int x=0, email_type, dots=0;
  
  p = email;
  if (!*email)
    return 0;
  while( (*p != '@') && (x<MAX_EMAIL-2) && (*p) )
  {
    p++;
    x++;
  }
  if (*p != '@')
    return 0;
  p++;
  if (*p == ' ' || *p == '.' || *p == '@')
    return 0;
  while (x<MAX_EMAIL-2)
  {
    x++;
    while ( (*p != '.') && (x<MAX_EMAIL-2) )
    {
      if (!*p)
	if (!(dots))
	  return 0;
	else
	  return 1;
      p++;
      x++;
    }
    dots++;
    p++;
    if ((*p == ' ') ||
	(*p == '.') ||
	(!*p) ||
	(x == MAX_EMAIL-2))
      return 0;
    if (!*p)
      return 1;
  }
}
    

	


    
	
      
