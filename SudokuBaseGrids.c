#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>
#include <time.h>

//Version séquentielle
// N est la taille de la matrice N*N
#define N 9
#define NUM_GRIDS 100 // Nombre de grilles à résoudre

/* Fonction utilitaire pour afficher la grille */
void print(int arr[N][N])
{
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++)
            printf("%d ", arr[i][j]);
        printf("\n");
    }
    printf("\n");
}

// Vérifie s'il est légal de placer le nombre num
// à la position donnée (row, col)
int isSafe(int grid[N][N], int row, 
                       int col, int num)
{
    
    // Vérifie si on trouve le même nombre
    // dans la même ligne, retourne 0 si trouvé
    for (int x = 0; x <= 8; x++)
        if (grid[row][x] == num)
            return 0;

    // Vérifie si on trouve le même nombre
    // dans la même colonne, retourne 0 si trouvé
    for (int x = 0; x <= 8; x++)
        if (grid[x][col] == num)
            return 0;

    // Vérifie si on trouve le même nombre
    // dans le carré 3x3 correspondant
    int startRow = row - row % 3, 
                 startCol = col - col % 3;
  
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (grid[i + startRow][j + 
                          startCol] == num)
                return 0;

    return 1;
}

/* Prend une grille partiellement remplie et tente
d'attribuer des valeurs à tous les emplacements non assignés
de manière à respecter les règles du Sudoku
(pas de doublons dans les lignes, colonnes et carrés) */
int solveSudoku(int grid[N][N], int row, int col)
{
    // Vérifie si on a atteint la 8ème ligne
    // et la 9ème colonne (matrice indexée à 0)
    // on retourne vrai pour éviter plus de backtracking
    if (row == N - 1 && col == N)
        return 1;

    // Si la valeur de colonne devient 9,
    // on passe à la ligne suivante et
    // on repart de la colonne 0
    if (col == N) 
    {
        row++;
        col = 0;
    }
  
    // Si la position actuelle de la grille
    // contient déjà une valeur > 0,
    // on passe à la colonne suivante
    if (grid[row][col] > 0)
        return solveSudoku(grid, row, col + 1);

    for (int num = 1; num <= N; num++) 
    {
        // Vérifie s'il est sûr de placer
        // le nombre (1-9) dans la position donnée
        if (isSafe(grid, row, col, num)==1) 
        {
            /* Assigne le nombre à la position
               actuelle (row,col) de la grille
               en supposant que ce choix est correct */
            grid[row][col] = num;
          
            // Vérifie la possibilité suivante avec
            // la colonne suivante
            if (solveSudoku(grid, row, col + 1)==1)
                return 1;
        }
      
        // Si l'hypothèse était fausse,
        // on retire le nombre et on essaie
        // une autre valeur
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
        /*printf("\nGrille %d avant résolution :\n", g+1);
        print(grids[g]);*/
        
        if (solveSudoku(grids[g], 0, 0) == 1) {
            /*printf("Grille %d après résolution :\n", g+1);
            print(grids[g]);*/
            solved++;
        } else {
            /*printf("Grille %d impossible à résoudre\n", g+1);*/
        }
        //printf("----------------------------------------\n");
    }
        
    end = omp_get_wtime();  // Fin du chronométrage
    printf("\nNombre de grilles résolues : %d/%d\n", solved, NUM_GRIDS);
    printf("Temps d'execution sequentiel : %.4f secondes\n", end - start);

    free(grids);  // Libération de la mémoire
    return 0;
}