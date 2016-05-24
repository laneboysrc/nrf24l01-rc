
// This file has been ported from Deviation.


#include <stdlib.h>
#include <stdint.h>


/*
    {-100, 0, 100},
    {-100, -50, 0, 50, 100},
    {-100, -67, -33, 0, 33, 67, 100},
    {-100, -75, -50, -25, 0 25, 50, 75, 100},
    {-100, -80, -60, -40, -20, 0, 20, 40, 60, 80, 100},
    {-100, -83, -67, -50, -33, -17, 0, 17, 33, 50, 67, 83, 100}
*/

#define CURVE_TYPE(x)       (((x)->type) & 0x7F)
#define CURVE_SMOOTHING(x)  (((x)->type) & 0x80)
#define CURVE_SET_TYPE(x, y) ((x)->type = ((x)->type & ~0x7F) | (y))
#define CURVE_SET_SMOOTHING(x, y) ((x)->type = ((x)->type & ~0x80) | ((y) ? 0x80 : 0))

#define CHAN_MULTIPLIER 100
#define PCT_TO_RANGE(x) ((x) * CHAN_MULTIPLIER)
#define RANGE_TO_PCT(x) ((x) / CHAN_MULTIPLIER)
#define CHAN_MAX_VALUE (100 * CHAN_MULTIPLIER)
#define CHAN_MIN_VALUE (-100 * CHAN_MULTIPLIER)
#define MAX_POINTS 13

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
    int8_t points[MAX_POINTS];
} curve_t;


int32_t CURVE_Evaluate(int32_t xval, curve_t *curve);
// const char *CURVE_GetName(char *str, curve_t *curve);
unsigned CURVE_NumPoints(curve_t *curve);



