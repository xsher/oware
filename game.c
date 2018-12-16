#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>


struct BoardCell {
    int red;
    int black;
    int special;
    int total;
};

typedef struct BoardCell Cell;

struct MoveRequest {
    int hole;
    char colour[2];
    int spos; // position for special seed
};

typedef struct MoveRequest Hole;

struct Position {
    Cell * cells;      // each cell contains a certain number of seeds
    int player;         // 0 if the computer's play and 1 otherwise
    int seeds_player;   // seeds taken by the player
    int seeds_computer; // seeds taken by the computer
    int parent_idx;     // position's index in the array
    int valid_move;     // 0 if not valid
    int evaluation;
    Hole move;        // which hole was chosen
    int last_pos;
};

typedef struct Position Pos;

struct Move {
    Pos position;
    int score;
};

void startGame(Pos position, int maxDepth);
void move(Pos position);
struct MoveRequest requestMove(int player, Cell * cells);
int capture(Hole move, int last_pos, int looped, int player, Cell * cells);
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

void printBoard(Cell * b, int seeds_computer, int seeds_player) {
    printf("\nComputer side\n");
    printf("             0           1            2            3             4           5\n");
    printf("-------------------------------------------------------------------------------------------\n");
    printf("|%5d|%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%5s|\n", seeds_computer,
            b[0].red, b[0].black, b[0].special, b[1].red, b[1].black, b[1].special, b[2].red, b[2].black, b[2].special,
            b[3].red, b[3].black, b[3].special, b[4].red, b[4].black, b[4].special, b[5].red, b[5].black, b[5].special, "");
    printf("       -----------------------------------------------------------------------------      \n");
    printf("|%5s|%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%5d|\n", "",
            b[11].red, b[11].black, b[11].special, b[10].red, b[10].black, b[10].special, b[9].red, b[9].black, b[9].special,
            b[8].red, b[8].black, b[8].special, b[7].red, b[7].black, b[7].special, b[6].red, b[6].black, b[6].special,
            seeds_player);
    printf("-------------------------------------------------------------------------------------------\n");
    printf("           11            10           9            8             7           6\n");
    printf("Player side\n\n");
}

int main(void) {

    // Initialize all the initial values
    int maxDepth;

    // Generate initial position
    Cell * cells = (Cell *) malloc (sizeof(Cell)* 12);
    Cell original_cell = {3, 3, 0, 6};
    for (int i = 0; i < 12; i++) cells[i] = original_cell;
    Hole hole = { .hole = -1 };
    Pos position = { .cells = cells, .seeds_player = 0, .seeds_computer = 0,
                    .parent_idx = -1, .valid_move = 0, .evaluation = -80,
                    .move = hole, .last_pos = -1};

    printf("Initializing Game Board\n");
    printBoard(position.cells, position.seeds_computer, position.seeds_player);

    printf("What is the maxDepth?  ");
    scanf("%d", &maxDepth);

    do {
        printf("\nWho starts? Indicate 0 for computer, 1 for player.  ");
        scanf("%d", &position.player);
    } while (position.player != 0 && position.player != 1);
    printf("\nStarting the game with player: %d\n", position.player);

    startGame(position, maxDepth);
}

void requestSpecialSeed(Pos position) {
    int special_hole;

    do {
        printf("\nWhere would you like to place the special seed?  ");
        scanf("%d", &special_hole);
    } while (!(special_hole >= 0 && special_hole <= 11));

    position.cells[special_hole].special = 1;
    position.cells[special_hole].total += 1;
    printf("Board status after placing special seed.\n");
    printBoard(position.cells, position.seeds_computer, position.seeds_player);
}

