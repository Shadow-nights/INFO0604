#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <omp.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

#define N 9  // Taille du Sudoku
#define TIME_LIMIT 5  // Limite de temps en secondes pour résoudre un sudoku

// Vérifie si un nombre peut être placé dans la case (row, col)
bool isSafe(int grid[N][N], int row, int col, int num) {
    for (int x = 0; x < N; x++) {
        if (grid[row][x] == num || grid[x][col] == num)
            return false;
    }
    int startRow = row - row % 3, startCol = col - col % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grid[i + startRow][j + startCol] == num)
                return false;
        }
    }
    return true;
}

// Fonction récursive pour résoudre le Sudoku avec backtracking
bool solveSudoku(int grid[N][N], int row, int col) {
    if (row == N) return true;
    if (col == N) return solveSudoku(grid, row + 1, 0);
    if (grid[row][col] != 0) return solveSudoku(grid, row, col + 1);

    for (int num = 1; num <= 9; num++) {
        if (isSafe(grid, row, col, num)) {
            grid[row][col] = num;
            if (solveSudoku(grid, row, col + 1)) {
                return true;
            }
            grid[row][col] = 0;  // Backtracking
        }
    }
    return false;
}

// Fonction pour afficher la grille
void printGrid(int grid[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            printf("%d ", grid[i][j]);
        printf("\n");
    }
}

// Charge un Sudoku depuis un fichier
int loadSudokuFromFile(const char* filename, int grid[N][N]) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erreur ouverture fichier");
        return 0;
    }

    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            fscanf(file, "%d", &grid[i][j]);

    fclose(file);
    return 1;
}

// Charge tous les fichiers Sudoku d'un dossier
int loadSudokusFromDirectory(const char* dir_path, char sudoku_files[][256], int max_files) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Erreur ouverture dossier");
        return 0;
    }

    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) && count < max_files) {
        if (strstr(entry->d_name, ".txt")) {  // Filtre les fichiers .txt
            #pragma GCC diagnostic push
            #pragma GCC diagnostic ignored "-Wformat-truncation"
            snprintf(sudoku_files[count], sizeof(sudoku_files[count]), "%s/%s", dir_path, entry->d_name);
            #pragma GCC diagnostic pop
            count++;
        }
    }

    closedir(dir);
    return count;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <dossier_sudokus>\n", argv[0]);
        return 1;
    }

    int rank, size;
    double start_time, end_time;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char sudoku_files[100][256];
    int num_sudokus = 0;
    num_sudokus = loadSudokusFromDirectory(argv[1], sudoku_files, 100);

    // Diffuser le nombre total de fichiers
    MPI_Bcast(&num_sudokus, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(sudoku_files, num_sudokus * 256, MPI_CHAR, 0, MPI_COMM_WORLD);

    int grid[N][N];
    double total_time = 0.0;

    MPI_Barrier(MPI_COMM_WORLD);
    start_time = MPI_Wtime();

    for (int i = 0; i < num_sudokus; i++) {
        if (rank == 0) {
            if (!loadSudokuFromFile(sudoku_files[i], grid)) {
                printf("Erreur de lecture du fichier %s\n", sudoku_files[i]);
                continue;
            }
        }

        // Diffuser la grille à tous les processus
        MPI_Bcast(grid, N*N, MPI_INT, 0, MPI_COMM_WORLD);

        bool solved = false;
        clock_t start, end;
        double cpu_time_used;

        if (rank == 0) {
            start = clock();
            solved = solveSudoku(grid, 0, 0);
            end = clock();
            cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

            if (cpu_time_used > TIME_LIMIT) {
                printf("Temps limite atteint pour %s\n", sudoku_files[i]);
                continue;
            }
        }

        bool global_solved;
        MPI_Reduce(&solved, &global_solved, 1, MPI_C_BOOL, MPI_LOR, 0, MPI_COMM_WORLD);
        /*if(rank == 0 && global_solved) {
            printf("%s résolu !\n", sudoku_files[i]);
            printGrid(grid);
            printf("\n");
        } */
    }

    MPI_Barrier(MPI_COMM_WORLD);
    end_time = MPI_Wtime();
    total_time = end_time - start_time;

    if (rank == 0) {
        printf("\nTemps total pour résoudre %d Sudokus : %f secondes\n", num_sudokus, total_time);
    }

    MPI_Finalize();
    return 0;
}
