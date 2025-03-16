#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <sstream>
#include <omp.h>
#include <chrono>
#include <math.h>
#include <mutex>

#define MAX_INTENSITY 256  // Grayscale intensity levels
const int CHUNK_SIZE = 25; // define and adjust the chunk size
std::mutex histogram_mutex;
std::mutex log_mutex;  



// Function to read a PGM file (P2 format - ASCII PGM)
std::vector<unsigned char> readPGM(const std::string &filename, int &width, int &height) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERROR: Could not open file " << filename << std::endl;
        return {};
    }

    std::string format;
    file >> format;
    if (format != "P2") {
        std::cerr << "ERROR: Input image is not a valid ASCII PGM (P2) image.\n";
        return {};
    }

    // Skip comments
    std::string line;
    while (file.peek() == '#') std::getline(file, line);

    // Read width and height
    file >> width >> height;

    // Read max intensity (not used, but must be read)
    int maxShades;
    file >> maxShades;

    // Read pixel data
    std::vector<unsigned char> image(width * height);
    for (int i = 0; i < width * height; i++) {
        int pixelVal;
        file >> pixelVal;
        image[i] = static_cast<unsigned char>(pixelVal);
    }

    return image;
}

// Function to compute the histogram sequentially
void computeHistogramSequential(const std::vector<unsigned char> &image, int width, int height, std::array<int, MAX_INTENSITY> &histogram) {
    histogram.fill(0);

    for (int i = 0; i < width * height; i++) {
        histogram[image[i]]++;
    }
}



// Function to compute the histogram in parallel using OpenMP
void computeHistogramParallel(const std::vector<unsigned char> &image, int width, int height, std::array<int, MAX_INTENSITY> &histogram) {
    histogram.fill(0);
    int num_threads = 8;
    int chunk_size = 70;

    std::vector<std::pair<int, int> > threadInfo;
    std::vector<std::chrono::duration<double, std::milli>> threadTime;

    auto global_start = std::chrono::high_resolution_clock::now();
    #pragma omp parallel for num_threads(num_threads), schedule(dynamic)
    for (int i = 0; i < height; i++) {
        int thread_id = omp_get_thread_num();
        auto start_time = std::chrono::high_resolution_clock::now();

        for (int j = 0; j < width; j++) {
            std::lock_guard<std::mutex> lock(histogram_mutex);
            histogram[image[i * width + j]]++;
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> row_time = end_time - start_time;

        if (i % chunk_size == 0) { 
            std::lock_guard<std::mutex> log_lock(log_mutex);
            threadInfo.emplace_back(thread_id, i);
            threadTime.emplace_back(row_time);
        }

        // Log the row execution details
        std::lock_guard<std::mutex> log_lock(histogram_mutex);
        std::cout << "Thread " << thread_id << " -> Processing Chunk starting at Row " << i << "->time: " << row_time.count() << " ms.\n";
    }

    auto global_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> diff = global_end - global_start;
    std::cout << "Total Execution Time for Parallelization with NumThreads = " << num_threads << " and ChunkSize = " << chunk_size << " with Time: " << diff.count() << " ms.\n";

    #pragma omp barrier
    for (long unsigned int i = 0; i < threadTime.size(); i++) {
        std::cout << "Thread " << threadInfo[i].first << " -> Processing Chunk starting at Row " << threadInfo[i].second << " -> time: " << threadTime[i].count() << " ms.\n";
    }

}

// Function to print the histogram
void printHistogram(const std::array<int, MAX_INTENSITY> &histogram) {
    std::lock_guard<std::mutex> lock(histogram_mutex);
    for (int i = 0; i < MAX_INTENSITY; i++) {
        std::cout << "Intensity " << i << ": " << histogram[i] << std::endl;
    }
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <input_pgm_file>" << std::endl;
        return -1;
    }

    int width, height;
    std::vector<unsigned char> image = readPGM(argv[1], width, height);
    if (image.empty()) {
        std::cerr << "Error reading the image.\n";
        return -1;
    }

    std::array<int, MAX_INTENSITY> histogram;

    std::cout << "Computing histogram sequentially...\n";
    computeHistogramSequential(image, width, height, histogram);
    printHistogram(histogram);

    std::cout << "\nComputing histogram in parallel...\n";
    computeHistogramParallel(image, width, height, histogram);
    #pragma omp barrier
    printHistogram(histogram);

    return 0;
}
