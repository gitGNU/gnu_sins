#include "sins.h"

#ifdef HAVE_GETOPT_H
#include <getopt.h>		/* getopt_long ( ! on {Open,Free}BSD ) */
#endif

/* public functions */
int parse_cmdline_opts (int, char **);
void parse_cmdline_args (int, char **);

/* private functions */
static void usage (char *);

/*
 * get command line options
 */
int
parse_cmdline_opts (int argc, char **argv)
{
  extern int want_commandlist;

#ifdef HAVE_GETOPT_LONG
  static struct option opts[] = {
    {"help", 0, 0, 'h'},
    {"version", 0, 0, 'v'},
    {"lines", 1, 0, 'l'},
    {"cols", 1, 0, 'c'},
    {"list-commands", 0, 0, 'L'},
    {0, 0, 0, 0}
  };
  int opts_idx;
#endif
  int c;

  while (1)
  {

    /* scan next option argument */
#ifdef HAVE_GETOPT_LONG
    c = getopt_long (argc, argv, "c:l:hLv", opts, &opts_idx);
#else
    c = getopt (argc, argv, "c:l:hLv");
#endif

    /*
     * end of options , return index of next non-option argument
     */
    if (c == -1) return optind;

    switch (c)
    {

      case 0:
        fputs ("bug in getopt?\n", stderr);
        break;

      case 'h':
      case '?':
        usage (argv[0]);
        break;		/* usage will exit anyway */

      case 'l':
        arena.lines  = abs(atoi(optarg));
        break;

      case 'c':
        arena.cols  = abs(atoi(optarg));
        break;

      case 'v':
        printf ("snake %s\n", VERSION);
        exit (0);

      case 'L':
        want_commandlist=1;
        break;

    }			/* switch (c) */

  }				/* while (1) */
}


/* parse arguments on the command line (commands) */
void
parse_cmdline_args (int argc, char **argv)
{
  while (argc--)
    parse_command (*argv++);
}


static void
usage (char *me)
{
  fprintf (stderr, "Usage: %s [OPTIONS] [COMMANDS]...\n", me);
  fputs ("Options:\n", stderr);
  fputs ("  -l, --lines=#        set arena lines to #\n", stderr);
  fputs ("  -c, --cols=#         set arena columns to #\n", stderr);
  fputs ("  -h, --help           show this screen and exit\n", stderr);
  fputs ("  -v, --version        output version and exit\n", stderr);
  fputs ("  -L, --list-commands  list available commands and exit\n", stderr);
  exit (1);
}