void startGame(Pos position, int maxDepth) {
    int scores_gain;
    int maxSeeds = 74;  // 12 holes, 6 seeds each and 2 special seeds
    bool first_p = true;  // used to indicate initialization of special seed
    bool first_c = true;

    int over = 0;
    // while it is not game over yet
    // while (position.seeds_computer <= maxSeeds/2 ||
    //         position.seeds_player <= maxSeeds/2) {
    while (over < 5) {
        printf("\nPlayer %d turn\n", position.player);

        if (position.player == 1) {
            if (first_p) {
                requestSpecialSeed(position);
                first_p = false;
            } 

            position.move = requestMove(position.player, position.cells);
            // memcpy(position.cells, move(position), sizeof(Cell)*12);
            move(position);
            printf("Board after player 1 move");
            printBoard(position.cells, position.seeds_computer, position.seeds_player);
            // scores_gain = capture(position.move, position.last_pos, looped,
            //                       position.player, position.cells);
            // printf("Board after player 1 capture");
            // printBoard(position.cells, position.seeds_computer, position.seeds_player);
            // position.seeds_player += scores_gain;

            // to remove after this line;
            position.player = 0;
        } else {
            if (first_c) {
                requestSpecialSeed(position);
                first_c = false;
            } else {
                printf("no longer firstc");
            }
            // to remove after this line
            position.player = 1;
        }

        over++; //to remove
        //     struct timeval start, end;
        //     gettimeofday(&start, NULL);

        //     position = computeComputerMove(position, maxDepth);
        //     gettimeofday(&end, NULL);
        //     double elapsed = (end.tv_sec - start.tv_sec) +
        //       ((end.tv_usec - start.tv_usec)/1000000.0);
        //     printf("Computer has played the move on hole: %d\n", position.hole);
        //     printf("Time taken for computer %f\n\n", elapsed);
        // }
        // printf("Current board status:\n");
        // printBoard(position.cells, position.seeds_computer, position.seeds_player);
        // position.player = (position.player == 1) ? 0 : 1;
    }

    printf("GAME OVER.\n");
}

// Pos computeComputerMove(Pos initial, int maxDepth) {
//     // for now this returns all moves , next version we should return the best move only
//     int sizeOfArray = pow(6, maxDepth + 1) / 5;
//     Pos * nodes = (struct Position *)malloc(sizeof(struct Position)*sizeOfArray);

//     // starts from index 0
//     initial.player = 1;
//     nodes[0] = initial;

//     int * org_cells = (int *)malloc(sizeof(int)*12);
//     memcpy(org_cells, initial.cells, sizeof(int)*12);

//     struct Move nextMove;
//     bool maximisingPlayer = true;

//     // the first move that is feeded is always the move that has been done by the player
//     int counter = 0;
//     // nextMove = minimax(nodes, initial, maximisingPlayer, maxDepth, 0, &counter);
//     nextMove = minimaxAlphaBeta(nodes, initial, maximisingPlayer, -9999999, 9999999, maxDepth, 0, &counter);

//     printf("Number of nodes traversed: %d\n", counter);

//     nextMove.position.last_pos = nextMove.position.hole + org_cells[nextMove.position.hole];
//     if (org_cells[nextMove.position.hole] > 11) {
//         nextMove.position.last_pos += 1;
//     }
//     printf("Score for the move: %d\n", nextMove.score);

//     return nextMove.position;
// }

// int evaluate(Pos position) {
//     return position.seeds_computer - position.seeds_player;
// }

// Pos generatePosition(Pos position, int hole, int * cells, bool maximisingPlayer, int parent_idx) {

//     Pos newPos;

//     // check for valid move
//     if (cells[hole] > 0) {
//         int looped = (cells[hole] > 11) ? 1 : 0;
//         int last_pos = hole + cells[hole];

//         int * newMove = move(hole, position.player, cells);
//         memcpy(newPos.cells, newMove, sizeof(int) * 12);

//         int scores_gain = capture(hole, last_pos, looped, maximisingPlayer? 0 : 1, newPos.cells);
//         if (maximisingPlayer) {
//             newPos.seeds_computer = position.seeds_computer + scores_gain;
//             newPos.seeds_player = position.seeds_player;
//         } else {
//             newPos.seeds_player = position.seeds_player + scores_gain;
//             newPos.seeds_computer = position.seeds_computer;
//         }
//         newPos.valid_move = 1;
//         newPos.last_pos = last_pos;
//         // newPos.evaluation = evaluate(newPos);
//     } else {
//         newPos.valid_move = 0;
//     }

