#ifndef SINS_H
#define SINS_H

#include <config.h>
#include <stdio.h>
#include <snake_compat.h>
#include <snake_limits.h>


/* version string */
#define VERSION "0.22"


/* delay range (microseconds) */
#define MINDELAY 0
#define MAXDELAY 2000000 /* two seconds */
#define DEFAULTDELAY 50000
#define DELAYSTEP 10000

/* Direction type */
typedef char DIRECTION;

/*
 *
 * Opposite directions MUST BE both odd or even
 */
#define LEFT	0		/* left direction */
#define UP  	1		/* up direction */
#define RIGHT	2		/* right direction */
#define DOWN	3		/* down direction */

#define IS_OPPOSITE(x,y) ( ((x)&1) == ((y)&1) )

/* Loop on directions */
#define LOOPDIRECTION(x) for ((x)=0; (x)<4; (x)++)


#define FRUITVAL(x) (((x)->value)-48)

/* get sprite pointer @ given coordinate */
#define GETSPRITE(a,y,x) *((a)->board+((y)*(a)->cols+(x)))

#define ARENAGET(y,x) arenaget((y),(x))
#define ARENAPUT(y,x,z) arenaput((y), (x), (z))

/* cell */
typedef struct t_cell
{
  unsigned int x;
  unsigned int y;
}
cell;

/* sprite types */

/* Can't change this type to char ... don't know where it segfaults */
typedef short int SPRITE_TYPE;

#define SNAKE 1
#define EXSNAKE 2
#define FRUIT 3
#define WALL 4

#define IS_FRUIT(x) ( (x) ? ((x)->type == FRUIT) : 0 )
#define IS_SNAKE(x) ( (x) ? ((x)->type == SNAKE) : 0 )
#define IS_EXSNAKE(x) ( (x) ? ((x)->type == EXSNAKE) : 0 )
#define IS_WALL(x) ( (x) ? ((x)->type == WALL) : 0 )


/* sprite template structure */
typedef struct t_sprite
{
  SPRITE_TYPE type;
  char data[1]; /* should this be (void *) ? */
}
sprite;


/* chain of points */
typedef struct t_snakebody
{
  SPRITE_TYPE type;
  struct t_snake *self;
  struct t_cell cell;
  struct t_snakebody *next;
  struct t_snakebody *prev;
}
snakebody;

/* fruit structure */
typedef struct t_fruit
{
  SPRITE_TYPE type;
  struct t_cell cell;
  short int value;
}
fruit;

/* snake structure */
typedef struct t_snake
{
  short int playernum;		/* player number */
  snakebody *head;	/* snake's head chain */
  snakebody *tail;	/* snake's tail chain */
  int length;			  /* snake's length */
  char grow;			    /* snake's growing state */
  char died;			    /* did this snake die? */
  DIRECTION dir;	  /* snake's movement direction */
  unsigned long int score;	/* score */
  DIRECTION (*play) (struct t_snake *);		/* decision taker */
  void (*kill) (struct t_snake *);		/* killer */
  void (*destroy) (struct t_snake *);		/* destroier */
  void *brain;			/* snake's brain */
}
snake;

/* MESSAGES ************************************/
extern void message (char *format, ...);

/* ARENA ***************************************/


/* Arena structure */
typedef struct arena_t
{
  int x0;	/* Camera origin */
  int y0;

  int lines;        /* dimensions of the "bidimensional" sprite array */
  int cols;                               

  char init;         /* integrity check flag */
  sprite **board;   /* pointer to the sprite array */
} ARENA;



/* EXTERNAL FUNCTIONS */
extern int arenainit (void);
extern int arenalines (void);
extern int arenacols (void);
extern sprite *arenaget (int y, int x);
extern int arenaput (int y, int x, sprite * sp);

/* UI API */
extern void ui_drawmessage (char *);
extern void ui_drawarena (ARENA *);
extern void ui_updatescreen (void);
extern char *ui_prompt (char *);
extern int ui_getkey (void);

extern void finish (void);
extern int rand_between (int, int);
extern void *safe_malloc (size_t size, char *);
extern void *safe_calloc (size_t nmemb, size_t size, char *);
extern void *safe_realloc (void *ptr, size_t size, char *);

extern snake *snake_make (void);
extern void snake_init (snake *);
extern int register_player (snake *);
extern void snake_reset (snake *);
extern void snake_kill (snake *);
extern void snake_trim (snake *);

extern void move_cell (cell *, DIRECTION, int);
extern int cell_distance (cell *, cell *);
extern void init_fruit (void);

extern int register_command (char *, int (*)(char *), char *);
extern int parse_command (char *);
extern int fetchtypein (void);
extern void pause_game (void);
extern void resume_game (void);

extern char *find_file (char *);

/* EXTERNAL DATA */
extern long delay;
extern snake *zoo[];
extern char *cmdfifofile;
extern FILE *cmdfifo;
extern int screen_initialized;
extern ARENA arena;
extern fruit frt;
extern char path[];


/* arena operations */
#define ARENA_HEIGHT (arena.lines)
#define ARENA_WIDTH (arena.cols)

#endif /* SINS_H */
