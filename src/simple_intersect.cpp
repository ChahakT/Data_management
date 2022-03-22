#include <mpi.h>
#include <mpi_helper.h>
#include <vector>
#include <unordered_set>
#include "intersect.h"

template<class T>
std::vector<T> SimpleIntersect::run(std::vector<T> &vR, std::vector<T> &vS, MPI_Comm comm) {
    return run(vR.data(), vS.data(), vR.size(), vS.size(), comm);
}

template<class T>
std::vector<T> SimpleIntersect::run(T *vR, T *vS, const int nR, const int nS, MPI_Comm comm) {
    const int WORLD_SIZE = MPIHelper::get_world_size();
//    const int rank = MPIHelper::get_rank();
    const auto MPI_TYPE = GET_TYPE<T>;

    std::vector<T> buf(WORLD_SIZE * nR);
    MPI_Allgather(vR, nR, MPI_TYPE, buf.data(), nR, MPI_TYPE, comm);

    std::unordered_set<T> bufHash;
    for(auto val : buf) {
        bufHash.insert(val);
    }

    std::vector<T> result;
    for (int i = 0; i < nS; i++) {
        if(bufHash.find(vS[i]) != bufHash.end())
            result.push_back(vS[i]);
    }

    return result;
}

template std::vector<int> SimpleIntersect::run(std::vector <int> &vR, std::vector <int> &vS, MPI_Comm comm);
template std::vector<int> SimpleIntersect::run(int *vR, int *vS, const int nR, const int nS, MPI_Comm comm);
