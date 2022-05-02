
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
#include <chrono>
#include <map>
#include <vector>
#include <algorithm>
#include "mpi_helper.h"
#include "agg.h"
#include "timing.hpp"
#include "constants.h"

namespace {
    constexpr size_t initialVectorSize = 1024;
    constexpr size_t maxVectorSize =
            std::min<size_t>(std::numeric_limits<int>::max(), std::numeric_limits<size_t>::max()) / 4;
    constexpr uint64_t MAX_TIME_LIMIT_RUN_SEC = 5;
    constexpr uint64_t MAX_TIME_LIMIT_ITER_SEC = 10;
}

uint64_t convertToNanoSec(uint64_t x) {
    constexpr uint64_t NANOSECONDS = 1e9;
    return x * NANOSECONDS;
}

template<RunType R, class T>
void testAgg(std::vector<int> &vec, T& obj) {
    if constexpr(R == RunType::SIMPLE_AGG) {
        obj->run(vec, current_root, MPI_COMM_WORLD);
    } else if constexpr(R == RunType::SMART_AGG) {
        obj->run(vec);
    } else if constexpr(R == RunType::SMART_AGG_V2) {
      obj->run(vec, current_root);
    }
    // We are not checking correctness here probably
}

std::vector<int> createVector(size_t vectorSize) {
    int rank = MPIHelper::get_rank();
    std::vector<int> vec(vectorSize);
    std::generate(vec.begin(), vec.end(), [&rank] { return rank += 1; });
    return vec;
}

template<RunType R>
auto getAggObject(size_t vectorSize) {
    std::string measureName;
    if constexpr(R == RunType::SIMPLE_AGG) {
        auto obj = std::make_unique<SimpleAgg<int>>(vectorSize);
        return std::make_pair<std::string, std::unique_ptr<SimpleAgg<int>>>("simple_agg", std::move(obj));
    } else if constexpr(R == RunType::SMART_AGG) {
        std::map<int, std::vector<std::vector<int>>> stepCommInstructions = {
                {0, {{1, 2}}},
                {1, {{0, 1}}},
        };
        auto obj = std::make_unique<SmartAgg<int>>(stepCommInstructions);
        return std::make_pair<std::string, std::unique_ptr<SmartAgg<int>>>("smart_agg", std::move(obj));
    } else {
        auto obj = std::make_unique<SmartAggV2<int>>(BETA);
        return std::make_pair<std::string, std::unique_ptr<SmartAggV2<int>>>("smart_agg_v2", std::move(obj));
    }
}

template<RunType R>
void runTestAgg() {
    int rank = MPIHelper::get_rank();
    size_t runningSize = initialVectorSize;
    while (true) {
        uint64_t ts = 0;
        std::vector<int> vec = createVector(runningSize);
        auto[measureName, obj] = getAggObject<R>(runningSize);
        {
            Stats dummy;
            MPI_Barrier(MPI_COMM_WORLD);
            ClockTracker loopBreak(dummy);
            testAgg<R>(vec, obj);
            ts = loopBreak.get_ns();
            if (ts > convertToNanoSec(MAX_TIME_LIMIT_RUN_SEC)) break;
        }
        {
            Stats dummy;
            ClockTracker loopBreak(dummy);
            Stats measurement(measureName + "," + std::to_string(runningSize), rank == 0);
            uint loopCount = (rank == 0) ? (uint) convertToNanoSec(MAX_TIME_LIMIT_ITER_SEC) / ts : 0;
            loopCount = std::max<uint>(loopCount, 5);
            loopCount = std::min<uint>(loopCount, 1e4);
            MPI_Bcast(&loopCount, 1, GET_TYPE<int>, 0, MPI_COMM_WORLD);
            for (uint i = 0; i < loopCount; i++) {
                MPI_Barrier(MPI_COMM_WORLD);
                ClockTracker _(measurement);
                testAgg<R>(vec, obj);
            }
        }
        runningSize = (size_t) ((double) runningSize * 1.5);
        if (runningSize > maxVectorSize) break;
    }
    std::cerr << "Completed tests rank " << rank << std::endl;
    MPI_Abort(MPI_COMM_WORLD, 0); // Change this hack if possible, not at all good
}

void getHostnameDetails(int argc, char *argv[]) {
    char hostname[HOST_NAME_MAX + 1];
    gethostname(hostname, HOST_NAME_MAX + 1);

    std::cerr << hostname << " " << argc << " -> ";
    for (int i = 0; i < argc; i++)
        std::cerr << argv[i] << " | ";
    std::cerr << "\n";
}

int main(int argc, char *argv[]) {
    if constexpr (!(currentRun == RunType::SIMPLE_AGG
              || currentRun == RunType::SMART_AGG
              || currentRun == RunType::SMART_AGG_V2))
      return -1;
    MPI_Init(&argc, &argv);
    MPIHelper::init(MPI_COMM_WORLD);
    getHostnameDetails(argc, argv);

    runTestAgg<currentRun>();
    MPI_Finalize();
}