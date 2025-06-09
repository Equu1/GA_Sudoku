#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <math.h>

#define SIZE 9
#define BOX_SIZE 3
#define POP_SIZE 500
#define MAX_GENERATIONS 10000
#define TOURNAMENT_SIZE 5
#define PC 0.85
#define PM 0.2

typedef struct {
    int grid[SIZE][SIZE];
    int fitness;
} Individual;

int exampleGrid[SIZE][SIZE] = {
    {0, 0, 3, 0, 2, 0, 6, 0, 0},
    {9, 0, 0, 3, 0, 5, 0, 0, 1},
    {0, 0, 1, 8, 0, 6, 4, 0, 0},
    {0, 0, 8, 1, 0, 2, 9, 0, 0},
    {7, 0, 0, 0, 0, 0, 0, 0, 8},
    {0, 0, 6, 7, 0, 8, 2, 0, 0},
    {0, 0, 2, 6, 0, 9, 5, 0, 0},
    {8, 0, 0, 2, 0, 3, 0, 0, 9},
    {0, 0, 5, 0, 1, 0, 3, 0, 0}
};

void shuffleArray(int *array, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

void copyGrid(int dest[SIZE][SIZE], int src[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            dest[i][j] = src[i][j];
        }
    }
}

// Oblicz koszt dla wiersza (liczba duplikatów)
int computeRowCost(int grid[SIZE][SIZE], int row) {
    int count[SIZE + 1] = {0};
    for (int j = 0; j < SIZE; j++) count[grid[row][j]]++;
    
    int cost = 0;
    for (int num = 1; num <= SIZE; num++) 
        if (count[num] > 1) cost += count[num] - 1;
    return cost;
}

// Oblicz koszt dla kolumny (liczba duplikatów)
int computeColCost(int grid[SIZE][SIZE], int col) {
    int count[SIZE + 1] = {0};
    for (int i = 0; i < SIZE; i++) count[grid[i][col]]++;
    
    int cost = 0;
    for (int num = 1; num <= SIZE; num++) 
        if (count[num] > 1) cost += count[num] - 1;
    return cost;
}

// Oblicz całkowity koszt (suma kosztów wierszy i kolumn)
int computeCost(int grid[SIZE][SIZE]) {
    int cost = 0;
    for (int i = 0; i < SIZE; i++) cost += computeRowCost(grid, i);
    for (int j = 0; j < SIZE; j++) cost += computeColCost(grid, j);
    return cost;
}

// Inicjalizacja osobników z poprawnymi kwadratami 3x3
void initializeIndividual(Individual *ind, int initial_grid[SIZE][SIZE], bool frozen[SIZE][SIZE]) {
    copyGrid(ind->grid, initial_grid);
    
    // Inicjalizuj każdy kwadrat 3x3
    for (int boxRow = 0; boxRow < BOX_SIZE; boxRow++) {
        for (int boxCol = 0; boxCol < BOX_SIZE; boxCol++) {
            int startRow = boxRow * BOX_SIZE;
            int startCol = boxCol * BOX_SIZE;
            
            // Znajdź które liczby są już obecne w tym kwadracie
            bool present[SIZE + 1] = {false};
            for (int i = startRow; i < startRow + BOX_SIZE; i++) {
                for (int j = startCol; j < startCol + BOX_SIZE; j++) {
                    if (ind->grid[i][j] != 0) {
                        present[ind->grid[i][j]] = true;
                    }
                }
            }
            
            // Stwórz tablicę brakujących liczb
            int missing[SIZE], missingCount = 0;
            for (int num = 1; num <= SIZE; num++) {
                if (!present[num]) {
                    missing[missingCount++] = num;
                }
            }
            
            // Wymieszaj brakujące liczby
            shuffleArray(missing, missingCount);
            
            // Wypełnij puste komórki brakującymi liczbami
            int idx = 0;
            for (int i = startRow; i < startRow + BOX_SIZE; i++) {
                for (int j = startCol; j < startCol + BOX_SIZE; j++) {
                    if (ind->grid[i][j] == 0) {
                        ind->grid[i][j] = missing[idx++];
                    }
                }
            }
        }
    }
}

// Oblicz fitness (ujemny koszt, więc wyższy jest lepszy)
int compute_fitness(Individual *ind) {
    return -computeCost(ind->grid);
}

// Selekcja turniejowa
Individual tournament_selection(Individual *population, int size) {
    int best_idx = rand() % size;
    for (int i = 1; i < TOURNAMENT_SIZE; i++) {
        int candidate_idx = rand() % size;
        if (population[candidate_idx].fitness > population[best_idx].fitness) {
            best_idx = candidate_idx;
        }
    }
    return population[best_idx];
}

