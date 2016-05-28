// This file has been ported from Deviation.

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <curves.h>
#include <mixer.h>

/*
    {-100, 0, 100},
    {-100, -50, 0, 50, 100},
    {-100, -67, -33, 0, 33, 67, 100},
    {-100, -75, -50, -25, 0 25, 50, 75, 100},
    {-100, -80, -60, -40, -20, 0, 20, 40, 60, 80, 100},
    {-100, -83, -67, -50, -33, -17, 0, 17, 33, 50, 67, 83, 100}
*/


#define MMULT 1024


// ****************************************************************************
static int32_t compute_tangent(curve_t *curve, int num_points, int i)
{
    int32_t m;
    int32_t delta = 2 * 100 / (num_points - 1);

    if (i == 0) {
        // Linear interpolation between the first two points
        // Keep 3 decimal-places for m
        m = (MMULT * (curve->points[i + 1] - curve->points[i])) / delta;
    }
    else if (i == num_points - 1) {
        // Linear interpolation between the last two points
        // Keep 3 decimal-places for m
        m = (MMULT * (curve->points[i] - curve->points[i - 1])) / delta;
    }
    else {
        // Apply monotone rules from
        // http://en.wikipedia.org/wiki/Monotone_cubic_interpolation

        // 1) Compute slopes of secant lines
        int32_t d0 = (MMULT * (curve->points[i] - curve->points[i - 1])) / delta;
        int32_t d1 = (MMULT * (curve->points[i + 1] - curve->points[i])) / delta;

        // 2) Compute initial average tangent
        m = (d0 + d1) / 2;

        // 3) Check for horizontal lines
        if (d0 == 0 || d1 == 0 || (d0 > 0 && d1 < 0) || (d0 < 0 && d1 > 0)) {
            m = 0;
        }
        else {
            if (MMULT * m / d0 >  3 * MMULT) {
                m = 3 * d0;
            }
            else if (MMULT * m / d1 > 3 * MMULT) {
                m = 3 * d1;
            }
        }
    }

    return m;
}


// ****************************************************************************
/* The following is a hermite cubic spline.
   The basis functions can be found here:
   http://en.wikipedia.org/wiki/Cubic_Hermite_spline
   The tangents are computed via the 'cubic monotone' rules (allowing for local-maxima)
*/
static int32_t hermite_spline(curve_t *curve, int32_t value)
{
    int num_points = (curve->type - CURVE_3POINT) * 2 + 3;
    int32_t step = PERCENT_TO_CHANNEL(2 * 100) / (num_points - 1) ;

    if (value < PERCENT_TO_CHANNEL(-100)) {
        value = PERCENT_TO_CHANNEL(-100);
    }
    else if(value > PERCENT_TO_CHANNEL(100)) {
        value = PERCENT_TO_CHANNEL(100);
    }

    for (int i = 0; i < num_points -1; i++) {
        int32_t x = PERCENT_TO_CHANNEL(-100) + i * step;
        int32_t p0x = x;
        int32_t p3x;

        //If there are rounding errors, we need to deal with them here
        if (i == num_points - 2) {
            p3x = PERCENT_TO_CHANNEL(100);
        }
        else {
            p3x = x + step;
        }

        if (value >= p0x  &&  value <= p3x) {
            int32_t p0y = PERCENT_TO_CHANNEL(curve->points[i]);
            int32_t p3y = PERCENT_TO_CHANNEL(curve->points[i+1]);
            int32_t m0 = compute_tangent(curve, num_points, i);
            int32_t m3 = compute_tangent(curve, num_points, i+1);
            int32_t y;
            int32_t h = p3x - p0x;
            int32_t t = (MMULT * (value - p0x)) / h;
            int32_t t2 = t * t / MMULT;
            int32_t t3 = t2 * t / MMULT;
            int32_t h00 = 2*t3 - 3*t2 + MMULT;
            int32_t h10 = t3 - 2*t2 + t;
            int32_t h01 = -2*t3 + 3*t2;
            int32_t h11 = t3 - t2;
            y = p0y * h00 + h * (m0 * h10 / MMULT) + p3y * h01 + h * (m3 * h11 / MMULT);
            y /= MMULT;
            return y;
        }
    }
    return 0;
}


