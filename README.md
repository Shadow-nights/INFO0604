# INFO0604: Le SUDOKU
![image](https://github.com/user-attachments/assets/398ebf4a-44ec-4548-994e-9c6c434b1289)
# Projet SUDOKU

## Solveur de Sudoku parallélisé sur le CPU

### Objectif
Vous devez implémenter un programme parallélisé capable de résoudre efficacement un puzzle Sudoku en utilisant un algorithme de backtracking.

## Contexte

### Introduction au Sudoku
Le Sudoku est un jeu de réflexion populaire qui se joue sur une grille de 9x9 cases comportant des chiffres entre 1 et 9. Le but du jeu est de remplir la grille en respectant les règles suivantes :
- Chaque ligne ne peut contenir qu'un seul des chiffres de 1 à 9.
- Chaque colonne ne peut contenir qu'un seul des chiffres de 1 à 9.
- Chaque sous-grille 3x3 ne peut contenir qu'un seul des chiffres de 1 à 9.

Un Sudoku typique peut comporter entre 50 et 60 espaces vides à résoudre. Résoudre un Sudoku est un problème **NP-complet**, ce qui rend l'optimisation et la parallélisation essentielles pour améliorer les performances.

## Algorithme de résolution

Un algorithme couramment utilisé pour résoudre les grilles de Sudoku est le **backtracking**. Il s'agit d'une recherche en profondeur dans l'arbre de toutes les combinaisons possibles pour remplir les espaces vides.

### Pseudocode du backtracking :
```pseudo
recursive_backtrack() :
    si la grille est valide :
        index = index du premier emplacement vide dans la grille
        pour valeur = 1 à 9 :
            set tableau[index] = valeur
            si recursive_backtrack() :
                return true ; // résolu !
            set tableau[index] = 0
    // Retour en arrière si toutes les valeurs ont été testées sans succès
    retourner faux ;
```

## Défis de parallélisation

L'algorithme de backtracking repose sur une pile d'appels récursifs, ce qui le rend difficile à paralléliser directement. Cependant, plusieurs stratégies permettent d'exécuter le solveur en parallèle :

### Parallélisation sur différentes branches de recherche
Une approche consiste à découper la recherche en profondeur en plusieurs tâches indépendantes :
1. Générer toutes les grilles valides possibles en remplissant les 20 premiers espaces vides d'un puzzle.
2. Distribuer ces grilles aux différents threads, où chaque thread applique l'algorithme de backtracking indépendamment.
3. Dès qu'un thread trouve une solution, tous les threads s'arrêtent et retournent la solution au **main**.

## Code séquentiel de référence
Si vous n'avez pas de code séquentiel, vous pouvez vous inspirer de cet exemple :
[https://www.geeksforgeeks.org/sudoku-backtracking-7/](https://www.geeksforgeeks.org/sudoku-backtracking-7/)

Bien sûr, adaptez ce code à vos besoins (ex : lecture de plusieurs grilles en entrée).

## Autres consignes

### Entrées et sorties
Résoudre un Sudoku individuellement ne prend pas beaucoup de temps, même en force brute. Pour évaluer les gains de performance, chargez plusieurs grilles simultanément (ex : **100 grilles**). Cela permet aussi d'explorer d'autres stratégies de parallélisation comme l'utilisation de pools de workers.

### Langages et bibliothèques
Vous avez le choix entre différents langages et bibliothèques :
- **Langages** : C, Python
- **Bibliothèques** : PThreads, Threading, Multiprocessing, OpenMP, MPI

Un point de bonus peut être accordé si vous comparez deux implémentations ou réalisez une implémentation mixte (ex : **MPI + OpenMP**).

### Mesurer le temps d'exécution
Plusieurs méthodes permettent de mesurer les performances :
- Commande `time` dans le shell
- Bibliothèque `time.h` en C
- Fonctions dédiées de MPI et OpenMP, qui offrent un meilleur contrôle :
  - [MPI_Wtime](https://rookiehpc.org/mpi/docs/mpi_wtime/index.html)
  - [omp_get_wtime](https://gcc.gnu.org/onlinedocs/gcc-4.5.4/libgomp/omp_005fget_005fwtime.html)

## Rapport et code source

Vous devez fournir :
1. **Le code source** (tous les fichiers dans une archive `.zip`)
2. **Un rapport en format PDF**, incluant :
   
### Contenu du rapport
- **Nom des membres du groupe** (min 3, max 5 personnes)
- **Résumé** *(1-3 phrases)* : Décrivez brièvement votre approche.
- **Informations générales** : Présentez le projet et justifiez vos choix techniques.
- **Défis rencontrés** :
  - Quels problèmes avez-vous rencontrés ?
  - Quels résultats espériez-vous obtenir avant d'optimiser votre code ?
- **Résultats** :
  - Instructions pour compiler et exécuter le code.
  - Format des grilles d'entrée (fichier d'exemple ou générateur de grilles).
  - Comparaison des performances (avec tableaux et graphiques).
  - Calcul du **speedup** et analyse des gains de performance.
  - Discussion sur l’efficacité du programme : est-elle **linéaire, supra-linéaire ou asymptotique** ?
- **Répartition du travail** dans le groupe.

---

### Êtes-vous satisfaits des résultats ?
Si c'était à refaire, quels choix feriez-vous différemment ?


