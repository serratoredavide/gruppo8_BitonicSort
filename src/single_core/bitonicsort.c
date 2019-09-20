#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief : Compare a and b in order to: make a > b if sorting_type = 0, a < b otherwise
 * 
 * @param a : integer to compare
 * @param b : integer to compare
 * @param sorting_type : 0 for increasing order, 1 for decreasing order
 */
void compex(int *a, int *b, unsigned char sorting_type);

/**
 * @brief sort numbers_array
 * 
 * @param numbers_array : array of integer
 * @param array_len : size of numbers_array
 * @param sorting_type : 0 for increasing order, 1 for decreasing order
 */
void bitonic_sort(int *numbers_array, unsigned int array_len, unsigned char sorting_type);

/**
 * @brief merge a bitonic sequence
 * 
 * @param numbers_array : array of integer
 * @param array_len : size of numbers_array
 * @param sorting_type : 0 for increasing order, 1 for decreasing order
 */
void bitonic_merge(int *numbers_array, unsigned int array_len, unsigned char sorting_type);

int main(int argc, char **argv) {
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
    int *array = (int *)malloc(size * sizeof(int));

    /* time variables */
    double start, end;

    /* read data from the file */
    printf("Reading input file\n");
    start = MPI_Wtime();

    FILE *file = fopen(argv[2], "r");
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

    /* Bitonic Sort */
    printf("Start Bitonic Sort\n");
    start = MPI_Wtime();

    bitonic_sort(array, size, 0);

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

void compex(int *a, int *b, unsigned char sorting_type) {
    if (sorting_type == 0) {
        /* increasing order */
        if (a[0] > b[0]) {
            *a = *a ^ *b;
            *b = *a ^ *b;
            *a = *a ^ *b;
        }
    } else {
        /* decreasing order */
        if (a[0] < b[0]) {
            *a = *a ^ *b;
            *b = *a ^ *b;
            *a = *a ^ *b;
        }
    }
}

void bitonic_merge(int *numbers_array, unsigned int array_len, unsigned char sorting_type) {
    if (array_len == 2) {
        compex(numbers_array, numbers_array + 1, sorting_type);
    } else {
        for (int i = 0; i < array_len / 2; ++i) {
            compex(numbers_array + i, numbers_array + array_len / 2 + i, sorting_type);
        }
        bitonic_merge(numbers_array, array_len / 2, sorting_type);
        bitonic_merge(numbers_array + (array_len / 2), array_len / 2, sorting_type);
    }
}

void bitonic_sort(int *numbers_array, unsigned int array_len, unsigned char sorting_type) {
    if (array_len == 2)
        compex(numbers_array, numbers_array + 1, sorting_type);
    else {
        bitonic_sort(numbers_array, array_len / 2, 0);
        bitonic_sort(numbers_array + (array_len / 2), array_len / 2, 1);
        bitonic_merge(numbers_array, array_len, sorting_type);
    }
}