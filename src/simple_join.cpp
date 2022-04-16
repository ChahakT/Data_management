#include <mpi.h>
#include <mpi_helper.h>
#include <vector>
#include <unordered_set>
#include "join.h"

template<class T>
std::vector<T> SimpleJoin<T>::run(std::vector<T> &vR, std::vector<T> &vS, MPI_Comm comm) {
    return run(vR.data(), vS.data(), vR.size(), vS.size(), comm);
}

template<class T>
std::vector<T> SimpleJoin<T>::run(T *vR, T *vS, const int nR, const int nS, MPI_Comm comm) {
    const int WORLD_SIZE = MPIHelper::get_world_size();
    const auto MPI_TYPE = GET_TYPE<T>;

    std::vector<T> buf(WORLD_SIZE * nR);
    MPI_Allgather(vR, nR, MPI_TYPE, buf.data(), nR, MPI_TYPE, comm);

    std::unordered_multiset<T> bufHash(buf.begin(), buf.end());
    std::vector<T> result;
    for (int i = 0; i < nS; i++) {
        const auto& [start, end] = bufHash.equal_range(vS[i]);
        for (auto itr = start; itr != end; itr++) {
            result.push_back(vS[i]);
        }
    }
    return result;
}

template
class SimpleJoin<int>;
