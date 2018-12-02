#include <stdio.h>
#include <stdbool.h>

int move(int pos, int player, int * cells);

void printBoard(int * b, int seeds_computer, int seeds_player) {
    printf("Computer side\n");
    printf("         0     1     2     3     4     5\n");
    printf("-------------------------------------------------\n");
    printf("|%5d|%5d|%5d|%5d|%5d|%5d|%5d|%5s|\n", seeds_computer, b[0], b[1], b[2], b[3], b[4], b[5], "");
    printf("       -----------------------------------\n");
    printf("|%5s|%5d|%5d|%5d|%5d|%5d|%5d|%5d|\n", "", b[11], b[10], b[9], b[8], b[7], b[6], seeds_player);
    printf("-------------------------------------------------\n");
    printf("         11    10    9     8     7     6\n");
    printf("Player side\n");
}

int main(void) {

    // Initialize all the initial values
//    int max_seeds = 48;  // #seeds per hole * #holes
    int cells[12] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
    int seeds_player = 0;
    int seeds_computer= 0;
    int position;

    for (int i=0; i<12; i++) {
        printf("Cell values: %d.\n", cells[i]);
    }
    int player; // 0 represents comp, 0 represents player
    printf("Who starts? 0: computer, 1: player.\n");
    scanf("%d", &player);

    if (player == 0) {
        printf("Which position to sow? Between 0 to 5.\n");
        scanf("%d", &position);
    } else if (player == 1) {
        printf("Which position to sow? Between 6 to 11.\n");
        scanf("%d", &position);
    }

    printBoard(cells, seeds_computer, seeds_player);
    return 0;    
}