// ****************************************************************************
static int32_t compute_tangent(curve_t *curve, int num_points, int i)
{
    int32_t m;
    int32_t delta = 2 * 100 / (num_points - 1);
    #define MMULT 1024
    if (i == 0) {
        //linear interpolation between 1st 2 points
        //keep 3 decimal-places for m
        m = (MMULT * (curve->points[i+1] - curve->points[i])) / delta;
    } else if (i == num_points - 1) {
        //linear interpolation between last 2 points
        //keep 3 decimal-places for m
        m = (MMULT * (curve->points[i] - curve->points[i-1])) / delta;
    } else {
        //apply monotone rules from
        //http://en.wikipedia.org/wiki/Monotone_cubic_interpolation
        //1) compute slopes of secant lines
        int32_t d0 = (MMULT * (curve->points[i] - curve->points[i-1])) / (delta);
        int32_t d1 = (MMULT * (curve->points[i+1] - curve->points[i])) / (delta);
        //2) compute initial average tangent
        m = (d0 + d1) / 2;
        //3 check for horizontal lines
        if (d0 == 0 || d1 == 0 || (d0 > 0 && d1 < 0) || (d0 < 0 && d1 > 0)) {
            m = 0;
        } else {
            if (MMULT * m / d0 >  3 * MMULT) {
                m = 3 * d0;
            } else if (MMULT * m / d1 > 3 * MMULT) {
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
    int num_points = (CURVE_TYPE(curve) - CURVE_3POINT) * 2 + 3;
    int32_t step = PCT_TO_RANGE(2 * 100) / (num_points - 1) ;
    if (value < PCT_TO_RANGE(-100)) {
        value = PCT_TO_RANGE(-100);
    } else if(value > PCT_TO_RANGE(100)) {
        value = PCT_TO_RANGE(100);
    }
    for (int i = 0; i < num_points -1; i++) {
        int32_t x = PCT_TO_RANGE(-100) + i * step;
        int32_t p0x = x;
        int32_t p3x;
        //If there are rounding errors, we need to deal with them here
        if (i == num_points - 2) {
            p3x = PCT_TO_RANGE(100);
        } else {
            p3x = x + step;
        }
        if(value >= p0x && value <= p3x) {
            int32_t p0y = PCT_TO_RANGE(curve->points[i]);
            int32_t p3y = PCT_TO_RANGE(curve->points[i+1]);
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
    int num_points = (CURVE_TYPE(curve) - CURVE_3POINT) * 2 + 3;
    int32_t step = 2 * 10000 / (num_points - 1) ;
    for (i = 0; i < num_points - 1; i++) {
        int32_t x = -10000 + i * step;
        int32_t pos1 = PCT_TO_RANGE(x / 100);
        int32_t pos2 = PCT_TO_RANGE((x + step) / 100);
        if(value >= pos1 && value <= pos2) {
            int32_t tmp = (value - pos1) * (curve->points[i + 1] - curve->points[i]) / (pos2 - pos1) + curve->points[i];
            return PCT_TO_RANGE(tmp);
        }
    }
    return PCT_TO_RANGE(curve->points[num_points - 1]);
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
    uint32_t val = (x * x / CHAN_MAX_VALUE * x / CHAN_MAX_VALUE * k
               + (KMAX - k) * x + KMAX / 2) / KMAX;
    return val;
}


// ****************************************************************************
static int32_t expo(curve_t *curve, int32_t value)
{

    int32_t  y;
    int32_t k;
    unsigned neg = value < 0;

    k = neg ? curve->points[1] : curve->points[0];
    if (k == 0)
        return value;

    if (neg)
        value = -value; //absval

    if (k < 0) {
        y = CHAN_MAX_VALUE - expou(CHAN_MAX_VALUE - value, -k);
    }else{
        y = expou(value, k);
    }
    return neg ? -y : y;
}


// ****************************************************************************
static int32_t deadband(curve_t *curve, int32_t value)
{
    unsigned neg = value < 0;
    int32_t k = neg ? (uint8_t)curve->points[1] : (uint8_t)curve->points[0];
    int32_t max = CHAN_MAX_VALUE;

    if (k == 0)
        return CHAN_MAX_VALUE;
    value = abs(value);
    if (value < k * CHAN_MULTIPLIER / 10)
        return 0;
    return max * ((1000 * (value - max) + (1000 - k) * max) / (1000 - k)) / value;
}


// ****************************************************************************
int32_t CURVE_Evaluate(int32_t xval, curve_t *curve)
{
    int32_t divisor;

    //We let CURVE_NONE get a pass to allow creating a pre-scaler to chain mixers together
    if (CURVE_TYPE(curve) == CURVE_NONE) {
        return xval;
    }

    //interpolation doesn't work if theinput is out of bounds, so bound it here
    if (xval > CHAN_MAX_VALUE) {
        xval = CHAN_MAX_VALUE;
    }
    else if (xval < CHAN_MIN_VALUE) {
        xval = CHAN_MIN_VALUE;
    }

    switch (CURVE_TYPE(curve)) {
        case CURVE_FIXED:
            return CHAN_MAX_VALUE;

        case CURVE_MIN_MAX:
            return (xval < PCT_TO_RANGE(curve->points[0])) ? CHAN_MIN_VALUE : CHAN_MAX_VALUE;

        case CURVE_ZERO_MAX:
            return (xval < PCT_TO_RANGE(curve->points[0])) ? 0 : CHAN_MAX_VALUE;

        case CURVE_GT_ZERO:
            divisor = (PCT_TO_RANGE(100) - PCT_TO_RANGE(curve->points[0]));
            if (divisor <= 0) {
                return 0;
            }
            return (xval < PCT_TO_RANGE(curve->points[0]))
                   ? 0
                   : PCT_TO_RANGE(100) * (xval - PCT_TO_RANGE(curve->points[0])) / divisor;

        case CURVE_LT_ZERO:
            divisor = (PCT_TO_RANGE(-100) - PCT_TO_RANGE(curve->points[0]));
            if (divisor >= 0) {
                return 0;
            }
            return (xval > PCT_TO_RANGE(curve->points[0]))
               ? 0
               : PCT_TO_RANGE(-100) * (xval - PCT_TO_RANGE(curve->points[0])) / divisor;

        case CURVE_ABSVAL:
            if (xval < PCT_TO_RANGE(curve->points[0])) {
               divisor = (PCT_TO_RANGE(-100) - PCT_TO_RANGE(curve->points[0]));
               if (divisor >= 0) {
                   return 0;
               }
               return PCT_TO_RANGE(100) * (xval - PCT_TO_RANGE(curve->points[0])) / divisor;
            }
            else {
                divisor = (PCT_TO_RANGE(100) - PCT_TO_RANGE(curve->points[0]));
                if (divisor <= 0) {
                    return 0;
                }
                return PCT_TO_RANGE(100) * (xval - PCT_TO_RANGE(curve->points[0])) / divisor;
            }
        case CURVE_EXPO:
            return expo(curve, xval);

        case CURVE_DEADBAND:
            return deadband(curve, xval);

        case CURVE_NONE:
            // Can never happen as we return CURVE_NONE at the begin of the
            // function.
            return xval;

        default:
            if (CURVE_SMOOTHING(curve)) {
                return hermite_spline(curve, xval);
            }
            return interpolate(curve, xval);
    }
}


// ****************************************************************************
// const char *CURVE_GetName(char *str, curve_t *curve)
// {
//     switch (CURVE_TYPE(curve)) {
//         case CURVE_NONE: return _tr("1-to-1");
//         case CURVE_FIXED: return _tr("Fixed");
//         case CURVE_MIN_MAX:  return _tr("Min/Max");
//         case CURVE_ZERO_MAX: return _tr("Zero/Max");
//         case CURVE_GT_ZERO:  return "> 0"; //Don't translate these
//         case CURVE_LT_ZERO:  return "< 0"; //Don't translate these
//         case CURVE_ABSVAL:   return _tr("ABSVAL");
//         case CURVE_EXPO:     sprintf(str, _tr("EXPO %d"), curve->points[0]); return str;
//         case CURVE_DEADBAND: return _tr("Deadband");
//         case CURVE_3POINT:   return _tr("3 Point");
//         case CURVE_5POINT:   return _tr("5 Point");
//         case CURVE_7POINT:   return _tr("7 Point");
//         case CURVE_9POINT:   return _tr("9 Point");
//         case CURVE_11POINT:  return _tr("11 Point");
//         case CURVE_13POINT:  return _tr("13 Point");
//     }
//     return _tr("Unknown");
// }


// ****************************************************************************
unsigned CURVE_NumPoints(curve_t *curve)
{
    switch (CURVE_TYPE(curve)) {
        case CURVE_NONE:
        case CURVE_FIXED:
            return 0;

        case CURVE_MIN_MAX:
        case CURVE_ZERO_MAX:
        case CURVE_GT_ZERO:
        case CURVE_LT_ZERO:
        case CURVE_ABSVAL:
            return 1;

        case CURVE_EXPO:
        case CURVE_DEADBAND:
             return 2;

        default:
             return (CURVE_TYPE(curve) + 1 - CURVE_3POINT) * 2 + 1;
    }
}

