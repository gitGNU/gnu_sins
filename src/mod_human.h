/* local definitions */
#define MAXQUEUELEN 2

/* Queue element */
typedef struct t_qelem
{
  int key;
  struct t_qelem *next;
}
qelem;

/* Queue */
typedef struct t_queue
{
  qelem *first;
  qelem *last;
  int len;
}
queue;

/* Snake brain */
typedef struct t_brain
{
  int kl;			/* left key */
  int ku;			/* up key */
  int kr;			/* right key */
  int kd;			/* down key */
  int ks;			/* stop key */
  int kg;			/* grow key */
  queue *kqueue;		/* key queue */
}
brain;
