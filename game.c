#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int * move(int pos, int player, int * cells);
int requestMove(int player, int * cells);
int capture(int org_pos, int last_pos, int player, int * cells);
//int generate_tree(int * cells, int player, int depth);

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
//    int cells_player[6] = {4, 4, 4, 4, 4, 4};
//    int cells_computer[6] = {4, 4, 4, 4, 4, 4};
    int seeds_player = 0;
    int seeds_computer = 0;
    int position; // fix this

    printBoard(cells, seeds_computer, seeds_player);

    int player; // 0 represents comp, 0 represents player
    printf("Who starts? 0: computer, 1: player.\n");
    scanf("%d", &player);

    position = requestMove(player, cells);

    int over = 0;
    int * new_cells = (int*)malloc(sizeof(int)*12);
    memcpy(new_cells, cells, sizeof(int)*12);

    // change this to the condition such that the game is not over yet
    // can do the check based on the seeds_player and seeds_computer
    while (over < 10) {
        // note that it loops
        int last_pos = position + new_cells[position];
        new_cells = move(position, player, new_cells);
        int scores_gain = capture(position, last_pos, player, new_cells);
        if (player == 0) {
            seeds_computer += scores_gain;
        } else {
            seeds_player += scores_gain;
        }
        printBoard(new_cells, seeds_computer, seeds_player);
        player = (player == 1) ? 0 : 1;
        position = requestMove(player, new_cells);
        // check for whether you can capture
        over++;
    }

    return 0;    
}

int requestMove(int player, int * cells) {
    int pos;
    // validate move in this function!

    if (player == 0) {
        do {
            printf("Which position to sow? Choices: ");
            for (int i=0; i < 6; i++) {
                if (cells[i] > 0) printf("%d ", i);
            }
            printf("\n");
            scanf("%d", &pos);
        } while(!(pos >= 0 && pos <= 5) || cells[pos] == 0);
    } else {
        do {
            printf("Which position to sow? Choices: ");
            for (int i=6; i < 12; i++) {
                if (cells[i] > 0) printf("%d ", i);
            }
            printf("\n");
            scanf("%d", &pos);
        } while(!(pos >= 6 && pos <= 11) || cells[pos] == 0);
    }

    return pos;
}

int * move(int pos, int player, int * cells) {
    // skip original position- change it to while loop
    int * cellsc = (int*)malloc(sizeof(int)*12);
    memcpy(cellsc, cells, sizeof(int)*12);

    int stones = cellsc[pos];
    cellsc[pos] = 0;

    int i = 0;
    while (stones > 0) {
        int idx = (pos + i + 1) % 12;
        // moving the pos ahead
        i++;

        // skip original hole
        if (idx == pos) continue;

        cellsc[idx] += 1;
        stones--;
    }
//    for (int i = 0; i < stones; i++) {
//        int idx = (pos + i + 1) % 12;
//        // add in logic to skip the original pos
//        cellsc[idx] += 1;
//    }

    return cellsc;
}

int capture(int org_pos, int last_pos, int player, int * cells) {
    // take note of edge case when it loops the entire round
    // check based on last pos backwards to see if can capture
    // can we only seeds on our side?! yes if the last position is on the other side, reset to the max on our side.
    int scores = 0;
    int first_index = (player == 0) ? 6 : 0;
    for (int i = last_pos; i > org_pos && i >= first_index; i--) {
        int idx = i % 12;
        if (idx >= first_index && cells[idx] >= 2 && cells[idx] <= 3) {
            scores += cells[idx];
            cells[idx] = 0;
        } else {
            break;
        }
    }

    return scores;
}

// write a recursive function that generates tree up to depth 3
// int generate_tree(int * cells, int player, int depth) {}