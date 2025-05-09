#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "pico/stdlib.h"
#include "hardware/adc.h"
#include <math.h>

// init módulo ADC e pinos GPIO com ADC
void init_joystick(uint X_pin, uint Y_pin);

// converte p/ coordenadas X e Y
// -1 : 1
void getXY_joystick(float *x, float *y);

// converte p/ raio e ang
// raio 0 : 1
// ang -pi : pi
void getRA_joystick(float *r, float *angle);

#endif // JOYSTICK_H
