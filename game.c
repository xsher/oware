#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>


typedef struct BoardCell {
    int red;
    int black;
    int special;
    int total;
} Cell;

typedef struct MoveRequest {
    int hole;
    char colour[2];
    int spos; // position for special seed
} Hole;

typedef struct Position {
    Cell * cells;       // each cell contains a certain number of seeds
    int player;         // 0 if the computer's play and 1 otherwise
    int seeds_player;   // seeds taken by the player
    int seeds_computer; // seeds taken by the computer
    int parent_idx;     // position's index in the array
    int valid_move;     // 0 if not valid
    int evaluation;
    int current_idx;    // idx on the array
    Hole move;          // which hole was chosen
    int last_pos;
} Pos;

typedef struct Mouve {
    int idx;
    int score;
} Move;

void startGame(Pos position, int maxDepth);
Pos move(Pos position);
int capture(Hole move, int last_pos, int looped, int player, Cell * cells);
int evaluate(Pos position);
Pos copyPos(Pos p1);
Hole requestMove(int player, Cell * cells);

Move minimax(Pos* nodes, Pos position, bool maximisingPlayer,
    int depth, int parent_idx, int * count);
Move minimaxAlphaBeta(Pos * nodes, Pos position, bool maximisingPlayer,
    int alpha, int beta, int depth, int parent_idx, int * count);
Pos generatePosition(Pos position, int hole, bool maximisingPlayer, int parent_idx);
Pos computeComputerMove(Pos initial, int maxDepth);


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
                    .move = hole, .last_pos = -1, .current_idx = 0};

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
    int maxSeeds = 74;    // 12 holes, 6 seeds each and 2 special seeds
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
            position = move(position);
        } else {
            if (first_c) {
                requestSpecialSeed(position);
                first_c = false;
            }

            struct timeval start, end;
            gettimeofday(&start, NULL);

            position = computeComputerMove(position, maxDepth);
            gettimeofday(&end, NULL);
            double elapsed = (end.tv_sec - start.tv_sec) +
              ((end.tv_usec - start.tv_usec)/1000000.0);
            printf("Computer has played the move on hole: %d\n", position.move.hole);
            printf("Time taken for computer %f\n\n", elapsed);
        }

        over++; //to remove
        printf("Current board status:\n");
        printBoard(position.cells, position.seeds_computer, position.seeds_player);
        position.player = (position.player == 1) ? 0 : 1;
    }

    printf("GAME OVER.\n");
}

Pos computeComputerMove(Pos initial, int maxDepth) {
    // array to store all the nodes traversed
    int sizeOfArray = pow(6, maxDepth + 1) / 5;
    Pos * nodes = (struct Position *)malloc(sizeof(struct Position)*sizeOfArray);

    // At Node 0, it is a move by player 1
    initial.player = 1;
    nodes[0] = initial;

    Move nextMove;
    bool maximisingPlayer = true;

    // the first move that is feeded is always the move that has been done by the player
    int counter = 0;
    // nextMove = minimax(nodes, initial, maximisingPlayer, maxDepth, 0, &counter);
    nextMove = minimaxAlphaBeta(nodes, initial, maximisingPlayer, -76, 76, maxDepth, 0, &counter);

    printf("Number of nodes traversed: %d\n", counter);

    // nextMove.position.last_pos = nextMove.position.move.hole + org_cells[nextMove.position.move.hole];
    // if (org_cells[nextMove.position.hole] > 11) {
    //     nextMove.position.last_pos += 1;
    // }
    printf("Score for the move: %d\n", nextMove.score);
    // printBoard(f, nodes[nextMove.idx].cells, nodes[nextMove.idx].seeds_computer, nodes[nextMove.idx].seeds_player);
    printf("Next move is on idx %d\n", nextMove.idx);
    return nodes[nextMove.idx];
}

int evaluate(Pos position) {
    return position.seeds_computer - position.seeds_player;
}

Pos copyPos(Pos p1) {
    Pos p2;
    printf("Inside copypos\n");
    printBoard(p1.cells, p1.seeds_computer, p1.seeds_player);

    p2.cells = (Cell *) malloc(sizeof(Cell)*12);
    memcpy(p2.cells, p1.cells, sizeof(Cell)*12);
    printf("1  ");
    p2.player = p1.player;                 // 0 if the computer's play and 1 otherwise
    printf("2  ");
    p2.seeds_player = p1.seeds_player;     // seeds taken by the player
    printf("3  ");
    p2.seeds_computer = p1.seeds_computer; // seeds taken by the computer
    printf("4  ");
    p2.parent_idx = p1.parent_idx;         // position's index in the array
    printf("5  ");
    p2.valid_move = p1.valid_move;         // 0 if not valid
    printf("6  ");
    p2.move = p1.move;                     // which hole was chosen
    printf("7\n");

    printBoard(p2.cells, p2.seeds_computer, p2.seeds_player);

    return p2;
}

