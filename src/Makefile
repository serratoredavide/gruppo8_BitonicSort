dEc_bitonicsort.o: single_core/bitonicsort.c
	mpcc single_core/bitonicsort.c -O3 -o dEc_bitonicsort.o

iterative_bitonicsort.o: single_core/iterative_bitonicsort.c
	mpcc single_core/iterative_bitonicsort.c -O3 -o iterative_bitonicsort.o

parallel_bitonicsort.o: parallel_bitonicsort.c
	mpcc parallel_bitonicsort.c -O2 -o parallel_bitonicsort.o

.PHONY: clean

clean:
	rm *.o