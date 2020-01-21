#include "platform.h"


#include <execinfo.h>
#include <sys/time.h>
#include <cstdlib>
#include <iostream>
#include <sstream>

std::string getTmpDir() {
    char *test = getenv("TMPDIR");
    if (!test) {
        return "/tmp/";
    }
    return std::string(test).append("/");
}



std::string getBacktrace() {
    std::ostringstream ostr;
    const int maxsz = 20;

    void *array[maxsz];
    char **strings;

    int size = backtrace(array, maxsz);
    strings = backtrace_symbols(array, size);

    ostr << "Obtained " << size << " stack frames.\n";

    for (int i = 0; i < size; i++) {
        ostr << strings[i] << "\n";
    }

    free(strings);

    return ostr.str();
}



microTime getMicroTime() {
    timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (microTime) ts.tv_sec * 1000000LL + (microTime) ts.tv_nsec / 1000LL;
}



TimestampMs getTimestampMs() {
    struct timeval tv;
    static int start = 0; // dirty hack to determine framework start time

    gettimeofday(&tv, NULL);
    if (start == 0) {
        start = tv.tv_sec;    // not really, but close enough to prevent TimestampMs from overflowing
    }
    TimestampMs ret = tv.tv_sec - start;
    ret *= 1000;
    ret += tv.tv_usec / 1000;
    //assert(ret >= 0);
    return ret;
}



void _exit(const std::string &msg) {
    std::cout << "---------------------" << std::endl;
    std::cout << "FRAMEWORK EXIT-CALLED" << std::endl;
    std::cout << "MSG: " << msg << std::endl;
    std::exit(1);
}

// vim: set ts=4 sw=4 sts=4 expandtab:
