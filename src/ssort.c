
/* ssort.c -- version 2.1.3  */

/* Written by Michael "traP" Bourdaa on 10/11/95 for use in Playground 96 */
/* last update: 4/1/96 */
/* parts of this code are from EW-too by Simon Marsh */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "config.h"

#define MAX_SORT 100
#define TOP_LIST 50 
#define TALKER_NAME "Stupidland"

void load_raw_file();
void sort_users();
void output_file();

int cnt = 0;

typedef struct {
	char name[20];
	int login;
        int partic;
	} USER;

USER user[MAX_SORT];

main () {

	load_raw_file();
	sort_users();
	output_file();
}


/* copied directly from PlayCode */

char           *sys_time()
{
   time_t          t;
   static char     time_string[50];
   t = time(0);
   strftime(time_string, 50, "%I:%M:%S %p, %B %d, %Y", localtime(&t));
   return time_string;
}

void load_raw_file()
{
	char waste[100];
	/* get the two unneeded header lines */
	gets(waste);
	gets(waste);
	
	scanf("%d %d %s\n", &(user[cnt].login), &(user[cnt].partic), user[cnt].name); 	
	while (user[cnt].login != 0) {
		cnt++;
	        scanf("%d %d %s\n", &(user[cnt].login), &(user[cnt].partic), user[cnt].name); 	
		}
}

/* uses a bubble sort.. another sorting algorithm would be much faster */
/* because of this implementation, we have to be careful to not take in
   too big of a list of players on the users end. */

void swap(int a, int b)
{
	USER temp;

	strcpy(temp.name, user[a].name);
	strcpy(user[a].name, user[b].name);
	strcpy(user[b].name, temp.name);

	temp.login = user[a].login;
	user[a].login = user[b].login;
	user[b].login = temp.login;

	temp.partic = user[a].partic;
	user[a].partic = user[b].partic;
	user[b].partic = temp.partic;

	return;
}

void sort_users()
{
	int sort_again = 1;
	int i;

	while (sort_again) {

		sort_again = 0;
		for (i=0; i < (cnt + 1); i++) {

			if (user[i].login < user[i+1].login)  {
				swap(i, i+1);
				sort_again = 1;
				}

			} /* end for */
		}
}

/* copied straight from EW-too -- if it works, why rewrite? */

char           *word_time(int t)
{
   static char     time_string[100], *fill;
   int             days, hrs, mins, secs;
   if (!t)
      return "no time at all";
   days = t / 86400;
   hrs = (t / 3600) % 24;
   mins = (t / 60) % 60;
   secs = t % 60;
   fill = time_string;
   if (days)
   {
      sprintf(fill, "%d day", days);
      while (*fill)
    fill++;
      if (days != 1)
    *fill++ = 's';
      if (hrs || mins || secs)
      {
    *fill++ = ',';
    *fill++ = ' ';
      }
   }
   if (hrs)
   {
      sprintf(fill, "%d hour", hrs);
      while (*fill)
    fill++;
      if (hrs != 1)
    *fill++ = 's';
      if (mins && secs)
      {
    *fill++ = ',';
    *fill++ = ' ';
      }
      if ((mins && !secs) || (!mins && secs))
      {
    strcpy(fill, " and ");
    while (*fill)
       fill++;
      }
   }
   if (mins)
   {
      sprintf(fill, "%d min", mins);
      while (*fill)
    fill++;
      if (mins != 1)
    *fill++ = 's';
      if (secs)
      {
    strcpy(fill, " and ");
    while (*fill)
       fill++;
      }
   }
   if (secs)
   {
      sprintf(fill, "%d sec", secs);
      while (*fill)
    fill++;
      if (secs != 1)
    *fill++ = 's';
   }
   *fill++ = 0;
   return time_string;
}

char *bufit(int i) {

	if (user[i].partic >= 100)
		return "-- ";
	else
		return "--  ";
}

/* uses PG96 color codes to colorize the output file */
/* uses straight printf, so must use data redirection from the shell. */
void output_file() {

	int i; 

        printf("   -=> ^R%s^N ^Btop^N ^Y50^N ^Gspods^N ^P%s PST^N <=-\n", TALKER_NAME, sys_time()); 
	for (i=0; (i < cnt && i < TOP_LIST); i++) {

               if (i == 0)
		printf("^U^H %d. %-20s %s %d.%d -- %s^N\n", (i+1), user[i].name, bufit(i),user[i].partic/10, user[i].partic%10, word_time(user[i].login)); 
               else if (i == 1)
		printf("^U^R %d. %-20s %s %d.%d -- %s^N\n", (i+1), user[i].name, bufit(i),user[i].partic/10, user[i].partic%10, word_time(user[i].login)); 
               else if (i == 2)
		printf("^U^Y %d. %-20s %s %d.%d -- %s^N\n", (i+1), user[i].name, bufit(i),user[i].partic/10, user[i].partic%10, word_time(user[i].login)); 
               else if (i < 5)
		printf("^U^G %d. %-20s %s %d.%d -- %s^N\n", (i+1), user[i].name, bufit(i),user[i].partic/10, user[i].partic%10, word_time(user[i].login)); 
               else if (i < 9)
		printf("^U^B %d. %-20s %s %d.%d -- %s^N\n", (i+1), user[i].name, bufit(i),user[i].partic/10, user[i].partic%10, word_time(user[i].login)); 
               else if (i == 9)
		printf("^U^B%d. %-20s %s %d.%d -- %s^N\n", (i+1), user[i].name, bufit(i),user[i].partic/10, user[i].partic%10, word_time(user[i].login)); 
               else if (i < 25)
		printf("^U^p%d. %-20s %s %d.%d -- %s^N\n", (i+1), user[i].name, bufit(i),user[i].partic/10, user[i].partic%10, word_time(user[i].login)); 
               else 
		printf("^U^y%d. %-20s %s %d.%d -- %s^N\n", (i+1), user[i].name, bufit(i),user[i].partic/10, user[i].partic%10, word_time(user[i].login)); 

		}
}

