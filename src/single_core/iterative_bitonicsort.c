#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    /* check of input parameters size */
    if (argc != 4) {
        printf("Error! Usage: num_elements  read_input_data_filename  write_sorted_sequence_filename\n\n");
        return 0;
    }

    MPI_Init(&argc, &argv);
    const int size = atoi(argv[1]);
    int check_power_2 = size;
    /* Check if number of elements is a power of two */
    if (check_power_2 == 0) {
        printf("Nothing to sort\n");
        MPI_Finalize();
        return 0;
    } else {
        while (check_power_2 != 1) {
            if (check_power_2 % 2 != 0) {
                printf("The number of elements isn't a power of two\n");
                MPI_Finalize();
                return 0;
            }
            check_power_2 /= 2;
        }
    }
    /* array that will contain read_input_data_filename */
    int* array = (int*)malloc(size * sizeof(int));
    
    /* time variables */
    double start, end;

    /* read data from the file */
    printf("Reading input file\n");
    start = MPI_Wtime();

    FILE* file = fopen(argv[2], "r");
    /* check file */
    if (file == NULL) {
        printf("Unable to open file %s\n", argv[2]);
        return 0;
    }
    /* read file  */
    int i = 0;
    for (i = size - 1; i >= 0; --i) {
        fscanf(file, "%d\n", &array[i]);
    }
    fclose(file);

    end = MPI_Wtime();
    printf("Read input file: %f\n\n", end - start);

    int j, k, dest, dimensions = -1;

    /* Compute dimensions */
    /* use of dest as temp */
    dest = size;
    while (dest) {
        ++dimensions;
        dest >>= 1;
    }

    printf("Start Bitonic Sort\n");
    start = MPI_Wtime();

    /* iterative Bitonic Sort */
    for (i = 0; i < dimensions; ++i) {
        for (j = i; j >= 0; --j) {
            for (k = (size - 1); k >= 0; --k) {
                if (((k >> (i + 1)) & 0x01) != ((k >> j) & 0x01)) {
                    dest = k ^ (1 << j);
                    if (array[k] < array[dest]) {
                        /* swap */
                        array[k] = array[k] ^ array[dest];
                        array[dest] = array[k] ^ array[dest];
                        array[k] = array[k] ^ array[dest];
                    }
                }
            }
        }
    }

    end = MPI_Wtime();
    printf("Bitonic Sort time: %f\n\n", end - start);

    /* create file that will contain the sorted sequence */
    printf("Saving ordered sequence\n");
    start = MPI_Wtime();

    file = fopen(argv[3], "w");
    /* write on the file */
    for (i = 0; i < size; ++i) {
        fprintf(file, "%d\n", array[i]);
    }

    /* close it */
    if (!fclose(file)) {
        end = MPI_Wtime();
        printf("Writing time: %f\n", end - start);
        printf("Sequence successfully sorted! Check %s \n", argv[3]);
    }

    free(array);
    MPI_Finalize();
    return 0;
}