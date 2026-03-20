#include <stdio.h>
#include <setjmp.h>
#include <netinet/in.h>
#include <netdb.h>
#include <time.h>

#include "pflags.h"
#include "kernel.h"

#include "sflags.h"

#include "uaf.h"

#include "mobile.h"
#include "flags.h"
#include "mud.h"
#include "wizlist.h"
#include "bootstrap.h"


#include "global.h"

/* Core runtime */
jmp_buf to_main_loop;
FILE *log_fl;

/* World / game data */
Location   *room_data;
Object     *objects;
UBLOCK_REC *ublock;

ZONE  *zoname;
char  **messages;
char  **verbtxt;
char  *txt1;
char  *txt2;
int   levels[LVL_WIZARD + 1];

char  strbuf[MAX_COM_LEN];
char  wordbuf[MAX_COM_LEN];
char  item1[MAX_COM_LEN];
char  item2[MAX_COM_LEN];

/* System / environment */
char *progname;
char my_hostname[MAXHOSTNAMELEN];
struct hostent *my_hostent;
struct sockaddr_in s_in;
char **envp;
int pid;

/* Players / world */
PLAYER_REC *players;
WORLD_REC the_world_rec;
WORLD_REC *the_world = &the_world_rec;

/* Player state */
int mynum;
int real_mynum;
int quit_list;
PLAYER_REC *cur_player;
UBLOCK_REC *cur_ublock;

/* Limits / counts */
int max_players = 32;
int num_const_chars;
int numchars;
int char_array_len;

int num_const_obs;
int numobs;
int obj_array_len;

int numzon;
int num_const_zon;
int zon_array_len;

int num_const_locs;
int numloc;
int loc_array_len;

/* IDs */
long int id_counter;
int_table id_table;

/* Parsing / command state */
int *verbnum;
int ob1;
int ob2;
int pl1;
int pl2;
int prep;

int stp;
int verbcode;

/* Timing */
time_t last_reset;
time_t global_clock;


char *data_dir = NULL;
