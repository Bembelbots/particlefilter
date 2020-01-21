#include "visiondefinitions.h"

#include <stdlib.h>
#include <ostream>
#include <string>

VisionResult::VisionResult(const std::string &vals) :
    type(JSVISION_NOTHING),
    timestamp(0),
    ics_x1(0),
    ics_y1(0),
    ics_x2(0),
    ics_y2(0),
    ics_width(0),
    ics_height(0),
    ics_confidence(0.0f),
    rcs_x1(0.0f),
    rcs_y1(0.0f),
    rcs_x2(0.0f),
    rcs_y2(0.0f),
    rcs_alpha(0.0f),
    rcs_distance(0.0f),
    rcs_confidence(0.0f),
    camera(0),
    extra_int(0),
    extra_float(0.0f) {

    size_t p = vals.find(';');
    unsigned int c = 0;
    const size_t elementsCount = 18;
    std::string valsAggr = vals;
    while (p != std::string::npos || c == elementsCount) {
        std::string v = valsAggr.substr(0, p);
        if (c == 0) {
            type = static_cast<VisionClass>(atoi(v.c_str()));
        } else if (c == 1) {
            timestamp = atoi(v.c_str());
        } else if (c == 2) {
            ics_x1 = atoi(v.c_str());
        } else if (c == 3) {
            ics_y1 = atoi(v.c_str());
        } else if (c == 4) {
            ics_x2 = atoi(v.c_str());
        } else if (c == 5) {
            ics_y2 = atoi(v.c_str());
        } else if (c == 6) {
            ics_width = atoi(v.c_str());
        } else if (c == 7) {
            ics_height = atoi(v.c_str());
        } else if (c == 8) {
            ics_confidence = static_cast<float>(atof(v.c_str()));
        } else if (c == 9) {
            rcs_x1 = static_cast<float>(atof(v.c_str()));
        } else if (c == 10) {
            rcs_y1 = static_cast<float>(atof(v.c_str()));
        } else if (c == 11) {
            rcs_x2 = static_cast<float>(atof(v.c_str()));
        } else if (c == 12) {
            rcs_y2 = static_cast<float>(atof(v.c_str()));
        } else if (c == 13) {
            rcs_alpha = static_cast<float>(atof(v.c_str()));
        } else if (c == 14) {
            rcs_distance = static_cast<float>(atof(v.c_str()));
        } else if (c == 15) {
            rcs_confidence = static_cast<float>(atof(v.c_str()));
        } else if (c == 16) {
            camera = atoi(v.c_str());
        } else if (c == 17) {
            extra_int = atoi(v.c_str());
        } else if (c == 18) {
            extra_float = static_cast<float>(atof(v.c_str()));
        }

        if (c < elementsCount) {
            valsAggr = valsAggr.substr(p + 1, valsAggr.size());
            p = valsAggr.find(';');
        }
        ++c;
    }
}

std::ostream &operator<<(std::ostream &s, const VisionResult &r) {
    s   << static_cast<int>(r.type) << ";"
        << r.timestamp << ";"
        << r.ics_x1 << ";"
        << r.ics_y1 << ";"
        << r.ics_x2 << ";"
        << r.ics_y2 << ";"
        << r.ics_width << ";"
        << r.ics_height << ";"
        << r.ics_confidence << ";"
        << r.rcs_x1 << ";"
        << r.rcs_y1 << ";"
        << r.rcs_x2 << ";"
        << r.rcs_y2 << ";"
        << r.rcs_alpha << ";"
        << r.rcs_distance << ";"
        << r.rcs_confidence << ";"
        << r.camera << ";"
        << r.extra_int << ";"
        << r.extra_float;
    return s;
}


// vim: set ts=4 sw=4 sts=4 expandtab:
