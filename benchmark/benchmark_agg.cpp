
//int sz = 100B;
//
//while (1) {
//int ts= 0;
//{
//ClockTracker loop_break;
//do_something(sz);
//if ((ts = loop_brea.get_ns()) > 5s) break;
//}
//Stats measu("smart_agg," + std::to_string(sz))
//for (i in 20s/ts times) { ClockTracker _(measu); do_something(sz); }
//sz *= 2;
//}

#include <mpi.h>
#include <bits/stdc++.h>
#include <unistd.h>
#include <chrono>
#include "mpi_helper.h"
#include "agg.h"
#include <algorithm>
#include "timing.hpp"

#define DISABLE_LOG

const size_t initialVectorSize = 1024;
const size_t maxVectorSize = 1e5;
const uint64_t MAX_TIME_LIMIT_RUN_SEC = 5;
const uint64_t MAX_TIME_LIMIT_ITER_SEC = 20;

enum class RunType {
    SIMPLE_AGG,
    SMART_AGG,
};

const RunType currentRun = RunType::SMART_AGG;

uint64_t convertToNanoSec(uint64_t x) {
    auto fac = (uint64_t) 1e9;
    return x * fac;
}

void testAgg(std::vector<int> &vec) {
    switch (currentRun) {
        case RunType::SIMPLE_AGG:
            SimpleAgg(false).run(vec, 0, MPI_COMM_WORLD);
            break;
        case RunType::SMART_AGG:
            std::vector<std::vector<int>> comm_groups;
            // host-3 is the weak link
            comm_groups.push_back({0, 1, 3});
            comm_groups.push_back({2, 0});
            SmartAgg(false).run(vec, comm_groups);
            break;
    }
    // We are not checking correctness here probably
}

std::vector<int> createVector(size_t vectorSize) {
    int rank = MPIHelper::get_rank();
    std::vector<int> vec(vectorSize);
    std::generate(vec.begin(), vec.end(), [&rank] { return rank += 1; });
    return vec;
}

void runTestAgg(std::string measureName) {
    int rank = MPIHelper::get_rank();
    size_t runningSize = initialVectorSize;
    while (true) {
        uint64_t ts = 0;
        std::vector<int> vec = createVector(runningSize);
        {
            Stats dummy;
            MPI_Barrier(MPI_COMM_WORLD);
            ClockTracker loopBreak(dummy);
            testAgg(vec);
            ts = loopBreak.get_ns();
            if (ts > convertToNanoSec(MAX_TIME_LIMIT_RUN_SEC)) break;
        }

        Stats measurement(measureName + "," + std::to_string(runningSize), rank == 0);
        for (uint64_t i = 0; i < convertToNanoSec(MAX_TIME_LIMIT_ITER_SEC) / ts; i++) {
            MPI_Barrier(MPI_COMM_WORLD);
            ClockTracker _(measurement);
            testAgg(vec);
        }
        runningSize *= 2;
        if (runningSize > maxVectorSize) break;
    }
}

void getHostnameDetails(int argc, char *argv[]) {
    char hostname[HOST_NAME_MAX + 1];
    gethostname(hostname, HOST_NAME_MAX + 1);

    std::cerr << hostname << " " << argc << " -> ";
    for (int i = 0; i < argc; i++)
        std::cerr << argv[i] << " | ";
    std::cout << "\n";
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    MPIHelper::init(MPI_COMM_WORLD);
    getHostnameDetails(argc, argv);

    std::string measureName;
    switch (currentRun) {
        case RunType::SMART_AGG:
            measureName = "smart_agg";
            break;
        case RunType::SIMPLE_AGG:
            measureName = "simple_agg";
            break;
    }

    runTestAgg(measureName);
    MPI_Finalize();
}