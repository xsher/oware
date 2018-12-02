#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int * move(int pos, int * cells);
int requestMove(int player);

void printBoard(int * b, int seeds_computer, int seeds_player) {
    printf("\nComputer side\n");
    printf("         0     1     2     3     4     5\n");
    printf("-------------------------------------------------\n");
    printf("|%5d|%5d|%5d|%5d|%5d|%5d|%5d|%5s|\n", seeds_computer, b[0], b[1], b[2], b[3], b[4], b[5], "");
    printf("       -----------------------------------\n");
    printf("|%5s|%5d|%5d|%5d|%5d|%5d|%5d|%5d|\n", "", b[11], b[10], b[9], b[8], b[7], b[6], seeds_player);
    printf("-------------------------------------------------\n");
    printf("         11    10    9     8     7     6\n");
    printf("Player side\n\n");
}

int main(void) {

    // Initialize all the initial values
//    int max_seeds = 48;  // #seeds per hole * #holes
    int cells[12] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
    int seeds_player = 0;
    int seeds_computer= 0;
    int position; // fix this

    printBoard(cells, seeds_computer, seeds_player);

    int player; // 0 represents comp, 0 represents player
    printf("Who starts? 0: computer, 1: player.\n");
    scanf("%d", &player);

    position = requestMove(player);

    int * new_cells = move(position, cells);
    printBoard(new_cells, seeds_computer, seeds_player);

    return 0;    
}

int requestMove(int player) {
    int pos;

    if (player == 0) {
        do {
            printf("Which position to sow? Between 0 to 5.\n");
            scanf("%d", &pos);
        } while(!(pos >= 0 && pos <= 5));
    } else {
        do {
            printf("Which position to sow? Between 6 to 11.\n");
            scanf("%d", &pos);
        } while(!(pos >= 6 && pos <= 11));
    }

    return pos;
}

int * move(int pos, int * cells) {
    int * cellsc = (int*)malloc(sizeof(int)*12);
    memcpy(cellsc, cells, sizeof(int)*12);
    int stones = cellsc[pos];
    cellsc[pos] = 0;

    for (int i = 0; i < stones; i++) {
        int idx = (pos + i + 1) % 12;
        // add in logic to skip the original pos
        cellsc[idx] += 1;
    }

    return cellsc;
}
