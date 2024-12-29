# Parallel Image Renderer in C++ with OpenMP

Midterm Assignment for the *Parallel Programming for Machine Learning* course, taught by Professor Marco Bertini at the University of Florence, Italy.

<img src="https://github.com/giovancombo/ImageRenderer_PPMLMidTerm/blob/main/images/outputs/output_stretched2048_20000.png">

## Abstract
*This report describes the development of a simple Image Renderer in C++ and quantitatively compares its sequential and parallel implementations. The analysis demonstrates the significant potential of parallel computing and multithreading using the OpenMP API. Performance measurements including speedup and efficiency are conducted across various environmental configurations, hyperparameters, and different combinations of directives and clauses.*

## 1 - Introduction
This report presents the development of a simple *Image Renderer* that generates and projects 3D circles onto a 2D rectangular surface, simulating a canvas. The renderer handles overlapping circles by mixing their colors in a proper way, at different levels of depth. For simplicity, we constrain the 2D surface to be square, with dimensions *(canvas_size x canvas_size)*.

Each generated circle has 6 attributes:
- 2D coordinates (x, y): random values in the range [0, canvas_size]
- Depth coordinate (z): random value in the range [0, 1000]
- Radius: random value in the range [10, 50]
- Color: struct defined by its (r,g,b) components, which are random values in range [0, 1]
- Transparency (alpha): random value in the range [0.1, 0.5]

The z-coordinate, indicating the depth in the 3D space, represents the distance of circles from the 2D projection surface. This allows defining an ordering of projection levels, which will be the basis for getting a coherent rendering. Determining the correct circle ordering is crucial for a successful rendering, as each circle is semi-transparent. Therefore, a circle's color won't completely cover the color of an underlying circle. Instead, *color blending* occurs according to the *alpha blending* formula:

$$Value_{color} = (1 - α)Value_{color}^A + αValue_{color}^B$$

where $`color = \{r,g,b\}`$, and $A$ and $B$ are two consecutive circles after ordering, with $A$ being closer to the 2D surface than $B$.

<p float="center", align="center">
  <img src="https://github.com/giovancombo/ImageRenderer_PPMLMidTerm/blob/main/images/blending.png" width="40%" />
</p>
<p align="center"><b>Figure 1</b> <i>Representation of the application of alpha blending.</i></p>


So, after creating the circles, it's essential to perform two key operations **for each pixel** on the 2D surface: first, ordering the circles that would project onto that pixel according to their z-coordinate, and then calculating the pixel's final color following the alpha blending logic.

A circle with center coordinates *(xc, yc)* belongs to a pixel with coordinates *(x, y)* if the distance between these two points is less than the circle's radius.

<p float="center", align="center">
  <img src="https://github.com/giovancombo/ImageRenderer_PPMLMidTerm/blob/main/images/pixelincircle.png" width="25%" />
</p>
<p align="center"><b>Figure 2</b> <i>Representation of the rule for determining whether a pixel belongs to a circle.</i></p>

The final output of the *Image Renderer* is a *(canvas_size x canvas_size)* image showing semi-transparent colored circles, which are overlapping each other.

<p float="left", align="center">
  <img src="https://github.com/giovancombo/ImageRenderer_PPMLMidTerm/blob/main/images/outputs/output_1024_1000.png" width="24%" />
  <img src="https://github.com/giovancombo/ImageRenderer_PPMLMidTerm/blob/main/images/outputs/output_1024_10000.png" width="24%" />
  <img src="https://github.com/giovancombo/ImageRenderer_PPMLMidTerm/blob/main/images/outputs/output_1024_50000.png" width="24%" />
  <img src="https://github.com/giovancombo/ImageRenderer_PPMLMidTerm/blob/main/images/outputs/output_1024_100000.png" width="24%" />
</p>
<p align="center"><b>Figure 3</b> <i>Rendering of 1000, 10000, 50000 and 100000 circles projected on a (1024 x 1024) canvas.</i></p>

Beyond correctly generating images that respect blending rules, the main goal of this project is to develop both sequential and parallel C++ implementations of the program, leveraging **OpenMP**'s parallel computing capabilities. We will quantitatively evaluate the effect of parallelization by measuring specific metrics such as *speedup* and *efficiency*.

## 2 - Method and Code
From an operational perspective, an *Image Renderer* as described in Section 1 essentially performs two main operations:

1) *sorting* circles based on their distance from the 2D surface, indicated by their z-coordinate;
2) *alpha blending* on individual pixels to determine the final color distribution.

Since circles are randomly positioned in space, different pixels on the 2D surface will belong to different numbers of circles with different colors, resulting in varying final pixel colors. Importantly, whether a circle belongs to a pixel (and vice versa) is a condition that depends **solely** on that pixel.

It's then crucial to note that both operations can be performed **independently** on each of the $`canvas\_size^2`$ pixels of the 2D surface, making this *Image Renderer* an *embarrassingly parallel* problem. This characteristic promises effective code parallelization with OpenMP.

