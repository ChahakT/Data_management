#include "intersect.h"
#include "iostream"
#include <mpi.h>
#include <mpi_helper.h>
#include <vector>
#include <numeric>

enum class MPI_CUSTOM_TAGS {
    SIZE_SEND_TAG, DATA_SEND_TAG
};

template<class T>
SmartIntersect<T>::SmartIntersect(std::vector<std::vector<int>> &commGroupRanks_,
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
void SmartIntersect<T>::run(std::vector<T> &vR, std::vector<T> &vS) {
    run(vR.data(), vS.data(), vR.size(), vS.size());
}

template<class T>
[[nodiscard]] std::unordered_map<int, std::vector<T>> SmartIntersect<T>::partition_vector(T *v, int n, int comm_pos) {
    auto &hashObj = hashObjList[comm_pos];
    std::unordered_map<int, std::vector<T>> partition_results;
    for (int i = 0; i < n; i++)
        partition_results[hashObj.template hash(v[i])].push_back(v[i]);
    return partition_results;
}

template<class T>
void SmartIntersect<T>::exchangePartitions(std::unordered_map<int, std::vector<T>> &partition_results, int comm_pos) {
    const auto MPI_TYPE = GET_TYPE<T>;
    int comm_rank;
    int comm_size;
    MPI_Comm_rank(commList[comm_pos], &comm_rank);
    MPI_Comm_size(commList[comm_pos], &comm_size);

    MPI_Request size_send_requests[comm_size];
    for (int i = 0; i < comm_size; i++) size_send_requests[i] = MPI_REQUEST_NULL;
    MPI_Request data_send_requests[comm_size];
    for (int i = 0; i < comm_size; i++) data_send_requests[i] = MPI_REQUEST_NULL;

    for (int i = 0; i < comm_size; i++) {
        if (i == comm_rank) continue;
        int partition_size = partition_results[i].size();
        MPI_Issend(&partition_size, 1, MPI_INT, i, static_cast<int>(MPI_CUSTOM_TAGS::SIZE_SEND_TAG), commList[comm_pos],
                   &size_send_requests[i]);
        if (partition_size == 0) continue;
        std::vector<T> &partitionData = partition_results[i];
        MPI_Issend(partitionData.data(), partition_size, MPI_TYPE, i, static_cast<int>(MPI_CUSTOM_TAGS::DATA_SEND_TAG),
                   commList[comm_pos], &data_send_requests[i]);
    }

    MPI_Request size_receive_requests[comm_size];
    for (int i = 0; i < comm_size; i++) size_receive_requests[i] = MPI_REQUEST_NULL;
    std::vector<int> partition_sizes(comm_size);
    for (int i = 0; i < comm_size; i++) {
        if (i == comm_rank) continue;
        MPI_Irecv(&partition_sizes[i], 1, MPI_INT, i, static_cast<int>(MPI_CUSTOM_TAGS::SIZE_SEND_TAG),
                  commList[comm_pos], &size_receive_requests[i]);
    }
    MPI_Waitall(comm_size, size_receive_requests, MPI_STATUSES_IGNORE);

    std::vector<T> all_data(
            std::accumulate(partition_sizes.begin(), partition_sizes.end(), 0) + partition_results[comm_rank].size());
    uint all_data_counter = 0;

    for (int i = 0; i < comm_size; i++) {
        if (i == comm_rank || partition_sizes[i] == 0) continue;
        std::unique_ptr<T[]> buf = std::make_unique<T[]>(partition_sizes[i]);
        MPI_Status data_receive_status;
        MPI_Recv(buf.get(), partition_sizes[i], MPI_TYPE, i, static_cast<int>(MPI_CUSTOM_TAGS::DATA_SEND_TAG),
                 commList[comm_pos], &data_receive_status);

        int data_count;
        MPI_Get_count(&data_receive_status, MPI_TYPE, &data_count);
        assert(data_count == partition_sizes[i]);

        for (int j = 0; j < data_count; j++)
            all_data[all_data_counter++] = buf[j];
    }
    for ([[maybe_unused]] auto &x: partition_results[comm_rank])
        all_data[all_data_counter++] = x;

    MPI_Waitall(comm_size, size_send_requests, MPI_STATUSES_IGNORE);
    MPI_Waitall(comm_size, data_send_requests, MPI_STATUSES_IGNORE);

}

template<class T>
void SmartIntersect<T>::run(T *vR, T *vS, const int nR, const int nS) {
    const auto MPI_TYPE = GET_TYPE<T>;

    for (uint i = 0; i < commList.size(); i++) {
        if (commList[i] == MPI_COMM_NULL) continue;
        std::unordered_map<int, std::vector<T>> partition_results = partition_vector(vS, nS, i);
        exchangePartitions(partition_results, i);
    }
}

template
class SmartIntersect<int>;