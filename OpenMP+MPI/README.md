# Sudo_OpenMP_MPI

## Description

Ce programme résout plusieurs grilles de sudoku en parallèle en utilisant OpenMP pour la parallélisation partagée et MPI pour la parallélisation distribuée. Il charge les grilles de sudoku depuis un répertoire spécifié, les résout et affiche le temps total nécessaire pour résoudre toutes les grilles.

## Prérequis

- Un compilateur C compatible avec OpenMP et MPI (par exemple, `mpicc`).
- Un environnement MPI installé (par exemple, OpenMPI ou MPICH).

## Compilation

Pour compiler le programme, utilisez la commande suivante :

```sh
mpicc -fopenmp -o Sudo_OpenMP_MPI Sudo_OpenMP_MPI.c

mpirun -np <nombre_de_processus> ./Sudo_OpenMP_MPI <dossier_sudokus>