### 2.1 - Hardware and Software setup
[Section to be completed with hardware specifications, OpenMP and C++ versions, and a brief CMake study]

### 2.2 - Code general structure
The project is built around a main **`Renderer`** class, which is the core of the implementation. This class manages all the operations for circle rendering. It defines the 2D surface (named "canvas") through its dimensions (for simplicity, `width = height = canvas_size`), a collection of circles to render stored in a vector, and the canvas itself implemented as a linear array of pixels, where each pixel is represented by a `Color` object, which defines its RGB values.

Operationally, the class implements three fundamental private methods: `isPixelInCircle`, which determines if a point belongs to a circle, `processPixel`, which calculates a pixel's final color considering all circles containing it, and `alphaBlending`, which handles color composition considering circle transparency effects.

The class' public interface includes the two methods `renderSequential` and `renderParallel` that implement the sequential and parallel versions of the program respectively.

The outputs of these methods are two structs, `SequentialResult` and `ParallelResult`, containing essential data and measurements for quantitative characterization of each run.

The `main.cpp` file allows defining several constants, to enable multiple combinations in the same run:
- `CANVAS_SIZES`: 2D surface dimensions {256, 512, 1024}
- `NUM_CIRCLES`: number of generated circles {1000, 5000, 10000, 20000, 50000, 100000}
- `NUM_THREADS`: number of threads created during fork {2, 3, 4, 6, 8, 12, 16, 24, 32, 48, 64}
- `BLOCK_SIZES`: number of blocks/chunks for scheduling {16, 24, 32}

### 2.3 - Sequential Implementation
The sequential implementation lies in the `renderSequential` method. The process consists of two main steps:

1. **Global Circle Sorting**: 
   - Sorts all circles by their z-coordinate in descending order
   - Uses the standard C++ `sort` algorithm
   - Complexity: $`O(num\_circles * log(num\_circles))`$

1. **Pixel Processing**:
   - Nested loops iterate over each pixel (x, y) in the canvas
   - For each pixel, checks if the pixel lies within the circle using Euclidean distance, and then applies alpha blending for overlapping circles
   - Complexity: $`O(canvas\_size^2 * num\_circles)`$

The total complexity is therefore: $`O(num\_circles * log(num\_circles) + canvas\_size^2 * num\_circles)`$

For realistic scenarios, the pixel processing phase dominates the execution time due to its higher complexity and the intensive floating-point operations required for color blending.

### 2.4 - Parallel implementation
The parallel implementation of the Image Renderer is implemented in the *renderParallel* method. 

While the sorting algorithm's complexity remains the same as in the sequential implementation, as it is applied globally, the color blending operation represents the true bottleneck of the sequential implementation. Therefore, this operation presents the main opportunity for improving program performance.

This operation can be performed **independently** on each pixel of the 2D surface, making it possible to leverage OpenMP directives to execute these operations simultaneously on multiple pixels across num_threads threads, thus significantly reducing the total Execution Time.

The total complexity of the *Image Renderer* therefore becomes:
$$`O(num\_circles * log(num\_circles) + (canvas\_size * canvas\_size * num\_circles)/(overhead*num\_threads))`$$
where `num_threads > 1`, and a small component `overhead < 1` accounts for the overhead of creating multiple parallel threads.

By placing a simple `#pragma omp parallel` directive before the nested loops and `#pragma omp for` at the *outer* loop level, work can be easily distributed among a team of threads, in a number that is automatically decided. This initial implementation provides a baseline for parallel performance.

**CODICE PARALLEL + FOR vs SEQUENTIAL**

These two directives were then merged into the combined `#pragma omp parallel for` construct, which provides a more concise and potentially more efficient implementation. This refinement eliminates potential overhead from separate parallel region creation and work distribution directives.

**CODICE PARALLEL + FOR vs PARALLEL FOR vs SEQUENTIAL**

To investigate the impact of the number of forked threads on the general performance, the `num_threads` clause was introduced. This addition allows control over the number of parallel threads, enabling to study how the program scales with different thread counts and to identify potential bottlenecks in the parallelization strategy.

Having two nested for loops at the pixel processing level, another interesting clause that could be evaluated was the `collapse(2)` clause, which merges the two nested loops into a single parallel region, potentially providing better load balancing and work distribution among threads.

The final optimization phase explored different scheduling strategies and their impact on performance. OpenMP provides various scheduling options, including `static` and `dynamic` approaches, each with its own `block_size` parameter. This exploration aims to find the optimal balance between work distribution overhead and effective parallel execution.

## 3 - Performance Analysis and Results

<p float="left", align="center">
  <img src="https://github.com/giovancombo/ParallelImageRenderer/blob/main/images/plots/execution_time_vs_circles.png" width="60%" />
</p>
<p align="center"><b>Figure 4</b> <i>Execution times for rendering canvas with different numbers of circles.</i></p>

