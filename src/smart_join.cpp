#include "join.h"
#include <mpi.h>
#include <mpi_helper.h>
#include <unordered_set>
#include <numeric>

namespace {
    enum class MPI_CUSTOM_TAGS {
        SIZE_SEND_TAG, DATA_SEND_TAG
    };
}

template<class T>
SmartJoin<T>::SmartJoin(std::vector<std::vector<int>> &commGroupRanks_,
                                  std::vector<std::vector<int>> &group_dist) : commGroupRanks(commGroupRanks_) {
    MPI_Group world_group;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);
    assert(commGroupRanks.size() == group_dist.size());

    auto create_group = [&world_group](std::vector<int> &comm_group) {
        MPI_Group custom_grp;
        MPI_Group_incl(world_group, (int) comm_group.size(), comm_group.data(), &custom_grp);

        MPI_Comm custom_comm;
        MPI_Comm_create_group(MPI_COMM_WORLD, custom_grp, 0, &custom_comm);
        return custom_comm;
    };

    for (auto &comm_group: commGroupRanks) {
        MPI_Comm temp = create_group(comm_group);
        commList.push_back(temp);
    }

    for (auto &one_group_dist: group_dist)
        hashObjList.emplace_back(one_group_dist);
}

template<class T>
SmartJoin<T>::~SmartJoin<T>() {
    for (auto &comm: commList) {
        if (comm != MPI_COMM_NULL)
            MPI_Comm_free(&comm);
    }
}

template<class T>
[[nodiscard]] std::unordered_map<int, std::vector<T>>
SmartJoin<T>::partition_vector_intra(T *v, int n, int comm_pos) const {
    auto &hashObj = hashObjList[comm_pos];
    std::unordered_map<int, std::vector<T>> partition_results;
    for (int i = 0; i < n; i++)
        partition_results[hashObj.hash(v[i])].push_back(v[i]);
    return partition_results;
}

template<class T>
[[nodiscard]] std::unordered_map<int, std::vector<T>> SmartJoin<T>::partition_vector_inter(T *v, int n) const {
    std::unordered_map<int, std::vector<T>> partition_results;
    for (uint i = 0; i < commList.size(); i++) {
        auto &hashObj = hashObjList[i];
        for (int j = 0; j < n; j++) {
            int world_rank_partner = commGroupRanks[i][hashObj.hash(v[j])];;
            partition_results[world_rank_partner].push_back(v[j]);
        }
    }
    return partition_results;
}

template<class T>
[[nodiscard]] std::vector<T>
SmartJoin<T>::exchange_partitions(std::unordered_map<int, std::vector<T>> &partition_results, int comm_rank,
                                       int comm_size, MPI_Comm comm) {
    const auto MPI_TYPE = GET_TYPE<T>;
    std::vector<MPI_Request> size_send_requests(comm_size, MPI_REQUEST_NULL);
    std::vector<MPI_Request> data_send_requests(comm_size, MPI_REQUEST_NULL);

    for (int i = 0; i < comm_size; i++) {
        if (i == comm_rank) continue;
        int partition_size = partition_results[i].size();
        MPI_Issend(&partition_size, 1, MPI_INT, i, static_cast<int>(MPI_CUSTOM_TAGS::SIZE_SEND_TAG), comm,
                   &size_send_requests[i]);
        if (partition_size == 0) continue;
        std::vector<T> &partitionData = partition_results[i];
        MPI_Issend(partitionData.data(), partition_size, MPI_TYPE, i, static_cast<int>(MPI_CUSTOM_TAGS::DATA_SEND_TAG),
                   comm, &data_send_requests[i]);
    }

    std::vector<MPI_Request> size_receive_requests(comm_size, MPI_REQUEST_NULL);
    std::vector<int> partition_sizes(comm_size);
    for (int i = 0; i < comm_size; i++) {
        if (i == comm_rank) continue;
        MPI_Irecv(&partition_sizes[i], 1, MPI_INT, i, static_cast<int>(MPI_CUSTOM_TAGS::SIZE_SEND_TAG),
                  comm, &size_receive_requests[i]);
    }
    MPI_Waitall(comm_size, size_receive_requests.data(), MPI_STATUSES_IGNORE);

    std::vector<T> all_data(
            std::accumulate(partition_sizes.begin(), partition_sizes.end(), 0) + partition_results[comm_rank].size());
    uint all_data_counter = 0;

    for (int i = 0; i < comm_size; i++) {
        if (i == comm_rank || partition_sizes[i] == 0) continue;
        std::unique_ptr<T[]> buf = std::make_unique<T[]>(partition_sizes[i]);
        MPI_Status data_receive_status;
        MPI_Recv(buf.get(), partition_sizes[i], MPI_TYPE, i, static_cast<int>(MPI_CUSTOM_TAGS::DATA_SEND_TAG),
                 comm, &data_receive_status);

        int data_count;
        MPI_Get_count(&data_receive_status, MPI_TYPE, &data_count);
        assert(data_count == partition_sizes[i]);

        for (int j = 0; j < data_count; j++)
            all_data[all_data_counter++] = buf[j];
    }
    for (auto &x: partition_results[comm_rank])
        all_data[all_data_counter++] = x;

    MPI_Waitall(comm_size, size_send_requests.data(), MPI_STATUSES_IGNORE);
    MPI_Waitall(comm_size, data_send_requests.data(), MPI_STATUSES_IGNORE);

    return all_data;
}

template<class T>
[[nodiscard]] std::vector<T>
SmartJoin<T>::exchange_partitions_intra(std::unordered_map<int, std::vector<T>> &partition_results, int comm_pos) {
    int comm_rank, comm_size;
    MPI_Comm_rank(commList[comm_pos], &comm_rank);
    MPI_Comm_size(commList[comm_pos], &comm_size);

    return exchange_partitions(partition_results, comm_rank, comm_size, commList[comm_pos]);
}

template<class T>
[[nodiscard]] std::vector<T>
SmartJoin<T>::exchange_partitions_inter(std::unordered_map<int, std::vector<T>> &world_partitions) {
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    return exchange_partitions(world_partitions, world_rank, world_size, MPI_COMM_WORLD);
}

template<class T>
std::vector<T> SmartJoin<T>::findSetIntersection(std::vector<T> &v1, std::vector<T> &v2) {
    std::unordered_set<T> m(v1.begin(), v1.end());
    std::vector<T> final_elems;

    for (uint i = 0; i < v2.size(); i++) {
        const auto& [start, end] = m.equal_range(v2[i]);
        for (auto itr = start; itr != end; itr++) {
            final_elems.push_back(v2[i]);
        }
    }

    return final_elems;
}

template<class T>
std::vector<T> SmartJoin<T>::run(T *vR, T *vS, const uint nR, const uint nS) {
    std::vector<T> all_dataS;
    for (uint i = 0; i < commList.size(); i++) {
        if (commList[i] == MPI_COMM_NULL) continue;
        std::unordered_map<int, std::vector<T>> partition_results = partition_vector_intra(vS, nS, i);
        all_dataS = exchange_partitions_intra(partition_results, i);
    }

    std::unordered_map<int, std::vector<T>> world_partition_results = partition_vector_inter(vR, nR);
    std::vector<T> all_dataR = exchange_partitions_inter(world_partition_results);

    return findSetIntersection(all_dataR, all_dataS);
}

template<class T>
std::vector<T> SmartJoin<T>::run(std::vector<T> &vR, std::vector<T> &vS) {
    return run(vR.data(), vS.data(), vR.size(), vS.size());
}

template
class SmartJoin<int>;