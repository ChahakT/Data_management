#include <bits/stdc++.h>
#include <mpi.h>
#include <unistd.h>
#include "mpi_helper.h"

class SimpleAgg {
    template <class T>
    void run(T* v, const int n, const int root, MPI_Comm comm);
};

int main (int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    MPIHelper::init(MPI_COMM_WORLD);

    char hostname[HOST_NAME_MAX + 1];
    gethostname(hostname, HOST_NAME_MAX + 1);

    std::cerr << hostname << " " << argc << " -> ";
    for (int i = 0; i < argc; i++)
        std::cerr << argv[i] << " | ";
    std::cout << "\n";


    MPI_Finalize();
}