// ****************************************************************************
static int32_t interpolate(curve_t *curve, int32_t value)
{
    int i;
    int num_points = (curve->type - CURVE_3POINT) * 2 + 3;
    int32_t step = 2 * 10000 / (num_points - 1) ;
    for (i = 0; i < num_points - 1; i++) {
        int32_t x = -10000 + i * step;
        int32_t pos1 = PERCENT_TO_CHANNEL(x / 100);
        int32_t pos2 = PERCENT_TO_CHANNEL((x + step) / 100);
        if(value >= pos1 && value <= pos2) {
            int32_t tmp = (value - pos1) * (curve->points[i + 1] - curve->points[i]) / (pos2 - pos1) + curve->points[i];
            return PERCENT_TO_CHANNEL(tmp);
        }
    }
    return PERCENT_TO_CHANNEL(curve->points[num_points - 1]);
}

// ****************************************************************************
/* This came from er9x/th9x
 * expo-function:
 * ---------------
 * kmplot
 * f(x,k)=exp(ln(x)*k/10) ;P[0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20]
 * f(x,k)=x*x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
 * f(x,k)=x*x*k/10 + x*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
 * f(x,k)=1+(x-1)*(x-1)*(x-1)*k/10 + (x-1)*(1-k/10) ;P[0,1,2,3,4,5,6,7,8,9,10]
 */
static int32_t expou(uint32_t x, uint16_t k)
{
    // k*x*x*x + (1-k)*x
    // 0 <= k <= 100
    #define KMAX 100
    uint32_t val = (x * x / CHANNEL_100_PERCENT * x / CHANNEL_100_PERCENT * k
               + (KMAX - k) * x + KMAX / 2) / KMAX;
    return val;
}


// ****************************************************************************
static int32_t expo(curve_t *curve, int32_t value)
{

    int32_t  y;
    int32_t k;
    bool neg = value < 0;

    k = neg ? curve->points[1] : curve->points[0];
    if (k == 0)
        return value;

    if (neg)
        value = -value; //absval

    if (k < 0) {
        y = CHANNEL_100_PERCENT - expou(CHANNEL_100_PERCENT - value, -k);
    }else{
        y = expou(value, k);
    }
    return neg ? -y : y;
}


// ****************************************************************************
static int32_t gt_zero(curve_t *curve, int32_t value)
{
    int32_t divisor;

    divisor = CHANNEL_100_PERCENT - PERCENT_TO_CHANNEL(curve->points[0]);
    if (divisor <= 0) {
        return 0;
    }

    if (value < PERCENT_TO_CHANNEL(curve->points[0])) {
       return 0;
    }

    return CHANNEL_100_PERCENT * (value - PERCENT_TO_CHANNEL(curve->points[0])) / divisor;
}


// ****************************************************************************
static int32_t lt_zero(curve_t *curve, int32_t value)
{
    int32_t divisor;

    divisor = (CHANNEL_N100_PERCENT - PERCENT_TO_CHANNEL(curve->points[0]));
    if (divisor >= 0) {
        return 0;
    }

    if (value > PERCENT_TO_CHANNEL(curve->points[0])) {
       return 0;
    }
    return CHANNEL_N100_PERCENT * (value - PERCENT_TO_CHANNEL(curve->points[0])) / divisor;
}


// ****************************************************************************
static int32_t deadband(curve_t *curve, int32_t value)
{
    bool neg = value < 0;
    int32_t k = neg ? curve->points[1] : curve->points[0];
    int32_t max = CHANNEL_100_PERCENT;

    if (k == 0) {
        return CHANNEL_100_PERCENT;
    }

    // FIXME: this algo does not seem to work as expected

    value = abs(value);
    if (value < PERCENT_TO_CHANNEL(k) / 10) {
        return 0;
    }

    return max * ((1000 * (value - max) + (1000 - k) * max) / (1000 - k)) / value;
}


