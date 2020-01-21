#include "coords.h"
#include <constants.h>
#include <vector>
#include <ostream>
#include <cassert>


Coord::Coord() : x(0.0f), y(0.0f) {}

Coord::Coord(const float &x, const float &y) : x(x), y(y) {}

Coord::Coord(const int &x, const int &y)
    : x(static_cast<float>(x)), y(static_cast<float>(y)) {}

Coord::Coord(const std::vector<float> &xy) {
    assert(xy.size() == 2);
    x = xy.at(0);
    y = xy.at(1);
}

Coord::Coord(const Angle &theta)
    : Coord(1.f, 0.f) {
    *this = rotate(theta);
}

float Coord::norm2() const{
    return std::sqrt(x*x+y*y);
}

float Coord::dot(const Coord &other) const{
    return (x*other.x+y*other.y);
}

Coord Coord::normalized() const{
    return Coord(x/this->norm2(), y/this->norm2());
}

Coord Coord::clamp(float maxVal) const{
    float maxVal_tmp = std::max(std::abs(x),std::abs(y));
    float factor;
    factor = (maxVal_tmp != 0) ? maxVal/maxVal_tmp : 0.0;
    if (std::abs(factor)<1.0){
        return Coord(x*std::abs(factor),y*std::abs(factor));
    }
    else{
        return Coord(x,y);
    }
}

float Coord::direction() const{
    return atan2f(y, x);
}

float Coord::dist(const Coord &target) const {
    float dx = target.x - x;
    float dy = target.y - y;
    return hypotf(dx, dy);
}

float Coord::dist() const {
    return hypotf(x, y);
}

Angle Coord::angle() const {
    return Angle(atan2f(y, x));
}

Coord Coord::rotate(const Angle &angle) const {
    float alpha = angle.rad;
    float cosalpha = cosf(alpha);
    float sinalpha = sinf(alpha);

    Coord out(*this);
    float x = out.x * cosalpha - out.y * sinalpha;
    out.y = out.x * sinalpha + out.y * cosalpha;
    out.x = x;
    
    return out;
}

Coord &Coord::add(const Coord &other) {
    x += other.x;
    y += other.y;
    return *this;
}

Coord &Coord::sub(const Coord &other) {
    x -= other.x;
    y -= other.y;
    return *this;
}

Coord Coord::operator-(const Coord &other) const {
    return {x - other.x, y - other.y};
}

Coord Coord::operator+(const Coord &other) const {
    return {x + other.x, y + other.y};
}

Coord Coord::operator*(const Coord &other) const {
    return {x * other.x, y * other.y};
}

Coord operator*(float scalar, const Coord &other) {
    return {scalar * other.x, scalar * other.y};
}

Coord operator*(const Coord &other, float scalar) {
    return {other.x * scalar, other.y * scalar};
}

Coord operator/(float scalar, const Coord &other) {
    return {scalar / other.x, scalar / other.y};
}

Coord &Coord::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    return *this;
}

bool Coord::operator==(const Coord &other) {
    return floatEQ(other.x, x) && floatEQ(other.y, y);
}

bool Coord::operator!=(const Coord &other) {
    return !(*this == other);
}

Coord operator/(const Coord &other, float scalar) {
    return {other.x / scalar, other.y / scalar};
}

Coord &Coord::add(const Angle &dir, const float &dist) {
    x += dist * cosf(dir.rad);
    y += dist * sinf(dir.rad);
    return *this;
}



Coord Coord::closestPointOnLine(Coord start, Coord end,bool onLine) const {
    //berrechne Faktor der Geraden, mit der der Punkt auf der Geraden bestimmt werden kann, der die kürzeste Entfernung zu this hat
    //skalarprodukt: (gerade -Punkt) * (end-start) =0
    if (((end.x - start.x) + (end.y - start.y)) == 0) {
        return Coord(start.x, start.y);
    }

    float dxSquared = end.x - start.x;
    dxSquared *= dxSquared;

    float dySquared = end.y - start.y;
    dySquared *= dySquared;
    float scale = -(((start.x - x) * (end.x - start.x)) + ((start.y - y) *
                    (end.y - start.y)))
                  / (dxSquared + dySquared);
    if (onLine) {
        scale = std::max(0.0f, std::min(1.f,scale));//point shoudl be on line
    }
    return Coord((start.x + scale * (end.x - start.x)),
                 start.y + scale * (end.y - start.y));
};

