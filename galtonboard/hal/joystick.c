#include "joystick.h"

void init_joystick(uint X_pin, uint Y_pin) {
    adc_init();
    adc_gpio_init(X_pin);
    adc_gpio_init(Y_pin);
}

void getXY_joystick(float *x, float *y) {
    adc_select_input(0);
    uint adc_y_raw = adc_read();
    adc_select_input(1);
    uint adc_x_raw = adc_read();

    // ADC values -> joystick coordinates
    // 0 , 4095 -> -1 , 1
    *x = adc_x_raw / 2047.5 - 1;
    *y = adc_y_raw / 2047.5 - 1;
}

void getRA_joystick(float *r, float *angle) {
    float x, y;
    getXY_joystick(&x, &y);

    // XY coordinates -> polar coordinates
    *r = sqrt(x * x + y * y);
    *angle = atan2(y, x);
}