// Krzyżowanie
void crossover(Individual *p1, Individual *p2, Individual *c1, Individual *c2, bool frozen[SIZE][SIZE]) {
    // Zacznij od kopii rodziców
    copyGrid(c1->grid, p1->grid);
    copyGrid(c2->grid, p2->grid);
    
    if ((double)rand() / RAND_MAX < PC) {
        // Wybierz losowy punkt krzyżowania (który kwadrat zacząć zamieniać)
        int point = rand() % (BOX_SIZE * BOX_SIZE);
        
        for (int boxRow = 0; boxRow < BOX_SIZE; boxRow++) {
            for (int boxCol = 0; boxCol < BOX_SIZE; boxCol++) {
                int box_idx = boxRow * BOX_SIZE + boxCol;
                
                // Zamieniaj tylko kwadraty po punkcie krzyżowania
                if (box_idx >= point) {
                    int startRow = boxRow * BOX_SIZE;
                    int startCol = boxCol * BOX_SIZE;
                    
                    // Dla każdej komórki w kwadracie
                    for (int i = startRow; i < startRow + BOX_SIZE; i++) {
                        for (int j = startCol; j < startCol + BOX_SIZE; j++) {
                            // Zamieniaj tylko niezamrożone komórki
                            if (!frozen[i][j]) {
                                int temp = c1->grid[i][j];
                                c1->grid[i][j] = c2->grid[i][j];
                                c2->grid[i][j] = temp;
                            }
                        }
                    }
                }
            }
        }
    }
}

// Mutacja
void mutate(Individual *ind, bool frozen[SIZE][SIZE]) {
    for (int boxRow = 0; boxRow < BOX_SIZE; boxRow++) {
        for (int boxCol = 0; boxCol < BOX_SIZE; boxCol++) {
            // Zastosuj mutację z prawdopodobieństwem PM
            if ((double)rand() / RAND_MAX < PM) {
                int startRow = boxRow * BOX_SIZE;
                int startCol = boxCol * BOX_SIZE;
                
                // Znajdź niezamrożone komórki w tym kwadracie
                int cells[SIZE][2];
                int count = 0;
                
                for (int i = startRow; i < startRow + BOX_SIZE; i++) {
                    for (int j = startCol; j < startCol + BOX_SIZE; j++) {
                        if (!frozen[i][j]) {
                            cells[count][0] = i;
                            cells[count][1] = j;
                            count++;
                        }
                    }
                }
                
                // Zamieniaj tylko jeśli mamy przynajmniej 2 niezamrożone komórki
                if (count >= 2) {
                    // Wybierz dwie różne losowe komórki do zamiany
                    int idx1 = rand() % count;
                    int idx2 = rand() % count;
                    
                    // Upewnij się, że są różne
                    int attempts = 0;
                    while (idx2 == idx1 && attempts < 10) {
                        idx2 = rand() % count;
                        attempts++;
                    }
                    
                    if (idx1 != idx2) {
                        int i1 = cells[idx1][0], j1 = cells[idx1][1];
                        int i2 = cells[idx2][0], j2 = cells[idx2][1];
                        
                        // Zamień wartości
                        int temp = ind->grid[i1][j1];
                        ind->grid[i1][j1] = ind->grid[i2][j2];
                        ind->grid[i2][j2] = temp;
                    }
                }
            }
        }
    }
}

// Znajdź najlepszego osobnika w populacji
Individual find_best(Individual *pop, int size) {
    int best_idx = 0;
    for (int i = 1; i < size; i++) {
        if (pop[i].fitness > pop[best_idx].fitness) {
            best_idx = i;
        }
    }
    return pop[best_idx];
}

// Wydrukuj planszę Sudoku
void printGrid(int grid[SIZE][SIZE]) {
    printf("-------------------------\n");
    for (int i = 0; i < SIZE; i++) {
        printf("| ");
        for (int j = 0; j < SIZE; j++) {
            printf("%d ", grid[i][j]);
            if ((j + 1) % 3 == 0) printf("| ");
        }
        printf("\n");
        if ((i + 1) % 3 == 0) printf("-------------------------\n");
    }
}

