# Parallel Image Renderer in C++ with OpenMP

Mid Term Assignment for the Parallel Programming for Machine Learning course, held by professor Marco Bertini at University of Florence, Italy.

## 1 - Introduction
Presentazione del problema, menzionando i vantaggi ottenibili tramite l'utilizzo di OpenMP.
Logica dietro all'operazione di base del rendering: 2 operazioni = sorting in base alla z + processing del colore con alpha blending.

## 2 - Code
Logica dietro all'operazione di base del rendering: 2 operazioni = sorting in base alla z + processing del colore con alpha blending

### 2.1 - Hardware and Software setup
Hardware; versione di OpenMP e di C++; studio rapido del CMake.

### 2.2 - Hyperparameters
Valori di range del raggio dei cerchi, valori di alpha blending, ampiezza del canvas, numero di cerchi creati.

### 2.3 - Sequential implementation
Elenco degli svantaggi dell'implementazione sequenziale, dei punti deboli che causano rallentamenti; complessità operazionale con l'O grande.

### 2.4 - Parallel implementation
Presentazione iniziale delle aspettative che ho riguardo l'implementazione parallela, ovvero lo speedup.
Poi, implementazione passo passo di clausole diverse partendo dalla direttiva base.
1. parallel + for
2. parallel for inner loop
3. parallel for outer loop
4. parallel for num_threads(num_threads)
5. parallel for num_threads(num_threads) schedule(static/dynamic, block_size)
6. parallel for collapse(2) num_threads(num_threads) schedule(static/dynamic, block_size)

## 3 - Tests and Results
Serie di plot

### 3.1 - Number of Threads

### 3.2 - Number of circles

### 3.3 - Type of scheduling

#### 3.3.1 - Effect of the block size

### 3.4 - Using collapse(2)

## 4 - Conclusion
recap totale molto rapido dell'esperienza, menzionando la configurazione di iperparametri migliore, e la configurazione di direttiva+clausole migliore per questo problema.
