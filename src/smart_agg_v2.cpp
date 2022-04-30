#include <mpi.h>
#include <mpi_helper.h>
#include <vector>
#include <cassert>
#include <map>
#include "agg.h"
#include <numeric>
#include <future>

template <class T>
void SmartAggV2<T>::run(T* v, size_t n, int root, MPI_Comm comm) {
    if (beta <= 0) {
        throw std::logic_error("beta not set properly");
    }
    const int WORLD_SIZE = MPIHelper::get_world_size();
    const int rank = MPIHelper::get_rank();
    const auto MPI_TYPE = GET_TYPE<T>;

    assert(root < WORLD_SIZE);

    const double frac = get_fraction();
    const size_t lastn_rows = frac * n;
    const size_t firstn_rows = n - lastn_rows;

    const auto rank_m = (root + 1) % WORLD_SIZE; // collect firstn here

    constexpr static int LASTN_TAG = 0xf3a;
    constexpr static int FIRSTN_TAG = 0xa34f;

    auto recv_gather = [&comm, MPI_TYPE, rank](T* v, int n, const std::vector<int>& others,
                int TAG) {
        const auto osz = others.size();
        std::vector<std::unique_ptr<T[]>> bufs(osz);
        std::vector<MPI_Request> reqs(osz);
        for (auto& i: bufs)
            i = std::make_unique<T[]>(n);
        int k = 0;
        for (auto i: others) {
//            std::cerr << rank << " recv from: " << i << "\n";
            MPI_Irecv(bufs[k].get(), n, MPI_TYPE, i,
                      TAG, comm, &reqs[k]);
            k++;
        }
//        std::cerr << rank << " "
//                             ""
//                             "Waitall\n";
        MPI_Waitall(osz, reqs.data(), MPI_STATUSES_IGNORE);
        for (auto& j: bufs) {
            for (int i = 0; i < n; i++) {
                v[i] += j[i];
            }
        }
    };
    const auto RANK_LIST = [WORLD_SIZE]() {
        std::vector<int> ret(WORLD_SIZE);
        std::iota(ret.begin(), ret.end(), 0);
        return ret;
    }();
    MPI_Request reqs[3];
    int k = 0;
    if (rank == root) {
        auto rank_list = RANK_LIST;
        std::swap(rank_list[root], rank_list.back());
        rank_list.pop_back();
//        std::cerr << rank << " " << "recv_gather lastn_rows\n";
        recv_gather(v + n - lastn_rows, lastn_rows, rank_list, LASTN_TAG);
        // recv firstn_rows and
//        std::cerr << rank << " " << "recv_gather firstn_rows\n";
        recv_gather(v, firstn_rows, {rank_m}, FIRSTN_TAG);
    } else {
        // send lastn_rows to root
//        std::cerr << rank << " " << "send lastn_rows\n";
        MPI_Isend(v + n - lastn_rows, lastn_rows, MPI_TYPE,
                  root, LASTN_TAG, comm, reqs + k++);
        if (rank == rank_m) {
            auto rank_list = RANK_LIST;
            rank_list.erase(std::find(rank_list.begin(), rank_list.end(), root));
            rank_list.erase(std::find(rank_list.begin(), rank_list.end(), rank_m));
//            std::cerr << rank << " " << "recv_gather firstn_rows\n";
            recv_gather(v, firstn_rows, rank_list, FIRSTN_TAG);
//            std::cerr << rank << " " << "send firstn_rows\n";
            MPI_Isend(v, firstn_rows, MPI_TYPE, root, FIRSTN_TAG, comm, reqs + k++);
        } else {
            // send firstn_rows to rank_m
//            std::cerr << rank << " " << "send firstn_rows\n";
            MPI_Isend(v, firstn_rows, MPI_TYPE,
                      rank_m, FIRSTN_TAG, comm, reqs + k++);
        }
    }
    MPI_Waitall(k, reqs, MPI_STATUSES_IGNORE);
}

template class SmartAggV2<int>;