// ****************************************************************************
static int32_t absval(curve_t *curve, int32_t value)
{
    int32_t divisor;

    if (value < PERCENT_TO_CHANNEL(curve->points[0])) {
       divisor = (PERCENT_TO_CHANNEL(-100) - PERCENT_TO_CHANNEL(curve->points[0]));
       if (divisor >= 0) {
           return 0;
       }
       return PERCENT_TO_CHANNEL(100) * (value - PERCENT_TO_CHANNEL(curve->points[0])) / divisor;
    }
    else {
        divisor = (PERCENT_TO_CHANNEL(100) - PERCENT_TO_CHANNEL(curve->points[0]));
        if (divisor <= 0) {
            return 0;
        }
        return PERCENT_TO_CHANNEL(100) * (value - PERCENT_TO_CHANNEL(curve->points[0])) / divisor;
    }
}


// ****************************************************************************
static int32_t min_max(curve_t *curve, int32_t value)
{
    if (value < PERCENT_TO_CHANNEL(curve->points[0])) {
        return CHANNEL_N100_PERCENT;
    }
    else {
        return CHANNEL_100_PERCENT;
    }
}


// ****************************************************************************
static int32_t zero_max(curve_t *curve, int32_t value)
{
    if (value < PERCENT_TO_CHANNEL(curve->points[0])) {
        return 0;
    }
    else {
        return CHANNEL_100_PERCENT;
    }
}


// ****************************************************************************
int32_t CURVE_evaluate(int32_t value, curve_t *curve)
{
    // Interpolation doesn't work if the input is out of bounds, so we clamp
    // it here.
    // However, we let CURVE_NONE pass without clamping to allow creating a
    // pre-scaler to chain mixers together
    if (curve->type != CURVE_NONE) {
        if (value > CHANNEL_100_PERCENT) {
            value = CHANNEL_100_PERCENT;
        }
        else if (value < CHANNEL_N100_PERCENT) {
            value = CHANNEL_N100_PERCENT;
        }
    }


    switch (curve->type) {
        case CURVE_NONE:
            return value;

        case CURVE_FIXED:
            return PERCENT_TO_CHANNEL(curve->points[0]);

        case CURVE_MIN_MAX:
            return min_max(curve, value);

        case CURVE_ZERO_MAX:
            return zero_max(curve, value);

        case CURVE_GT_ZERO:
            return gt_zero(curve, value);

        case CURVE_LT_ZERO:
            return lt_zero(curve, value);

        case CURVE_ABSVAL:
            return absval(curve, value);

        case CURVE_EXPO:
            return expo(curve, value);

        case CURVE_DEADBAND:
            return deadband(curve, value);

        case CURVE_3POINT:
        case CURVE_5POINT:
        case CURVE_7POINT:
        case CURVE_9POINT:
        case CURVE_11POINT:
        case CURVE_13POINT:
        default:
            if (curve->smoothing) {
                return hermite_spline(curve, value);
            }
            else {
                return interpolate(curve, value);
            }
    }
}


// ****************************************************************************
unsigned CURVE_get_number_of_points(curve_t *curve)
{
    switch (curve->type) {
        case CURVE_FIXED:
        case CURVE_MIN_MAX:
        case CURVE_ZERO_MAX:
        case CURVE_GT_ZERO:
        case CURVE_LT_ZERO:
        case CURVE_ABSVAL:
            return 1;

        case CURVE_EXPO:
        case CURVE_DEADBAND:
             return 2;

        case CURVE_3POINT:
             return 3;

        case CURVE_5POINT:
             return 5;

        case CURVE_7POINT:
             return 7;

        case CURVE_9POINT:
             return 9;

        case CURVE_11POINT:
             return 11;

        case CURVE_13POINT:
             return 13;

        case CURVE_NONE:
        default:
             return 0;
    }
}

