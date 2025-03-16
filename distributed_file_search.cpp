#include "mpi.h"
#include <algorithm>
#include <functional>
#include <cstdlib>
#include <ctime>
#include <cctype>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <cstring>

const static int ARRAY_SIZE = 130000;
using Lines = char[ARRAY_SIZE][16];

struct letter_only: std::ctype<char> 
{
    letter_only(): std::ctype<char>(get_table()) {}

    static std::ctype_base::mask const* get_table()
    {
        static std::vector<std::ctype_base::mask> 
            rc(std::ctype<char>::table_size, std::ctype_base::space);
        std::fill(&rc['A'], &rc['z' + 1], std::ctype_base::alpha);
        return &rc[0];
    }
};

void DoOutput(std::string word, int result)
{
    std::cout << "Word Frequency: " << word << " -> " << result << std::endl;
}

int countFrequency(std::vector<std::string>& data, const std::string& word)
{
    int freq = 0;
    for (const auto& workString : data) {
        if (workString == word)
            freq++;
    }
    return freq;
}

int main(int argc, char* argv[])
{
    int processId, numberOfProcesses;
    numberOfProcesses = 8;
    double start_time, end_time;
    
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);
    MPI_Comm_size(MPI_COMM_WORLD, &numberOfProcesses);
    
    if (argc != 4)
    {
        if (processId == 0)
        {
            std::cout << "ERROR: Incorrect number of arguments. Format is: <path to search file> <search word> <b1/b2>" << std::endl;
        }
        MPI_Finalize();
        return 0;
    }
    
    std::string word = argv[2];
    Lines lines;
    
    int chunk_size = ARRAY_SIZE / numberOfProcesses;
    char local_buf[chunk_size][16];
    
    if (processId == 0) {
        std::ifstream file;
        file.imbue(std::locale(std::locale(), new letter_only()));
        file.open(argv[1]);
        std::string workString;
        int i = 0;
        while (file >> workString && i < ARRAY_SIZE) {
            memset(lines[i], '\0', 16);
            memcpy(lines[i++], workString.c_str(), workString.length());
        }
    }
    
    MPI_Scatter(lines, chunk_size * 16, MPI_CHAR, local_buf, chunk_size * 16, MPI_CHAR, 0, MPI_COMM_WORLD);
    
    std::vector<std::string> local_data;
    for (int i = 0; i < chunk_size; i++) {
        local_data.emplace_back(local_buf[i]);
    }
    
    int local_count = countFrequency(local_data, word);
    int global_count = local_count;
    
    start_time = MPI_Wtime();
    
    std::string mode = argv[3];
    if (mode == "b1") {
        MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    } else {
        if (processId > 0) {
            int received_count;
            MPI_Recv(&received_count, 1, MPI_INT, processId - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            global_count += received_count;
        }
        if (processId < numberOfProcesses - 1) {
            MPI_Send(&global_count, 1, MPI_INT, processId + 1, 0, MPI_COMM_WORLD);
        }
    }
    
    end_time = MPI_Wtime();
    
    if (processId == numberOfProcesses - 1 && mode == "b2") {
        MPI_Send(&global_count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
    
    if (processId == 0) {
        if (mode == "b2") {
            MPI_Recv(&global_count, 1, MPI_INT, numberOfProcesses - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        DoOutput(word, global_count);
        std::cout << "Time: " << (end_time - start_time) << " seconds" << std::endl;
    }
    
    MPI_Finalize();
    return 0;
}