### 3.1 - Thread Scaling Analysis
- Comparison between separate and combined parallel directives
- Analysis of overhead costs
  
- Performance scaling with different thread counts
- Identification of optimal thread count
- Discussion of potential bottlenecks and their impact

<p float="left", align="center">
  <img src="https://github.com/giovancombo/ParallelImageRenderer/blob/main/images/plots/implementations_comparison_circles2000.png" width="70%" />
</p>
<p float="left", align="center">
  <img src="https://github.com/giovancombo/ParallelImageRenderer/blob/main/images/plots/implementations_comparison_circles50000.png" width="70%" />
</p>
<p align="center"><b>Figure 5a</b> <i>Rendering of 1000, 10000, 50000 and 100000 circles projected on a (1024 x 1024) canvas.</i></p>

<p float="left", align="center">
  <img src="https://github.com/giovancombo/ParallelImageRenderer/blob/main/images/plots/execution_time_implementations_circles2000.png" width="70%" />
</p>
<p float="left", align="center">
  <img src="https://github.com/giovancombo/ParallelImageRenderer/blob/main/images/plots/execution_time_implementations_circles50000.png" width="70%" />
</p>
<p align="center"><b>Figure 5b</b> <i>Rendering of 1000, 10000, 50000 and 100000 circles projected on a (1024 x 1024) canvas.</i></p>

#### Loop Collapse Impact
- Effect of collapse clause on performance
- Analysis of workload distribution
- Comparison with non-collapsed implementation

#### False Sharing Analysis
- Identification of false sharing issues
- Implementation of padding solution
- Performance impact of cache line optimization

2) Si può notare come contrastare il false sharing provoca in realtà un grande peggioramento in termini di execution time. Questo accade in quanto gli errori provocati dal false sharing sono molto trascurabili rispetto all'overhead che si crea in più per evitarlo. Si può notare infatti che le implementazioni che risolvono false sharing utilizzando cache line paddate a 32 o 64 byte hanno execution time addirittura superiori all'implementazione base con direttiva nell'inner loop, ovvero una implementazione effettuata con una messa in pratica scorretta delle direttive parallel.

4) E' poi chiaro che all'aumentare del numero di cerchi generati, l'execution time totale aumenta sensibilmente.

### 3.2 - Scheduling Strategy Evaluation
- Comparison of static vs dynamic scheduling
- Impact of different block sizes
- Analysis of load balancing effectiveness

<p float="left", align="center">
  <img src="https://github.com/giovancombo/ParallelImageRenderer/blob/main/images/plots/block_size_analysis_circles2000.png" width="70%" />
</p>
<p float="left", align="center">
  <img src="https://github.com/giovancombo/ParallelImageRenderer/blob/main/images/plots/block_size_analysis_circles50000.png" width="70%" />
</p>
<p align="center"><b>Figure 6a</b> <i>Rendering of 1000, 10000, 50000 and 100000 circles projected on a (1024 x 1024) canvas.</i></p>

<p float="left", align="center">
  <img src="https://github.com/giovancombo/ParallelImageRenderer/blob/main/images/plots/execution_time_scheduling_circles2000.png" width="70%" />
</p>
<p float="left", align="center">
  <img src="https://github.com/giovancombo/ParallelImageRenderer/blob/main/images/plots/execution_time_scheduling_circles50000.png" width="70%" />
</p>
<p align="center"><b>Figure 6b</b> <i>Rendering of 1000, 10000, 50000 and 100000 circles projected on a (1024 x 1024) canvas.</i></p>

1) Mie considerazioni: si può facilmente notare come, per qualunque numero fissato di cerchi, lo scheduling dinamico porti generalmente a speedup maggiori rispetto allo scheduling statico, con valori tra 0.5 e 1.5 più alti. Tuttavia, l'efficienza dello scheduling dinamico è solo di poco maggiore rispetto allo statico, ed evidentemente cala all'aumentare del numero di threads, rimanendo però sempre leggermente maggiore di quella dello statico.
A prescindere dal tipo di scheduling, l'effetto di una diversa block_size è generalmente molto limitato, ad eccezione di alcuni casi particolari come lo scheduling dinamico con 2000 cerchi.

3) Lo speedup maggiore nello scheduling dinamico porta evidentemente anche a execution times minori nello scheduling dinamico.

## 4 - Conclusions and Future Work
- Summary of key findings
- Optimal configuration recommendations
- Potential areas for further optimization
- Lessons learned and best practices

Un semplice *Image Renderer* è stato implementato in una versione sequenziale, e una parallela sfruttando le funzionalità di OpenMP. Test quantitativi delle performance in termini di speedup ed efficiency hanno provato che la parallelizzazione del codice utilizzando le direttive di OpenMP hanno nettamente migliorato le performance di esecuzione.
La migliore combinazione di direttive e clausole è risultata essere

DIRETTIVA, PARTE DI CODICE

avendo prodotto uno speedup di ... e un'efficiency di ...
