/* 
 *    The following code is adapted from Doug Gwyn's System V emulation
 *    support for 4BSD and exploits the 4BSD select() system call.
 *    Doug originally called it 'nap()'; you probably want to call it
 *    "usleep()";
 *
 *    Copied from:
 *    http://www.faqs.org/faqs/unix-faq/faq/part4/section-6.html
 */

#ifndef HAVE_USLEEP

extern int        select();

int
usleep( usec )                            /* returns 0 if ok, else -1 */
long                usec;           /* delay in microseconds */
{
  static struct                       /* `timeval' */
  {
    long        tv_sec;         /* seconds */
    long        tv_usec;        /* microsecs */
  }
  delay;          /* _select() timeout */

  delay.tv_sec = usec / 1000000L;
  delay.tv_usec = usec % 1000000L;
  return select( 0, (long *)0, (long *)0, (long *)0, &delay );
}

#endif