//     newPos.parent_idx = parent_idx;
//     newPos.hole = hole;
//     newPos.player = maximisingPlayer ? 0 : 1;

//     return newPos;
// }

// struct Move minimaxAlphaBeta(Pos * nodes, Pos position, bool maximisingPlayer, int alpha,
//                           int beta, int depth, int parent_idx, int * count) {
//     *count += 1;
//     int * counter = count;

//     if (depth == 0) {
//         struct Move move;
//         move.position = position;
//         int score = evaluate(position);
//         move.position.evaluation = score;
//         move.score = score;

//         return move;
//     }

//     Pos possib_moves[6];
//     int idx = 0;
//     int newDepth = depth - 1;
//     struct Move bestMove;

//     // generating evaluation
//     struct Move childPos;
//     // for all moves generated at depth N
//     int first_valid = 0;
//     int index = (parent_idx * 6) + 1;

//     // to prevent updates on original cell
//     int * cells = (int*)malloc(sizeof(int)*12);
//     memcpy(cells, position.cells, sizeof(int)*12);

//     int start_index = maximisingPlayer ? 0 : 6;
//     int final_index = maximisingPlayer ? 5 : 11;

//     while (start_index <= final_index) {

//         Pos newPos;
//         newPos = generatePosition(position, start_index, cells, maximisingPlayer, parent_idx);
//         if (newPos.valid_move == 1) {
//             bool toMax = maximisingPlayer ? false : true;
//             childPos = minimaxAlphaBeta(nodes, newPos, toMax, alpha, beta, newDepth, index + idx, counter);

//             if (childPos.position.valid_move == 1) {
//                 if (first_valid == 0) {
//                     // initialization of value if not given
//                     bestMove.score = childPos.score;
//                     bestMove.position = (childPos.position.parent_idx > 0) ?
//                                     newPos : childPos.position;
//                     first_valid = 1;
//                 } else if ((maximisingPlayer && childPos.score > bestMove.score) ||
//                         (!maximisingPlayer && childPos.score < bestMove.score)) {
//                     bestMove.score = childPos.score;
//                     bestMove.position = (childPos.position.parent_idx > 0) ?
//                                     newPos : childPos.position;

//                     if (maximisingPlayer && bestMove.score > alpha) {
//                         alpha = bestMove.score;
//                     } else if (!maximisingPlayer && bestMove.score < beta) {
//                         beta = bestMove.score;
//                     }
//                 }
//             }
//         }

//         possib_moves[idx] = newPos;
//         if (beta <= alpha) break;
//         idx++;
//         start_index++;
//     }

//     memcpy(&nodes[index], &possib_moves[0], sizeof(Pos)*6);
//     // end of generation of children nodes and saving them into the nodes array
//     return bestMove;
// }


// struct Move minimax(Pos * nodes, Pos position, bool maximisingPlayer,
//                           int depth, int parent_idx, int * count) {
//     *count += 1;
//     int * counter = count;

//     if (depth == 0) {
//         struct Move move;
//         move.position = position;
//         int score = evaluate(position);
//         move.position.evaluation = score;
//         move.score = score;
//         return move;
//     }

//     Pos possib_moves[6];
//     int idx = 0;
//     int newDepth = depth - 1;
//     struct Move bestMove;

//     // generating evaluation
//     struct Move childPos;
//     // for all moves generated at depth N
//     int first_valid = 0;
//     int index = (parent_idx * 6) + 1;

//     // to prevent updates on original cell
//     int * cells = (int*)malloc(sizeof(int)*12);
//     memcpy(cells, position.cells, sizeof(int)*12);

//     int start_index = maximisingPlayer ? 0 : 6;
//     int final_index = maximisingPlayer ? 5 : 11;

