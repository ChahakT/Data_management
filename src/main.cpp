#include <bits/stdc++.h>
#include <mpi.h>
#include <unistd.h>
#include "mpi_helper.h"
#include "agg.h"
#include <algorithm>
#include "intersect.h"

enum class RunType {
    SIMPLE_AGG,
    SMART_AGG,
    SIMPLE_INTERSECT,
    SMART_INTERSECT,
};

const RunType currentRun = RunType::SIMPLE_INTERSECT;

void getHostnameDetails(int argc, char *argv[]) {
    char hostname[HOST_NAME_MAX + 1];
    gethostname(hostname, HOST_NAME_MAX + 1);

    std::cerr << hostname << " " << argc << " -> ";
    for (int i = 0; i < argc; i++)
        std::cerr << argv[i] << " | ";
    std::cout << "\n";
}

void testSimpleAggregation() {
    const int rank = MPIHelper::get_rank();
    std::vector<int> vec = {1 + rank, 2, 3 + rank};

    SimpleAgg().run(vec, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        std::cout << "\n[*] output = ";
        for (auto i: vec)
            std::cout << i << " ";
        std::cout << "\n";
    }
}

void testSmartAggregation() {
    const int rank = MPIHelper::get_rank();
    std::vector<int> vec = {1 + rank, 2, 3 + rank};
    // [1 2 3] [2 2 4] [3 2 5] = [6 6 12]
    std::vector<std::vector<int>> comm_groups;
    comm_groups.push_back({1, 2});
    comm_groups.push_back({0, 1});

    SmartAgg().run(vec, comm_groups);
    if (rank == 0) {
        std::cout << "\n[*] output = ";
        for (auto i: vec)
            std::cout << i << " ";
        std::cout << "\n";
    }
}

void testSimpleIntersection() {
    const int rank = MPIHelper::get_rank();
    const int WORLD_SIZE = MPIHelper::get_world_size();

    auto returnSection = [&rank, &WORLD_SIZE] (int n) {
        std::vector<int> v(WORLD_SIZE*n);
        std::iota(v.begin(), v.end(), 0);

        std::mt19937 g(seed);
        std::shuffle(v.begin(), v.end(), g);
        std::vector<int> returnV(v.begin() + rank*n, v.begin() + (rank+1)*n);

        std::cout << "\n[*] rank" << rank <<" vR/vS = ";
        for (auto i: returnV)
            std::cout << i << " ";
        std::cout << "\n";

        return returnV;
    };
    // vS = [0 1 2 3 4 5 6 7 8 9 10 11] -> shuffled
    // vR = [0 1 2 3 4 5 6 7 8] -> shuffled
    // common = [0 1 2 3 4 5 6 7 8] -> shuffled

    std::vector<int> vR = returnSection(nR);
    std::vector<int> vS = returnSection(nS);

    std::vector<int> vec = SimpleIntersect().run(vR, vS, MPI_COMM_WORLD);

    std::cout << "\n[*] rank" << rank <<" output = ";
    for (auto i: vec)
        std::cout << i << " ";
    std::cout << "\n";
}

void testSmartIntersection() {

}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    MPIHelper::init(MPI_COMM_WORLD);
    getHostnameDetails(argc, argv);

    std::map<RunType, std::function<void()>> functionMap = {
            {RunType::SIMPLE_AGG,       testSimpleAggregation},
            {RunType::SMART_AGG,        testSmartAggregation},
            {RunType::SIMPLE_INTERSECT, testSimpleIntersection},
            {RunType::SMART_INTERSECT,  testSmartIntersection},
    };

    functionMap[currentRun]();
    MPI_Finalize();
}
