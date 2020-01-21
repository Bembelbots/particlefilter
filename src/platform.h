#pragma once

#include <types.h>

#include <string>

// get tmp dir, including trailing slash or backslash
std::string getTmpDir();

// get a string containing a backtrace (gnu only)
std::string getBacktrace();

// get unix time-stamp in microseconds (1e-6 seconds)
microTime getMicroTime();

// get timestamp in milliseconds (1e-3 seconds)
TimestampMs getTimestampMs();

// custom exit
__attribute__((noreturn)) void _exit(const std::string &msg);




// vim: set ts=4 sw=4 sts=4 expandtab:
