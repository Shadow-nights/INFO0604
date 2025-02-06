#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 9

void print(int arr[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            printf("%d ", arr[i][j]);
        printf("\n");
    }
    printf("\n");
}

int isSafe(int grid[N][N], int row, int col, int num) {
    for (int x = 0; x < N; x++)
        if (grid[row][x] == num || grid[x][col] == num)
            return 0;

    int startRow = row - row % 3, startCol = col - col % 3;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (grid[i + startRow][j + startCol] == num)
                return 0;

    return 1;
}

int solveSudoku(int grid[N][N], int row, int col) {
    if (row == N - 1 && col == N)
        return 1;

    if (col == N) {
        row++;
        col = 0;
    }

    if (grid[row][col] > 0)
        return solveSudoku(grid, row, col + 1);

    for (int num = 1; num <= N; num++) {
        if (isSafe(grid, row, col, num)) {
            grid[row][col] = num;
            if (solveSudoku(grid, row, col + 1))
                return 1;
        }
        grid[row][col] = 0;
    }
    return 0;
}

void readAndSolveSudoku(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Erreur d'ouverture du fichier\n");
        return;
    }

    double total_time = 0.0;  // Variable pour accumuler le temps total
    int grid[N][N];
    char line[256];
    int grid_count = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "Grille")) {
            grid_count++;
            for (int i = 0; i < N; i++) {
                fgets(line, sizeof(line), file);
                char *token = strtok(line, " ");
                for (int j = 0; j < N; j++) {
                    grid[i][j] = (token[0] == '.') ? 0 : atoi(token);
                    token = strtok(NULL, " ");
                }
            }

            struct timespec start, end;
            clock_gettime(CLOCK_MONOTONIC, &start);

            if (solveSudoku(grid, 0, 0)) {
                printf("Résolution de la Grille %d:\n", grid_count);
                print(grid);
            } else {
                printf("Pas de solution pour la Grille %d\n", grid_count);
            }

            clock_gettime(CLOCK_MONOTONIC, &end);
            double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
            total_time += time_taken;  // Ajouter le temps de la grille au total

            printf("Temps d'exécution: %f secondes\n\n", time_taken);
        }
    }

    fclose(file);
    printf("Temps total de résolution: %f secondes\n", total_time);
}

int main() {
    readAndSolveSudoku("sudoku_grids.txt");
    return 0;
}
