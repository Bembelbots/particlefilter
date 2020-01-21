#pragma once

#include <platform.h>

#include <vector>
#include <limits>
#include <cmath>
#include "constants.h"


constexpr float operator "" _deg(long double value) { 
    return static_cast<float>(DEG_TO_RAD * value);
}


/**
* calculates the distance from a point to a line
* all in homogenius coordinates
*/
float lineToPointDist(const std::vector<float> &point,
                      const std::vector<float> &line);


std::vector<float> getHesseNormalFormOfLine(const std::vector<float> &line);


//// The 6 above either to Line or Circle.... see definition-comment below
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


////// these BELOW are needed, somehow ...
//  maybe these fit inside a NumericVector class, but they are not used often
//  enough to cover the expenses of writing one, thus.... put them in a separate
//  - VectorOperations namespace or something like that ....
//  - make a (fake)class realizing all as static inside it (notation-wise equal)
//
//////// needed in measurementtoolbox .....
//////// cross product of vectors seem to be needed, but isn't this
//////// provided by the cv::mat stuff ????
// - this can not easily be removed: measuretoolsbox, playingfield + cornerdetect
/**
* calculates the vec crossproduct for a 3d vector
* in this case a 2d homogenius vector
* normed, so that the 3rd componet is 1
* @param vector a == size 3
* @param vector b ||
* @return vector
*/
std::vector<float> vectorCrossProduct(const std::vector<float> &a,
                                      const std::vector<float> &b);

/////// does some math, why? and even more, why is this not related to a class
// - pose/playingfield.cpp ->
//   'line.equationNormal = vectorCrossProductUnNormed(startP, endP);'
// - core/util/mathtoolbox.cpp ->
//   'return normVector(vectorCrossProductUnNormed(a, b));'
// - <no other uses, so remove usage in playingfield (if appropriate) and
//   fully remove func here!
/**
* calculates the vec crossproduct for a 3d vector
* in this case a 2d homogenius vector
* @param vector a == size 3
* @param vector b ||
* @return vector
*/
std::vector<float> vectorCrossProductUnNormed(const std::vector<float> &a,
        const std::vector<float> &b);


/////// normVector is also an UNUSED [;)] method in vision/visinotoolbox....
// - vision/toolbox/visiontoolbox.h (commented out now!)
// - <no other uses, so -> remove it!>
/**
* normalizes a vector in homogenius coords
* to the scalingfactor 1
* @param vector size3
* @return vector size3, 3rd elemet = 1
*/
std::vector<float> normVector(const std::vector<float> &vec);



class Measurement1D {
public:
    Measurement1D();
    Measurement1D(float x, float confidence);

    float x;
    float confidence;
    float localConfidence;
};

class Measurement2D {
public:
    Measurement2D();
    Measurement2D(float x, float y, float confidence, TimestampMs ts = 0);
    Measurement2D operator-(Measurement2D const &rhs);
    float x;
    float y;
    float confidence;
    TimestampMs timestamp;
};


#include "coords.h"

///// following are some helpers for numbers / ranges / scaling

// test, and 'true'  if value is 'between' two other values
template <typename T>
inline bool between(const T &val, const T &lower, const T &upper) {
    return (val >= lower && val <= upper);
}


/*
// sign detect and return with same type (T)
template <typename T>
inline const T sgn(const T &val) {
    return (T(0) < val) - (val < T(0));
}
 */

// clip 'x' to range [min_val ... max_val]
template <typename T>
inline const T clip(const T &x, const T &min_val, const T &max_val) {
    if (x > max_val) {
        return max_val;
    }
    if (x < min_val) {
        return min_val;
    }
    return x;
}

// scale 'src_val' to target codomain [from ... to] default: [0 ... 1]
// using source values 'src_max', 'src_min' (safe -> check max/min)
template <typename T, typename S>
inline const T scale(const S &x, const S &src_min, const S &src_max,
                     const T &targ_from = 0.0f, const T &targ_to = 1.0f) {

    // shift, norm to [0..1], divide with own range
    // multiply with target range, shift to legal range
    const T out = (float(x - src_min) / float(src_max - src_min)) *
                  (targ_to - targ_from) + targ_from;

    // make sure the boundaries are not violated
    return clip(out, targ_from, targ_to);
}

// maps any 'x' to a gauss-distribution [targ_from ... targ_to]
// c1 -> defines how steep the values decrease near x=0 and f(0) itself
// c2 -> defines how early the outer bounds are hit, the higher the faster
template <typename T, typename S>
inline const S map2gauss(const T &x, const T &src_min, const T &src_max,
                         const S &targ_from = 0.0f,
                         const S &targ_to = 1.0f, const float &c1 = 1.2f,
                         const float &c2 = 9.0f) {

    // prescale (down to [0.0 ... 1.0])
    const S x2 = scale<S, T>(x, src_min, src_max);

    // calculate: general form used: f(x) = (1/c) * e^(-c^2 * x^2)
    const S y = 1.f - (1.f / c1) * std::exp(-c2 * (x2 * x2));

    // and scale back up, to target codomain
    return scale<S, T>(y, 0.0f, 1.0f, targ_from, targ_to);
}

//////////////////////// end of fully reviewed mathtoolbox!



typedef std::pair<Coord, Coord> line_t;

float angleOfIntersection(const line_t &m, const line_t &n);

Coord intersection(const line_t &m, const line_t &n);

// vim: set ts=4 sw=4 sts=4 expandtab:
