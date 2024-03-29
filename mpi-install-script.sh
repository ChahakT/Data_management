#!/bin/bash

mpi-install() {
  sudo apt-get update
  sudo apt-get install openmpi-bin libopenmpi-dev -y
  mpic++ --version
}

cmake-install() {
  sudo apt purge --auto-remove cmake
  sudo apt update && sudo apt install -y software-properties-common lsb-release && sudo apt clean all
  wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
  sudo apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
  sudo apt update
  sudo apt install cmake -y
}

mpi-copy() {
  echo "WARNING: No need to mpi-copy anymore, directly run mpi-run/mpi-benchmark"
  cp /users/kkhare/git_proj/hostfile /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp
  cp /users/kkhare/git_proj/build/main /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp
}

mpi-run() {
  rm -rf /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp/main
  cp /users/kkhare/git_proj/build/main /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp
  cp /users/kkhare/git_proj/hostfile /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp
  node_count=$(wc -l /users/kkhare/git_proj/hostfile | sed 's/[^0-9]*//g')
  mpirun -n "${node_count}" --hostfile /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp/hostfile --mca btl_base_warn_component_unused 0 /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp/main 
}

mpi-agg-benchmark() {
  rm -rf /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp/benchmark_agg
  cp /users/kkhare/git_proj/build/benchmark_agg /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp
  cp /users/kkhare/git_proj/hostfile /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp
  node_count=$(wc -l /users/kkhare/git_proj/hostfile | sed 's/[^0-9]*//g')
  mpirun -n "${node_count}" --mca btl_base_warn_component_unused 0 --hostfile /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp/hostfile /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp/benchmark_agg
}

mpi-intersect-benchmark() {
  rm -rf /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp/benchmark_intersect
  cp /users/kkhare/git_proj/build/benchmark_intersect /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp
  cp /users/kkhare/git_proj/hostfile /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp
  node_count=$(wc -l /users/kkhare/git_proj/hostfile | sed 's/[^0-9]*//g')
  mpirun -n "${node_count}" --mca btl_base_warn_component_unused 0 --hostfile /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp/hostfile /proj/AdvOSUWMadison/ds/groups/group4/dm/tmp/benchmark_intersect
}

# Check if the function exists (bash specific)
if declare -f "$1" >/dev/null; then
  # call arguments verbatim
  "$@"
else
  # Show a helpful error
  echo "'$1' is not a known function name" >&2
  exit 1
fi
