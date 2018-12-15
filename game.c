#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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
int capture(int org_pos, int last_pos, int looped, int player, int * cells);
struct Move minimax(Pos* nodes, Pos position, bool maximisingPlayer,
    int depth, int parent_idx, int * count);
Pos computeComputerMove(Pos initial, int maxDepth);
Pos generatePosition(Pos position, int hole, int * cells, bool maximisingPlayer, int parent_idx);
int evaluate(Pos position);

struct Move minimaxAlphaBeta(Pos * nodes, Pos position, bool maximisingPlayer, int alpha,
                            int beta, int depth, int parent_idx, int * count);

void printArray(int * cells, int size) {
    for (int i = 0; i<size; i++) {
        printf("%d  ", cells[i]);
    }
    printf("\n");
}

void printBoard(int * b, int seeds_computer, int seeds_player) {
    printf("\nComputer side\n");
    printf("         0     1     2     3     4     5\n");
    printf("-------------------------------------------------\n");
    printf("|%5d|%5d|%5d|%5d|%5d|%5d|%5d|%5s|\n", seeds_computer,
            b[0], b[1], b[2], b[3], b[4], b[5], "");
    printf("       -----------------------------------\n");
    printf("|%5s|%5d|%5d|%5d|%5d|%5d|%5d|%5d|\n", "",
            b[11], b[10], b[9], b[8], b[7], b[6], seeds_player);
    printf("-------------------------------------------------\n");
    printf("         11    10    9     8     7     6\n");
    printf("Player side\n\n");
}

