#ifndef _ball
#define _ball

#include "pico/rand.h"
#include "ssd1306.h"

#define MAX_BALL_QUANT 50

typedef struct {
    int x;
    int y;
    bool force_fall;
} ball_t;

void draw_balls(ssd1306_t *disp, int scale);
void erase_balls(ssd1306_t *disp, int scale);
int updt_balls_pos(int n_lines);

#endif
