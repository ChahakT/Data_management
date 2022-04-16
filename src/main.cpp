#include <unistd.h>
#include <climits>
#include <map>
#include <vector>
#include <numeric>
#include <random>
#include <mpi.h>
#include <join.h>
#include "mpi_helper.h"
#include "agg.h"
#include "intersect.h"
#include "constants.h"

enum class RunType {
    SIMPLE_AGG,
    SMART_AGG,
    SIMPLE_INTERSECT,
    SMART_INTERSECT,
    SIMPLE_JOIN
};

const RunType currentRun = RunType::SIMPLE_JOIN;

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

    SimpleAgg<int>(vec.size()).run(vec, 2, MPI_COMM_WORLD);
    if (rank == 2) {
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
    std::map<int, std::vector<std::vector<int>>> stepCommInstructions = {
            {0, {{2, 1}}},
            {1, {{0, 2}}},
    };
    SmartAgg<int>(stepCommInstructions).run(vec);
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

    auto returnSection = [&rank, &WORLD_SIZE](int n) {
        std::vector<int> v(WORLD_SIZE * n);
        std::iota(v.begin(), v.end(), 0);

        std::mt19937 g(seed);
        std::shuffle(v.begin(), v.end(), g);
        std::vector<int> returnV(v.begin() + rank * n, v.begin() + (rank + 1) * n);

        std::cout << "\n[*] rank" << rank << " vR/vS = ";
        for (auto i: returnV)
            std::cout << i << " ";
        std::cout << "\n";

        return returnV;
    };
    // vS = [0 1 2 3 4 5 6 7 8 9 10 11] -> shuffled
    // vR = [0 1 2 3 4 5 6 7 8] -> shuffled
    // common = [0 1 2 3 4 5 6 7 8] -> shuffled

    constexpr int nS = 4, nR = 3;
    std::vector<int> vR = returnSection(nR);
    std::vector<int> vS = returnSection(nS);

    std::vector<int> vec = SimpleIntersect<int>().run(vR, vS, MPI_COMM_WORLD);
    std::cout << "\n[*] rank" << rank << " output = ";
    for (auto i: vec)
        std::cout << i << " ";
    endl(std::cout);
}

void testSmartIntersection() {
    const int rank = MPIHelper::get_rank();
    const int WORLD_SIZE = MPIHelper::get_world_size();

    auto returnSection = [&rank, &WORLD_SIZE](int n) {
        std::vector<int> v(WORLD_SIZE * n);
        std::iota(v.begin(), v.end(), 0);

        std::mt19937 g(seed);
        std::shuffle(v.begin(), v.end(), g);
        std::vector<int> returnV(v.begin() + rank * n, v.begin() + (rank + 1) * n);

        std::cout << "\n[*] rank" << rank << " vR/vS = ";
        for (auto i: returnV)
            std::cout << i << " ";
        std::cout << "\n";

        return returnV;
    };
    // vS = [0 1 2 3 4 5 6 7 8 9 10 11] -> shuffled
    // vR = [0 1 2 3 4 5 6 7 8] -> shuffled
    // common = [0 1 2 3 4 5 6 7 8] -> shuffled

    constexpr int nS = 4, nR = 3;
    std::vector<int> vR = returnSection(nR);
    std::vector<int> vS = returnSection(nS);

    std::vector<std::vector<int>> comm_groups;
    comm_groups.push_back({0, 1});
    comm_groups.push_back({2, 3});

    std::vector<std::vector<int>> dist;
    dist.push_back({2, 2});
    dist.push_back({2, 2});

    std::vector<int> vec = SmartIntersect<int>(comm_groups, dist).run(vR, vS);
    std::cout << "\n[*] rank" << rank << " output = ";
    for (int &x: vec) std::cout << x << " ";
    endl(std::cout);
}

void testSimpleJoin() {
    const int rank = MPIHelper::get_rank();
    const int WORLD_SIZE = MPIHelper::get_world_size();

    auto returnSection = [&rank, &WORLD_SIZE](int n) {
        std::vector<int> v(WORLD_SIZE * n);
        std::iota(v.begin(), v.begin() + v.size() / 2, 0);
        std::iota(v.begin() + v.size() / 2, v.end(), 0);

        std::mt19937 g(seed);
        std::shuffle(v.begin(), v.end(), g);
        std::vector<int> returnV(v.begin() + rank * n, v.begin() + (rank + 1) * n);

        std::cout << "\n[*]Inital Values -> rank" << rank << " vR/vS = ";
        for (auto i: returnV)
            std::cout << i << " ";
        std::cout << "\n";

        return returnV;
    };

    constexpr int nS = 4, nR = 3;
    std::vector<int> vR = returnSection(nR);
    std::vector<int> vS = returnSection(nS);

    std::vector<int> vec = SimpleJoin<int>().run(vR, vS, MPI_COMM_WORLD);
    std::cout << "\nFinal OP -> [*] rank" << rank << " output = ";
    for (auto i: vec)
        std::cout << i << " ";
    endl(std::cout);
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
            {RunType::SIMPLE_JOIN,      testSimpleJoin},
    };

    functionMap[currentRun]();
    MPI_Finalize();
}
