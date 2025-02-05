#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <time.h>

#define N 9
#define NUM_GRIDS 10000
#define SEQUENTIAL_RUN 0
#define PARALLEL_RUN 1

void print(int arr[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("%d ", arr[i][j]);
        }
        printf("\n");
    }
}

int isSafe(int grid[N][N], int row, int col, int num) {
    // Vérification rapide des valeurs
    if (num < 1 || num > N) return 0;
    
    // Cache la ligne et la colonne pour éviter les accès mémoire répétés
    int box_row = row - row % 3;
    int box_col = col - col % 3;
    
    // Vérification optimisée avec un tableau de flags
    int flags[N + 1] = {0};  // +1 car on utilise les indices 1-9
    
    // Vérification ligne et colonne en un seul passage
    for (int x = 0; x < N; x++) {
        if (grid[row][x] == num || grid[x][col] == num) return 0;
        flags[grid[row][x]] = 1;
        flags[grid[x][col]] = 1;
    }
    
    // Vérification de la sous-grille 3x3
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (grid[box_row + i][box_col + j] == num) return 0;
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
    if (!hasMinimumClues(grid)) return 0;  // Vérification rapide
    
    int row = 0, col = 0;
    while (row < N && grid[row][col] != 0) {
        col++;
        if (col == N) { col = 0; row++; }
    }
    if (row == N) return 1;

    int found = 0;
    int possible_nums[N];
    int num_count = 0;

    // Pré-calcul des nombres possibles pour réduire la charge de travail
    for (int num = 1; num <= N; num++) {
        if (isSafe(grid, row, col, num)) {
            possible_nums[num_count++] = num;
        }
    }

    if (num_count == 0) return 0;  // Aucune solution possible

    #pragma omp parallel shared(found, grid)
    {
        int local_grid[N][N];
        #pragma omp for schedule(dynamic, 1)
        for (int i = 0; i < num_count; i++) {
            if (!found) {
                memcpy(local_grid, grid, sizeof(local_grid));
                int num = possible_nums[i];
                local_grid[row][col] = num;
                if (solveSudoku(local_grid, row, col + 1)) {
                    #pragma omp critical
                    {
                        if (!found) {
                            memcpy(grid, local_grid, sizeof(local_grid));
                            found = 1;
                        }
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

int isValidGrid(int grid[N][N]) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j] != 0) {
                int temp = grid[i][j];
                grid[i][j] = 0;
                if (!isSafe(grid, i, j, temp)) {
                    grid[i][j] = temp;
                    return 0;
                }
                grid[i][j] = temp;
            }
        }
    }
    return 1;
}

int hasMinimumClues(int grid[N][N]) {
    int count = 0;
    for (int i = 0; i < N && count < 17; i++) {
        for (int j = 0; j < N && count < 17; j++) {
            if (grid[i][j] != 0) count++;
        }
    }
    return count >= 17; // Une grille de Sudoku nécessite au moins 17 indices pour avoir une solution unique
}

// Modification de la fonction solve_grids
double solve_grids(int (*grids)[N][N], int num_grids, int mode) {
    int solved = 0;
    double min_time = 999999.0;
    double max_time = 0.0;
    int invalid_grids = 0;
    double start = omp_get_wtime();
    
    if (mode == SEQUENTIAL_RUN) {
        // Version séquentielle
        for(int g = 0; g < num_grids; g++) {
            double grid_start = omp_get_wtime();
            if (parallel_solve(grids[g])) {
                solved++;
            } else {
                invalid_grids++;
            }
            double grid_time = omp_get_wtime() - grid_start;
            min_time = grid_time < min_time ? grid_time : min_time;
            max_time = grid_time > max_time ? grid_time : max_time;
        }
    } else {
        // Version parallèle
        #pragma omp parallel for reduction(+:solved,invalid_grids) reduction(min:min_time) reduction(max:max_time) schedule(dynamic, 1)
        for(int g = 0; g < num_grids; g++) {
            double grid_start = omp_get_wtime();
            if (parallel_solve(grids[g])) {
                solved++;
            } else {
                invalid_grids++;
            }
            double grid_time = omp_get_wtime() - grid_start;
            min_time = grid_time < min_time ? grid_time : min_time;
            max_time = grid_time > max_time ? grid_time : max_time;
        }
    }
    
    double end = omp_get_wtime();
    double total_time = end - start;
    
    printf("Mode : %s\n", mode == SEQUENTIAL_RUN ? "Séquentiel" : "Parallèle");
    printf("Grilles résolues : %d/%d (%.1f%%)\n", solved, num_grids, (solved * 100.0) / num_grids);
    printf("Grilles invalides : %d (%.1f%%)\n", invalid_grids, (invalid_grids * 100.0) / num_grids);
    printf("Temps total : %.4f secondes\n", total_time);
    printf("Temps par grille :\n");
    printf("  - Minimum : %.4f secondes\n", min_time);
    printf("  - Maximum : %.4f secondes\n", max_time);
    printf("  - Moyen   : %.4f secondes\n", total_time / num_grids);
    printf("----------------------------------------\n");
    
    return total_time;
}

int main() {
    double start, end;
    
    // Initialisation du générateur de nombres aléatoires
    srand(time(NULL));

    // Au début du main, après les déclarations
    int num_threads = omp_get_max_threads();
    omp_set_num_threads(num_threads);

    // Dans le main, juste après la définition de num_threads :
    if (num_threads < 2) {
        printf("⚠️ Attention : Exécution avec seulement %d thread(s). Les performances peuvent être limitées.\n", num_threads);
    } else {
        printf("Nombre de threads disponibles : %d\n", num_threads);
    }
    printf("----------------------------------------\n");

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
    #pragma omp parallel for schedule(dynamic)
    for(int g = 0; g < NUM_GRIDS; g++) {
        int attempts = 0;
        do {
            copyGrid(baseGrid, grids[g]);
            #pragma omp critical
            {
                for(int i = 0; i < N; i++) {
                    for(int j = 0; j < N; j++) {
                        if (rand() % 7 != 0) {
                            grids[g][i][j] = 0;
                        }
                    }
                }
            }
            attempts++;
            if (attempts > 100) {  // Éviter une boucle infinie
                copyGrid(baseGrid, grids[g]);
                break;
            }
        } while (!isValidGrid(grids[g]) || !hasMinimumClues(grids[g]));
    }

    // Copier les grilles pour avoir deux jeux identiques
    int (*grids_seq)[N][N] = malloc(NUM_GRIDS * sizeof(int[N][N]));
    memcpy(grids_seq, grids, NUM_GRIDS * sizeof(int[N][N]));

    // Mesure séquentielle
    printf("Exécution séquentielle...\n");
    double time_seq = solve_grids(grids_seq, NUM_GRIDS, SEQUENTIAL_RUN);

    // Mesure parallèle
    printf("Exécution parallèle...\n");
    double time_par = solve_grids(grids, NUM_GRIDS, PARALLEL_RUN);

    // Calcul et affichage du speedup
    double speedup = time_seq / time_par;
    printf("Speedup: %.2fx\n", speedup);
    printf("Efficacité: %.2f%%\n", (speedup / num_threads) * 100);

    // À la fin du main, avant les free :
    printf("\n=== Résumé des performances ===\n");
    printf("Nombre de grilles traitées : %d\n", NUM_GRIDS);
    printf("Nombre de threads utilisés : %d\n", num_threads);
    printf("Temps séquentiel total    : %.4f secondes\n", time_seq);
    printf("Temps parallèle total     : %.4f secondes\n", time_par);
    printf("Speedup                   : %.2fx\n", speedup);
    printf("Efficacité               : %.2f%%\n", (speedup / num_threads) * 100);
    printf("Gain en performance      : %.1f%%\n", ((time_seq - time_par) / time_seq) * 100);
    printf("=============================\n");

    // Libérer la mémoire
    free(grids_seq);
    free(grids);
    return 0;
}