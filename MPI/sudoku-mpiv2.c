#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>

#define N 9

// Fonction pour trouver la première case vide
int find_empty(int grid[N][N], int *row, int *col) {
    for (*row = 0; *row < N; (*row)++)
        for (*col = 0; *col < N; (*col)++)
            if (grid[*row][*col] == 0)
                return 1;
    return 0;
}

// Fonction pour vérifier si un nombre peut être placé dans une case
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

// Fonction de résolution Sudoku (backtracking)
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

// Fonction pour charger les grilles depuis un fichier
int load_grids(const char* filename, int grids[][N][N], int max_grids) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Erreur lors de l'ouverture du fichier");
        return 0;
    }

    char line[100];
    int count = 0;

    while (fgets(line, sizeof(line), file) && count < max_grids) {
        if (strstr(line, "Grille") != NULL) {
            // Lire les 9 lignes suivantes pour la grille
            for (int i = 0; i < N; i++) {
                fgets(line, sizeof(line), file);
                for (int j = 0; j < N; j++) {
                    char c = line[j * 2]; // Chiffres séparés par des espaces
                    grids[count][i][j] = (c == '.' || c == ' ') ? 0 : c - '0';
                }
            }
            count++;
        }
    }

    fclose(file);
    return count;
}

// Fonction pour afficher une grille
void print_grid(int grid[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            printf("%d ", grid[i][j]);
        printf("\n");
    }
    printf("\n");
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    double start = MPI_Wtime();

    const int max_grids = 100;
    int grids[max_grids][N][N];
    int num_grids = 0;

    if (rank == 0) { // Master
        // Charger les grilles depuis le fichier
        num_grids = load_grids("sudoku_grids.txt", grids, max_grids);
        if (num_grids == 0) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        printf("%d grilles chargées depuis sudoku_grids.txt\n", num_grids);
    }

    // Diffuser le nombre de grilles à tous les processus
    MPI_Bcast(&num_grids, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // Diffuser toutes les grilles
    MPI_Bcast(grids, num_grids * N * N, MPI_INT, 0, MPI_COMM_WORLD);

    if (rank == 0) { // Master
        int solved_grids[max_grids][N][N];
        int next_grid = 0;
        MPI_Status status;

        // Distribuer les grilles initiales aux workers
        for (int i = 1; i < size && next_grid < num_grids; i++) {
            MPI_Send(&next_grid, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(grids[next_grid], N * N, MPI_INT, i, 0, MPI_COMM_WORLD);
            next_grid++;
        }

        // Recevoir les solutions et distribuer les grilles restantes
        while (next_grid < num_grids) {
            int recv_grid[N][N];
            int grid_index;
            
            MPI_Recv(&grid_index, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(recv_grid, N * N, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD, &status);
            
            memcpy(solved_grids[grid_index], recv_grid, N * N * sizeof(int));
            
            MPI_Send(&next_grid, 1, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
            MPI_Send(grids[next_grid], N * N, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
            next_grid++;
        }

        // Envoyer un signal de fin à tous les workers
        for (int i = 1; i < size; i++) {
            MPI_Send(&i, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
        }

        // Recevoir les dernières solutions
        for (int i = 1; i < size; i++) {
            int recv_grid[N][N];
            int grid_index;
            
            MPI_Recv(&grid_index, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(recv_grid, N * N, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD, &status);
            
            memcpy(solved_grids[grid_index], recv_grid, N * N * sizeof(int));
        }

        // Afficher toutes les grilles résolues
        printf("\n=== Solutions ===\n");
        for (int g = 0; g < num_grids; g++) {
            printf("Grille %d:\n", g + 1);
            print_grid(solved_grids[g]);
        }

    } else { // Workers
        while (1) {
            int grid_index;
            MPI_Status status;
            
            MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG == 1) {
                MPI_Recv(&grid_index, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);
                break;
            }

            int grid[N][N];
            MPI_Recv(&grid_index, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            MPI_Recv(grid, N * N, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            
            solveSudoku(grid);
            
            MPI_Send(&grid_index, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Send(grid, N * N, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
    }

    double end = MPI_Wtime();
    printf("Temps total: %.6f secondes\n", end - start);

    MPI_Finalize();
    return 0;
}