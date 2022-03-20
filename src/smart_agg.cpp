#include <mpi.h>
#include <mpi_helper.h>
#include <vector>
#include "agg.h"

std::vector<MPI_Comm> SmartAgg::get_comm(std::vector <std::vector<int>> comm_groups) {
    MPI_Group world_group;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);

    auto create_group = [&world_group](std::vector<int> &comm_group) {
        MPI_Group custom_grp;
        MPI_Group_incl(world_group, comm_group.size(), comm_group.data(), &custom_grp);

        MPI_Comm custom_comm;
        MPI_Comm_create_group(MPI_COMM_WORLD, custom_grp, 0, &custom_comm);
        return custom_comm;
    };

    std::vector <MPI_Comm> all_comm;
    for (auto &comm_group: comm_groups) {
        MPI_Comm temp = create_group(comm_group);
        all_comm.push_back(temp);
    }
    return all_comm;
}

template<class T>
void SmartAgg::run(std::vector <T> &v, std::vector <std::vector<int>> &comm_groups) {
    run(v.data(), v.size(), comm_groups);
}

template<class T>
void SmartAgg::run(T *v, const int n, std::vector <std::vector<int>> &comm_groups) {
    //const int rank = MPIHelper::get_rank();
    const auto MPI_TYPE = GET_TYPE<T>;

    std::vector<MPI_Comm> comm = get_comm(comm_groups);
    MPI_Comm comm1 = comm[0], comm2 = comm[1];

    if (MPI_COMM_NULL != comm1) {
        int comm1_rank;
        MPI_Comm_rank(comm1, &comm1_rank);
        if (comm1_rank == 0) { // root for comm1
            MPI_Reduce(MPI_IN_PLACE, v, n, MPI_TYPE, MPI_SUM, 0, comm1);
        } else {
            MPI_Reduce(v, nullptr, n, MPI_TYPE, MPI_SUM, 0, comm1);
        }
    }


    if (MPI_COMM_NULL != comm2) {
        int comm2_rank;
        MPI_Comm_rank(comm2, &comm2_rank);
        if (comm2_rank == 0) { // root for comm1
            MPI_Reduce(MPI_IN_PLACE, v, n, MPI_TYPE, MPI_SUM, 0, comm2);
        } else {
            MPI_Reduce(v, nullptr, n, MPI_TYPE, MPI_SUM, 0, comm2);
        }
    }

}

template void SmartAgg::run<int>(int *, const int, std::vector <std::vector<int>> &);
template void SmartAgg::run<int>(std::vector<int> &, std::vector <std::vector<int>> &);