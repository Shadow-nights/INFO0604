#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <time.h>

#define N 9
#define NUM_GRIDS 10000

void print(int arr[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", arr[i][j]);
        }
        printf("\n");
    }
}

int isSafe(int grid[N][N], int row, int col, int num) {
    // Vérifie la ligne et la colonne
    for (int x = 0; x < N; x++) {
        if (grid[row][x] == num || grid[x][col] == num) return 0;
    }
    // Vérifie la sous-grille 3x3
    int startRow = row - row % 3;
    int startCol = col - col % 3;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grid[i + startRow][j + startCol] == num) return 0;
        }
    }
    return 1;
}

int solveSudoku(int grid[N][N], int row, int col) {
    if (row == N - 1 && col == N) return 1;
    if (col == N) { row++; col = 0; }
    if (grid[row][col] > 0) return solveSudoku(grid, row, col + 1);

    for (int num = 1; num <= N; num++) {
        if (isSafe(grid, row, col, num)) {
            grid[row][col] = num;
            if (solveSudoku(grid, row, col + 1)) return 1;
            grid[row][col] = 0;
        }
    }
    return 0;
}

// Fonction parallélisée pour explorer les premières possibilités
int parallel_solve(int grid[N][N]) {
    int row = 0, col = 0;
    
    // Trouver la première case vide
    while (row < N && grid[row][col] != 0) {
        col++;
        if (col == N) { col = 0; row++; }
    }
    if (row == N) return 1; // Grille déjà remplie

    int found = 0;
    #pragma omp parallel for
    for (int num = 1; num <= N; num++) {
        if (found) continue; // Évite le travail inutile

        int grid_copy[N][N];
        memcpy(grid_copy, grid, sizeof(grid_copy)); // Copie thread-safe

        if (isSafe(grid_copy, row, col, num)) {
            grid_copy[row][col] = num;
            if (solveSudoku(grid_copy, row, col + 1)) {
                #pragma omp critical
                {
                    if (!found) {
                        memcpy(grid, grid_copy, sizeof(grid_copy)); // Sauvegarde la solution
                        found = 1;
                    }
                }
            }
        }
    }
    return found;
}

// Fonction pour copier une grille source vers une grille destination
void copyGrid(int src[N][N], int dest[N][N]) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            dest[i][j] = src[i][j];
        }
    }
}

int main() {
    double start, end;
    
    // Initialisation du générateur de nombres aléatoires
    srand(time(NULL));

    // Grille de base à partir de laquelle nous créerons des variations
    int baseGrid[N][N] = {
        {3, 0, 6, 5, 0, 8, 4, 0, 0},
        {5, 2, 0, 0, 0, 0, 0, 0, 0},
        {0, 8, 7, 0, 0, 0, 0, 3, 1},
        {0, 0, 3, 0, 1, 0, 0, 8, 0},
        {9, 0, 0, 8, 6, 3, 0, 0, 5},
        {0, 5, 0, 0, 9, 0, 6, 0, 0},
        {1, 3, 0, 0, 0, 0, 2, 5, 0},
        {0, 0, 0, 0, 0, 0, 0, 7, 4},
        {0, 0, 5, 2, 0, 6, 3, 0, 0}
    };

    // Allocation dynamique du tableau de grilles
    int (*grids)[N][N] = malloc(NUM_GRIDS * sizeof(int[N][N]));
    if (grids == NULL) {
        printf("Erreur d'allocation mémoire\n");
        return 1;
    }

    // Création de variations de la grille de base
    for(int g = 0; g < NUM_GRIDS; g++) {
        copyGrid(baseGrid, grids[g]);
        // Création de variations en ajoutant plus de cases vides
        for(int i = 0; i < N; i++) {
            for(int j = 0; j < N; j++) {
                // On garde 15% des chiffres originaux
                if (rand() % 7 != 0) {
                    grids[g][i][j] = 0;
                }
            }
        }
    }

    start = omp_get_wtime();
    
    int solved = 0;
    #pragma omp parallel for reduction(+:solved)
    for(int g = 0; g < NUM_GRIDS; g++) {
        if (parallel_solve(grids[g])) {
            solved++;
        }
    }
    
    end = omp_get_wtime();
    printf("Nombre de grilles résolues : %d/%d\n", solved, NUM_GRIDS);
    printf("Temps d'execution parallèle : %.4f secondes\n", end - start);

    free(grids);
    return 0;
}