Pos generatePosition(Pos position, int hole, bool maximisingPlayer, int parent_idx) {
    printf("is it here ");
    Pos newPos = copyPos(position);
    printf("copy pos, newPos  ");
    printBoard(newPos.cells, newPos.seeds_computer, newPos.seeds_player);

    newPos.move.hole = hole;
    printf("HERE inside generatePos  ");
    if (newPos.cells[hole].total > 0) {
        newPos = move(newPos);
        newPos.valid_move = 1;
    } else {
        newPos.valid_move = 0;
    }
    printf("INSIDE generatePosition\n");
    newPos.player = maximisingPlayer ? 0 : 1;
    newPos.evaluation = evaluate(position);

    return newPos;
}

Move minimaxAlphaBeta(Pos * nodes, Pos position, bool maximisingPlayer, int alpha,
                      int beta, int depth, int parent_idx, int * count) {
    *count += 1;
    int * counter = count;
    if (depth == 0) {
        Move move;
        move.idx = parent_idx;
        move.score = evaluate(position);
        printf("I am at leaf, returning Move as it is for depth %d my idx %d, parent_id %d has score of %d\n",
            depth, parent_idx, move.score);
        return move;
    }

    Pos * possib_moves = (Pos *) malloc (sizeof(Pos)* 6);

    int idx = 0;
    int newDepth = depth - 1;
    Move bestMove;

    int first_valid = 0;
    int index = (parent_idx * 6) + 1;

    // to prevent updates on original cell
    Cell * cells = (Cell *)malloc(sizeof(Cell)*12);
    memcpy(cells, position.cells, sizeof(Cell)*12);

    int start_index = maximisingPlayer ? 0 : 6;
    int final_index = maximisingPlayer ? 5 : 11;

    while (start_index <= final_index) {   
 
        Pos newPos = generatePosition(position, start_index, maximisingPlayer, parent_idx);
        printf("GENERATING POSITIOn MOVES\n");
        printBoard(newPos.cells, newPos.seeds_computer, newPos.seeds_player);

        newPos.current_idx = idx + index;

        if (newPos.valid_move == 1) {
            // check if need to copy
            possib_moves[idx] = newPos;

            Move childPos;
            printf("generatin feedPos\n");
            Pos feedPos = copyPos(newPos);
            printf("generated feedPos\n");

            bool toMax = maximisingPlayer ? false : true;
            printf("generatin childPos\n");
            childPos = minimaxAlphaBeta(nodes, feedPos, toMax, alpha, beta, newDepth, index + idx, counter);
            printf("generated childPos\n");

            if (first_valid == 0) {
                // initialization of value if not given
                bestMove.score = childPos.score;
                bestMove.idx = (newPos.current_idx == 0) ? childPos.idx : newPos.current_idx;
                first_valid = 1;
            } else if ((maximisingPlayer && childPos.score > bestMove.score) ||
                    (!maximisingPlayer && childPos.score < bestMove.score)) {
                bestMove.score = childPos.score;
                bestMove.idx = (newPos.current_idx == 0) ? childPos.idx : newPos.current_idx;

                if (maximisingPlayer && bestMove.score > alpha) {
                    alpha = bestMove.score;
                } else if (!maximisingPlayer && bestMove.score < beta) {
                    beta = bestMove.score;
                }
            }
        }

        if (beta <= alpha) break;
        idx++;
        start_index++;
    }

    memcpy(&nodes[index], &possib_moves[0], sizeof(Pos)*6);
    printf("Returned best move for maximising %d at depth %d parent %d\n", maximisingPlayer, depth, parent_idx);
    // printBoard(f, bestMove.position.cells, bestMove.position.seeds_computer, bestMove.position.seeds_player);
    // end of generation of children nodes and saving them into the nodes array
    return bestMove;
}


//Move minimax(Pos * nodes, Pos position, bool maximisingPlayer,
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

