#include <mpi.h>
#include <unistd.h>
#include <climits>
#include <chrono>
#include <map>
#include <vector>
#include <algorithm>
#include "mpi_helper.h"
#include "intersect.h"
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
void testIntersect(std::vector<int> &vR, std::vector<int> &vS, T&& obj) {
    if constexpr(R == RunType::SIMPLE_INTERSECT) {
        obj->run(vR, vS, MPI_COMM_WORLD);
    } else if constexpr(R == RunType::SMART_INTERSECT) {
        obj->run(vR, vS);
    }
    // We are not checking correctness here probably
}

std::vector<int> createVector(size_t vectorSize, size_t threshold) {
    size_t rank = MPIHelper::get_rank();
    std::vector<int> vec(vectorSize);
    std::iota(vec.begin(), vec.end(), threshold + rank);
    return vec;
}

template<RunType R>
auto getIntersectObject() {
    std::string measureName;
    if constexpr(R == RunType::SIMPLE_INTERSECT) {
        auto obj = std::make_unique<SimpleIntersect<int>>();
        return std::make_pair<std::string, std::unique_ptr<SimpleIntersect<int>>>("simple_intersect", std::move(obj));
    } else if constexpr(R == RunType::SMART_INTERSECT) {
        std::vector<std::vector<int>> comm_groups;
        comm_groups.push_back({0, 1});
        comm_groups.push_back({2, 3});

        std::vector<std::vector<int>> dist;
        dist.push_back({2, 2});
        dist.push_back({2, 2});

        auto obj = std::make_unique<SmartIntersect<int>>(comm_groups, dist);
        return std::make_pair<std::string, std::unique_ptr<SmartIntersect<int>>>("smart_intersect", std::move(obj));
    } else
        return std::make_pair("", std::nullopt);
}

template<RunType R>
void runTestIntersect() {
    int rank = MPIHelper::get_rank();
    size_t runningSize = initialVectorSize;
    auto[measureName, obj] = getIntersectObject<R>();
    while (true) {
        uint64_t ts = 0;
        auto vR = createVector(runningSize/2, runningSize/2);
        auto vS = createVector(runningSize, 0);
        {
            Stats dummy;
            MPI_Barrier(MPI_COMM_WORLD);
            ClockTracker loopBreak(dummy);
            testIntersect<R>(vR, vS, obj);
            ts = loopBreak.get_ns();
            if (ts > convertToNanoSec(MAX_TIME_LIMIT_RUN_SEC)) break;
        }
        {
            Stats dummy;
            ClockTracker loopBreak(dummy);
            Stats measurement(std::string(measureName) + "," + std::to_string(runningSize), rank == 0);
            uint64_t loopCount = (rank == 0) ? (uint64_t) convertToNanoSec(MAX_TIME_LIMIT_ITER_SEC) / ts : 0;
            loopCount = std::max<uint64_t>(loopCount, 5);
            loopCount = std::min<uint64_t>(loopCount, 1e4);
            MPI_Bcast(&loopCount, 1, GET_TYPE<int>, 0, MPI_COMM_WORLD);
            for (uint64_t i = 0; i < loopCount; i++) {
                MPI_Barrier(MPI_COMM_WORLD);
                ClockTracker _(measurement);
                testIntersect<R>(vR, vS, obj);
            }
        }
        runningSize = (size_t) ((double) runningSize * 1.5);
        if (runningSize > maxVectorSize) break;
    }
    std::cerr << "Completed tests rank " << rank << std::endl;
    MPI_Abort(MPI_COMM_WORLD, 0); // Change this hack if possible, not at all good
}

template<RunType R>
void runTestIntersectV2() {
  int rank = MPIHelper::get_rank();
  auto[measureName, obj] = getIntersectObject<R>();
  if(measureName == "") return;
  for(uint factor = 1; factor <= 1024; factor *= 2) {
    uint64_t ts = 0;
    const auto vR = createVector(maxVectorSize / factor, maxVectorSize - maxVectorSize / factor);
    const auto vS = createVector(maxVectorSize, 0);
    {
      Stats dummy;
      MPI_Barrier(MPI_COMM_WORLD);
      ClockTracker loopBreak(dummy);
      testIntersect<R>(vR, vS, obj);
      ts = loopBreak.get_ns();
      if (ts > convertToNanoSec(MAX_TIME_LIMIT_RUN_SEC)) break;
    }
    {
      Stats dummy;
      ClockTracker loopBreak(dummy);
      Stats measurement(measureName + "," + std::to_string(factor), rank == 0);
      uint64_t loopCount = (rank == 0) ? (uint64_t) convertToNanoSec(MAX_TIME_LIMIT_ITER_SEC) / ts : 0;
      loopCount = std::max<uint64_t>(loopCount, 5);
      loopCount = std::min<uint64_t>(loopCount, 1e4);
      MPI_Bcast(&loopCount, 1, GET_TYPE<int>, 0, MPI_COMM_WORLD);
      for (uint64_t i = 0; i < loopCount; i++) {
        MPI_Barrier(MPI_COMM_WORLD);
        ClockTracker _(measurement);
        testIntersect<R>(vR, vS, obj);
      }
    }
  }
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
  if constexpr (!(currentRun == RunType::SMART_INTERSECT
                  || currentRun == RunType::SIMPLE_INTERSECT))
                  return -1;
    MPI_Init(&argc, &argv);
    MPIHelper::init(MPI_COMM_WORLD);
    getHostnameDetails(argc, argv);

    if constexpr(intersectTest == 0)
      runTestIntersect<currentRun>();
    else
      runTestIntersectV2<currentRun>();
    MPI_Finalize();
}
