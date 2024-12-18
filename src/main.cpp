#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include "renderer.hpp"
using namespace std;

int main() {
    const int SEED = 1492;
    mt19937 gen(SEED);
    vector<int> CANVAS_SIZES = {256, 512, 1024};
    vector<int> NUM_THREADS = {2, 3, 4, 6, 8, 12, 16, 24, 32};
    vector<int> NUM_CIRCLES = {1000, 5000, 10000, 20000, 50000, 100000};
    vector<int> BLOCK_SIZES = {16, 24, 32};

    for(auto canvas_size : CANVAS_SIZES) {
        for(auto num_circles : NUM_CIRCLES) {
            Renderer renderer(canvas_size, canvas_size);

            // Random creation of circles
            uniform_real_distribution<> pos_x(0, canvas_size);
            uniform_real_distribution<> pos_y(0, canvas_size);
            uniform_real_distribution<> pos_z(0, 1000);
            uniform_real_distribution<> radius(10, 50);
            uniform_real_distribution<> color(0, 1);
            uniform_real_distribution<> alpha(0.1, 0.5);

            for(int i = 0; i < num_circles; i++) {
                Circle c(pos_x(gen), pos_y(gen), pos_z(gen),
                         radius(gen),
                         Color(color(gen), color(gen), color(gen)),
                         alpha(gen));
                renderer.addCircle(c);
            }

            SequentialResult seqResult = renderer.renderSequential();

            for(auto num_threads : NUM_THREADS) {
                for(auto block_size : BLOCK_SIZES) {
                    ParallelResult parResult = renderer.renderParallel(num_threads, block_size, seqResult.seqExecutionTime);

                    cout << "\nCanvas: " << canvas_size << "x" << canvas_size
                         << ", Circles: " << num_circles
                         << ", Threads: " << parResult.numThreads
                         << ", Blocks: " << parResult.blockSize
                         << "\nSequential Sorting Time: " << seqResult.seqSortingTime
                         << ", Parallel Sorting Time: " << parResult.parSortingTime
                         << "\nSequential Rendering Time: " << seqResult.seqRenderTime
                         << ", Parallel Rendering Time: " << parResult.parRenderTime
                         << "\nSequential Execution Time: " << seqResult.seqExecutionTime
                         << ", Parallel Execution Time: " << parResult.parExecutionTime
                         << "\nSpeedup: " << parResult.speedup
                         << ", Efficiency: " << parResult.efficiency * 100 << "%";
                }
            }
        }
    }
    return 0;
}