Hole requestMove(int player, Cell * cells) {
    Hole request;

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

Pos move(Pos position) {
    // skip original position- change it to while loop
    Pos newPos;
    newPos = copyPos(position);

    printf("IN MOVE\n");
    printBoard(position.cells, position.seeds_computer, position.seeds_player);
    // printBoard(newPos.cells, newPos.seeds_computer, newPos.seeds_player);

    Cell * cellscpy = (Cell *)malloc(sizeof(Cell)*12);
    memcpy(cellscpy, position.cells, sizeof(Cell)*12);
    printBoard(cellscpy, newPos.seeds_computer, newPos.seeds_player);


    Cell stones = cellscpy[newPos.move.hole];
    Cell empty = {0, 0 ,0};
    cellscpy[position.move.hole] = empty;

    int scores_gain = 0;
    bool capture = false; // determines if still fit the capture criteria
    int looped = (newPos.cells[newPos.move.hole].total > 11) ? 1 : 0;
    newPos.last_pos = newPos.move.hole + newPos.cells[newPos.move.hole].total;
    if (looped) newPos.last_pos += 1;
    int special_seed_pos = newPos.move.hole + newPos.move.spos;

    int capt_index;
    int first_index = (newPos.player == 0) ? 6 : 0;
    int last_index = (newPos.player == 0) ? 11 : 5;
    capt_index = (looped == 1) ? last_index : newPos.last_pos;

    // breaks out when there is no stones left
    for (int j = newPos.last_pos; ; j--) {
        if (j == capt_index) capture = true;
        if (j == newPos.move.hole) {
            capture = false;
            continue;
        }

        char seed_placed[2];

        if (j == special_seed_pos & stones.special > 0) {
            // fprintf(f, "increase S at %d    ", j);    
            cellscpy[j%12].special += 1;
            strncpy(seed_placed, "S", 2);
            stones.special--;
        } else if (strcmp(newPos.move.colour, "R") == 0 && stones.black > 0) {
            // fprintf(f, "increase B at %d    ", j);    
            // printf("increase B at %d\n", j);
            cellscpy[j%12].black += 1;
            strncpy(seed_placed, "B", 2);
            stones.black--;
        } else if (strcmp(newPos.move.colour, "B") == 0 && stones.red > 0) {
            // fprintf(f, "increase R at %d    ", j);    
            // printf("increase R at %d\n", j);
            cellscpy[j%12].red += 1;
            strncpy(seed_placed, "R", 2);
            stones.red--;
        } else if (stones.black > 0) {
            // fprintf(f, "increase B at %d    ", j);    
            // printf("increase B at %d\n", j);
            cellscpy[j%12].black += 1;
            strncpy(seed_placed, "B", 2);
            stones.black--;
        } else if (stones.red > 0) {
            // fprintf(f, "increase R at %d    ", j);    
            // printf("increase R at %d\n", j);
            cellscpy[j%12].red += 1;
            strncpy(seed_placed, "R", 2);
            stones.red--;
        }

        cellscpy[j%12].total += 1;
        stones.total--;
        // fprintf(f, "Total stones now : %d\n", stones.total);

        // remove from total!
        if (capture) {
            // fprintf(f, "Still can capture\n");
            if (strcmp(seed_placed, "R") == 0 && cellscpy[j].red >= 2 && cellscpy[j].red <= 3) {
                // fprintf(f, "Capturing red cells %d\n", cellscpy[j].red);
                scores_gain += cellscpy[j].red;
                cellscpy[j].red = 0;
            } else if (strcmp(seed_placed, "B") == 0 && cellscpy[j].black >= 2 && cellscpy[j].black <= 3) {
                // fprintf(f, "Capturing black cells %d\n", cellscpy[j].black);
                scores_gain += cellscpy[j].black;
                cellscpy[j].black = 0;
            } else if (strcmp(seed_placed, "S") == 0) {
                // fprintf(f, "Capturing special cells B %d R %d S %d \n", cellscpy[j].black, cellscpy[j].red, cellscpy[j].special);
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
            // fprintf(f, "No more stonesss!\n");
            break;
        }
    }

    if (scores_gain > 0 && newPos.player == 0) {
        printf("computer gained: %d", scores_gain);
        newPos.seeds_computer += scores_gain;
    } else if (scores_gain > 0 && newPos.player == 0) {
        printf("player gained: %d", scores_gain);
        newPos.seeds_player += scores_gain;
    }

    memcpy(newPos.cells, cellscpy, sizeof(Cell)*12);
    return newPos;
}

// int capture(Hole move, int last_pos, int looped, int player, Cell * cells) {
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
//     int special_seed_pos = move.hole + move.spos;
    

//     for (int i = last_pos; i > move.hole && i >= first_index && i <= last_index; i--) {
//         // int idx = i % 12;
//         if (strcmp(move.colour, "R") == 0) {
//             // this means that last colour is B
//         }
//         if (cells[i] >= 2 && cells[i] <= 3) {
//             scores += cells[i];
//             cells[i] = 0;
//         } else {
//             break;
//         }
//     }

//     return scores;
// }