//     while (start_index <= final_index) {

//         Pos newPos;
//         newPos = generatePosition(position, start_index, cells, maximisingPlayer, parent_idx);
//         if (newPos.valid_move == 1) {
//             bool toMax = maximisingPlayer ? false : true;
//             childPos = minimax(nodes, newPos, toMax, newDepth, index + idx, counter);

//            if (childPos.position.valid_move == 1) {
//                 if ((maximisingPlayer && childPos.score > bestMove.score) ||
//                         (!maximisingPlayer && childPos.score < bestMove.score) || first_valid == 0) {
//                     bestMove.score = childPos.score;
//                     bestMove.position = (childPos.position.parent_idx > 0) ?
//                                     newPos : childPos.position;
//                     first_valid = 1;
//                 }
//             }
//         }
//         possib_moves[idx] = newPos;

//         idx++;
//         start_index++;
//     }

//     memcpy(&nodes[index], &possib_moves[0], sizeof(Pos)*6);
//     // end of generation of children nodes and saving them into the nodes array
//     return bestMove;
// }

struct MoveRequest requestMove(int player, Cell * cells) {
    struct MoveRequest request;

    if (player == 0) {
        do {
            printf("Which position to sow? Choices: ");
            for (int i=0; i < 6; i++) {
                if (cells[i].total > 0) printf("%d ", i);
            }
            printf("\t");
            scanf("%d", &request.hole);
        } while(!(request.hole >= 0 && request.hole <= 5) || cells[request.hole].total == 0);
    } else {
        do {
            printf("Which position to sow? Choices: ");
            for (int i=6; i < 12; i++) {
                if (cells[i].total > 0) printf("%d ", i);
            }
            scanf("%d", &request.hole);
        } while(!(request.hole >= 6 && request.hole <= 11) || cells[request.hole].total == 0);
    }

    do {
        printf("Which colour to start with? R or B?    ");
        scanf("%1s", &request.colour);
    } while(strcmp(request.colour, "R") != 0  && strcmp(request.colour, "B") != 0);

    do {
        printf("Which step to place the special seed? Max of %d steps.   ", cells[request.hole].total);
        scanf("%d", &request.spos);
    } while(!(request.spos >= 0 && request.spos <= cells[request.hole].total));

    return request;
}

