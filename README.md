# Parallel Image Renderer in C++ with OpenMP

Mid Term Assignment for the *Parallel Programming for Machine Learning* course, held by professor Marco Bertini at University of Florence, Italy.

Il report descrive lo sviluppo di un semplice Image Renderer in C++, e ne confronta quantitativamente l'implementazione sequenziale e parallela, mostrando le grandi potenzialità dell'utilizzo della computazione parallela e multithread tramite l'uso della API di OpenMP. Vengono effettuate misurazioni di speedup ed efficiency per differenti configurazioni d'ambiente, iperparametri, oltre a diverse combinazioni di direttive e clausole.

<img src="https://github.com/giovancombo/ImageRenderer_PPMLMidTerm/blob/main/images/outputs/output_stretched2048_20000.png">


## 1 - Introduction

In questo report verrà sviluppato un semplice *Image Renderer*, le cui operazioni consistono nella generazione di un certo numero di cerchi nello spazio 3D a diversi livelli di profondità, e la loro proiezione su una superficie 2D rettangolare, a simulare una tela, avendo cura che venga rispettato l'ordine corretto di mistura dei colori per ogni pixel nel caso di cerchi sovrapposti tra loro. Per questo progetto, imponiamo che la superficie 2D abbia forma quadrata, di dimensione *(canvas_size x canvas_size)*.

Ogni cerchio creato è dotato di 6 attributi:
- Coordinate (x, y) nello spazio 2D: randomiche nell'intervallo [0, canvas_size];
- Coordinata (z) indicante la profondità: randomica nell'intervallo [0, 1000];
- Raggio (radius): randomico nell'intervallo [0, 50];
- Colore (Color): struct definita dalle componenti (r,g,b), randomiche nell'intervallo [0, 1];
- Trasparenza (alpha): randomica nell'intervallo [0.1, 0.5].

La coordinata z, indicante le "profondità" nello spazio 3D, di fatto è un indicatore delle distanze di tali cerchi dalla superficie 2D su cui verranno proiettati, e pertanto consente di definire un ordine spaziale, a livelli, di proiezione. Ricavare correttamente l'ordinamento dei cerchi è di fondamentale importanza per la riusci ta corretta dell'operazione di rendering, a causa del fatto che ogni cerchio è semi-trasparente, e quindi il colore di un cerchio non coprirà completamente il colore di un cerchio sottostante, ma avverrà un blending che produrrà un colore finale secondo la formula di *alpha blending*:

$$Value_{color} = (1 - α)Value_{color}^A + αValue_{color}^B$$

where $`color = \{r,g,b\}`$, and $A$ and $B$ are two circles che dopo l'ordinamento sono in posizioni consecutive, rispettivamente più vicino e più lontano rispetto alla superficie 2D. Più semplicemente, considerando i cerchi come strati su un foglio, il cerchio $A$ si trova dietro, o sotto, al cerchio $B$.

<p float="center", align="center">
  <img src="https://github.com/giovancombo/ImageRenderer_PPMLMidTerm/blob/main/images/blending.png" width="40%" />
</p>

Pertanto, al termine della creazione dei cerchi, è importante effettuare per ciascun pixel della superficie 2D l'ordinamento dei cerchi che si proietterebbero su tale pixel secondo il valore della coordinata z, e calcolare il colore finale del pixel secondo la logica dettata dal fenomeno di alpha blending.

L'appartenenza di un certo cerchio a un pixel è dettata da una precisa regola: un cerchio avente centro di coordinate *(xc, yc)* appartiene al pixel di coordinate *(x, y)* se la distanza tra i due punti è inferiore al raggio del cerchio.

<p float="center", align="center">
  <img src="https://github.com/giovancombo/ImageRenderer_PPMLMidTerm/blob/main/images/pixelincircle.png" width="25%" />
</p>

Il risultato finale dell'*Image Renderer* è un'immagine di dimensione *(canvas_size x canvas_size)* di cerchi colorati semi-trasparenti sovrapposti tra loro.

