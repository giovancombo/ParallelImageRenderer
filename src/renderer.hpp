#include <vector>
#include <string>
#ifndef RENDERER_HPP
#define RENDERER_HPP

const int CACHE_LINE_LENGTH = 32;

struct Color {
    float r, g, b;
    Color(float r = 0, float g = 0, float b = 0): r(r), g(g), b(b) {}
};

// struct to address False Sharing
struct alignas(CACHE_LINE_LENGTH) PaddedColor {
    float r, g, b;
    char padding[CACHE_LINE_LENGTH - 12];  // 64 - (3 * sizeof(float))
};

struct Circle {
    float x, y, z;
    float radius;
    Color color;
    float alpha;

    Circle(float x = 0, float y = 0, float z = 0, float radius = 1, const Color& color = Color(), float alpha = 1.0f): x(x), y(y), z(z), radius(radius), color(color), alpha(alpha) {}
};

struct SequentialResult {
    float seqSortingTime, seqRenderTime, seqExecutionTime;
};

struct ParallelResult {
    float parSortingTime, parRenderTime, parExecutionTime;
    int numThreads;
    int blockSize;
    float speedup, efficiency;
};

class Renderer {
    private:
        int width, height;
        std::vector<Circle> circles;
        Color* canvas;      // PaddedColor* canvas;     to address False Sharing
        void processPixel(int x, int y);
        bool isPixelInCircle(int x, int y, const Circle& circle) const;
        Color alphaBlending(const Color& source, const Color& dest, float alpha) const;

    public:
        Renderer(int width, int height);
        ~Renderer();
        void addCircle(const Circle& circle);

        SequentialResult renderSequential();
        ParallelResult renderParallel(int num_threads, int block_size, float seq_execution_time);
};

#endif
