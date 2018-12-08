#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct Position {
    int cells[12]; // each cell contains a certain number of seeds
    int player; // boolean true if the computer has to play and false otherwise
    int seeds_player; // seeds taken by the player
    int seeds_computer; // seeds taken by the computer
    int index;        // position's index in the array
    int valid_move; // 0 if not valid
};

typedef struct Position Pos;

int * move(int pos, int player, int * cells);
int capture(int org_pos, int last_pos, int player, int * cells);
void generateMoves(Pos* nodes, Pos position, int depth, int parent_idx);
Pos* computeNextMove(Pos initial, int maxDepth);



void printArray(int * cells, int size) {
    for (int i = 0; i<size; i++) {
        printf("Cell %d, value %d\n", i, cells[i]);
    }
    printf("\n");
}

int main(void) {

    /* TODO
    * 1. Generate function to compute the nextMove for computer
    * 2. Generate the function that makes the decision i.e.
    *    with evaluation function (this is a recursive fn)
    * 3. Function to calculate all the possibilities from a position
    * 
    * 
    * since computer "represents us" so when it is
    * computer turn it will be a max
    */

    // set maxDepth for the tree
    // Generate Initial Position
    Pos initial_pos;
    for (int i = 0; i < 12; i++) initial_pos.cells[i] = 4;
    initial_pos.player = 0;
    initial_pos.seeds_player = 0;
    initial_pos.seeds_computer = 0;
    initial_pos.index = -1;

    int maxDepth = 3;
    computeNextMove(initial_pos, maxDepth);
}

void printStruct(Pos * positions, int size) {
    for (int i = 0; i<size; i++) {
        printf("\nValue of position %d  ", i);
        for (int j = 0; j<12; j++) {
            printf("%d ", positions[i].cells[j]);
        }
        printf("player %d, seedsplayer %d, seedscomp %d ", positions[i].player, positions[i].seeds_player, positions[i].seeds_computer);
        printf("index %d, valid_move %d\n", positions[i].index, positions[i].valid_move);
    }
    printf("\n\n");
}

Pos* computeNextMove(Pos initial, int maxDepth) {
    // for now this returns all moves , next version we should return the best move only
    int sizeOfArray = pow(6, maxDepth + 1) / 5;
    Pos * nodes = (struct Position *)malloc(sizeof(struct Position)*sizeOfArray);

    // starts from index 0
    nodes[0] = initial;
    generateMoves(nodes, initial, maxDepth, 0);
    printStruct(nodes, sizeOfArray);
}


void generateMoves(Pos* nodes, Pos position, int depth, int parent_idx) {
    if (depth == 0) {
        return;
    }

    Pos possib_moves[6];
    int idx = 0;

    // to prevent updates on original cell
    int * cells = (int*)malloc(sizeof(int)*12);
    memcpy(cells, position.cells, sizeof(int)*12);

    // for each position we have 6 possible moves for each player
    // we will now determine which moves are valid and store it in the array

    int start_index = (position.player == 0) ? 0 : 6;
    int final_index = (position.player == 0) ? 5 : 11;

    while (start_index <= final_index) {

        Pos newPos;

        // check for valid move
        if (cells[start_index] > 0) {
            int last_pos = start_index + cells[start_index];

            int * newMove = move(start_index, position.player, cells);
            memcpy(newPos.cells, newMove, sizeof(int) * 12);
            // newPos.cells = move(start_index, position.player, cells);

            newPos.player = (position.player == 0) ? 1 : 0;
            int scores_gain = capture(start_index, last_pos, position.player, cells);
            if (position.player == 0) {
                newPos.seeds_computer = position.seeds_computer + scores_gain;
                newPos.seeds_player = position.seeds_player;
            } else {
                newPos.seeds_player = position.seeds_player + scores_gain;
                newPos.seeds_computer = position.seeds_computer;
            }
            newPos.index = parent_idx;
            newPos.valid_move = 1;
        }

        // if it is not valid just store the intialized pos with nothing, might want to reconsider this
        possib_moves[idx] = newPos;
        idx++;
        start_index++;
    }

    int index = (parent_idx * 6) + 1;
    memcpy(&nodes[index], &possib_moves[0], sizeof(Pos)*6);

    int newDepth = depth - 1;
    if (newDepth > 0) {
        for (int i = 0; i < 6; i++) {
            if (possib_moves[i].valid_move == 1) {
                generateMoves(nodes, possib_moves[i], newDepth, index + i);
            }
        }
    }
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