#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <time.h>



//Version sequentielle
// N is the size of the 2D matrix   N*N
#define N 9
#define NUM_GRIDS 10000  // Réduit à 50000 grilles

/* A utility function to print grid */
void print(int arr[N][N])
{
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            printf("%d ", arr[i][j]);
        printf("\n");
    }
    printf("\n");
}

// Checks whether it will be legal  
// to assign num to the
// given row, col
int isSafe(int grid[N][N], int row, 
                       int col, int num)
{
    
    // Check if we find the same num 
    // in the similar row , we return 0
    for (int x = 0; x <= 8; x++)
        if (grid[row][x] == num)
            return 0;

    // Check if we find the same num in the 
    // similar column , we return 0
    for (int x = 0; x <= 8; x++)
        if (grid[x][col] == num)
            return 0;

    // Check if we find the same num in the 
    // particular 3*3 matrix, we return 0
    int startRow = row - row % 3, 
                 startCol = col - col % 3;
  
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (grid[i + startRow][j + 
                          startCol] == num)
                return 0;

    return 1;
}

/* Takes a partially filled-in grid and attempts
to assign values to all unassigned locations in
such a way to meet the requirements for
Sudoku solution (non-duplication across rows,
columns, and boxes) */
int solveSudoku(int grid[N][N], int row, int col)
{
    // Check if we have reached the 8th row 
    // and 9th column (0
    // indexed matrix) , we are 
    // returning true to avoid
    // further backtracking
    if (row == N - 1 && col == N)
        return 1;

    //  Check if column value  becomes 9 ,
    //  we move to next row and
    //  column start from 0
    if (col == N) 
    {
        row++;
        col = 0;
    }
  
    // Check if the current position 
    // of the grid already contains
    // value >0, we iterate for next column
    if (grid[row][col] > 0)
        return solveSudoku(grid, row, col + 1);

    for (int num = 1; num <= N; num++) 
    {
        
        // Check if it is safe to place 
        // the num (1-9)  in the
        // given row ,col  ->we move to next column
        if (isSafe(grid, row, col, num)==1) 
        {
            /* assigning the num in the 
               current (row,col)
               position of the grid
               and assuming our assigned num 
               in the position
               is correct     */
            grid[row][col] = num;
          
            //  Checking for next possibility with next
            //  column
            if (solveSudoku(grid, row, col + 1)==1)
                return 1;
        }
      
        // Removing the assigned num ,
        // since our assumption
        // was wrong , and we go for next 
        // assumption with
        // diff num value
        grid[row][col] = 0;
    }
    return 0;
}

// Fonction pour copier une grille source vers une grille destination
void copyGrid(int src[N][N], int dest[N][N]) {
    for(int i = 0; i < N; i++) {
        for(int j = 0; j < N; j++) {
            dest[i][j] = src[i][j];
        }
    }
}

int main()
{
    double start, end;
    
    // Initialisation du générateur de nombres aléatoires
    srand(time(NULL));

    // Grille de base à partir de laquelle nous créerons des variations
    int baseGrid[N][N] = {
        { 3, 0, 6, 5, 0, 8, 4, 0, 0 },
        { 5, 2, 0, 0, 0, 0, 0, 0, 0 },
        { 0, 8, 7, 0, 0, 0, 0, 3, 1 },
        { 0, 0, 3, 0, 1, 0, 0, 8, 0 },
        { 9, 0, 0, 8, 6, 3, 0, 0, 5 },
        { 0, 5, 0, 0, 9, 0, 6, 0, 0 },
        { 1, 3, 0, 0, 0, 0, 2, 5, 0 },
        { 0, 0, 0, 0, 0, 0, 0, 7, 4 },
        { 0, 0, 5, 2, 0, 6, 3, 0, 0 }
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

    start = omp_get_wtime();  // Début du chronométrage
    
    int solved = 0;
    for(int g = 0; g < NUM_GRIDS; g++) {
        if (solveSudoku(grids[g], 0, 0) == 1) {
            solved++;
        }
    }
        
    end = omp_get_wtime();  // Fin du chronométrage
    printf("Nombre de grilles résolues : %d/%d\n", solved, NUM_GRIDS);
    printf("Temps d'execution sequentiel : %.4f secondes\n", end - start);

    free(grids);  // Libération de la mémoire
    return 0;
}