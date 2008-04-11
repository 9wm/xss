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

#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/extensions/scrnsaver.h>

#include "obj.h"

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

  if (argc < 2) {
    fprintf(stderr, "Usage: %s PROGRAM [ARGUMENT ...]\n", argv[0]);
    return 64;                  /* EX_USAGE */
  }
  signal(SIGCHLD, sigchld);

  try {
    int    ss_event, ss_error;
    XEvent event;

    if (! (display = XOpenDisplay(NULL))) raise("cannot open display");
    if (! XScreenSaverQueryExtension(display, &ss_event, &ss_error)) {
      raise("X server does not support MIT-SCREEN-SAVER extension.");
    }
    XScreenSaverSelectInput(display, DefaultRootWindow(display), ScreenSaverNotifyMask);
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
  } while (0);

  if (display) {
    (void)XCloseDisplay(display);
  }

  except {
    fprintf(stderr, "Error: %s\n", exception);
    return 69;                  /* EX_UNAVAILABLE */
  }

  return 0;
}