void move(Pos position) {
    // skip original position- change it to while loop
    Cell * cellscpy = (Cell *)malloc(sizeof(Cell)*12);
    memcpy(cellscpy, position.cells, sizeof(Cell)*12);

    Cell stones = cellscpy[position.move.hole];
    Cell empty = {0, 0 ,0};
    cellscpy[position.move.hole] = empty;


    int scores_gain = 0;
    bool capture = false; // determines if still fit the capture criteria
    int looped = (position.cells[position.move.hole].total > 11) ? 1 : 0;
    position.last_pos = position.move.hole + position.cells[position.move.hole].total;
    if (looped) position.last_pos += 1;
    int special_seed_pos = position.move.hole + position.move.spos;

    int capt_index;
    int first_index = (position.player == 0) ? 6 : 0;
    int last_index = (position.player == 0) ? 11 : 5;
    capt_index = (looped == 1) ? last_index : position.last_pos;

    // breaks out when there is no stones left
    for (int j = position.last_pos; ; j--) {
        if (j == capt_index) capture = true;
        if (j == position.move.hole) {
            capture = false;
            continue;
        }

        char seed_placed[2];

        if (j == special_seed_pos & stones.special > 0) {
            printf("increase S at %d\n", j);
            cellscpy[j%12].special += 1;
            strncpy(seed_placed, "S", 2);
            stones.special--;
        } else if (strcmp(position.move.colour, "R") == 0 && stones.black > 0) {
            printf("increase B at %d\n", j);
            cellscpy[j%12].black += 1;
            strncpy(seed_placed, "B", 2);
            stones.black--;
        } else if (strcmp(position.move.colour, "B") == 0 && stones.red > 0) {
            printf("increase R at %d\n", j);
            cellscpy[j%12].red += 1;
            strncpy(seed_placed, "R", 2);
            stones.red--;
        } else if (stones.black > 0) {
            printf("increase B at %d\n", j);
            cellscpy[j%12].black += 1;
            strncpy(seed_placed, "B", 2);
            stones.black--;
        } else if (stones.red > 0) {
            printf("increase R at %d\n", j);
            cellscpy[j%12].red += 1;
            strncpy(seed_placed, "R", 2);
            stones.red--;
        }

        cellscpy[j%12].total += 1;
        stones.total--;

        if (capture) {
            if (strcmp(seed_placed, "R") == 0 && cellscpy[j].red >= 2 && cellscpy[j].red <= 3) {
                scores_gain += cellscpy[j].red;
                cellscpy[j].red = 0;
            } else if (strcmp(seed_placed, "B") == 0 && cellscpy[j].black >= 2 && cellscpy[j].black <= 3) {
                scores_gain += cellscpy[j].black;
                cellscpy[j].black = 0;
            } else if (strcmp(seed_placed, "S") == 0) {
                if (cellscpy[j].black >= 2 && cellscpy[j].black <= 3) {
                    scores_gain += cellscpy[j].black;
                    cellscpy[j].black = 0;
                }
                if (cellscpy[j].red >= 2 && cellscpy[j].red <= 3) {
                    scores_gain += cellscpy[j].red;
                    cellscpy[j].red = 0;
                }
            } else {
                capture = false;
            }
        }
        if (stones.total == 0) {
            break;
        }
    }

    if (scores_gain > 0 && position.player == 0) {
        position.seeds_computer += scores_gain;
    } else if (scores_gain > 0 && position.player == 0) {
        position.seeds_player += scores_gain;
    }
    // int i = 0;
    // while (stones.total > 0) {
    //     int idx = (move.hole + i + 1) % 12;
    //     // moving the pos ahead
    //     i++;
    //     // skip original hole
    //     if (idx == move.hole) {
    //         seeds[i] = "N";
    //         continue;
    //     }
    //     if (i == move.spos && stones.special > 0) {
    //         cellscpy[idx].special += 1;
    //         seeds[i] = "S";
    //         stones.special--;
    //     } else if (strcmp(move.colour, "R") == 0 && stones.red > 0) {
    //         cellscpy[idx].red += 1;
    //         seeds[i] = "R";
    //         stones.red--;
    //     } else if (strcmp(move.colour, "B") == 0 && stones.black > 0) {
    //         cellscpy[idx].black += 1;
    //         seeds[i] = "B";
    //         stones.black--;
    //     } else if (stones.black > 0) {
    //         cellscpy[idx].black += 1;
    //         seeds[i] = "B";
    //         stones.black--;
    //     } else if (stones.red > 0) {
    //         cellscpy[idx].red += 1;
    //         seeds[i] = "B";
    //         stones.red--;
    //     }

    //     cellscpy[idx].total += 1;
    //     stones.total--;
    // }
    // return cellscpy;
    memcpy(position.cells, cellscpy, sizeof(Cell)*12);
}

// int capture(int org_pos, int last_pos, int looped, int player, int * cells) {
//     // take note of edge case when it loops the entire round
//     // check based on last pos backwards to see if can capture
//     // can we only seeds on our side?! yes if the last position is on
//     // the other side, reset to the max on our side.
//     int scores = 0;
//     int first_index = (player == 0) ? 6 : 0;
//     int last_index = (player == 0) ? 11 : 5;
//     if (looped == 1) {
//         last_pos = last_index;
//     }

//     for (int i = last_pos; i > org_pos && i >= first_index && i <= last_index; i--) {
//         // int idx = i % 12;
//         if (i >= first_index && cells[i] >= 2 && cells[i] <= 3) {
//             scores += cells[i];
//             cells[i] = 0;
//         } else {
//             break;
//         }
//     }

//     return scores;
// }