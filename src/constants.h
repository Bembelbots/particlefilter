#pragma once


// pi (float)
#define M_PI_F   3.14159265358979323846f

// factor to be multiplied with a radiant to convert it to degree
#ifndef RAD_TO_DEG
#define RAD_TO_DEG (180.0f / M_PI_F)
#endif
// and vice-versa
#ifndef DEG_TO_RAD
#define DEG_TO_RAD (M_PI_F / 180.0f)
#endif

// vim: set ts=4 sw=4 sts=4 expandtab:
