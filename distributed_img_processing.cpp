#include <mpi.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>
#include <sstream>
#include <unordered_set>

#define MAX_INTENSITY 256 // Grayscale intensity levels

int rank, size; // MPI process ID and total processes
// Read PGM Image
std::vector<unsigned char> readPGM(const std::string &filename, int &width, int &height) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        if (rank == 0) std::cerr << "ERROR: Could not open file " << filename << std::endl;
        return {};
    }

    std::string format;
    file >> format;
    if (format != "P2") {
        if (rank == 0) std::cerr << "ERROR: Not a valid ASCII PGM (P2) image.\n";
        return {};
    }

    // Skip comments
    std::string line;
    while (file.peek() == '#') std::getline(file, line);

    // Read width, height, and max intensity
    int maxShades;
    file >> width >> height >> maxShades;

    // Read pixel data
    std::vector<unsigned char> image(width * height);
    for (int i = 0; i < width * height; i++) {
        int pixelVal;
        file >> pixelVal;
        image[i] = static_cast<unsigned char>(pixelVal);
    }

    return image;
}

// Read adjacency matrix from file
std::vector<std::vector<int> > readAdjacencyMatrix(const std::string &filename, int &numNodes) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        if (rank == 0) std::cerr << "ERROR: Could not open adjacency matrix file.\n";
        return {};
    }

    std::vector<std::vector<int> > adjacencyMatrix;
    std::string line;
    while (getline(file, line)) {
        std::vector<int> row;
        std::istringstream iss(line);
        int value;
        while (iss >> value) {
            row.push_back(value);
        }
        adjacencyMatrix.push_back(row);
    }
    
    numNodes = adjacencyMatrix.size();
    return adjacencyMatrix;
}

// Compute local histogram [only worry about the amount of the image that must be computed]
std::array<int, MAX_INTENSITY> computeLocalHistogram(const std::vector<unsigned char> &image) {
    std::array<int, MAX_INTENSITY> localHistogram;
    localHistogram.fill(0);
    for (int i = 0; i < (int)image.size(); i++) {
        localHistogram[image[i]]++;
    }
    return localHistogram;
}

// Write final histogram to file
void writeHistogramToFile(const std::string &filename, const std::array<int, MAX_INTENSITY> &histogram) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        if (rank == 0) std::cerr << "ERROR: Could not open output file.\n";
        return;
    }
    for (int i = 0; i < MAX_INTENSITY; i++) {
        file << histogram[i] << std::endl;
    }
}

int errorCorrection(int argc, char *argv[]) {
    if (argc != 4) {
        // if (rank == 0) 
        std::cerr << "Usage: " << argv[0] << " <input_pgm> <adjacency_matrix> <output_file>\n";
        MPI_Finalize();
        return -1;
    }

    if (size <= 1) {
        // if (rank == 0) 
        std::cerr << "ERROR: Need at least two processes.\n";
        MPI_Finalize();
        return -1;
    }
    return 0;
}

void tarryAlgorithm(int node, const std::vector<std::vector<int>> &adjMatrix, 
                    std::unordered_set<int> &visited, 
                    std::array<int, MAX_INTENSITY> &localHist) {
    visited.insert(node);
    if (visited.size() == 4) {
        std::cout << "All nodes have been visited." << std::endl;
        return;
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // Process each neighbor
    for (size_t neighbor = 0; neighbor < adjMatrix[node].size(); ++neighbor) {
        if (adjMatrix[node][neighbor] == 1 && visited.find(neighbor) == visited.end()) {

            // Send and receive local histogram from neighbor using MPI_Sendrecv (non-blocking type)
            std::array<int, MAX_INTENSITY> receivedHist;
            MPI_Sendrecv(localHist.data(), MAX_INTENSITY, MPI_INT, neighbor, node,
                         receivedHist.data(), MAX_INTENSITY, MPI_INT, neighbor, node,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            // Merge histograms
            for (int i = 0; i < MAX_INTENSITY; i++) {
                localHist[i] += receivedHist[i];
            }
            // Recursively perform DFS (or Tarry's traversal) on the neighbor
            tarryAlgorithm(neighbor, adjMatrix, visited, localHist);
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }
    return;
}


int alt_main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    errorCorrection(argc, argv); // ** ERROR DETECTION

    int width, height, numNodes;
    numNodes = 4;
    int initiatorNode = 3;
    int chunk_size = width * height / size;

    std::vector<unsigned char> image;
    std::vector<std::vector<int> > adjacencyMatrix;
    if (rank == initiatorNode) {
        image = readPGM(argv[1], width, height); // processed images correctly
        adjacencyMatrix = readAdjacencyMatrix(argv[2], numNodes); // read and constructed adjacency matrix correctly
        if (image.empty() || adjacencyMatrix.empty()) { MPI_Finalize(); return -1; }
        chunk_size = width * height / size;
    }MPI_Bcast(&chunk_size, 1, MPI_INT, 3, MPI_COMM_WORLD);
    std::cout << "B-Casted chunk size to all nodes" << std::endl;
    std::cout << "Chunk size: " << chunk_size << " For rank: " << rank << std::endl;

    std::vector<unsigned char> localChunk(chunk_size);
    std::fill(localChunk.begin(), localChunk.end(), 0);
    MPI_Scatter(image.data(), chunk_size, MPI_UNSIGNED_CHAR, localChunk.data(), chunk_size, MPI_UNSIGNED_CHAR, 3, MPI_COMM_WORLD);
    std::cout << "Node " << rank << " received data of chunk size " << chunk_size << std::endl;

    std::array<int, MAX_INTENSITY> localHistogram = computeLocalHistogram(localChunk);
    std::cout << "Computed @ node " << rank << ". Histogram successfully computed. " << std::endl;


    std::array<int, MAX_INTENSITY> globalHistogram;
    globalHistogram.fill(0);

    std::unordered_set<int> visited;
    bool isFinished;
    int root = (size > 4) ? 3 : 0;
    if (rank == root) {
        // MPI_Bcast(&visited, 1, MPI_INT, rank, MPI_COMM_WORLD);
        std::cout << "Tarry algorithm started @ node: " << rank << std::endl;
        tarryAlgorithm(rank, adjacencyMatrix, visited, localHistogram);
        isFinished = true;
        std::cout << "FINISHED FLAG" << std::endl;
    } MPI_Bcast(&isFinished, 1, MPI_CXX_BOOL, rank, MPI_COMM_WORLD);


    MPI_Reduce(localHistogram.data(), globalHistogram.data(), 256, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 3) {
        std::cout << "Writing histogram" << std::endl;
        writeHistogramToFile(argv[3], localHistogram);
        std::cout << "Final histogram saved to " << argv[3] << ".\n";
        // isFinished = true;
        // MPI_Finalize();   
    }
    // MPI_Barrier(MPI_COMM_WORLD);
    // MPI_Finalize();
    // std::cout << "@ node " << rank << " is still running" << std::endl;
    // if (isFinished == true) {
    //     MPI_Finalize();
    //     // std::exit(0);
    // }
    MPI_Finalize();
    return 1;
}


int main(int argc, char* argv[]) {
    alt_main(argc, argv);
    MPI_Finalize();
    std::exit(0);
    return 1;
}
// int main(int argc, char *argv[]) {
//     MPI_Init(&argc, &argv);
//     MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//     MPI_Comm_size(MPI_COMM_WORLD, &size);
//     errorCorrection(argc, argv); // ** ERROR DETECTION

//     int width, height, numNodes;
//     numNodes = 4;
//     int initiatorNode = 3;
//     std::vector<unsigned char> image;
//     std::vector<std::vector<int> > adjacencyMatrix;

//     // Node 3 reads the image and adjacency matrix and distributes them
//     if (rank == initiatorNode) {
//         image = readPGM(argv[1], width, height); // processed images correctly
//         adjacencyMatrix = readAdjacencyMatrix(argv[2], numNodes); // read and constructed adjacency matrix correctly
//         if (image.empty() || adjacencyMatrix.empty()) { MPI_Finalize(); return -1; }
//         int chunk_size = width * height / size;
//         std::cout << "Chunk size: " << chunk_size << " For rank: " << rank << std::endl;
//         MPI_Bcast(&chunk_size, 1, MPI_INT, 3, MPI_COMM_WORLD);
//         std::cout << "B-Casted chunk size to all nodes" << std::endl;
        
//     } else {
//         // Here, the chunk_size will be already received by non-initiator nodes
//         MPI_Barrier(MPI_COMM_WORLD);
//         // int chunk_size;
//         // MPI_Bcast(&chunk_size, 1, MPI_INT, initiatorNode, MPI_COMM_WORLD);
//         // std::cout << "Chunk size: " << chunk_size << " For rank: " << rank << std::endl;
//     }
//     // int chunk_size;
//     // MPI_Bcast(&chunk_size, 1, MPI_INT, initiatorNode, MPI_COMM_WORLD);
//     // std::cout << "Chunk size: " << chunk_size << " For rank: " << rank << std::endl;
//     int chunk_size = 0;
//     std::vector<unsigned char> localChunk(chunk_size);
//     std::fill(localChunk.begin(), localChunk.end(), 0);

//     MPI_Scatter(image.data(), chunk_size, MPI_UNSIGNED_CHAR, localChunk.data(), chunk_size, MPI_UNSIGNED_CHAR, 3, MPI_COMM_WORLD);
//     std::cout << "Node " << rank << " received data of chunk size " << chunk_size << std::endl;
//     std::array<int, MAX_INTENSITY> localHistogram = computeLocalHistogram(localChunk);
//     for (int i = 200; i < (int)localHistogram.size()-12000; i++) {
//         std::cout << localHistogram[i] << " " << std::endl;
//     }
//     std::cout << "Computed @ node " << rank << ". Histogram successfully " << std::endl;

//     // Start Tarry’s algorithm at Node 4 (or available root)
//     std::unordered_set<int> visited;
//     int root = (size > 4) ? 3 : 0; // Use rank 4 if available, else fall back to rank 0
//     if (rank == root) {
//         tarryAlgorithm(rank, adjacencyMatrix, visited, localHistogram);
//     }

//     // Gather results at root node using MPI_Reduce
//     std::array<int, MAX_INTENSITY> finalHistogram = {};
//     MPI_Reduce(localHistogram.data(), finalHistogram.data(), MAX_INTENSITY, MPI_INT, MPI_SUM, root, MPI_COMM_WORLD);

//     // Root writes the final histogram to the output file
//     if (rank == root) {
//         std::cout << "Writing histogram" << std::endl;
//         writeHistogramToFile(argv[3], finalHistogram);
//         std::cout << "Final histogram saved to " << argv[3] << "\n";
//     }

//     MPI_Finalize();
//     return 0;
// }





// THAT CHUNK OF CODE IS BELOW:
    // int chunkSize = width*height/size;
    // std::cout << chunkSize << std::endl;
    // image.resize(chunkSize); // Keep only node 4's portion

    // for (int i = 0; i < size; i++) {
    // std::cout << "Initiator node " << rank << " about to send data to Node " << i << std::endl;
    // MPI_Send(image.data() + i * chunkSize, chunkSize, MPI_UNSIGNED_CHAR, i, initiatorNode, MPI_COMM_WORLD);
    // std::cout << "Sent data to " << i << std::endl;
    // }
    // std::cout << "Casted data to all members in network" << std::endl;


    // std::cout << "Waiting for initiator to start, Node Address: " << rank << std::endl;
    // // Receive image portion
    // MPI_Status status;
    // int chunkSize = width*height/size;
    // image.resize(chunkSize);
    // MPI_Recv(image.data(), chunkSize, MPI_UNSIGNED_CHAR, 0, initiatorNode, MPI_COMM_WORLD, &status);
    // std::cout << "Received the data" << std::endl;
