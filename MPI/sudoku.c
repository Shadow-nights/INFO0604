#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 9

void print(int arr[N][N])
{
     for (int i = 0; i < N; i++)
      {
         for (int j = 0; j < N; j++)
            printf("%d ",arr[i][j]);
         printf("\n");
       }
}

int isSafe(int grid[N][N], int row, int col, int num)
{
    for (int x = 0; x <= 8; x++)
        if (grid[row][x] == num)
            return 0;

    for (int x = 0; x <= 8; x++)
        if (grid[x][col] == num)
            return 0;

    int startRow = row - row % 3, 
                 startCol = col - col % 3;
  
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (grid[i + startRow][j + 
                          startCol] == num)
                return 0;

    return 1;
}

int solveSudoku(int grid[N][N], int row, int col)
{
    if (row == N - 1 && col == N)
        return 1;

    if (col == N) 
    {
        row++;
        col = 0;
    }

    if (grid[row][col] > 0)
        return solveSudoku(grid, row, col + 1);

    for (int num = 1; num <= N; num++) 
    {
        if (isSafe(grid, row, col, num)==1) 
        {
            grid[row][col] = num;
            if (solveSudoku(grid, row, col + 1)==1)
                return 1;
        }
        grid[row][col] = 0;
    }
    return 0;
}

int main()
{
    /*int grid[N][N] = { { 3, 0, 6, 5, 0, 8, 4, 0, 0 },
                       { 5, 2, 0, 0, 0, 0, 0, 0, 0 },
                       { 0, 8, 7, 0, 0, 0, 0, 3, 1 },
                       { 0, 0, 3, 0, 1, 0, 0, 8, 0 },
                       { 9, 0, 0, 8, 6, 3, 0, 0, 5 },
                       { 0, 5, 0, 0, 9, 0, 6, 0, 0 },
                       { 1, 3, 0, 0, 0, 0, 2, 5, 0 },
                       { 0, 0, 0, 0, 0, 0, 0, 7, 4 },
                       { 0, 0, 5, 2, 0, 6, 3, 0, 0 } };*/
    
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

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start); // On démarre le chronomètre

    if (solveSudoku(grid, 0, 0)==1)
        print(grid);
    else
        printf("No solution exists");
    
    clock_gettime(CLOCK_MONOTONIC, &end); // On arrête le chronomètre
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Temps d'exécution: %f secondes\n", time_taken);

    return 0;
}