DirectedCoord DirectedCoord::clamp(float maxVal) const{
    return DirectedCoord(coord.clamp(maxVal),angle.clamp(maxVal));
}

DirectedCoord Coord::lookAt(const Coord &other) {
    float x = this->x;
    float y = this->y;
    float dx = other.x - x;
    float dy = other.y - y;
    float angle = atan2f(dy, dx);
    DirectedCoord result(x, y, angle);
    return result;
}


DirectedCoord DirectedCoord::walk(const DirectedCoord &delta) const {
    // The same as toRCS() but fixed
    // Didn't touch the other one because of backwards compatibility fear
    float x1 = this->coord.x;
    float y1 = this->coord.y;
    float a1 = this->angle.rad;
    float x2 = delta.coord.x;
    float y2 = delta.coord.y;
    float a2 = delta.angle.rad;
    DirectedCoord out(0, 0, 0);
    // Transformation Matrix
    //out.angle = Angle().normalize( a1 + a2);
    out.angle.set(a1 + a2);
    out.coord.x = x1 + cosf(out.angle.rad) * x2 - sinf(out.angle.rad) * y2;
    out.coord.y = y1 + sinf(out.angle.rad) * x2 + cosf(out.angle.rad) * y2;
    return out;
}

DirectedCoord DirectedCoord::toRCS(const DirectedCoord &mypos) const {
    DirectedCoord out(this->coord.x, this->coord.y, this->angle.rad);
    // transform from wcs to rcs
    // 1) POSE
    // 1a) add translation
    out.coord.x -= mypos.coord.x;
    out.coord.y -= mypos.coord.y;
    // 1b) rotate local coordinates according to alpha!
    float alpha = -mypos.angle.rad;
    float cosalpha = cosf(alpha);
    float sinalpha = sinf(alpha);
    float x = out.coord.x * cosalpha - out.coord.y * sinalpha;
    out.coord.y = out.coord.x * sinalpha + out.coord.y * cosalpha;
    out.coord.x = x;
    return out;
}

DirectedCoord DirectedCoord::toWCS(const DirectedCoord &mypos) const {
    DirectedCoord out(this->coord.x, this->coord.y, this->angle.rad);
    // 1) POSE
    // 1a) de-rotate local coordinates according to alpha!
    float alpha = mypos.angle.rad;
    float cosalpha = cosf(alpha);
    float sinalpha = sinf(alpha);
    float x = out.coord.x * cosalpha - out.coord.y * sinalpha;
    out.coord.y = out.coord.x * sinalpha + out.coord.y * cosalpha;
    out.coord.x = x;
    // 1b) add translation
    out.coord.x += mypos.coord.x;
    out.coord.y += mypos.coord.y;

    return out;
}

Angle::Angle() : deg(0), rad(0.0f) {}

Angle::Angle(float r) {
    set(r);
}

Angle::Angle(int d) {
    set(d);
}

float Angle::normalize(const float &radian) {
    return remainder(radian, 2.f * M_PI_F);
}

double Angle::normalize(const double &radian) {
    return remainder(radian, 2.f * M_PI);
}

int Angle::normalize(const int &degree) {
    return remainder(degree, 360);
}

float Angle::pos_normalize(const float &radian) {
    const float &out = Angle::normalize(radian);
    return (out < 0.f) ? out + 2.f * M_PI_F : out;
}

double Angle::pos_normalize(const double &radian) {
    const double &out = Angle::normalize(radian);
    return (out < 0.0) ? out + 2.0 * M_PI : out;
}

int Angle::pos_normalize(const int &degree) {
    const int &out = Angle::normalize(degree);
    return (out < 0) ? out + 360 : out;
}

Angle Angle::operator-(const Angle &other) const {
    return {this->rad - other.rad};
}

Angle Angle::operator+(const Angle &other) const {
    return {this->rad + other.rad};
}

Angle Angle::operator*(const Angle &other) const {
    return {this->rad * other.rad};
}

