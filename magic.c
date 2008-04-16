/* magic -- "Magic" (qix) screen saver hack
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

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <X11/Xlib.h>
#include "obj.h"

#define FPS 17
#define TRAILPCT 50
#define MAXDELTAPML 14
#define MINDELTAPML 2
#define COLORDELTA 50

#define nsec (1000000000 / FPS)
#define max(a,b) (((a) > (b)) ? (a) : (b))

#define sum(a, b, v, m)     \
  if (b + v < 0) {          \
    a = -1 * (b + v);       \
    v *= -1;                \
  } else if (b + v > m) {   \
    a = (m*2) - (b + v);    \
    v *= -1;                \
  } else {                  \
    a = b + v;              \
  }

int
main(int argc, char * const argv[])
{
  Display       *display;
  Window         w      = (Window)0;
  GC             egc    = (GC)0;
  XSegment      *lines  = NULL;
  int            i;

  for (i = 1; i < argc; i += 1) {
    if (0 == strcmp(argv[i], "-window-id")) {
      /* For compatibility reasons, just ignore */
    } else if (w) {
      w = (Window)-1;
      break;
    } else {
      char *end;

      if (argv[i][0] == '0' && argv[i][1] == 'x') {
        w = (Window)strtol(argv[i] + 2, &end, 16);
      } else {
        w = (Window)strtol(argv[i], &end, 10);
      }
      if ('\0' != *end) {
        w = (Window)-1;
        break;
      }
    }
  }
  if ((Window)-1 == w) {
    (void)fprintf(stderr, "Usage: %s [WINDOW_ID]\n", argv[0]);
    return 64;                /* EX_USAGE */
  }

  try {
    int            screen;
    Window         root;
    XSegment       velocity;
    int            width, height, nlines;
    unsigned short red, green, blue;
    int            dred, dgreen, dblue;

    if (! (display = XOpenDisplay(NULL))) raise("cannot open display");
    screen = DefaultScreen(display);
    root = RootWindow(display, screen);

    if (w) {
      XWindowAttributes wa;

      XGetWindowAttributes(display, w, &wa);
      width = wa.width;
      height = wa.height;
    } else {
      XSetWindowAttributes wa;
      XColor black;


      zero(wa);
      zero(black);

      wa.override_redirect = 1;
      wa.background_pixel = BlackPixel(display, screen);
      width = DisplayWidth(display, screen) / 3;
      height = DisplayHeight(display, screen) / 3;
      w = XCreateWindow(display, root,
                        0, 0,
                        width, height, 0,
                        CopyFromParent, CopyFromParent, CopyFromParent,
                        CWBackPixel,
                        &wa);
      XMapWindow(display, w);
    }

    {
      XGCValues values;

      egc = XCreateGC(display, w, 0, &values);
      if (! XSetForeground(display, egc, BlackPixel(display, screen))) break;
      if (! XSetBackground(display, egc, WhitePixel(display, screen))) break;
    }

    srandom((unsigned int)time(NULL));

    {
      int u, l;

      u = width * MAXDELTAPML / 1000;
      l = width * MINDELTAPML / 1000;
      velocity.x1 = (short)(random() % (u - l)) + l;
      velocity.y1 = (short)(random() % (u - l)) + l;
      velocity.x2 = (short)(random() % (u - l)) + l;
      velocity.y2 = (short)(random() % (u - l)) + l;
    }

    nlines = (width * TRAILPCT) / (max(velocity.y1, velocity.y2) * 100);
    lines = (XSegment *)calloc(nlines, sizeof(XSegment));

    lines[0].x1 = (short)(random() % width);
    lines[0].y1 = (short)(random() % height);
    lines[0].x2 = (short)(random() % width);
    lines[0].y2 = (short)(random() % height);
    red = (unsigned short)(random() % 65536);
    green = (unsigned short)(random() % 65536);
    blue = (unsigned short)(random() % 65536);
    dred = dgreen = dblue = COLORDELTA;

    for (i = 0; ;) {
      XSegment        segments[2];
      struct timespec req = {0, nsec};
      int             j   = (i + 1) % nlines;
      GC              gc;
      XGCValues       values;
      XColor          color;

      gc = XCreateGC(display, w, 0, &values);
      if (! XSetBackground(display, gc, BlackPixel(display, screen))) break;
      sum(red, red, dred, 65536);
      sum(green, green, dgreen, 65536);
      sum(blue, blue, dblue, 65536);
      color.red = red;
      color.green = green;
      color.blue = blue;

      if (! XAllocColor(display, DefaultColormap(display, screen), &color)) break;
      if (! XSetForeground(display, gc, color.pixel)) break;

      (void)memcpy(segments + 0, lines + j, sizeof(XSegment));
      (void)memcpy(segments + 1, lines + j, sizeof(XSegment));
      segments[1].x1 = width - segments[0].x1;
      segments[1].x2 = width - segments[0].x2;
      XDrawSegments(display, (Drawable)w, egc, segments, 2);

      sum(lines[j].x1, lines[i].x1, velocity.x1, width);
      sum(lines[j].y1, lines[i].y1, velocity.y1, height);
      sum(lines[j].x2, lines[i].x2, velocity.x2, width);
      sum(lines[j].y2, lines[i].y2, velocity.y2, height);

      (void)memcpy(segments + 0, lines + j, sizeof(XSegment));
      (void)memcpy(segments + 1, lines + j, sizeof(XSegment));
      segments[1].x1 = width - segments[0].x1;
      segments[1].x2 = width - segments[0].x2;
      XDrawSegments(display, (Drawable)w, gc, segments, 2);

      if (! XFreeColors(display, DefaultColormap(display, screen), &(color.pixel), 1, 0)) break;
      (void)XFreeGC(display, gc);
      XSync(display, True);

      (void)nanosleep(&req, NULL);

      i = j;
    }
  }

  if (lines) {
    free(lines);
  }

  if (display) {
    if (egc) {
      (void)XFreeGC(display, egc);
    }
    (void)XCloseDisplay(display);
  }

  except {
    (void)fprintf(stderr, "Error: %s\n", exception);
    return 69;                  /* EX_UNAVAILABLE */
  }
  return 0;
}
