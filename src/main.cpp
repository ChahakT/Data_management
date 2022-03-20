#include <bits/stdc++.h>
#include <mpi.h>
#include <unistd.h>
#include "mpi_helper.h"

class SimpleAgg {
public:
    template <class T>
    void run(T* v, const int n, const int root, MPI_Comm comm);
    template <class T>
    void run(std::vector<T> &vec, const int root, MPI_Comm comm);
};

class SmartAgg {
public:
    template <class T>
    void run(T* v, const int n);
    template <class T>
    void run(std::vector<T> &vec);
};

int main (int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    MPIHelper::init(MPI_COMM_WORLD);
    const int rank = MPIHelper::get_rank();
    const int size = MPIHelper::get_world_size();

    char hostname[HOST_NAME_MAX + 1];
    gethostname(hostname, HOST_NAME_MAX + 1);

    std::cerr << hostname << " " << argc << " -> ";
    for (int i = 0; i < argc; i++)
        std::cerr << argv[i] << " | ";
    std::cout << "\n";

    std::vector<int> vec = {1 + rank, 2, 3 + rank};
    // [1 2 3] [2 2 4] [3 2 5] = [6 6 12]
    SmartAgg().run(vec);
    if (rank == 0) {
        std::cout << "\n[*] output = ";
        for (auto i: vec) {
            std::cout << i << " ";
        }
        std::cout << "\n";
    }

    MPI_Finalize();
}
