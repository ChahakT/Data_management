#pragma once
const uint seed = 15;

enum class RunType {
    SIMPLE_AGG,
    SMART_AGG,
    SMART_AGG_V2,
    SIMPLE_INTERSECT,
    SMART_INTERSECT,
    SIMPLE_JOIN,
    SMART_JOIN,
    NONE
};

#ifndef _RUNTYPE
#define _RUNTYPE NONE
#endif
#ifndef _BETA
#define _BETA 0.0
#endif

constexpr RunType currentRun = RunType:: _RUNTYPE;
constexpr double BETA = _BETA;
constexpr int intersectTest = _INTERSECT_V;
constexpr int current_root = 0;