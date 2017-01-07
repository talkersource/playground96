/* 
 * admin.h 
 */

/* externs */
extern void         swap_list_names(char *, char *);
extern void         lower_case(char *);
extern char         *do_crypt(char *, player *);
extern char         *end_string(), *next_space(), *tag_string(), *bit_string();
extern player       *find_player_global(), *find_player_global_quiet(char *),
                    *create_player();
extern saved_player *find_saved_player();
extern int          remove_player_file(), set_update();
extern int          get_flag();
extern void         hard_load_one_file(), sync_to_file(), remove_entire_list(),
                    destroy_player(), TELLPLAYER(), LOGF(), SUWALL(), SWBUT(),
                    TELLROOM();
extern player       *find_player_absolute_quiet(char *);
extern file         newban_msg, version_msg, nonewbies_msg, connect_msg,
                    connect2_msg, fingerpaint_msg, motd_msg, spodlist_msg,
                    hitells_msg, connect3_msg, banned_msg, banish_file,
                    banish_msg, full_msg, newbie_msg, newpage1_msg,
                    newpage2_msg, disclaimer_msg, splat_msg, sumotd_msg,
                    load_file(char *), load_file_verbose(char *, int);
extern int          match_banish();
extern void         soft_eject(player *, char *);
extern player       *find_player_absolute_quiet(char *);
extern char         *self_string(player *p), *first_char(player *);
extern void         all_players_out(saved_player *),
                    begin_ressie(player *, char *), newsetpw0(player *, char *);
extern note         *find_note(int);
extern char          shutdown_reason[];
extern time_t        shutdown_count;
extern room         *comfy;
extern list_ent     *fle_from_save();
#ifdef TRACK
extern int addfunction(char *);
#endif

/* interns */
const char *HCAdminList[] = { "admin", "coder", "sysop"};
const int NUM_ADMINS = 3;  /* this MUST be the same as the # of names
                               in the *char[] above */

flag_list       permission_list[] = {
   {"residency", BASE | BUILD | LIST | ECHO_PRIV | MAIL | SESSION },
   {"nosync", NO_SYNC},
   {"base", BASE},
   {"echo", ECHO_PRIV},
   {"no_timeout", NO_TIMEOUT},
   {"banished", BANISHD},
   {"sysroom", SYSTEM_ROOM},
   {"mail", MAIL},
   {"list", LIST},
   {"git", GIT},
   {"condom", PROTECT},
   {"spod", SPOD},
   {"married", MARRIED},
   {"build", BUILD},
   {"session", SESSION},
   {"psu", PSU | REGULAR_STYLE_CHAN},
   {"warn", WARN},
   {"dumb", DUMB},
   {"script", SCRIPT},
   {"admin_channel", TESTCHAR},
   {"trace", TRACE},
   {"house", HOUSE},
   {"spoon", TESTCHAR},
   {"hcadmin", HCADMIN|CODER},
   {"lower_admin", LOWER_ADMIN},
   {"trainee", SU | PSU | WARN | DUMB |REGULAR_STYLE_CHAN},
   {"su", SU | ASU | PSU | WARN | DUMB |REGULAR_STYLE_CHAN},
   {"admin", ADMIN|LOWER_ADMIN|REGULAR_STYLE_CHAN|ASU|SU|PSU|WARN|DUMB},
   {"forest", FOREST_STYLE_CHAN},
   {"reg_chan", REGULAR_STYLE_CHAN},
{0, 0}};

int count_su(), check_privs();

/* and, just some annoying strings definitions... */
#define WHO_IS_THAT        " No such person in saved files...\n"
#define NOT_HERE_ATM       " That person isn't on right now...\n"

#define SNEEZED_ROOM	   " -=*> %s %s sneezed upon...gross !!\n"
#define YOU_BEEN_DRAGGED "\n -=*> You are dragged forcibly from the keyboard" \
			 " by a large burly beast -- maybe you should behave" \
			 " better when you logon here......\n\n"
#define DRAG_ROOM        " -=*> %s screams as his mommy grabs him away by his ear!\n"
#define YOU_BEEN_RMSHOUT " -=*> You suddenly discover that you have a sore throat\n"
#define YOU_BEEN_RMSING  " -=*> You notice yourself horridly out of tune and get stage fright!\n"
#define YOU_BEEN_UNRMD   " -=*> A kind soul hands you some throat drops.\n" 
#define YOU_BEEN_UNRSING " -=*> Someone pays for your singing lessons, and your confidence returns.\n"
#define WELCOME_TO_PG "\n" \
" --------------------------------------------------------------------------\n" \
" | You have been made a resident!                                         |\n" \
" |------------------------------------------------------------------------|\n" \
" | Please answer a few short questions so that we can save your character |\n" \
" --------------------------------------------------------------------------\n" 
#define NUKE_SCREEN "\n" \
"  BYEBYE!                                  LATERS!\n" \
"\n" \
"                   uuuuuuu\n" \
"               uu$$$$$$$$$$$uu\n" \
"            uu$$$$$$$$$$$$$$$$$uu\n" \
"           u$$$$$$$$$$$$$$$$$$$$$u\n" \
"          u$$$$$$$$$$$$$$$$$$$$$$$u\n" \
"         u$$$$$$$$$$$$$$$$$$$$$$$$$u\n" \
"         u$$$$$$$$$$$$$$$$$$$$$$$$$u           And don't come back!\n" \
"         u$$$$$$\"   \"$$$\"   \"$$$$$$u\n" \
"         \"$$$$\"      u$u       $$$$\"\n" \
"          $$$u       u$u       u$$$\n" \
"          $$$u      u$$$u      u$$$\n" \
"           \"$$$$uu$$$   $$$uu$$$$\"            You have just been nuked!\n" \
"            \"$$$$$$$\"   \"$$$$$$$\"\n" \
"              u$$$$$$$u$$$$$$$u\n" \
"               u$\"$\"$\"$\"$\"$\"$u\n" \
"    uuu        $$u$ $ $ $ $u$$       uuu\n" \
"   u$$$$        $$$$$u$u$u$$$       u$$$$\n" \
"    $$$$$uu      \"$$$$$$$$$\"     uu$$$$$$        Better luck next life\n" \
"  u$$$$$$$$$$$uu    \"\"\"\"\"    uuuu$$$$$$$$$$\n" \
"  $$$$\"\"\"$$$$$$$$$$uuu   uu$$$$$$$$$\"\"\"$$$\"\n" \
"   \"\"\"      \"\"$$$$$$$$$$$uu \"\"$\"\"\"\n" \
"             uuuu \"\"$$$$$$$$$$uuu\n" \
"    u$$$uuu$$$$$$$$$uu \"\"$$$$$$$$$$$uuu$$$\n" \
"    $$$$$$$$$$\"\"\"\"           \"\"$$$$$$$$$$$\"\n" \
"     \"$$$$$\"                      \"\"$$$$\"\"\n" \
"       $$$\"                         $$$$\"\n\n\n" 
 
