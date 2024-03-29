#include <mpi.h>
#include <mpi_helper.h>
#include <vector>
#include "agg.h"

namespace {
    constexpr int TAG = 1996;
}

template<class T>
SimpleAgg<T>::SimpleAgg(size_t n) {
    const int WORLD_SIZE = MPIHelper::get_world_size();
}

template<class T>
void SimpleAgg<T>::run(std::vector<T> &v, const int root, MPI_Comm comm) {
    run(v.data(), v.size(), root, comm);
}

template<class T>
void SimpleAgg<T>::run(T *v, const size_t n, const int root, MPI_Comm comm) {
    const int WORLD_SIZE = MPIHelper::get_world_size();
    const int rank = MPIHelper::get_rank();
    const auto MPI_TYPE = GET_TYPE<T>;

    MPI_Request reqs[100];
    if (rank == root) {
        std::vector<std::unique_ptr<T[]>> buf(WORLD_SIZE-1);
        for (auto& i: buf) { i = std::make_unique<T[]>(n); }

        int counter = 0;
        for (int i = 0; i < WORLD_SIZE; i++) {
            if(i == root) continue;
            auto& bufJ = buf[counter];
            MPI_Irecv(bufJ.get(), n, MPI_TYPE, i, TAG, comm, reqs +counter);
            counter++;
        }
        MPI_Waitall(WORLD_SIZE - 1, reqs, MPI_STATUSES_IGNORE);
        for (auto &bufJ: buf) {
            for (size_t i = 0; i < n; i++)
                v[i] += bufJ[i];
        }
    } else {
        MPI_Send(v, n, MPI_TYPE, root, TAG, comm);
    }
}

template class SimpleAgg<int>;