#pragma once

#include <iosfwd>
#include <vector>
#include <cmath>

inline bool floatEQ(const float x, const float y) {
    return fabsf(x - y) < 0.00001;
}

/** Angle class (high-lvl angle handling)
 *  - automated normalization and constraining
 *  - change 'angle' through methods:
 *    - DO _NOT_ WRITE ::rad and ::deg directly
 *    - READING from ::rad and ::deg is encouraged and prefered
 **/
class Angle {
public:
    Angle();
    Angle(float rad);
    Angle(int deg);

    // parse from string, should match operator<<
    //static Angle fromString(const std::string& s);

    // normalization stuff, static to be used everywhere EXCLUSIVLY!
    // radians --> [-pi ... pi]
    // degree  --> [-180 ... 180]
    static float normalize(const float &rad);
    static double normalize(const double &rad);
    static int normalize(const int &deg);

    // these are NOT TO BE USED, nevertheless it's too late,
    // so those are better kept here!
    // radians --> [0 ... 2*pi]
    // degree  --> [0 ... 360]
    static float pos_normalize(const float &rad);
    static double pos_normalize(const double &rad);
    static int pos_normalize(const int &deg);

    // use these to change the underlying angle
    Angle &set(const float &rad);
    Angle &set(const int &deg);
    Angle &set(const Angle &obj);

    Angle &add(const Angle &other);
    Angle &sub(const Angle &other);
    // TODO NEED THEM TOO TODO
    // Angle& add(const float& rad);
    // Angle& add(const int& deg);
    // Angle& sub(const float& rad);
    // Angle& sub(const int& deg);
    
    Angle clamp(float maxVal) const;
    // returns new Angle-object with shortest(!) turn dist between this and 'ang'
    // for [-180 ... 180] and [-pi ... pi] constrained angles,
    // this is always just the _normalized_ difference ;)
    Angle dist(const Angle &ang) const;

    // (weighted) merge two angles ... this is not as simple as one might think
    // nevertheless, it's never used by anyone inside the framework, thus
    // mark it here as potential TODO, but don't implement it...
    Angle merge(const Angle &other, const float &my_weight = 1.f,
                const float &other_weight = 1.f);

    Angle operator-(const Angle &other) const;
    Angle operator+(const Angle &other) const;
    Angle operator*(const Angle &other) const;

    // return the (calculated) degree equivalent of this angle
    // [-180 .. 180] (left negative)
    //int deg() const; TODO FIXME
    int deg;

    // UNDERLYING DATA ITEM
    // [-pi .. pi] (counter-clock wise)
    float rad;
};
// TODO: operators should return by-value, by default!

/** Coord class (high-lvl coordinate handling) */
class DirectedCoord;
class Coord {
public:
    Coord();
    Coord(const float &x, const float &y);
    Coord(const int &x, const int &y);
    Coord(const std::vector<float> &xy);

    Coord(const Angle &); // create unit vector for this angle
 
    float norm2() const;
    // parse from string, should match operator<<
    //static Coord fromString(const std::string& s);

    Coord normalized() const;

    Coord clamp(float maxVal) const;

    float direction() const;


    // get distance from 'target' to this coord
    float dist(const Coord &target) const;

    // get distance from Coord(0,0) (coords origin) to this coord
    float dist() const;

    // get angle for this coordinate (from Coord(0,0))
    Angle angle() const;

    float dot(const Coord &other) const;

    // get angle from 'target' arbitrary coordinate to this Coord TODO!!!!
    // Angle angle(const Coord& target) const;

    // add 'other' to this coordinate
    Coord &add(const Coord &other);
    // substract 'other' from this coordinate
    Coord &sub(const Coord &other);

    Coord rotate(const Angle &) const;

    Coord operator-(const Coord &other) const;
    Coord operator+(const Coord &other) const;
    Coord operator*(const Coord &other) const;

    inline Coord& operator-=(const Coord &other) {
        return *this = *this - other;
    }

    inline Coord& operator+=(const Coord &other) {
        return *this = *this +  other;
    }


    friend Coord operator*(float scalar, const Coord &other);
    friend Coord operator*(const Coord &other, float scalar);
    friend Coord operator/(float scalar, const Coord &other);
    friend Coord operator/(const Coord &other, float scalar);

    bool operator==(const Coord &other);
    bool operator!=(const Coord &other);

    Coord &operator/=(float scalar);

    // add 'dist' towards 'dir' to coordinate
    Coord &add(const Angle &dir, const float &dist);

    // get distance from coord to line with startpoint start and endpoint end
    Coord closestPointOnLine(Coord start, Coord end, bool onLine = false) const;


    DirectedCoord lookAt(const Coord &other);

    float x;
    float y;
};

// what belongs together->belongs together ;D
class DirectedCoord {
public:
    DirectedCoord();

    DirectedCoord(const DirectedCoord &other);

    // parse from string, should match operator<<
    //static DirectedCoord fromString(const std::string& s);

    // TODO / FIXME: STOP USING vector<float> as coord / coord+angle!!!!
    // as long as it still exists in the framework, we need this:
    DirectedCoord(const std::vector<float> &vals);

    DirectedCoord(const Angle &a, const Coord &c);
    DirectedCoord(const Coord &c, const Angle &a);  // hrhr take them' all ;D

    DirectedCoord(const float &x, const float &y, const float &rad);
    // DirectedCoord(const float& x, const float& y, const int& deg);
    // DirectedCoord(const int& x, const int& y, const float& rad);
    // DirectedCoord(const int& x, const int& y, const int& deg);

    DirectedCoord &add(const DirectedCoord &other);
    DirectedCoord &sub(const DirectedCoord &other);

    // TODO: need this badly, this is already somewhere
    // Coord get_intersection(const Angle& o_angle);

    //DirectedCoord normalized() const;
    DirectedCoord clamp(float maxVal) const;

    // walkTo goes starts from one coordinate and walks a relative path
    // x.walk(y) starts at x with direction x.angle
    // and walks by y and turns by angle y.angle
    // (same as toRCS but with correct angles)
    DirectedCoord walk(const DirectedCoord &delta) const;

    // transform to RCS/WCS (UNTESTED=?=)
    DirectedCoord toRCS(const DirectedCoord &origin) const;
    DirectedCoord toWCS(const DirectedCoord &origin) const;

    DirectedCoord operator=(const DirectedCoord &other);
    DirectedCoord &operator+=(const DirectedCoord &other);

    DirectedCoord operator-(const DirectedCoord &other) const;
    DirectedCoord operator+(const DirectedCoord &other) const;
    DirectedCoord operator*(const DirectedCoord &other) const;

    bool isNull() const;

    Angle angle;
    Coord coord;
};


std::ostream &operator<<(std::ostream &s, const Angle &obj);
std::ostream &operator<<(std::ostream &s, const Coord &obj);
std::ostream &operator<<(std::ostream &s, const DirectedCoord &obj);



// vim: set ts=4 sw=4 sts=4 expandtab:
