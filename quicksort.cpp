#include <chrono>
#include <iostream>
#include <omp.h>
#include <vector>
#include <cstdlib>
#include <mutex>

#define SET_THRESHOLD 100
int partition(std::vector<int>& inputArray, int low, int high) {
    int pivot = inputArray[high];
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (inputArray[j] < pivot) {
            i++;
            std::swap(inputArray[i], inputArray[j]);
        }
    }
    std::swap(inputArray[i + 1], inputArray[high]);
    return i + 1;
}


void quicksort(std::vector<int>& arr, int low, int high) {
    if (low < high) {
        int pivotIndex = partition(arr, low, high);
        if (high - low < SET_THRESHOLD) {  
            quicksort(arr, low, pivotIndex - 1);
            quicksort(arr, pivotIndex + 1, high);
            return;
        } if (omp_get_level() < omp_get_max_active_levels()) {
            #pragma omp task shared(arr), firstprivate(low, pivotIndex)
            quicksort(arr, low, pivotIndex - 1);
            #pragma omp task shared(arr), firstprivate(high, pivotIndex)
            quicksort(arr, pivotIndex + 1, high);
            #pragma omp taskwait 
        } else {
            quicksort(arr, low, pivotIndex - 1);
            quicksort(arr, pivotIndex + 1, high);
        }
    }
}


int main() {
    const int SIZE = 10000;
    std::vector<int> numbers(SIZE);
    for (int i = 0; i < SIZE; ++i) {
        numbers[i] = std::rand() % 100000;
    }
    auto start = std::chrono::high_resolution_clock::now();
    #pragma omp parallel
    {
        #pragma omp single 
        quicksort(numbers, 0, SIZE - 1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> diff = end-start;
    std::cout << "Quicksort executed in: " << diff.count() << " ms." << std::endl;
    return 0;
}
