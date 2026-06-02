#ifndef MOUI_EASE_H
#define MOUI_EASE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef float (*moui_ease_fn_t)(float t);

float moui_ease_linear(float t);
float moui_ease_out_quad(float t);
float moui_ease_in_out_quad(float t);
float moui_ease_out_cubic(float t);
float moui_ease_out_expo(float t);
float moui_ease_spring(float t);
float moui_ease_out_bounce(float t);
float moui_ease_out_elastic(float t);
float moui_ease_in_back(float t);
float moui_ease_out_back(float t);

float moui_ease_asymptotic(float current, float target, float factor);


#ifdef __cplusplus
}
#endif
#endif