<p float="left", align="center">
  <img src="https://github.com/giovancombo/ImageRenderer_PPMLMidTerm/blob/main/images/outputs/output_1024_1000.png" width="24%" />
  <img src="https://github.com/giovancombo/ImageRenderer_PPMLMidTerm/blob/main/images/outputs/output_1024_10000.png" width="24%" />
  <img src="https://github.com/giovancombo/ImageRenderer_PPMLMidTerm/blob/main/images/outputs/output_1024_50000.png" width="24%" />
  <img src="https://github.com/giovancombo/ImageRenderer_PPMLMidTerm/blob/main/images/outputs/output_1024_100000.png" width="24%" />
</p>
<p align="center"><i>Immagini renderizzate con <b>1000</b>, <b>10000</b>, <b>50000</b> e <b>100000</b> cerchi su una superficie <b>(1024 x 1024)</b>.</i></p>

Oltre alla corretta creazione di un'immagine nel rispetto delle regole di blending, il principale lo scopo del progetto è quello di produrre un'implementazione C++ del programma in versione sequenziale e parallela, sfruttando le funzionalità di computazione parallela della API di **OpenMP**. Si dovrà valutare quantitativamente l'effetto della parallelizzazione misurando metriche specifiche come **Speedup** ed **Efficiency**.


## 2 - Method and Code

Dal punto di vista strettamente operativo, un *Image Renderer* così come descritto nella sezione 1 prevede, di fatto, l'esecuzione di due operazioni principali:

1) *sorting* dei cerchi rispetto alla propria distanza dalla superficie 2D, indicata dalla propria coordinata z;
2) *alpha blending* sui singoli pixel, per ricavare la corretta distribuzione finale dei colori.

Dato che la disposizione dei cerchi nello spazio è randomica, è chiaro che, in fase di proiezione sulla superficie 2D, pixel diversi della superficie si troveranno ad appartenere a un numero di cerchi diverso, aventi colori diversi, che produrranno un colore finale del pixel diverso. Inoltre, l'appartenenza di un cerchio a un pixel (e viceversa) è una condizione che non dipende da nessun pixel all'infuori del pixel stesso.

È quindi di fondamentale importanza notare che entrambe le operazioni si possono effettuare in modo **indipendente** su ognuno dei $`canvas\_size^2`$ pixel della superficie 2D, rendendo di fatto lo sviluppo di questo *Image Renderer* un problema *imbarazzantemente parallelo*, garantendo una parallelizzazione promettente del codice con OpenMP.

### 2.1 - Hardware and Software setup

Hardware; versione di OpenMP e di C++; studio rapido del CMake.

### 2.2 - Code general structure

Il progetto si basa su una classe principale **`Renderer`**, che rappresenta il nucleo dell'implementazione. Qui sono gestite tutte le operazioni necessarie per il rendering dei cerchi. La classe mantiene le dimensioni della superficie 2D, definita *canvas* (con, per semplicità, `width = height = canvas_size`), una collezione di cerchi da renderizzare memorizzata in un *vector*, e il canvas stesso implementato come array lineare di pixel, dove ogni pixel è rappresentato da un oggetto `Color` che ne definisce i valori RGB.

A livello operazionale, la classe implementa tre metodi privati fondamentali: `processPixel`, che calcola il colore finale di un pixel considerando tutti i cerchi che lo contengono, `isPixelInCircle`, che determina se un punto appartiene a un cerchio, e `alphaBlending`, che gestisce la composizione di due colori considerando l'effetto dovuto alla trasparenza dei cerchi.

L'interfaccia pubblica della classe include, oltre al costruttore che inizializza il canvas e al distruttore che ne libera la memoria, i due metodi `renderSequential` e `renderParallel` che implementano rispettivamente la versione sequenziale e parallela del programma.

Gli output di questi ultimi metodi sono due struct, rispettivamente `SequentialResult` e `ParallelResult`, contenenti ciascuna i dati e le misurazioni essenziali per la caratterizzazione quantitativa di ogni singola run.

Nel file `main.cpp` è possibile definire diverse costanti, espresse sotto forma di vector in modo da rendere possibili multiple combinazioni nella stessa run:
- `CANVAS_SIZES` = dimensione della superficie 2D: {256, 512, 1024, 2048}
- `NUM_CIRCLES` = numero di cerchi creati: {1000, 2000, 5000, 10000, 15000, 20000, 50000, 100000}
- `NUM_THREADS` = numero di threads creati in fase di fork: {2, 3, 4, 6, 8, 12, 16, 24, 32, 64}
- `BLOCK_SIZES` = numero di blocchi/chunk per lo scheduling: {12, 16, 24, 32, 64}

