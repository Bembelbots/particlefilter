#pragma once
#include <string>


// type to keep unix stamp in microseconds
typedef int64_t microTime;

// type to keep unix stamp in milliseconds
typedef int32_t TimestampMs;


enum class FieldSize {
    JRL,
    SPL,
    HTWK,
    TINY,
    SPL2020
};

enum class RobotRole {
    NONE,
    STRIKER,
    DEFENDER,
    GOALKEEPER,
    SUPPORTER_DEFENSE,
    SUPPORTER_OFFENSE,
    SEARCHER,
    DEMO,
    PENALTYKICKER,
    PENALTYGOALIE
};

// SPL-gamecontroller game states
enum class GameState {
    INITIAL,
    READY,
    SET,
    PLAYING,
    FINISHED
};
