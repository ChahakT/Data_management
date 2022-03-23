#!/bin/bash

mpi-install() {
  sudo apt-get update
  sudo apt-get install openmpi-bin libopenmpi-dev
  mpic++ --version
}

cmake-install() {
  sudo apt purge --auto-remove cmake
  sudo apt update &&
    sudo apt install -y software-properties-common lsb-release &&
    sudo apt clean all
  wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | sudo tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
  sudo apt-add-repository "deb https://apt.kitware.com/ubuntu/ $(lsb_release -cs) main"
  sudo apt update
  sudo apt install kitware-archive-keyring
  sudo rm /etc/apt/trusted.gpg.d/kitware.gpg
  sudo apt update
  sudo apt install cmake
}

mpi-copy() {
  echo "WARNING: No need to mpi-copy anymore, directly run mpi-run/mpi-benchmark"
  cp /users/kkhare/git_proj/hostfile /proj/AdvOSUWMadison/MPI/tmp/
  cp /users/kkhare/git_proj/build/main /proj/AdvOSUWMadison/MPI/tmp
}

mpi-run() {
  rm -rf /proj/AdvOSUWMadison/MPI/tmp/main
  cp /users/kkhare/git_proj/build/main /proj/AdvOSUWMadison/MPI/tmp
  cp /users/kkhare/git_proj/hostfile /proj/AdvOSUWMadison/MPI/tmp/
  node_count=$(wc -l /users/kkhare/git_proj/hostfile | sed 's/[^0-9]*//g')
  mpirun -n "${node_count}" --hostfile /proj/AdvOSUWMadison/MPI/tmp/hostfile /proj/AdvOSUWMadison/MPI/tmp/main
}

mpi-agg-benchmark() {
  rm -rf /proj/AdvOSUWMadison/MPI/tmp/benchmark_agg
  cp /users/kkhare/git_proj/build/benchmark/benchmark_agg /proj/AdvOSUWMadison/MPI/tmp
  cp /users/kkhare/git_proj/hostfile /proj/AdvOSUWMadison/MPI/tmp/
  node_count=$(wc -l /users/kkhare/git_proj/hostfile | sed 's/[^0-9]*//g')
  mpirun -n "${node_count}" --hostfile /proj/AdvOSUWMadison/MPI/tmp/hostfile /proj/AdvOSUWMadison/MPI/tmp/benchmark_agg
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