// Sprawdź czy rozwiązanie jest poprawne
bool isValidSolution(int grid[SIZE][SIZE]) {
    // Sprawdź wiersze i kolumny
    for (int i = 0; i < SIZE; i++) {
        bool rowUsed[SIZE + 1] = {false};
        bool colUsed[SIZE + 1] = {false};
        
        for (int j = 0; j < SIZE; j++) {
            // Sprawdź wiersz
            if (rowUsed[grid[i][j]]) return false;
            rowUsed[grid[i][j]] = true;
            
            // Sprawdź kolumnę
            if (colUsed[grid[j][i]]) return false;
            colUsed[grid[j][i]] = true;
        }
    }
    
    // Sprawdź kwadraty 3x3
    for (int boxRow = 0; boxRow < BOX_SIZE; boxRow++) {
        for (int boxCol = 0; boxCol < BOX_SIZE; boxCol++) {
            bool used[SIZE + 1] = {false};
            
            for (int i = boxRow * BOX_SIZE; i < (boxRow + 1) * BOX_SIZE; i++) {
                for (int j = boxCol * BOX_SIZE; j < (boxCol + 1) * BOX_SIZE; j++) {
                    if (used[grid[i][j]]) return false;
                    used[grid[i][j]] = true;
                }
            }
        }
    }
    
    return true;
}

int main() {
    srand(time(NULL));
    bool frozen[SIZE][SIZE] = {false};
    int grid[SIZE][SIZE];
    
    // Skopiuj przykładową planszę i oznacz stałe komórki
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            grid[i][j] = exampleGrid[i][j];
            frozen[i][j] = (grid[i][j] != 0);
        }
    }
    
    printf("Poczatkowa plansza:\n");
    printGrid(grid);
    
    // Inicjalizuj populację
    Individual population[POP_SIZE];
    for (int i = 0; i < POP_SIZE; i++) {
        initializeIndividual(&population[i], grid, frozen);
        population[i].fitness = compute_fitness(&population[i]);
    }
    
    Individual best_individual = find_best(population, POP_SIZE);
    int generation = 0;
    clock_t start = clock();
    
    // Główna pętla algorytmu genetycznego
    while (generation < MAX_GENERATIONS && best_individual.fitness < 0) {
        Individual new_population[POP_SIZE];
        
        // Elitaryzm: zachowaj najlepszego osobnika
        new_population[0] = best_individual;
        int new_pop_count = 1;
        
        // Stwórz nową populację
        while (new_pop_count < POP_SIZE) {
            // Wybierz rodziców
            Individual parent1 = tournament_selection(population, POP_SIZE);
            Individual parent2 = tournament_selection(population, POP_SIZE);
            
            // Stwórz potomków
            Individual child1, child2;
            
            // Zastosuj krzyżowanie
            crossover(&parent1, &parent2, &child1, &child2, frozen);
            
            // Zastosuj mutację
            mutate(&child1, frozen);
            mutate(&child2, frozen);
            
            // Oblicz fitness
            child1.fitness = compute_fitness(&child1);
            child2.fitness = compute_fitness(&child2);
            
            // Dodaj potomków do nowej populacji
            if (new_pop_count < POP_SIZE) new_population[new_pop_count++] = child1;
            if (new_pop_count < POP_SIZE) new_population[new_pop_count++] = child2;
        }
        
        // Zastąp starą populację nową populacją
        for (int i = 0; i < POP_SIZE; i++) {
            population[i] = new_population[i];
        }
        
        // Zaktualizuj najlepszego osobnika
        Individual current_best = find_best(population, POP_SIZE);
        if (current_best.fitness > best_individual.fitness) {
            best_individual = current_best;
            printf("Gen %d: najlepszy fitness = %d\n", generation, best_individual.fitness);
        }
        
        // Wcześniejsze zakończenie jeśli znaleziono rozwiązanie
        if (best_individual.fitness == 0) break;
        
        // Restart populacji jeśli utknęła na zbyt długo
        if (generation % 1000 == 999) {
            printf("Restartowanie populacji w generacji %d\n", generation + 1);
            for (int i = 1; i < POP_SIZE; i++) {  // Zachowaj najlepszego osobnika
                initializeIndividual(&population[i], grid, frozen);
                population[i].fitness = compute_fitness(&population[i]);
            }
        }
        
        generation++;
    }
    
    clock_t end = clock();
    double time_elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("\nRozwiazanie Sudoku (znalezione w %d generacjach, czas: %.2f s):\n", generation, time_elapsed);
    printGrid(best_individual.grid);
    printf("Koncowy fitness: %d\n", best_individual.fitness);
    
    if (best_individual.fitness == 0) {
        printf("Rozwiazanie jest %s\n", isValidSolution(best_individual.grid) ? "poprawne" : "niepoprawne");
    } else {
        printf("Nie znaleziono idealnego rozwiazania.\n");
    }
    
    return 0;
}