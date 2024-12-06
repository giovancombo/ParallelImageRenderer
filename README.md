# Parallel Image Renderer in C++ with OpenMP

Mid Term Assignment for the *Parallel Programming for Machine Learning* course, held by professor Marco Bertini at University of Florence, Italy.

Il report descrive lo sviluppo di un semplice Image Renderer in C++, e ne confronta quantitativamente l'implementazione sequenziale e parallela, mostrando le grandi potenzialità dell'utilizzo della computazione parallela tramite la API di OpenMP. Vengono effettuate misurazioni di speedup ed efficiency per differenti configurazioni d'ambiente, iperparametri, oltre a diverse combinazioni di direttive e clausole.


## 1 - Introduction

In questo report verrà sviluppato un semplice Image Renderer, le cui operazioni consistono nella generazione di un certo numero di cerchi nello spazio 3D a diversi livelli di profondità, e la loro proiezione su una superficie 2D rettangolare, a simulare una tela, avendo cura che venga rispettato l'ordine corretto di mistura dei colori per ogni pixel nel caso di cerchi sovrapposti tra loro. Per questo progetto, imponiamo che la superficie 2D abbia forma quadrata, di dimensione ‘canvas_size‘.

Ogni cerchio creato è dotato di 6 attributi:
- Coordinate (x,y) nello spazio 2D: randomiche nell'intervallo [0, canvas_size];
- Coordinata (z) indicante la profondità: randomica nell'intervallo [0, 1000];
- Raggio (radius): randomico nell'intervallo [0, 50];
- Colore (color): vettore definito dalle componenti (r,g,b), randomiche nell'intervallo [0, 1];
- Trasparenza (alpha): randomica nell'intervallo [0.1, 0.5].

La coordinata z, indicante le "profondità" nello spazio 3D, di fatto è un indicatore delle distanze di tali cerchi dalla superficie 2D su cui verranno proiettati, e pertanto consente di definire un ordine spaziale, a livelli, di proiezione. Ricavare correttamente l'ordinamento dei cerchi è di fondamentale importanza per la riusci ta corretta dell'operazione di rendering, a causa del fatto che ogni cerchio è semi-trasparente, e quindi il colore di un cerchio non coprirà completamente il colore di un cerchio sottostante, ma avverrà un blending che produrrà un colore finale secondo la formula di *alpha blending*:

FORMULA ALPHA BLENDING

Pertanto, al termine della creazione dei cerchi, è importante effettuare per ciascun pixel della superficie 3D (canvas_size x canvas_size) l'ordinamento dei cerchi che si trovano su tale pixel secondo il valore della coordinata z, e calcolare il colore finale del pixel secondo la logica dettata dal fenomeno di alpha blending.

L'appartenenza di un certo cerchio a un pixel è dettata da una precisa regola: un cerchio avente centro di coordinate (xc,yc) appartiene al pixel di coordinate (x,y) se la distanza tra i due punti è inferiore al raggio del cerchio. Il metodo ’isPixelInCircle’ definisce tale valutazione.

FORMULA + IMMAGINE + CODICE

Sostanzialmente, questo Image Renderer si compone di due operazioni principali:

Logica dietro all'operazione di base dell'image renderer, menzionando brevemente le operazioni che potrebbero essere parallelizzabili: 2 operazioni = sorting in base alla z + processing del colore con alpha blending.

## 2 - Code
Presentazione del problema e riassunto breve dell'approccio, menzionando i vantaggi ottenibili tramite l'utilizzo di OpenMP.

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
