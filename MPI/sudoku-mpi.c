#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <time.h>
#include <string.h>

#define N 9

int find_empty(int grid[N][N], int *row, int *col) {
    for (*row = 0; *row < N; (*row)++)
        for (*col = 0; *col < N; (*col)++)
            if (grid[*row][*col] == 0)
                return 1;
    return 0;
}

int isSafe(int grid[N][N], int row, int col, int num) {
    for (int x = 0; x < N; x++)
        if (grid[row][x] == num) return 0;

    for (int x = 0; x < N; x++)
        if (grid[x][col] == num) return 0;

    int startRow = row - row % 3;
    int startCol = col - col % 3;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (grid[i + startRow][j + startCol] == num)
                return 0;
    return 1;
}

int solveSudoku(int grid[N][N]) {
    int row, col;
    
    if (!find_empty(grid, &row, &col))
        return 1;

    for (int num = 1; num <= 9; num++) {
        if (isSafe(grid, row, col, num)) {
            grid[row][col] = num;
            if (solveSudoku(grid))
                return 1;
            grid[row][col] = 0;
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Grille de test
    /*int grid[N][N] = { 
        {3,0,6,5,0,8,4,0,0},
        {5,2,0,0,0,0,0,0,0},
        {0,8,7,0,0,0,0,3,1},
        {0,0,3,0,1,0,0,8,0},
        {9,0,0,8,6,3,0,0,5},
        {0,5,0,0,9,0,6,0,0},
        {1,3,0,0,0,0,2,5,0},
        {0,0,0,0,0,0,0,7,4},
        {0,0,5,2,0,6,3,0,0}
    };*/

    int grid[N][N] = {
        {0,0,0,0,0,0,0,0,0},
        {0,0,0,0,0,3,0,8,5},
        {0,0,1,0,2,0,0,0,0},
        {0,0,0,5,0,7,0,0,0},
        {0,0,4,0,0,0,1,0,0},
        {0,9,0,0,0,0,0,0,0},
        {5,0,0,0,0,0,0,7,3},
        {0,0,2,0,1,0,0,0,0},
        {0,0,0,0,4,0,0,0,9}
    };

    double start = MPI_Wtime();
    int row, col;
    find_empty(grid, &row, &col);

    if (rank == 0) { // Master
        int solution[N][N];
        int found = 0;
        
        // Distribuer les possibilités aux workers
        for (int num = 1; num <= 9; num++) {
            if (isSafe(grid, row, col, num)) {
                int dest = (num % (size - 1)) + 1;
                MPI_Send(&num, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
            }
        }
        
        // Envoyer signal de fin
        for (int i = 1; i < size; i++) {
            MPI_Send(NULL, 0, MPI_INT, i, 1, MPI_COMM_WORLD);
        }

        // Recevoir la première solution trouvée
        MPI_Recv(solution, N*N, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        double end = MPI_Wtime();
        
        printf("Solution trouvee en %.4f s\n", end - start);
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++)
                printf("%d ", solution[i][j]);
            printf("\n");
        }
        
    } else { // Workers
        while (1) {
            MPI_Status status;
            int num;
            
            MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == 1) {
                MPI_Recv(NULL, 0, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
                break;
            }
            
            MPI_Recv(&num, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            int local_grid[N][N];
            memcpy(local_grid, grid, N*N*sizeof(int));
            
            if (isSafe(local_grid, row, col, num)) {
                local_grid[row][col] = num;
                if (solveSudoku(local_grid)) {
                    MPI_Send(local_grid, N*N, MPI_INT, 0, 0, MPI_COMM_WORLD);
                }
            }
        }
    }

    MPI_Finalize();
    return 0;
}