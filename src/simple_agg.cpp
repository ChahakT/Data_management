#include <mpi.h>
#include <mpi_helper.h>
#include <vector>
#include "agg.h"

template <class T>
void SimpleAgg::run(std::vector<T> &v, const int root, MPI_Comm comm) {
    run(v.data(), v.size(), root, comm);
}

template <class T>
void SimpleAgg::run(T* v, const int n, const int root, MPI_Comm comm) {
    const int WORLD_SIZE = MPIHelper::get_world_size();
    const int rank = MPIHelper::get_rank();
    const auto  MPI_TYPE = GET_TYPE<T>;
    MPI_Request reqs[100];
    if (rank == 0) {
        std::vector<T*> buf(WORLD_SIZE-1);
        for (auto& i: buf) { i = new T[n]; i[0] = 999; }
        for (int i = 1; i < WORLD_SIZE; i++) {
            auto& bufj = buf[i-1];
            MPI_Irecv(bufj, n, MPI_TYPE, i, 1996, comm, reqs + i - 1);
        }
        MPI_Waitall(WORLD_SIZE - 1, reqs, MPI_STATUSES_IGNORE);
        if(showLog) {
            for (auto& bufj: buf) {
                std::cout << "\n[*]: ";
                for (int i = 0; i < n; i++) {
                    std::cout << bufj[i] << " ";
                    v[i] += bufj[i];
                }
            }
        }

        for (auto& i: buf) delete[] i;
    } else {
        if(showLog) std::cerr << "reached rank " << rank;
        MPI_Send(v, n, MPI_TYPE, root, 1996, comm);
        if(showLog) std::cerr << "Finished rank " << rank;
    }
}

template void SimpleAgg::run<int>(int*, const int, const int, MPI_Comm);
template void SimpleAgg::run<int>(std::vector<int>&, const int, MPI_Comm);