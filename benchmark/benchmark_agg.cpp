
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
#include <unistd.h>
#include <climits>
#include <cmath>
#include <cassert>
#include <chrono>
#include "mpi_helper.h"
#include "agg.h"
#include <algorithm>
#include "timing.hpp"

namespace {
    constexpr size_t initialVectorSize = 1024;
    constexpr size_t maxVectorSize = 1e5;
    constexpr uint64_t MAX_TIME_LIMIT_RUN_SEC = 1;
    constexpr uint64_t MAX_TIME_LIMIT_ITER_SEC = 3;
}

enum class RunType {
    SIMPLE_AGG,
    SMART_AGG,
};

const RunType currentRun = RunType::SIMPLE_AGG;

uint64_t convertToNanoSec(uint64_t x) {
    auto fac = (uint64_t) 1e9;
    return x * fac;
}

void testAgg(std::vector<int> &vec, void *ptr) {
    switch (currentRun) {
        case RunType::SIMPLE_AGG: {
            auto *pSimpleAgg = (SimpleAgg<int> *) ptr;
            pSimpleAgg->run(vec, 0, MPI_COMM_WORLD);
        }
            break;
        case RunType::SMART_AGG:
            auto *psmartAgg = (SmartAgg *) ptr;
            std::vector<std::vector<int>> comm_groups;
            // host-3 is the weak link
            comm_groups.push_back({1, 2});
            comm_groups.push_back({0, 1});
            psmartAgg->run(vec, comm_groups);
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

std::tuple<std::string, void*> getAggObject(size_t vectorSize) {
    std::string measureName;
    switch (currentRun) {
        case RunType::SIMPLE_AGG: {
            void *ptr = new SimpleAgg<int>(vectorSize);
            return {"simple_agg", ptr};
        }
        case RunType::SMART_AGG: {
            void *ptr = new SmartAgg(false);
            return {"smart_agg", ptr};
        }
    }
    return {"", nullptr};
}

void runTestAgg() {
    int rank = MPIHelper::get_rank();
    size_t runningSize = initialVectorSize;
    while (true) {
        uint64_t ts = 0;
        std::vector<int> vec = createVector(runningSize);
        auto [measureName, ptr] = getAggObject(runningSize);
        assert(ptr != nullptr);
        {
            Stats dummy;
            MPI_Barrier(MPI_COMM_WORLD);
            ClockTracker loopBreak(dummy);
            testAgg(vec, ptr);
            ts = loopBreak.get_ns();
            if (ts > convertToNanoSec(MAX_TIME_LIMIT_RUN_SEC)) break;
        }
        {
            Stats dummy;
            ClockTracker loopBreak(dummy);
            Stats measurement(measureName + "," + std::to_string(runningSize), rank == 0);
            uint loopCount = (rank == 0) ? (uint) convertToNanoSec(MAX_TIME_LIMIT_ITER_SEC) / ts : 0;
            MPI_Bcast(&loopCount, 1, GET_TYPE<int>, 0, MPI_COMM_WORLD);
            for (uint i = 0; i < loopCount; i++) {
                MPI_Barrier(MPI_COMM_WORLD);
                ClockTracker _(measurement);
                testAgg(vec, ptr);
            }
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

    runTestAgg();
    MPI_Finalize();
}