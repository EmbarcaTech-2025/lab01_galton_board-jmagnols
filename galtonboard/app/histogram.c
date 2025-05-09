#include "histogram.h"

static uint histogram_values[MAX_BINS];
static int n_bins = 0;
static int max_value = 0;

void init_hist(int nbins) {
    for (int i = 0; i < MAX_BINS; i++) {
        histogram_values[i] = 0;
    }

    n_bins = nbins;
}

void addto_hist(int value) {
    if (value < 0 || value >= n_bins) {
        return;
    }
    histogram_values[value]++;

    if(histogram_values[value] > max_value) {
        max_value = histogram_values[value];
    }
}

void draw_hist(ssd1306_t *disp) {
    ssd1306_clear_square(disp, 64, 0, 64, 64);

    if (max_value == 0) {
        return;
    }

    for(int i = 0; i < n_bins; i++) {
        int bar_height = (histogram_values[i] * 64) / max_value;
        ssd1306_draw_square(disp, 64, 64*i/n_bins, bar_height, 64/n_bins);
    }
}