int main(void) {

    // Initialize all the initial values
    int maxSeeds = 48;
    int maxDepth;
    Pos position;
    for (int i = 0; i < 12; i++) position.cells[i] = 4;
    position.seeds_player = 0;
    position.seeds_computer = 0;
    position.parent_idx = -1;

    printf("Initializing Game Board\n");
    printBoard(position.cells, position.seeds_computer, position.seeds_player);

    printf("What is the maxDepth?  ");
    scanf("%d", &maxDepth);

    do {
        printf("\nWho starts? Indicate 0 for computer, 1 for player.  ");
        scanf("%d", &position.player);
    } while (position.player != 0 && position.player != 1);
    printf("\nStarting the game with player: %d\n", position.player);

    int scores_gain;

    // while it is not game over yet
    while (position.seeds_computer <= maxSeeds/2 ||
            position.seeds_player <= maxSeeds/2) {
        printf("\nPlayer %d turn\n", position.player);

        if (position.player == 1) {
            position.hole = requestMove(position.player, position.cells);
            int looped = (position.cells[position.hole] > 11) ? 1 : 0;
            position.last_pos = position.hole + position.cells[position.hole];

            if (position.cells[position.hole] > 11) {
                position.last_pos += 1;
            }

            memcpy(position.cells, move(position.hole, position.player,
                   position.cells), sizeof(int)*12);
            scores_gain = capture(position.hole, position.last_pos, looped,
                                  position.player, position.cells);
            position.seeds_player += scores_gain;
        } else {
            struct timeval start, end;
            gettimeofday(&start, NULL);

            position = computeComputerMove(position, maxDepth);
            gettimeofday(&end, NULL);
            double elapsed = (end.tv_sec - start.tv_sec) +
              ((end.tv_usec - start.tv_usec)/1000000.0);
            printf("Computer has played the move on hole: %d\n", position.hole);
            printf("Time taken for computer %f\n\n", elapsed);
        }
        printf("Current board status:\n");
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
    initial.player = 1;
    nodes[0] = initial;

    int * org_cells = (int *)malloc(sizeof(int)*12);
    memcpy(org_cells, initial.cells, sizeof(int)*12);

    struct Move nextMove;
    bool maximisingPlayer = true;

    // the first move that is feeded is always the move that has been done by the player
    int counter = 0;
    // nextMove = minimax(nodes, initial, maximisingPlayer, maxDepth, 0, &counter);
    nextMove = minimaxAlphaBeta(nodes, initial, maximisingPlayer, -9999999, 9999999, maxDepth, 0, &counter);

    printf("Number of nodes traversed: %d\n", counter);

    nextMove.position.last_pos = nextMove.position.hole + org_cells[nextMove.position.hole];
    if (org_cells[nextMove.position.hole] > 11) {
        nextMove.position.last_pos += 1;
    }
    printf("Score for the move: %d\n", nextMove.score);

    return nextMove.position;
}

int evaluate(Pos position) {
    return position.seeds_computer - position.seeds_player;
}

Pos generatePosition(Pos position, int hole, int * cells, bool maximisingPlayer, int parent_idx) {

    Pos newPos;

    // check for valid move
    if (cells[hole] > 0) {
        int looped = (cells[hole] > 11) ? 1 : 0;
        int last_pos = hole + cells[hole];

        int * newMove = move(hole, position.player, cells);
        memcpy(newPos.cells, newMove, sizeof(int) * 12);

        int scores_gain = capture(hole, last_pos, looped, maximisingPlayer? 0 : 1, newPos.cells);
        if (maximisingPlayer) {
            newPos.seeds_computer = position.seeds_computer + scores_gain;
            newPos.seeds_player = position.seeds_player;
        } else {
            newPos.seeds_player = position.seeds_player + scores_gain;
            newPos.seeds_computer = position.seeds_computer;
        }
        newPos.valid_move = 1;
        newPos.last_pos = last_pos;
        // newPos.evaluation = evaluate(newPos);
    } else {
        newPos.valid_move = 0;
    }

    newPos.parent_idx = parent_idx;
    newPos.hole = hole;
    newPos.player = maximisingPlayer ? 0 : 1;

    return newPos;
}

struct Move minimaxAlphaBeta(Pos * nodes, Pos position, bool maximisingPlayer, int alpha,
                          int beta, int depth, int parent_idx, int * count) {
    *count += 1;
    int * counter = count;

    if (depth == 0) {
        struct Move move;
        move.position = position;
        int score = evaluate(position);
        move.position.evaluation = score;
        move.score = score;

        return move;
    }

    Pos possib_moves[6];
    int idx = 0;
    int newDepth = depth - 1;
    struct Move bestMove;

    // generating evaluation
    struct Move childPos;
    // for all moves generated at depth N
    int first_valid = 0;
    int index = (parent_idx * 6) + 1;

    // to prevent updates on original cell
    int * cells = (int*)malloc(sizeof(int)*12);
    memcpy(cells, position.cells, sizeof(int)*12);

    int start_index = maximisingPlayer ? 0 : 6;
    int final_index = maximisingPlayer ? 5 : 11;

    while (start_index <= final_index) {

        Pos newPos;
        newPos = generatePosition(position, start_index, cells, maximisingPlayer, parent_idx);
        if (newPos.valid_move == 1) {
            bool toMax = maximisingPlayer ? false : true;
            childPos = minimaxAlphaBeta(nodes, newPos, toMax, alpha, beta, newDepth, index + idx, counter);

            if (childPos.position.valid_move == 1) {
                if (first_valid == 0) {
                    // initialization of value if not given
                    bestMove.score = childPos.score;
                    bestMove.position = (childPos.position.parent_idx > 0) ?
                                    newPos : childPos.position;
                    first_valid = 1;
                } else if ((maximisingPlayer && childPos.score > bestMove.score) ||
                        (!maximisingPlayer && childPos.score < bestMove.score)) {
                    bestMove.score = childPos.score;
                    bestMove.position = (childPos.position.parent_idx > 0) ?
                                    newPos : childPos.position;

                    if (maximisingPlayer && bestMove.score > alpha) {
                        alpha = bestMove.score;
                    } else if (!maximisingPlayer && bestMove.score < beta) {
                        beta = bestMove.score;
                    }
                }
            }
        }

        possib_moves[idx] = newPos;
        if (beta <= alpha) break;
        idx++;
        start_index++;
    }

    memcpy(&nodes[index], &possib_moves[0], sizeof(Pos)*6);
    // end of generation of children nodes and saving them into the nodes array
    return bestMove;
}


struct Move minimax(Pos * nodes, Pos position, bool maximisingPlayer,
                          int depth, int parent_idx, int * count) {
    *count += 1;
    int * counter = count;

    if (depth == 0) {
        struct Move move;
        move.position = position;
        int score = evaluate(position);
        move.position.evaluation = score;
        move.score = score;
        return move;
    }

    Pos possib_moves[6];
    int idx = 0;
    int newDepth = depth - 1;
    struct Move bestMove;

    // generating evaluation
    struct Move childPos;
    // for all moves generated at depth N
    int first_valid = 0;
    int index = (parent_idx * 6) + 1;

    // to prevent updates on original cell
    int * cells = (int*)malloc(sizeof(int)*12);
    memcpy(cells, position.cells, sizeof(int)*12);

    int start_index = maximisingPlayer ? 0 : 6;
    int final_index = maximisingPlayer ? 5 : 11;

    while (start_index <= final_index) {

        Pos newPos;
        newPos = generatePosition(position, start_index, cells, maximisingPlayer, parent_idx);
        if (newPos.valid_move == 1) {
            bool toMax = maximisingPlayer ? false : true;
            childPos = minimax(nodes, newPos, toMax, newDepth, index + idx, counter);

           if (childPos.position.valid_move == 1) {
                if ((maximisingPlayer && childPos.score > bestMove.score) ||
                        (!maximisingPlayer && childPos.score < bestMove.score) || first_valid == 0) {
                    bestMove.score = childPos.score;
                    bestMove.position = (childPos.position.parent_idx > 0) ?
                                    newPos : childPos.position;
                    first_valid = 1;
                }
            }
        }
        possib_moves[idx] = newPos;

        idx++;
        start_index++;
    }

    memcpy(&nodes[index], &possib_moves[0], sizeof(Pos)*6);
    // end of generation of children nodes and saving them into the nodes array
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

int capture(int org_pos, int last_pos, int looped, int player, int * cells) {
    // take note of edge case when it loops the entire round
    // check based on last pos backwards to see if can capture
    // can we only seeds on our side?! yes if the last position is on
    // the other side, reset to the max on our side.
    int scores = 0;
    int first_index = (player == 0) ? 6 : 0;
    int last_index = (player == 0) ? 11 : 5;
    if (looped == 1) {
        last_pos = last_index;
    }

    for (int i = last_pos; i > org_pos && i >= first_index && i <= last_index; i--) {
        // int idx = i % 12;
        if (i >= first_index && cells[i] >= 2 && cells[i] <= 3) {
            scores += cells[i];
            cells[i] = 0;
        } else {
            break;
        }
    }

    return scores;
}