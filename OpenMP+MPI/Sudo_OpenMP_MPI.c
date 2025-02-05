#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <mpi.h>
#include <omp.h>
#include <dirent.h>
#include <string.h>
#include <time.h>

#define N 9               // Taille des sudoku
#define TIME_LIMIT 5      // Limite de temps pour un sudoku
#define MAX_FILES 100     // Nombre maximum de sudokus
#define FNAME_SIZE 256    // Taille maximale d'un nom de fichier

// Vérifie si num peut être placé en sans violer les règles
bool isSafe(int grid[N][N], int row, int col, int num) {
    for (int x = 0; x < N; x++) {
        if (grid[row][x] == num || grid[x][col] == num)
            return false;
    }
    int startRow = row - row % 3, startCol = col - col % 3;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (grid[i + startRow][j + startCol] == num)
                return false;
    return true;
}

// Résolution séquentielle par backtracking
bool solveSudokuSeq(int grid[N][N], int row, int col) {
    if (row == N) return true;
    if (col == N) return solveSudokuSeq(grid, row + 1, 0);
    if (grid[row][col] != 0) return solveSudokuSeq(grid, row, col + 1);
    for (int num = 1; num <= N; num++) {
        if (isSafe(grid, row, col, num)) {
            grid[row][col] = num;
            if (solveSudokuSeq(grid, row, col + 1))
                return true;
            grid[row][col] = 0;
        }
    }
    return false;
}

// Affiche la grille 
void printGrid(int grid[N][N]) {
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++)
            printf("%d ", grid[i][j]);
        printf("\n");
    }
}

// Charge un sudoku depuis un fichier
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

// Charge les noms depuis un dossier
int loadSudokusFromDirectory(const char* dir_path, char sudoku_files[][FNAME_SIZE], int max_files) {
    DIR *dir = opendir(dir_path);
    if (!dir) {
        perror("Erreur ouverture dossier");
        return 0;
    }
    struct dirent *entry;
    int count = 0;
    while ((entry = readdir(dir)) && count < max_files) {
        if (strstr(entry->d_name, ".txt")) {  // On ne garde que les .txt
            snprintf(sudoku_files[count], FNAME_SIZE, "%s/%s", dir_path, entry->d_name);
            count++;
        }
    }
    closedir(dir);
    return count;
}

//Résolution parallèle d'un sudoku :
//- Repère la première case vide.
//- Pour chaque candidat (1 à 9), lance en parallèle une branche (OpenMP)
// qui effectue une copie locale de la grille et appelle la résolution séquentielle.
//- Si une branche trouve une solution, on la copie dans grid.
bool parallelSolveSudoku(int grid[N][N]) {
    int firstRow = -1, firstCol = -1;
    for (int i = 0; i < N && firstRow == -1; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j] == 0) { firstRow = i; firstCol = j; break; }
        }
    }
    if (firstRow == -1)
        return true;
    
    bool found = false;
    int localGrid[N][N];
    #pragma omp parallel for shared(found) firstprivate(localGrid)
    for (int num = 1; num <= N; num++) {
        if (found) continue;
        if (isSafe(grid, firstRow, firstCol, num)) {
            memcpy(localGrid, grid, sizeof(int)*N*N);
            localGrid[firstRow][firstCol] = num;
            if (solveSudokuSeq(localGrid, firstRow, firstCol + 1)) {
                #pragma omp critical
                {
                    if (!found) {
                        found = true;
                        memcpy(grid, localGrid, sizeof(int)*N*N);
                    }
                }
            }
        }
    }
    return found;
}

int main(int argc, char* argv[]){
    int rank, size;
    double commTime = 0.0; 
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    
    char sudoku_files[MAX_FILES][FNAME_SIZE];
    int num_sudokus = 0;
    double tCommStart, tCommEnd;
    
    // Le processus 0 lit la liste des fichiers
    if (rank == 0) {
        num_sudokus = loadSudokusFromDirectory(argv[1], sudoku_files, MAX_FILES);
        if (num_sudokus == 0) {
            printf("Aucun fichier sudoku trouvé dans %s\n", argv[1]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    
    // Diffuser le nombre de fichiers et la liste complète 
    tCommStart = MPI_Wtime();
    MPI_Bcast(&num_sudokus, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(sudoku_files, MAX_FILES * FNAME_SIZE, MPI_CHAR, 0, MPI_COMM_WORLD);
    tCommEnd = MPI_Wtime();
    commTime += (tCommEnd - tCommStart);
    
    double seqTotalTime = 0.0;
    //Exécution séquentielle
    if (rank == 0) {
        for (int i = 0; i < num_sudokus; i++) {
            int grid[N][N];
            if (!loadSudokuFromFile(sudoku_files[i], grid)) {
                printf("Erreur de lecture du fichier %s\n", sudoku_files[i]);
                continue;
            }
            double tstart = omp_get_wtime();
            bool solved = solveSudokuSeq(grid, 0, 0);
            double tend = omp_get_wtime();
            double dt = tend - tstart;
            if (!solved || dt > TIME_LIMIT) {
                printf("%s non résolu (%.4f s).\n", sudoku_files[i], dt);
                continue;
            }
            seqTotalTime += dt;
        }
        printf("Temps total séquentiel pour %d sudokus : %.4f secondes\n", num_sudokus, seqTotalTime);
    }
    
    //Exécution parallèle
    MPI_Barrier(MPI_COMM_WORLD);
    double parStart = MPI_Wtime();
    
    int localSolvedCount = 0;
    for (int i = rank; i < num_sudokus; i += size) {
        int grid[N][N];
        if (!loadSudokuFromFile(sudoku_files[i], grid)) {
            printf("Processus %d : Erreur de lecture du fichier %s\n", rank, sudoku_files[i]);
            continue;
        }
        double tstart = omp_get_wtime();
        bool solved = parallelSolveSudoku(grid);
        double tend = omp_get_wtime();
        double dt = tend - tstart;
        if (!solved || dt > TIME_LIMIT) {
            printf("Processus %d : %s non résolu (%.4f s).\n", rank, sudoku_files[i], dt);
            continue;
        }
        localSolvedCount++;
    }
    double parEnd = MPI_Wtime();
    double localParTime = parEnd - parStart;
    
    // Mesure du temps de communication MPI pendant la réduction
    tCommStart = MPI_Wtime();
    double parTotalTime = 0.0;
    MPI_Reduce(&localParTime, &parTotalTime, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    tCommEnd = MPI_Wtime();
    commTime += (tCommEnd - tCommStart);
    
    if (rank == 0) {
        double netParTime = parTotalTime - commTime;
        printf("Temps total parallèle (brut) pour %d sudokus : %.4f secondes\n", num_sudokus, parTotalTime);
        printf("Temps total de communication MPI              : %.4f secondes\n", commTime);
        printf("Temps total parallèle (nettoye)               : %.4f secondes\n", netParTime);
        if (seqTotalTime > 0)
            printf("Speedup (séquentiel / parallèle net)          : %.4f\n", seqTotalTime / netParTime);
    }
    
    MPI_Finalize();
    return 0;
}
