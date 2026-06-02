#include "moui_ease.h"
#include <math.h>

float moui_ease_linear(float t) { return t; }

float moui_ease_out_quad(float t) { return t * (2.0f - t); }

float moui_ease_in_out_quad(float t)
{
    if (t < 0.5f) return 2.0f * t * t;
    return -1.0f + (4.0f - 2.0f * t) * t;
}

float moui_ease_out_cubic(float t)
{
    float f = t - 1.0f;
    return f * f * f + 1.0f;
}

float moui_ease_out_expo(float t)
{
    return (t >= 1.0f) ? 1.0f : 1.0f - powf(2.0f, -10.0f * t);
}

float moui_ease_spring(float t)
{
    float s = 1.0f - t;
    return 1.0f - (cosf(t * t * 10.0f * 3.14159f) * s * s);
}

float moui_ease_out_bounce(float t)
{
    if (t < 1.0f / 2.75f) {
        return 7.5625f * t * t;
    } else if (t < 2.0f / 2.75f) {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    } else if (t < 2.5f / 2.75f) {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    } else {
        t -= 2.625f / 2.75f;
        return 7.5625f * t * t + 0.984375f;
    }
}

float moui_ease_out_elastic(float t)
{
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    return powf(2.0f, -10.0f * t) * sinf((t - 0.075f) * (2.0f * 3.14159f) / 0.3f) + 1.0f;
}

float moui_ease_in_back(float t)
{
    const float s = 1.70158f;
    return t * t * ((s + 1.0f) * t - s);
}

float moui_ease_out_back(float t)
{
    const float s = 1.70158f;
    t -= 1.0f;
    return t * t * ((s + 1.0f) * t + s) + 1.0f;
}

float moui_ease_asymptotic(float current, float target, float factor)
{
    (void)current;
    (void)factor;
    return target;   /* Snap immediately: E-Paper displays cannot handle multi-frame smooth animations */
}
