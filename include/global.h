extern jmp_buf to_main_loop;

extern FILE *log_fl;

extern Location   *room_data;
extern Object     *objects;
extern UBLOCK_REC *ublock;

extern ZONE	*zoname;
extern char	**messages;
extern char	**verbtxt;
extern char	*txt1;
extern char	*txt2;
extern int	levels[LVL_WIZARD + 1];
extern char	strbuf[MAX_COM_LEN];
extern char	wordbuf[MAX_COM_LEN];
extern char	item1[MAX_COM_LEN];
extern char	item2[MAX_COM_LEN];

extern char			*progname;
extern char			my_hostname[MAXHOSTNAMELEN];
extern struct hostent		*my_hostent;
extern struct sockaddr_in      s_in;
extern char			**envp;
extern int			pid;

extern PLAYER_REC      *players;
extern WORLD_REC       the_world_rec;
extern WORLD_REC	*the_world;

/************************
**
** mynum is now used to designate the index of the current player, i.e.
** the player who's message we are currently handling.
** the name should perhaps be 'cur_player' or something like that
** but 'mynum' is used in the old source and since we try to copy it over
** with minimum number of changes, we keep the name.
** new variables are cur_player which is set to &players[mynum] and
** cur_ublock which is set to &ublock[mynum].
**
*************************
*/
extern int	    mynum;          /* current player slot-number */
extern int         real_mynum;     /* real mynum if mynum is fake due to aliasing */
extern int         quit_list;      /* real mynum of player to quit */
extern PLAYER_REC *cur_player;     /* Current player info. */
extern UBLOCK_REC *cur_ublock;     /* Current ublock info. */


extern int	max_players;
extern int     num_const_chars;
extern int	numchars;       /* Number of players + mobiles */
extern int     char_array_len;

extern int	num_const_obs;  /* Number of constant (not created in-game) objects */
extern int	numobs;		/* Number of objects in the game */
extern int     obj_array_len;

extern int     numzon;		/* Number of zones in the world		*/
extern int     num_const_zon;
extern int     zon_array_len;

extern int     num_const_locs;
extern int	numloc;		/* Number of locations */
extern int     loc_array_len;

extern long int   id_counter;  /* Next ID number to be given to a wiz-creation */
extern int_table  id_table;    /* Lookup table for [ID numbers -> game indexes] */

extern int	*verbnum;
extern int	ob1;
extern int	ob2;
extern int	pl1;
extern int	pl2;
extern int	pptr;		/* The parameter pointer		*/
extern int	prep;

extern int	stp;
extern int	verbcode;
extern time_t   next_event;     /* check mud.c */
extern time_t   last_reset;     /* Last reset time */
extern time_t   global_clock;
