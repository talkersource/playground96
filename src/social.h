/*
 * social.h
 */


int	social_room(player * p, char *str, int try_room, char *msg) {

	player *p2;
	char *oldstack;

	p2 = find_player_global(str);
	if (!p2) return 0;
	/* add nasty remarks later */
	if (p==p2) {
		TELLPLAYER(p, " You spoon -- thats YOU!\n");
		return 0;
	}
	
	oldstack = stack;
	if (try_room && p2->location && p->location 
		&& p2->location == p->location) /* same room */
	{
		sprintf(stack, "%s | %s", msg, p2->name);
		stack = end_string(stack);
		send_to_room(p, oldstack, 0, 1);
	} else { 
		sprintf(stack, "%s %s", p2->name, msg);
		stack = end_string(stack);		
		remote_cmd(p, oldstack, 0);
	}
	stack = oldstack;
	return 1;
}

/* type COMPOUND: Can be done to a room or to a person */
void social_smile(player * p, char *str) {
	
	if (!*str) {
		send_to_room(p, "smiles happily", 0, 1);
		TELLPLAYER(p, " You smile happily\n");
		return;
		}
		
	if (social_room(p,str,1,"smiles happily at you.")) {
		TELLPLAYER(p, " You smile broadly at %s\n", str); 
	}
}

/* type ROOM: Can't be done to just a person */
void social_afk(player * p, char *str) {
	
		send_to_room(p, "goes afk.", 0, 1);
		TELLPLAYER(p, " You go afk.\n");
		return;
}

/* type PRIVATE: can't be done to a room */
void social_snog(player * p, char *str) {
	
	if (!*str) {
		TELLPLAYER(p, " Yes, but WHO do you want to snog?\n");
		return;
	}
	if (social_room(p, str, 0, "snogs you, leaving you breathless.")) 
		TELLPLAYER (p, " You snog %s =)\n", str);
}
