#pragma once

#include <stdint.h>

#define CURVE_MAX_POINTS 13

typedef enum {
    CURVE_NONE,
    CURVE_FIXED,
    CURVE_MIN_MAX,
    CURVE_ZERO_MAX,
    CURVE_GT_ZERO,
    CURVE_LT_ZERO,
    CURVE_ABSVAL,
    CURVE_EXPO,
    CURVE_DEADBAND,
    CURVE_3POINT,
    CURVE_5POINT,
    CURVE_7POINT,
    CURVE_9POINT,
    CURVE_11POINT,
    CURVE_13POINT,
    LAST_CURVE = CURVE_13POINT
} curve_type_t;

typedef struct {
    curve_type_t type;
    int8_t points[CURVE_MAX_POINTS];
} curve_t;


int32_t CURVE_evaluate(int32_t xval, curve_t *curve);
unsigned CURVE_get_number_of_points(curve_t *curve);