Angle &Angle::set(const Angle &obj) {
    this->rad = obj.rad;
    this->deg = obj.deg;
    return *this;
}

Angle &Angle::set(const float &r) {
    this->rad = Angle::normalize(r);
    this->deg = static_cast<int>(roundf(RAD_TO_DEG * r));
    return *this;
}

Angle &Angle::set(const int &d) {
    this->deg = Angle::normalize(d);
    this->rad = static_cast<float>(DEG_TO_RAD * d);
    return *this;
}

Angle &Angle::add(const Angle &other) {
    this->set(other.rad + this->rad);
    return *this;
}

Angle &Angle::sub(const Angle &other) {
    this->set(other.rad - this->rad);
    return *this;
}

Angle Angle::clamp(float maxVal) const{
    float factor;
    factor =  ( rad != 0) ? maxVal/rad : 0.0f;
    if (std::abs(factor)<1.0f){
        return Angle(rad*std::abs(factor));
    }
    else{
        return Angle(rad);
    }
}

Angle Angle::dist(const Angle &ang) const {
    return Angle(*this - ang);
}

Angle Angle::merge(const Angle &other, const float &my_weight,
                   const float &other_weight) {
    float a1 = Angle::normalize(rad);
    float a2 = Angle::normalize(other.rad);
    float diff = a2 - a1;

    static const float pi2 = M_PI_F * 2.f;
    if (diff > M_PI_F) {
        a1 += pi2;
    } else if (diff < -M_PI_F) {
        a1 -= pi2;
    }

    float m = ((my_weight * a1) + (other_weight * a2));
    m /= (my_weight + other_weight);

    return Angle::normalize(m);
}

DirectedCoord::DirectedCoord() : angle(Angle()), coord(Coord()) {}

DirectedCoord::DirectedCoord(const float &x, const float &y, const float &rad)
    : angle( {
    rad
}), coord(Coord(x, y)) {}

DirectedCoord::DirectedCoord(const Angle &a, const Coord &c)
    : angle(a), coord(c) {}

DirectedCoord::DirectedCoord(const Coord &c, const Angle &a)
    : angle(a), coord(c) {}

DirectedCoord::DirectedCoord(const DirectedCoord &other)
    : angle(other.angle), coord(other.coord) {}

DirectedCoord::DirectedCoord(const std::vector<float> &vals)
    : angle( {
    vals.at(2)
}), coord(vals.at(0), vals.at(1)) {}

DirectedCoord &DirectedCoord::add(const DirectedCoord &other) {
    this->angle.add(other.angle);
    this->coord.add(other.coord);
    return *this;
}

DirectedCoord &DirectedCoord::sub(const DirectedCoord &other) {
    this->angle.sub(other.angle);
    this->coord.sub(other.coord);
    return *this;
}

DirectedCoord DirectedCoord::operator=(const DirectedCoord &other) {
    angle = other.angle;
    coord = other.coord;
    return *this;
}

DirectedCoord DirectedCoord::operator-(const DirectedCoord &other) const {
    return {this->angle - other.angle, this->coord - other.coord};
}

DirectedCoord DirectedCoord::operator+(const DirectedCoord &other) const {
    return {this->angle + other.angle, this->coord + other.coord};
}

DirectedCoord DirectedCoord::operator*(const DirectedCoord &other) const {
    return {this->angle * other.angle, this->coord * other.coord};
}

DirectedCoord &DirectedCoord::operator+=(const DirectedCoord &other) {
    this->coord = this->coord + other.coord;
    this->angle = this->angle + other.angle;
    return *this;
}

bool DirectedCoord::isNull() const {
    return coord.x == 0 && coord.y == 0 && angle.rad == 0;
}

std::ostream &operator<<(std::ostream &s, const Angle &obj) {
    return (s << obj.rad << "(rad)");
}

std::ostream &operator<<(std::ostream &s, const Coord &obj) {
    return (s << obj.x << "," << obj.y);
}

std::ostream &operator<<(std::ostream &s, const DirectedCoord &obj) {
    return (s << obj.coord << "@" << obj.angle);
}


// vim: set ts=4 sw=4 sts=4 expandtab:
