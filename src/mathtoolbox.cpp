#include <mathtoolbox.h>

#include <iostream>
#include <vector>
float lineToPointDist(const std::vector<float> &point,
                      const std::vector<float> &line) {
    return (point[0] * line[0] + point[1] * line[1] + line[2]) /
           ((fabsf(line[2]) / -line[2]) * hypotf(line[0], line[1]));
}

std::vector<float> vectorCrossProduct(const std::vector<float> &a,
                                      const std::vector<float> &b) {

    return normVector(vectorCrossProductUnNormed(a, b));
}

std::vector<float> vectorCrossProductUnNormed(const std::vector<float> &a,
        const std::vector<float> &b) {
    return std::vector<float> {  a[1] *b[2] - a[2] *b[1],
                                 a[2] *b[0] - a[0] *b[2],
                                 a[0] *b[1] - a[1] *b[0]
                              };
}

std::vector<float> getHesseNormalFormOfLine(const std::vector<float> &lineIn) {
    std::vector<float> line(lineIn);

    float sign = (fabsf(line[2]) / line[2]);
    if (std::isnan(sign)) {
        sign = 1.0f;
    }
    float hnf = sign * hypotf(line[0], line[1]);
    line[0] /= hnf;
    line[1] /= hnf;
    line[2] /= hnf;
    return line;
}

std::vector<float> normVector(const std::vector<float> &vecIn) {
    std::vector<float> vec(vecIn);
    // jsassert(3 == vec.size());
    if (0.0f < fabsf(vec[2])) {
        vec[0] /= vec[2];
        vec[1] /= vec[2];
        vec[2] /= vec[2];
        return vec;
    } else {
        return vec;
    }
}

Measurement1D::Measurement1D()
    : x(0.0f), confidence(0.0f), localConfidence(0.0f) {}

Measurement1D::Measurement1D(float x, float confidence)
    : x(x), confidence(confidence), localConfidence(0.0f) {}

Measurement2D::Measurement2D()
    : x(0.0f), y(0.0f), confidence(0.0f), timestamp(0) {}

Measurement2D::Measurement2D(float x, float y, float confidence,
                             TimestampMs ts)
    : x(x), y(y), confidence(confidence), timestamp(ts) {}

/*
 * calculate motion vector of two measurements
 * problem: what is the confidence of the difference, and which ts to use?
 * timestamp, difference between both measurements
 */
Measurement2D Measurement2D::operator-(Measurement2D const &rhs) {
    Measurement2D ret;
    ret.x = x - rhs.x;
    ret.y = y - rhs.y;
    ret.confidence = 1.0f;
    ret.timestamp = timestamp - rhs.timestamp;
    if (ret.timestamp < 0) {
        ret.timestamp = 0 - ret.timestamp;
    }
    return ret;
}

////////
////////
////////
////////
////////
////////
////////
////////
//// new stuff starts here using weird, fancym swift,  new features like:
// -> code reuse! -> class-freaking-objects -> abstraction -> RELIABILITY
////////



//// below this point, you'll find the "old" mathtoolbox.h
//// pull and adapt as much as possible ...

float averageAnglesRad(float alphaRad, float betaRad, float weightAlpha,
                       float weightBeta) {
    float rota1 = cosf(alphaRad);
    float rota2 = -sinf(alphaRad);

    float rotb1 = cosf(betaRad);
    float rotb2 = -sinf(betaRad);

    float rotc1 = weightAlpha * rota1 + weightBeta * rotb1;
    float rotc2 = weightAlpha * rota2 + weightBeta * rotb2;

    return atan2f(rotc2, rotc1);
}

float angleOfIntersection(const line_t &m, const line_t &n) {
    auto dm = m.first - m.second;
    auto dn = n.first - n.second;
    Angle a1 = Angle(static_cast<float>(atan2f(dm.y, dm.x)));
    Angle a2 = Angle(static_cast<float>(atan2f(dn.y, dn.x)));
    return a1.sub(a2).rad;
}

Coord intersection(const line_t &m, const line_t &n) {
    auto &x1 = m.first.x;
    auto &y1 = m.first.y;
    auto &x2 = m.second.x;
    auto &y2 = m.second.y;
    auto &x3 = n.first.x;
    auto &y3 = n.first.y;
    auto &x4 = n.second.x;
    auto &y4 = n.second.y;

    auto nx = (x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4);
    auto ny = (x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4);
    auto cr = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

    if (cr == 0) {
        std::cerr << "Mathtoolbox intersection: Lines parallel." << std::endl;
    }

    return {nx / cr, ny / cr};
}


// vim: set ts=4 sw=4 sts=4 expandtab:
