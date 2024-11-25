#ifndef SIMPLE_MULTITHREADER_H
#define SIMPLE_MULTITHREADER_H

#include <pthread.h>
#include <functional>
#include <vector>
#include <cstdio>
#include <ctime>
#include <stdexcept>

// Structure to pass data to threads
struct ThreadData {
    int start;
    int end;
    std::function<void(int)> func1D;
    std::function<void(int, int)> func2D;
    int innerLow, innerHigh;
    bool is2D;
};

// Thread worker function
void* thread_worker(void* arg) {
    ThreadData* data = static_cast<ThreadData*>(arg);
    if (data->is2D) {
        for (int i = data->start; i < data->end; ++i) {
            for (int j = data->innerLow; j < data->innerHigh; ++j) {
                data->func2D(i, j);
            }
        }
    } else {
        for (int i = data->start; i < data->end; ++i) {
            data->func1D(i);
        }
    }
    return nullptr;
}

// 1D parallel_for
void parallel_for(int low, int high, std::function<void(int)> &&lambda, int numThreads) {
    if (numThreads <= 0) throw std::invalid_argument("Number of threads must be greater than zero");
    int range = high - low;
    if (range <= 0) throw std::invalid_argument("Invalid loop range");

    clock_t start_time = clock();

    std::vector<pthread_t> threads(numThreads);
    std::vector<ThreadData> threadData(numThreads);

    int chunk = range / numThreads;
    int remaining = range % numThreads;

    int start = low;
    for (int i = 0; i < numThreads; ++i) {
        int end = start + chunk + (remaining-- > 0 ? 1 : 0);
        threadData[i] = {start, end, lambda, nullptr, 0, 0, false};
        pthread_create(&threads[i], nullptr, thread_worker, &threadData[i]);
        start = end;
    }

    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    clock_t end_time = clock();
    double exec_time = double(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Execution time: %.4f seconds\n", exec_time);
}

// 2D parallel_for
void parallel_for(int low1, int high1, int low2, int high2, std::function<void(int, int)> &&lambda, int numThreads) {
    if (numThreads <= 0) throw std::invalid_argument("Number of threads must be greater than zero");
    int outerRange = high1 - low1;
    if (outerRange <= 0 || low2 >= high2) throw std::invalid_argument("Invalid loop range");

    clock_t start_time = clock();

    std::vector<pthread_t> threads(numThreads);
    std::vector<ThreadData> threadData(numThreads);

    int chunk = outerRange / numThreads;
    int remaining = outerRange % numThreads;

    int start = low1;
    for (int i = 0; i < numThreads; ++i) {
        int end = start + chunk + (remaining-- > 0 ? 1 : 0);
        threadData[i] = {start, end, nullptr, lambda, low2, high2, true};
        pthread_create(&threads[i], nullptr, thread_worker, &threadData[i]);
        start = end;
    }

    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], nullptr);
    }

    clock_t end_time = clock();
    double exec_time = double(end_time - start_time) / CLOCKS_PER_SEC;
    printf("Execution time: %.4f seconds\n", exec_time);
}

#endif 