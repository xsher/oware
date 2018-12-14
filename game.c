#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

struct Position {
    int cells[12];      // each cell contains a certain number of seeds
    int player;         // boolean true if the computer has to play and false otherwise
    int seeds_player;   // seeds taken by the player
    int seeds_computer; // seeds taken by the computer
    int parent_idx;     // position's index in the array
    int valid_move;     // 0 if not valid
    int evaluation;
    int hole;           // which hole was chosen
    int last_pos;
};

typedef struct Position Pos;

struct Move {
    Pos position;
    int score;
};

int * move(int pos, int player, int * cells);
int requestMove(int player, int * cells);
int capture(int org_pos, int last_pos, int player, int * cells);
// Pos computePlayerMove(Pos position);
struct Move generateMoves(Pos* nodes, Pos position, int depth, int parent_idx);
Pos computeComputerMove(Pos initial, int maxDepth);
int evaluate(Pos position);

void printArray(int * cells, int size) {
    for (int i = 0; i<size; i++) {
        printf("Cell %d, value %d\n", i, cells[i]);
    }
    printf("\n");
}

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
    int maxSeeds = 48;
    int maxDepth = 8;
    Pos position;
    for (int i = 0; i < 12; i++) position.cells[i] = 4;
    position.player = 0;
    position.seeds_player = 0;
    position.seeds_computer = 0;
    position.parent_idx = -1;

    printBoard(position.cells, position.seeds_computer, position.seeds_player);

    do {
        printf("Who starts? 0: computer, 1: player.\n");
        scanf("%d", &position.player);
    } while (position.player != 0 && position.player != 1);

    printf("Starts with player: %d\n", position.player);

    int scores_gain;

    // while it is not game over yet
    while (position.seeds_computer <= maxSeeds/2 || position.seeds_player <= maxSeeds/2) {
        printf("Player %d turn\n", position.player);
        if (position.player == 1) {
            position.hole = requestMove(position.player, position.cells);
            position.last_pos = position.hole + position.cells[position.hole];
            memcpy(position.cells, move(position.hole, position.player, position.cells), sizeof(int)*12);
            scores_gain = capture(position.hole, position.last_pos, position.player, position.cells);
            position.seeds_player += scores_gain;
        } else {
            struct timeval start, end;
            gettimeofday(&start, NULL);
            position = computeComputerMove(position, maxDepth);
            // calculate last pos in the computermove
            scores_gain = capture(position.hole, position.last_pos, position.player, position.cells);
            position.seeds_computer += scores_gain;
            gettimeofday(&end, NULL);
            double elapsed = (end.tv_sec - start.tv_sec) +
              ((end.tv_usec - start.tv_usec)/1000000.0);
            printf("Time taken for computer %f\n", elapsed);
        }
        printBoard(position.cells, position.seeds_computer, position.seeds_player);
        position.player = (position.player == 1) ? 0 : 1;
    }
    return 0;    
}

Pos computeComputerMove(Pos initial, int maxDepth) {
    // for now this returns all moves , next version we should return the best move only
    int sizeOfArray = pow(6, maxDepth + 1) / 5;
    Pos * nodes = (struct Position *)malloc(sizeof(struct Position)*sizeOfArray);

    // starts from index 0
    nodes[0] = initial;

    struct Move nextMove;
    nextMove = generateMoves(nodes, initial, maxDepth, 0);
    return nextMove.position;
}

int evaluate(Pos position) {
    return position.seeds_computer - position.seeds_player;
}

struct Move generateMoves(Pos * nodes, Pos position, int depth, int parent_idx) {
    if (depth == 0) {
        struct Move move;
        move.position = position;
        move.score = position.evaluation;
        return move;
    }

    Pos possib_moves[6];
    int idx = 0;

    // to prevent updates on original cell
    int * cells = (int*)malloc(sizeof(int)*12);
    memcpy(cells, position.cells, sizeof(int)*12);

    // start of generation of children nodes
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

            int scores_gain = capture(start_index, last_pos, position.player, cells);
            if (position.player == 0) {
                newPos.seeds_computer = position.seeds_computer + scores_gain;
                newPos.seeds_player = position.seeds_player;
            } else {
                newPos.seeds_player = position.seeds_player + scores_gain;
                newPos.seeds_computer = position.seeds_computer;
            }
            newPos.hole = start_index;
            newPos.parent_idx = parent_idx;
            newPos.valid_move = 1;
            newPos.evaluation = evaluate(newPos);
            newPos.player = position.player;
        } else {
            newPos.valid_move = 0;
        }

        // if it is not valid just store the intialized pos with nothing, might want to reconsider this
        possib_moves[idx] = newPos;
        idx++;
        start_index++;
    }

    int index = (parent_idx * 6) + 1;
    memcpy(&nodes[index], &possib_moves[0], sizeof(Pos)*6);
    // end of generation of children nodes and saving them into the nodes array

    // if player == 0 i.e. the computer we are maximising
    // if player == 1 i.e. the player we are minimizing
    int newDepth = depth - 1;
    struct Move bestMove;
    bestMove.score = (position.player == 0) ? -99999999 : 99999999;
    bestMove.position = possib_moves[0];

    // generating evaluation
    struct Move childPos;
    for (int i = 0; i < 6; i++) {
        if (possib_moves[i].valid_move == 1) {
            childPos = generateMoves(nodes, possib_moves[i], newDepth, index + i);
            if ((position.player == 0 && childPos.position.evaluation > bestMove.score) || (position.player == 1 && childPos.position.evaluation < bestMove.score)) {
                bestMove.score = childPos.position.evaluation;
                bestMove.position = (childPos.position.parent_idx > 0) ? nodes[childPos.position.parent_idx] : childPos.position;
            }
        }
    }
    return bestMove;
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