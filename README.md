# Algoritmo di Bitonic Sort Parallelo
### Relazione di progetto per il corso di Calcolo Parallelo
Università degli Studi di Padova | Dipartimento di Ingegneria Informatica


#### Autori
Miani Eleonora - M. 1206908 | Serratore Davide - M.1207660 | Vesco Omar - M.1197699

---


## Contenuto della repository

	Relazione.pdf					\\ relazione del progetto
	data/XXX_numbers.txt				\\ input file con XXX elementi da ordinare

	src/Makefile					\\ makefile per compilare i programmi del pacchetto
	src/parallel_bitonicsort.c			\\ sorgente algoritmo di bitonic sort parallelo
	src/single_core/bitonicsort.c			\\ sorgente algoritmo di bitonic sort single core con approccio divide&conquer
	src/single_core/iterative_bitonicsort.c		\\ sorgente algoritmo di bitonic sort single core iterativo

	src/parallel.job				\\ job file per eseguire parallel_bitonicsort.c sul power7
	src/singlecore.job				\\ job file per eseguire bitonicsort.c e iterative_bitonicsort.c sul power7



## Compilazione ed esecuzione

### Test parallelo


I file forniti così configurati permettono di eseguire il test del bitonic sort parallelo con 8 processori e 8388608 elementi da ordinare.
I file necessari vengono compilati con:

	make parallel_bitonicsort.o

Per far partire il job sul power7 eseguire il comando:

	llsubmit parallel.job

Per poter eseguire lo stesso test con diverse configurazioni di input e di processori è sufficiente modificare il file `parallel.job`: 
- sostituire a 8388608 il numero di elementi da ordinare (i file disponibili possono essere verificati all'interno della cartella `data/`)
- a `8` in `-procs 8` e in `total_tasks = 8` il numero di processori che si vuole utilizzare

Nota: deve essere modificato anche il file `src/host.list` per rispecchiare il numero di processori scelto.


### Test single core

I file forniti così configurati permettono di eseguire il test del bitonic sort single core 8388608 elementi da ordinare.
I file necessari vengono compilati con:

	make dEc_bitonicsort.o
	make iterative_bitonicsort.o

Per far partire il job sul power7 eseguire il comando:

	llsubmit singlecore.job

Per poter eseguire lo stesso test con diverse configurazioni di input è sufficiente modificare il file `singlecore.job `:
- sostituire a 8388608 il numero di elementi da ordinare (i file disponibili possono essere verificati all'interno della cartella `data/`)

Nota: deve essere modificato anche il file `src/host.list` per rispecchiare il numero di processori scelto. In `src/singlecore/` è già presente un file `host.list` correttamente configurato per questa opzione.

Con il comando `make clean` tutti i file eseguibili verranno eliminati.




