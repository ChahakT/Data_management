#include "iostream"
#include <mpi.h>
#include <mpi_helper.h>
#include <vector>

class CommHelper{
private:
    CommHelper() {}
public:
    static std::vector<MPI_Comm> get_comm(std::vector<std::vector<int>> comm_groups) {
        MPI_Group world_group;
        MPI_Comm_group(MPI_COMM_WORLD, &world_group);
//        unordered_map<int, unordered_map<int, int>> rank_map;
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
};