### 2.3 - Sequential implementation

L'implementazione sequenziale dell'*Image Renderer* si configura nel metodo `renderSequential`.

Inizialmente viene effettuato l'ordinamento globale del vettore di cerchi, un'operazione di complessità $`O(num\_circles*log(num\_circles))`$. In seguito effettua, tramite due for loop innestati, il processing uno ad uno di ciascun pixel della superficie 2D, consistente nel calcolo sequenziale del colore finale assunto dal pixel, seguendo la formula di alpha blending. La complessità di tale operazione è $`O(canvas\_size * canvas\_size * num\_circles)`$, in quanto ogni pixel richiede un controllo su tutti i cerchi per verificarne l'appartenenza al pixel.

Si raggiunge quindi una complessità totale di

$$`O(num\_circles * log(num\_circles) + canvas\_size * canvas\_size * num\_circles)`$$.

Per numeri realistici, la componente dominante sarà sempre quella dell'operazione di blending, poiché richiede molte più operazioni rispetto alla fase di ordinamento iniziale dei cerchi.

### 2.4 - Parallel implementation

L'implementazione parallela dell'Image Renderer si configura nel metodo *renderParallel*. 

Se la complessità dell'algoritmo di sorting è la medesima di quella dell'implementazione sequenziale, in quanto esso viene applicato a livello globale, l'operazione di blending dei colori è il vero collo di bottiglia dell'implementazione sequenziale, e quindi quella su cui è possibile migliorare le performance del programma.

Tale operazione è infatti facilmente effettuabile in modo **indipendente** su ciascun pixel della superficie 2D, e quindi è possibile sfruttare le direttive di OpenMP per garantire l'esecuzione di tale operazione su più pixel su num_threads threads simultaneamente, abbattendo quindi l'Execution Time totale.

La complessità totale dell'*Image Renderer*, infatti, diventerebbe

$$`O(num\_circles * log(num\_circles) + (canvas\_size * canvas\_size * num\_circles)/(overhead*num\_threads))`$$

With `num_threads > 1`, e una piccola componente `overhead < 1` dovuta all'overhead per la creazione di multipli threads paralleli.

La strategia di parallelizzazione prevede dunque di agire sul for loop innestato, nel quale avviene l'operazione di processing indipendente dei pixel, applicando per prima cosa la direttiva base `#pragma omp parallel for` fuori dal for loop esterno. In questo modo, l'operazione di *fork* e conseguente creazione dei threads avviene una sola volta, risultando in un quantitativo accettabile di overhead. Applicare la direttiva solamente al for loop interno avrebbe portato a un'operazione di *fork-join* ripetuta per ogni singola riga della superficie 2D, risultando in una gestione fortemente impattante dell'overhead di parallelizzazione, dovuto alla continua creazione e distruzione dei threads. Per fini puramente dimostrativi, ho valutato entrambi gli approcci.

PLOT

Poi, implementazione passo passo di clausole diverse partendo dalla direttiva base.
1. parallel + for
2. parallel for inner loop
3. parallel for outer loop
4. parallel for num_threads(num_threads)
5. parallel for num_threads(num_threads) schedule(static/dynamic, block_size)
6. parallel for collapse(2) num_threads(num_threads) schedule(static/dynamic, block_size)

#### 2.4.1 - Addressing False Sharing

## 3 - Tests and Results
Serie di plot

### 3.1 - Number of Threads

### 3.2 - Number of circles

### 3.3 - Type of scheduling

#### 3.3.1 - Effect of the block size

### 3.4 - Using the collapse(2) clause

## 4 - Conclusion

Un semplice *Image Renderer* è stato implementato in una versione sequenziale, e una parallela sfruttando le funzionalità di OpenMP. Test quantitativi delle performance in termini di speedup ed efficiency hanno provato che la parallelizzazione del codice utilizzando le direttive di OpenMP hanno nettamente migliorato le performance di esecuzione.
La migliore combinazione di direttive e clausole è risultata essere

DIRETTIVA, PARTE DI CODICE

avendo prodotto uno speedup di ... e un'efficiency di ...
