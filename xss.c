#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/scrnsaver.h>

int child = 0;

void
sigchld(int signum)
{
  /* Reap zombies */
  int   status;

  (void)wait(&status);
  child = 0;
}

int
main(int argc, char *argv[])
{
  Display *display = NULL;
  char    *errtxt  = "bummer dude.";
  Pixmap   blank;
  int      screen;
  int      ss_event, ss_error;
  Window   root;


  if (argc < 2) {
    fprintf(stderr, "Usage: %s PROGRAM [ARGUMENT ...]\n", argv[0]);
    exit(1);
  }

  do {
    display = XOpenDisplay(NULL);
    if (! display) {
      errtxt = "Cannot open display";
      break;
    }

    root = DefaultRootWindow(display);

    if (! XScreenSaverQueryExtension(display, &ss_event, &ss_error)) {
      errtxt = "X server does not support MIT-SCREEN-SAVER extension.";
      break;
    }

    XScreenSaverSelectInput(display, root, ScreenSaverNotifyMask);

    signal(SIGCHLD, sigchld);

    while (1) {
      XEvent event;

      if (XNextEvent(display, &event)) {
        errtxt = NULL;
        break;
      }

      if (event.type == ss_event) {
        XScreenSaverNotifyEvent *sevent;

        sevent = (XScreenSaverNotifyEvent *)&event;
        if (ScreenSaverOn == sevent->state) {
          if (child) {
            /* Don't do anything, we still have a child */
          } else {
            child = fork();
            if (0 == child) {
              (void)execvp(argv[1], argv + 1);
              perror("exec");
              exit(1);
            }
          }
        }
      }
    }
  } while (0);

  if (display) {
    (void)XScreenSaverUnregister(display, screen);
    (void)XFreePixmap(display, blank);
    (void)XCloseDisplay(display);
  }

  if (errtxt) {
    fprintf(stderr, "Error: %s\n", errtxt);
    return 1;
  }

  return 0;
}
