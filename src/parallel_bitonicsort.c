#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUMBER_PARAMETERS 4
#define MASTER_PROCESS 0

/**
 * @brief compare a and  b
 * 
 * @param a : integer
 * @param b : integer
 * @return int 
 */
int comparisonFunc(const void* a, const void* b);

/**
 * @brief Compares and exchanges data according to bitonic sort. Get the maximum numbers
 * 
 * @param j : index for bitonic sort
 * @param rank : number of the processor
 * @param local_numbers : array that contains partitioned number of processor "rank"
 * @param dim_array : size of the array
 * @param ordered_array : it contains the new local_numbers
 */
void comp_exchange_max(int j, int rank, int* local_numbers, int dim_array, int* ordered_array);

/**
 * @brief Compares and exchanges data according to bitonic sort. Get the minimum numbers
 * 
 * @param j : index for bitonic sort
 * @param rank : number of the processor
 * @param local_numbers : array that contains partitioned number of processor "rank"
 * @param dim_array : size of the array
 * @param ordered_array : it contains the new local_numbers
 */
void comp_exchange_min(int j, int rank, int* local_numbers, int dim_array, int* ordered_array);

int main(int argc, char** argv) {
    int i, j, k;
    int total_dim, rank, size, dimensions, check_power_2;
    double start, end;               /* time variables */
    int* array = NULL;               /* array that contains all numbers from the file */
    int* partitioned_numbers = NULL; /* local partitioned array of a processor */
    int* final_data = NULL;          /* array that contains sorted sequence */
    int* ordered_array = NULL;       /* new partitioned numbers after compare_exchange */

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    /* check of input parameters size */
    if (argc != NUMBER_PARAMETERS && rank == MASTER_PROCESS) {
        printf("Error! Usage: num_elements  read_input_data_filename  write_sorted_sequence_filename\n\n");
        MPI_Abort(MPI_COMM_WORLD, 0);
    }

    int dest = size;

    /* compute dimension of the hypercube */
    dimensions = -1;
    while (dest) {
        ++dimensions;
        dest >>= 1;
    }

    /* get number of elements */
    total_dim = atoi(argv[1]);

    check_power_2 = total_dim;

    if (rank == MASTER_PROCESS) {
        /* check if number of elements is a power of two */
        if (check_power_2 == 0) {
            printf("Nothing to sort\n");
            MPI_Abort(MPI_COMM_WORLD, 0);
        } else {
            while (check_power_2 != 1) {
                if (check_power_2 % 2 != 0) {
                    printf("The number of elements isn't a power of two\n");
                    MPI_Abort(MPI_COMM_WORLD, 0);
                }
                check_power_2 /= 2;
            }
        }

        if (total_dim < size) {
            printf("The number of processors requested is too high compared to the number of element to order\n");
            MPI_Abort(MPI_COMM_WORLD, 0);
        }

        /* array that will contain read_input_data_filename */
        array = (int*)malloc(total_dim * sizeof(int));
        /* read data from the file */
        printf("Reading input file\n");
        start = MPI_Wtime();
        FILE* file = fopen(argv[2], "r");
        if (file == NULL) {
            printf("Unable to open file %s\n", argv[2]);
            MPI_Abort(MPI_COMM_WORLD, 0);
        }

        for (i = total_dim - 1; i >= 0; --i) {
            fscanf(file, "%d\n", &array[i]);
        }
        fclose(file);
        end = MPI_Wtime();
        printf("Read input file: %f\n\n", end - start);
    }

    int dim_array = total_dim / size;
    /* local array of partitioned numbers */
    partitioned_numbers = (int*)malloc(dim_array * sizeof(int));
    /* sorted local array */
    ordered_array = (int*)malloc(dim_array * sizeof(int));

    /* send data from process 0 to all other processors */
    MPI_Scatter(&array[0], dim_array, MPI_INT, partitioned_numbers, dim_array, MPI_INT, 0, MPI_COMM_WORLD);

    free(array);

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == MASTER_PROCESS) {
        printf("Start Bitonic Sort\n");
        start = MPI_Wtime();
    }
    /* sort local array */
    qsort(partitioned_numbers, dim_array, sizeof(int), comparisonFunc);

    /* Bitonic Sort */
    for (i = 0; i < dimensions; ++i) {
        for (j = i; j >= 0; --j) {
            if (((rank >> (i + 1)) & 0x01) != ((rank >> j) & 0x01)) {
                comp_exchange_max(j, rank, partitioned_numbers, dim_array, ordered_array);
                /* update local array */
                for (k = dim_array - 1; k >= 0; --k) {
                    partitioned_numbers[k] = ordered_array[k];
                }
            } else {
                comp_exchange_min(j, rank, partitioned_numbers, dim_array, ordered_array);
                /* update local array */
                for (k = dim_array - 1; k >= 0; --k) {
                    partitioned_numbers[k] = ordered_array[k];
                }
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == MASTER_PROCESS) {
        end = MPI_Wtime();
        printf("Bitonic Sort time: %f\n\n", end - start);

        final_data = (int*)malloc(sizeof(int) * total_dim);
    }
    free(ordered_array);

    /* gather all data in the master process */
    MPI_Gather(partitioned_numbers, dim_array, MPI_INT, final_data, dim_array, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD);
    free(partitioned_numbers);

    if (rank == MASTER_PROCESS && final_data != NULL) {
        printf("Saving ordered sequence\n");
        start = MPI_Wtime();
        FILE* file;
        /* create file that will contain the sorted sequence */
        file = fopen(argv[3], "w");
        /* write on the file */
        for (i = 0; i < total_dim; ++i) {
            fprintf(file, "%d\n", final_data[i]);
        }

        /* close it */
        if (!fclose(file)) {
            end = MPI_Wtime();
            printf("Writing time: %f\n", end - start);
            printf("Sequence successfully sorted! Check %s \n", argv[3]);
        }

        free(final_data);
    }

    MPI_Finalize();
    return 0;
}

int comparisonFunc(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

void comp_exchange_max(int j, int rank, int* local_numbers, int dim_array, int* ordered_array) {
    int message_receive[dim_array];
    int i, k, q;
    MPI_Recv(&message_receive,
             dim_array,
             MPI_INT,
             rank ^ (1 << j),
             0,
             MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    MPI_Send(
        local_numbers,
        dim_array,
        MPI_INT,
        rank ^ (1 << j),
        0,
        MPI_COMM_WORLD);

    /* comparison */
    k = dim_array - 1;
    q = dim_array - 1;
    for (i = dim_array - 1; i >= 0; --i) {
        if (local_numbers[k] > message_receive[q]) {
            ordered_array[i] = local_numbers[k];
            --k;
        } else {
            ordered_array[i] = message_receive[q];
            --q;
        }
    }
}

void comp_exchange_min(int j, int rank, int* local_numbers, int dim_array, int* ordered_array) {
    int message_receive[dim_array];
    int i, k, q;
    MPI_Send(
        local_numbers,
        dim_array,
        MPI_INT,
        rank ^ (1 << j),
        0,
        MPI_COMM_WORLD);

    MPI_Recv(&message_receive,
             dim_array,
             MPI_INT,
             rank ^ (1 << j),
             0,
             MPI_COMM_WORLD,
             MPI_STATUS_IGNORE);

    /* comparison */
    k = 0;
    q = 0;
    for (i = 0; i < dim_array; ++i) {
        if (local_numbers[k] < message_receive[q]) {
            ordered_array[i] = local_numbers[k];
            ++k;
        } else {
            ordered_array[i] = message_receive[q];
            ++q;
        }
    }
}