#define YOU_BEEN_SNEEZED  "\n" \
 "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n" \
 "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM  MM''MMMMM\n" \
 "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM  M'  MMMMM\n" \
 "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM!\" ''' \"!MMMMMMMMMMMMMMMMMMMMM  M  MM\"'MM\n" \
 "MMMMMM'\"M' MMMMMMMMMMMMMMMV'                '\"MMMMMMMMMM.  'MM  M  M' .MM\n" \
 "MMMM'M :M ;MV MMMMMMMMMM'                      \"MMMMMMMMMM.  \": M .! .MMM\n" \
 "MMM; M  M :M' AMMMMMMV'                          \"MMMMMMMMM.  .'''. AMMMM\n" \
 "MMMM ;  ; M:  MMMMMM'                              'MMMMMMM'        MMMMM\n" \
 "MMMM. ; . M  AMMMMV       @@@@@         @@@@@       'MMMM\"\" ' '.   .MMMMM\n" \
 "MMMMM  .    MMMMM        @@ _ @@_______@@ _ @@        '. ..    ;  .MMMMMM\n" \
 "MMMM  '\"@\"  MMMM         @@ ~ @@       @@ ~ @@         MMMMMM.'   MMMMMMM\n" \
 "MMMM.  ;                  @@@@@         @@@@@           MMMMMM.   MMMMMMM\n" \
 "MMMMM..'.   .MM'                                        MMMMMMM    VMMMMM\n" \
 "MMMMMM  AMMMMMV                                         'MMMMMM.    MMMMM\n" \
 "MMMMM'  MMMMMM:                                     ..   MMMMMMM     MMMM\n" \
 "MMMMM   MMMMMM: @@.                              .'  @@  : 'MMMM.     MMM\n" \
 "MMMM'   MMM''': :@: '.                         .' ..@@@  :            .MM\n" \
 "MMMV   ''     : '@@@@: '.                    .'  .@@@@@  ........./MMMMMM\n" \
 "MMM           :  @@@@@. .' .              .' A. .@@@@@'  MMMMMMMMMMMMMMMM\n" \
 "MMM:..........:  '@@@@@@@.  ! '. - - - . '.  .@@@@@@@@   MMMMMMMMMMMMMMMM\n" \
 "MMMMMMMMMMMMMMM   '@@@@@@@@@@@@.    !    .@@@@@@@@@@@@'  MMMMMMMMMMMMMMMM\n" \
 "MMMMMMMMMMMMMMMM   '@@@@@@@@@@@@@...@@'..@@@@@@@@@@@@'  AMMMMMMMMMMMMMMMM\n" \
 "MMMMMMMMMMMMMMMMA   '@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@   .MMMMMMMMMMMMMMMMM\n" \
 "MMMMMMMMMMMMMMMMMA    @@@@@@@@@@@@\"'O'OOOO'@@@@@@@'   .MMMMMMMMMMMMMMMMMM\n" \
 "MMMMMMMMMMMMMMMMMMA    '@@@@@@@VOOOOOOO.OOO @@@V     AMMMMMMMMMMMMMMMMMMM\n" \
 "MMMMMMMMMMMMMMMMMMMMA     '@@@@OOOOOOOOO.OOO@'     .MMMMMMMMMMMMMMMMMMMMM\n" \
 "MMMMMMMMMMMMMMMMMMMMMMA        '.@@.OOOOO.OO     .AMMMMMMMMMMMMMMMMMMMMMM\n" \
 "MMMMMMMMMMMMMMMMMMMMMMMMA.          OOOOO OOOO .MMMMMMMMMMMMMMMMMMMMMMMMM\n" \
 "MMMMMMMMMMMMMMMMMMMMMMMMMMMA..      OOOOOOOOOOOMMMMMMMMMMMMMMMMMMMMMMMMMM\n" \
 "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMOOOOOOOOOO.MMMMMMMMMMMMMMMMMMMMMMMMMM\n" \
 "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM OOOOOOO.MMMMMMMMMMMMMMMMMMMMMMMMMMMM\n" \
 "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM---MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n" \
 "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n" \
 "M  You been kicked out.  Learn to behave, before you think of returning M\n" \
 "MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\n\n" 
