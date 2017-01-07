
/* Screen locking routines... spoon of password, basically... */

void            unlock_screen(player * p, char *str) {

	if(strcmp(p->slock_pw, str)) 
	{
		do_prompt(p, " Please enter CORRECT password to unlock screen:");
		p->input_to_fn = unlock_screen;
	}
	else
	{
		password_mode_off(p);
		p->input_to_fn = 0;
   		p->flags |= PROMPT;
      		strcpy(p->slock_pw, "");
		tell_player(p, " Screen is now unlocked.\n");
	}

}
 

void            confirm_slock2(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"got_password (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (strcmp(p->slock_pw, str))
   {
      password_mode_off(p);
      p->flags |= PROMPT;
      p->input_to_fn = 0;
      strcpy(p->slock_pw, "");
      tell_player(p, "\n But that doesn't match !!!\n"
                     " screen not locked ...\n");
   } else
   {
      do_prompt(p, "\n Screen is now locked - No one may type any commands"
		   " without unlocking it with the current password.\n"
		   " Please enter password to unlock screen:");
      p->input_to_fn = unlock_screen;
   }
}

void            confirm_slock1(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"got_password1 (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   if (strlen(str) > (MAX_PASSWORD - 2))
   {
      do_prompt(p, "\n Password too long, please try again.\n"
                   " Please enter a shorter password:");
      p->input_to_fn = confirm_slock1;
   } else
   {
      strcpy(p->slock_pw, str);
      do_prompt(p, "\n Enter password again to verify:");
      p->input_to_fn = confirm_slock2;
   }
}

void            set_screenlock(player * p, char *str)
{
#ifdef TRACK
   sprintf(functionin,"change_password (%s , SOMETHING)",p->name);
   addfunction(functionin);
#endif

   password_mode_on(p);
   p->flags &= ~PROMPT;
      do_prompt(p, " Entering screen lock mode:\n"
                   " Please enter a password:");
      p->input_to_fn = confirm_slock1;
}

