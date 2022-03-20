#include <mpi.h>
#include <mpi_helper.h>
#include <vector>

class SmartAgg {
    std::pair<MPI_Comm, MPI_Comm> get_comm() {
        MPI_Group world_group;
        MPI_Comm_group(MPI_COMM_WORLD, &world_group);
        int grp_size = 2;
        const int ranks[] = {1,2};
        MPI_Group custom_grp;
        MPI_Group_incl(world_group, grp_size, ranks, &custom_grp);

        MPI_Comm custom_comm;
        MPI_Comm_create_group(MPI_COMM_WORLD, custom_grp, 0, &custom_comm);


        const int ranks_new[] = {0,1};
        MPI_Group new_grp;
        MPI_Group_incl(world_group, grp_size, ranks_new, &new_grp);

        MPI_Comm final_comm;
        MPI_Comm_create_group(MPI_COMM_WORLD, new_grp, 0, &final_comm);
        return {custom_comm, final_comm};
    }
    template <class T>
    void run(std::vector<T>& v) {
        run(v.data(), v.size());
    }
    template <class T>

    void run(T* v, const int n) {
        const int rank = MPIHelper::get_rank();
        const auto  MPI_TYPE = GET_TYPE<T>;

        const auto [comm1, comm2] = get_comm();
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
};

template void SmartAgg::run<int>(int*, const int);
template void SmartAgg::run<int>(std::vector<int>&);