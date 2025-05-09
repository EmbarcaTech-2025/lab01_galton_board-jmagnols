#ifndef _histogram
#define _histogram

#include "ssd1306.h"

#define MAX_BINS 30

void init_hist(int nbins);
void addto_hist(int value);
void draw_hist(ssd1306_t *disp);

#endif
