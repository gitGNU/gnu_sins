/* DEVELOPMENT VERSION */

struct varent_t {
  char *name = NULL;
  char *value = NULL;
  struct varent_t *next = NULL;
} varent;

char *getvar (char *name)
{
}

int putvar (char *name, char *value)
{
  struct varent_t *entp;
  struct varent_t *newent;
  int cmp;
  int done=0;

  /* create a new entry */
  newent = safe_malloc(sizeof(struct varent), "new entry in variable table");

  /* place entry in alphabetical order */
  for ( entp = &varent; entp->next; entp=entp->next )
  {
    cmp = strcmp(entp->name, name);
    if ( ! cmp )
    {
      entp->value = value;
      return 1;
    }
  }

}
