import random

def is_valid(grid, row, col, num):
    """Vérifie si un nombre peut être placé dans une case donnée."""
    for i in range(9):
        if grid[row][i] == num or grid[i][col] == num:
            return False
    
    start_row, start_col = (row // 3) * 3, (col // 3) * 3
    for i in range(3):
        for j in range(3):
            if grid[start_row + i][start_col + j] == num:
                return False
    
    return True

def solve(grid):
    """Remplit la grille de Sudoku en utilisant un algorithme de backtracking."""
    for row in range(9):
        for col in range(9):
            if grid[row][col] == 0:
                numbers = list(range(1, 10))
                random.shuffle(numbers)
                for num in numbers:
                    if is_valid(grid, row, col, num):
                        grid[row][col] = num
                        if solve(grid):
                            return True
                        grid[row][col] = 0
                return False
    return True

def generate_full_sudoku():
    """Génère une grille de Sudoku complète et valide."""
    grid = [[0 for _ in range(9)] for _ in range(9)]
    solve(grid)
    return grid

def remove_numbers(grid, difficulty=40):
    """Supprime des nombres de la grille pour créer un puzzle Sudoku tout en restant valide."""
    puzzle = [row[:] for row in grid]
    positions = [(r, c) for r in range(9) for c in range(9)]
    random.shuffle(positions)
    
    for _ in range(difficulty):
        row, col = positions.pop()
        puzzle[row][col] = 0
    
    return puzzle

def print_grid(grid):
    """Retourne la grille de Sudoku sous forme de chaîne de caractères."""
    return "\n".join(" ".join(str(num) if num != 0 else '.' for num in row) for row in grid)

def save_sudokus_to_file(filename, count=100):
    """Génère et enregistre 100 grilles de Sudoku valides mais incomplètes dans un fichier texte."""
    with open(filename, "w") as file:
        for i in range(count):
            full_grid = generate_full_sudoku()
            sudoku_grid = remove_numbers(full_grid)
            file.write(f"Grille {i+1}:\n")
            file.write(print_grid(sudoku_grid) + "\n\n")

if __name__ == "__main__":
    save_sudokus_to_file("sudoku_grids.txt", 100)