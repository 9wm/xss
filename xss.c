/* xss -- xautolock replacement using MIT-SCREEN-SAVER (it doesn't poll)
 * Copyright (C) 2008  Neale Pickett <neale@woozle.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
  (void)waitpid(-1, NULL, WNOHANG);
  child = 0;
}

int
main(int argc, char *argv[])
{
  Display *display = NULL;
  char    *errtxt  = "bummer dude.";
  Pixmap   blank;
  int      ss_event, ss_error;
  Window   root;
  XEvent   event;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s PROGRAM [ARGUMENT ...]\n", argv[0]);
    exit(1);
  }
  signal(SIGCHLD, sigchld);

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
    while (! XNextEvent(display, &event)) {
      if (event.type == ss_event) {
        XScreenSaverNotifyEvent *sevent = (XScreenSaverNotifyEvent *)&event;

        if (ScreenSaverOn == sevent->state) {
          if (! child) {
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
    errtxt = NULL;
  } while (0);

  if (display) {
    (void)XFreePixmap(display, blank);
    (void)XCloseDisplay(display);
  }
  if (errtxt) {
    fprintf(stderr, "Error: %s\n", errtxt);
    return 1;
  }

  return 0;
}
