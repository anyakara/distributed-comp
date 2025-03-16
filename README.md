# Distributed Computing Project Portfolio
This repository contains various projects focused on distributed computing, utilizing MPI (Message Passing Interface) and OpenMP (Open Multi-Processing) to achieve efficient parallel and distributed task execution. These projects aim to showcase how these technologies can be used to solve computationally intensive problems in distributed and multi-core environments.

## Table of Contents
1. Overview
2. Technologies Used
3. Projects
4. Setup and Installation
5. Usage
6. Contributing
7. License

## Overview
Distributed computing is a method of computational execution where tasks are split across multiple processors or machines to achieve faster results and more efficient resource utilization. This portfolio demonstrates the practical use of MPI for inter-process communication and OpenMP for parallel processing on shared-memory systems.

In this portfolio, you'll find various projects that showcase how to implement distributed algorithms, parallelize computations, and optimize performance for real-world applications.

## Technologies Used
* MPI (Message Passing Interface): Used for communication between distributed processes across different nodes in a network.
* OpenMP (Open Multi-Processing): Used for parallelizing tasks in multi-threaded applications on shared-memory systems.
* C/C++: Programming languages used to implement the projects, leveraging both MPI and OpenMP for parallelism.
* Makefile: Build automation tool used to compile and manage project dependencies.

## Projects
Here are the key projects in this portfolio:
1. Matrix Multiplication (MPI & OpenMP): A distributed matrix multiplication algorithm that uses MPI to divide the matrices across nodes and OpenMP to parallelize matrix multiplication on each node.
*Key Concepts: Distributed computation, data parallelism, message passing, multi-threading.*

2. Parallel Search Algorithms: An implementation of parallel search algorithms (e.g., BFS, DFS) that utilizes MPI for task distribution and OpenMP for task execution on shared memory.
*Key Concepts: Search algorithms, graph processing, parallelism.*

3. Distributed Sort (MPI): A parallel sorting algorithm that uses MPI to distribute the data across multiple nodes, and then uses OpenMP to parallelize sorting on each node.
*Key Concepts: Sorting algorithms, distributed data processing, load balancing.*

4. Monte Carlo Simulations: A project demonstrating parallel Monte Carlo simulations for solving problems in areas like finance or physics. It uses MPI to distribute the simulation tasks across different processes and OpenMP to parallelize computations within each process.
*Key Concepts: Random number generation, statistical simulations, parallel computation.*

# Setup and Installation
## Prerequisites
* MPI: Ensure that an MPI implementation (e.g., OpenMPI, MPICH) is installed on your system.
* OpenMP: OpenMP is supported natively by most modern C/C++ compilers, such as GCC.
* CMake: Required to build the projects if a CMake-based build system is used.
* GCC: Compiler used for the C/C++ projects.

## Installation Steps
1. Clone the repository:
```git clone https://github.com/your-username/distributed-computing-portfolio.git```
```cd distributed-computing-portfolio```


2. Build the projects:
```mkdir build```
```cd build```
```cmake ..```
```make```

3. Run individual projects (depending on the project specifics, you can use ```mpiexec``` for MPI execution and normal ./ for OpenMP-based executions):
```mpiexec -n 4 ./matrix_multiplication```
```./monte_carlo_simulation```

## Usage
Each project in this repository demonstrates the use of MPI and OpenMP in different types of parallel computing tasks. Below are instructions for running individual projects:
1. Matrix Multiplication: 
Run the matrix multiplication project using the following command:
```mpiexec -n 4 ./matrix_multiplication```

2. Parallel Search:
Run the parallel search algorithm by executing:
```mpiexec -n 4 ./parallel_search```

3. Distributed Sort:
Use the following command to execute the distributed sorting algorithm:
```mpiexec -n 4 ./distributed_sort```

4. Monte Carlo Simulation:
Run the Monte Carlo simulation with the following command:
```./monte_carlo_simulation```

For more details on each project's execution, refer to the README files located within each project folder.

## Contributing
Feel free to fork this repository and contribute by creating pull requests. If you have any suggestions or improvements for any of the projects, open an issue, and we can discuss further.

## License
This project is licensed under the MIT License - see the LICENSE file for details.
