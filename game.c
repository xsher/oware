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
    int seeds_diff;
    int current_idx;    // idx on the array
    Hole move;          // which hole was chosen
    int last_pos;
} Pos;

typedef struct Mouve {
    int score;
    Pos position;
    Pos leaf;
    int found_better;
} Move;

void startGame(Pos position, int maxDepth, int first_player);
Pos move(Pos position);
int capture(Hole move, int last_pos, int looped, int player, Cell * cells);
int evaluate_score(Pos position);
int evaluate_seeds(Pos position);
Pos copyPos(Pos p1);
Hole requestMove(int player, Cell * cells);
void requestSpecialSeed(Pos position, int first_player);

Move minimaxAlphaBeta(Pos position, bool maximisingPlayer, int alpha,
    int beta, int depth, int maxDepth, int parent_idx, int * count, int * move_count);
Pos generatePosition(Pos position, int hole, int col, int spos,
    bool maximisingPlayer, int parent_idx, int * move_count);
Pos computeComputerMove(Pos initial, int maxDepth, int * move_cnt);


void printBoard(Cell * b, int seeds_computer, int seeds_player, int first_player) {
    printf("\nComputer side\n");
    printf("             1           2            3            4             5           6\n");
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
    printf("           12            11           10           9             8           7\n");
    printf("Player side\n\n");
}

void fprintBoard(FILE * f, Cell * b, int seeds_computer, int seeds_player, int first_player) {
    fprintf(f, "\nComputer side\n");
    fprintf(f, "             1           2            3            4             5           6\n");
    fprintf(f, "-------------------------------------------------------------------------------------------\n");
    fprintf(f, "|%5d|%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%5s|\n", seeds_computer,
            b[0].red, b[0].black, b[0].special, b[1].red, b[1].black, b[1].special, b[2].red, b[2].black, b[2].special,
            b[3].red, b[3].black, b[3].special, b[4].red, b[4].black, b[4].special, b[5].red, b[5].black, b[5].special, "");
    fprintf(f, "       -----------------------------------------------------------------------------      \n");
    fprintf(f, "|%5s|%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%5d|\n", "",
            b[11].red, b[11].black, b[11].special, b[10].red, b[10].black, b[10].special, b[9].red, b[9].black, b[9].special,
            b[8].red, b[8].black, b[8].special, b[7].red, b[7].black, b[7].special, b[6].red, b[6].black, b[6].special,
            seeds_player);
    fprintf(f, "-------------------------------------------------------------------------------------------\n");
    fprintf(f, "           12            11           10           9             8           7\n");
    fprintf(f, "Player side\n\n");
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
                    .move = hole, .last_pos = -1, .current_idx = 0,
                    .seeds_diff = 0};

    printf("Initializing Game Board\n");
    printBoard(position.cells, position.seeds_computer, position.seeds_player, 0);

    printf("What is the maxDepth?  ");
    scanf("%d", &maxDepth);

    do {
        printf("\nWho starts? Indicate 0 for computer, 1 for player.  ");
        scanf("%d", &position.player);
    } while (position.player != 0 && position.player != 1);
    printf("\nStarting the game with player: %d\n", position.player);

    int first_player = position.player;
    printf("Player %d", position.player);
    requestSpecialSeed(position, first_player);
    printf("Board status after placing special seed.\n");
    printBoard(position.cells, position.seeds_computer, position.seeds_player, first_player);

    printf("Player %d ", (position.player == 0) ? 1 : 0);
    requestSpecialSeed(position, first_player);
    printf("Board status after placing special seed.\n");
    printBoard(position.cells, position.seeds_computer, position.seeds_player, first_player);

    startGame(position, maxDepth, first_player);
}

void requestSpecialSeed(Pos position, int start_player) {
    int special_hole;

    do {
        printf("\nWhere would you like to place the special seed?  ");
        scanf("%d", &special_hole);
        special_hole -= 1;
    } while (!(special_hole >= 0 && special_hole <= 11));

    position.cells[special_hole].special += 1;
    position.cells[special_hole].total += 1;
}

void startGame(Pos position, int maxDepth, int first_player) {
    int scores_gain;
    int maxSeeds = 74;    // 12 holes, 6 seeds each and 2 special seeds
    int move_counter = 0;

    FILE * f = fopen("game.txt", "w");
    fprintBoard(f, position.cells, position.seeds_computer, position.seeds_player, first_player);

    // while it is not game over yet
    while (position.seeds_computer <= maxSeeds/2 ||
            position.seeds_player <= maxSeeds/2) {
        printf("\nPlayer %d turn\n", position.player);

        if (position.player == 1) {
            position.move = requestMove(position.player, position.cells);
            position = move(position);
            fprintf(f, "Player has played the move on hole %d, colour %s and special seed at %d.\n",
                position.move.hole + 1, position.move.colour, position.move.spos);
        } else {
            struct timeval start, end;
            gettimeofday(&start, NULL);

            position = computeComputerMove(position, maxDepth, &move_counter);
            gettimeofday(&end, NULL);
            double elapsed = (end.tv_sec - start.tv_sec) +
              ((end.tv_usec - start.tv_usec)/1000000.0);
            printf("Computer has played the move on hole %d, colour %s and special seed at %d.\n",
                position.move.hole + 1, position.move.colour, position.move.spos);
            fprintf(f, "Computer has played the move on hole %d, colour %s and special seed at %d.\n",
                position.move.hole + 1, position.move.colour, position.move.spos);
            printf("Time taken for computer %f\n\n", elapsed);
        }

        move_counter += 1;
        printf("Current board status:\n");
        printBoard(position.cells, position.seeds_computer, position.seeds_player, first_player);
        fprintBoard(f, position.cells, position.seeds_computer, position.seeds_player, first_player);

        position.player = (position.player == 1) ? 0 : 1;
    }

    printf("GAME OVER.\n");
}

Pos computeComputerMove(Pos initial, int maxDepth, int * move_cnt) {
    printf("Called compute computer\n");
    // Initial move is a move by player 1
    initial.player = 1;

    Move nextMove;
    bool maximisingPlayer = true;

    // the first move that is feeded is always the move that has been done by the player
    int counter = 0;
    int * move_counter = move_cnt;
    nextMove = minimaxAlphaBeta(initial, maximisingPlayer, -76, 76,
                                maxDepth, maxDepth, 0, &counter, move_counter);

    printf("Number of nodes traversed: %d\n", counter);
    printf("Score for the move: %d\n", nextMove.score);
    return nextMove.position;
}

int evaluate_score(Pos position) {
    return position.seeds_computer - position.seeds_player;
}

int evaluate_seeds(Pos position) {
    int sum_comp = 0;
    int sum_player = 0;
    for (int i = 0; i < 6; i++) sum_comp += position.cells[i].total;
    for (int j = 6; j < 12; j++) sum_player += position.cells[j].total;

    return sum_comp - sum_player;
}

Pos copyPos(Pos p1) {
    Pos p2;

    p2.cells = (Cell *) malloc(sizeof(Cell)*12);
    memcpy(p2.cells, p1.cells, sizeof(Cell)*12);

    p2.player = p1.player;                 // 0 if the computer's play and 1 otherwise
    p2.seeds_player = p1.seeds_player;     // seeds taken by the player
    p2.seeds_computer = p1.seeds_computer; // seeds taken by the computer
    p2.parent_idx = p1.parent_idx;         // position's index in the array
    p2.valid_move = p1.valid_move;         // 0 if not valid
    p2.current_idx = p1.current_idx;
    p2.move = p1.move;                     // which hole was chosen
    return p2;
}

Pos generatePosition(Pos position, int hole, int col, int spos, bool maximisingPlayer,
                    int parent_idx, int * move_count) {
    Pos newPos = copyPos(position);

    char colours[3] = "RB";
    int * move_counter = move_count;

    newPos.move.hole = hole;
    newPos.move.spos = spos;
    newPos.player = (position.player == 1) ? 0 : 1;
    memcpy(newPos.move.colour, &colours[col], sizeof(char));

    if (newPos.cells[hole].total > 0) {
        newPos = move(newPos);
        newPos.valid_move = 1;
    } else {
        newPos.valid_move = 0;
    }
    newPos.evaluation = evaluate_score(position);

    // start computing for remaining seeds only when the game is towards the end
    // prevent excessive computation at the start
    int remaining_seeds = 76 - newPos.seeds_player - newPos.seeds_computer;
    newPos.seeds_diff = (remaining_seeds < 25) ? evaluate_seeds(position) : 0;
    newPos.parent_idx = parent_idx;

    return newPos;
}

Move minimaxAlphaBeta(Pos position, bool maximisingPlayer, int alpha, int beta,
                int depth, int maxDepth, int parent_idx, int * count, int * move_count) {
    *count += 1;
    int * counter = count;
    int * move_counter = move_count;
    // printf("2");
    // if (*move_count % 1000 == 0 ) printf("1000");
    if (depth == 0) {
        bool game_ending = (76 - position.seeds_player - position.seeds_computer < 25) ? true : false;

        Move move;
        move.score = (game_ending) ? position.seeds_diff : position.evaluation;
        move.position = copyPos(position);
        move.leaf = copyPos(position);
        return move;
    }

    int idx = 0;
    int newDepth = depth - 1;
    Move bestMove;
    bestMove.found_better = 0;

    Cell * original_cells = (Cell *) malloc (sizeof(Cell)*12);
    memcpy(original_cells, position.cells, sizeof(Cell)*12);

    int first_valid = 0;
    int index = (parent_idx * 12) + 1;

    int start_index = maximisingPlayer ? 0 : 6;
    int final_index = maximisingPlayer ? 5 : 11;

    for (int j = start_index; j <= final_index; j++) {
        for (int i = 0; i < 2; i++) {
            int total_steps = (original_cells[j].special > 0) ? original_cells[j].total : 1;
            for (int l = 1; l <= total_steps; l++) {

                Pos newPos = generatePosition(position, j, i, l, maximisingPlayer,
                                parent_idx, move_counter);
                newPos.current_idx = idx + index;

                if (newPos.valid_move == 1) {

                    Move childPos;
                    Pos feedPos = copyPos(newPos);

                    bool toMax = maximisingPlayer ? false : true;
                    childPos = minimaxAlphaBeta(feedPos, toMax, alpha, beta, newDepth, maxDepth,
                                index + idx, counter, move_counter);

                    if (first_valid == 0) {
                        // initialization of value if not given
                        bestMove.score = childPos.score;
                        bestMove.position = (newPos.current_idx == 0) ? copyPos(childPos.position) : copyPos(newPos);
                        bestMove.leaf = (newDepth == 0) ? copyPos(childPos.position) : childPos.leaf;
                        first_valid = 1;

                    } else if ((maximisingPlayer && childPos.score > bestMove.score) ||
                            (!maximisingPlayer && childPos.score < bestMove.score)) {
                        bestMove.found_better = 1;
                        bestMove.score = childPos.score;
                        bestMove.position = (newPos.current_idx == 0) ? copyPos(childPos.position) : copyPos(newPos);
                        if (newDepth == 0 ) bestMove.leaf = copyPos(childPos.position);

                        if (maximisingPlayer && bestMove.score > alpha) {
                            alpha = bestMove.score;
                        } else if (!maximisingPlayer && bestMove.score < beta) {
                            beta = bestMove.score;
                        }
                    }
                }
                if (beta <= alpha) {
                    break;
                }
                idx++;
            }
        }
    }


    // if no best move, reinitialize the move
    if (depth == maxDepth && bestMove.found_better == 0) {
        int min = 999;
        int minhole = 0;
        int diff;

        for (int i = final_index; i > start_index; i--) {
            diff = abs(original_cells[i].total - original_cells[i-1].total);

            if (diff <= min && !(i-2 >= 0 && original_cells[i-2].total <= 4)) {
                min = diff;
                minhole = i-1;
            }
        }

        int sum_player = 0;
        for (int j = 6; j < 12; j++) sum_player += position.cells[j].total;
        if (sum_player == 0) {
            minhole += 1;
        }

        Pos bstPos = generatePosition(position, minhole, 1, position.move.spos,
                        maximisingPlayer, parent_idx, move_counter);
        if (bstPos.valid_move == 1) {
            bestMove.position = bstPos;
            bestMove.score = bstPos.evaluation;
        }
    }
    return bestMove;
}

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
            request.hole -= 1;
        } while(!(request.hole >= 0 && request.hole <= 5) || cells[request.hole].total == 0);
    } else {
        do {
            printf("Which position to sow? Choices: ");
            for (int i=6; i < 12; i++) {
                if (cells[i].total > 0) printf("%d ", i);
            }
            scanf("%d", &request.hole);
            request.hole -= 1;
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

    Cell * cellscpy = (Cell *)malloc(sizeof(Cell)*12);
    memcpy(cellscpy, position.cells, sizeof(Cell)*12);

    Cell stones = cellscpy[newPos.move.hole];
    Cell empty = {0, 0 ,0};
    cellscpy[position.move.hole] = empty;

    int scores_gain = 0;
    int capture = 0; // 0 refers to non-capture state, -1 has captured, 1 can capture
    int looped = (newPos.cells[newPos.move.hole].total > 11) ? 1 : 0;
    newPos.last_pos = newPos.move.hole + newPos.cells[newPos.move.hole].total;
    int special_seed_pos = (newPos.move.hole + newPos.move.spos);
    if (newPos.cells[newPos.move.hole].special > 1) special_seed_pos += 1;
    if (looped == 1) {
        newPos.last_pos += 1;
        if (special_seed_pos > 11) special_seed_pos += 1;
    }

    int capt_index;
    int first_index = (newPos.player == 0) ? 6 : 0;
    int last_index = (newPos.player == 0) ? 11 : 5;

    capt_index = newPos.last_pos % 12;
    if (!(capt_index >= first_index && capt_index <= last_index)) capt_index = last_index;

    char capture_colour[2] = "U"; // set to unknown

    // breaks out when there is no stones left
    for (int j = newPos.last_pos; ; j--) {
        int cellid = j % 12;

        if (cellid == capt_index && capture == 0) capture = 1;
        if (cellid == newPos.move.hole) {
            continue;
        }

        if (j == special_seed_pos & stones.special > 0) {
            cellscpy[cellid].special += 1;
            if (strcmp(capture_colour, "U") == 0) {
                strncpy(capture_colour, "S", 2);
            }
            special_seed_pos = (special_seed_pos - 1) % 12;
            stones.special--;
        } else if (strcmp(newPos.move.colour, "R") == 0 && stones.black > 0) {
            cellscpy[cellid].black += 1;
            if (strcmp(capture_colour, "U") == 0) {
                strncpy(capture_colour, "B", 2);
            }
            stones.black--;
        } else if (strcmp(newPos.move.colour, "B") == 0 && stones.red > 0) {
            cellscpy[cellid].red += 1;
            if (strcmp(capture_colour, "U") == 0) {
                strncpy(capture_colour, "R", 2);
            }
            stones.red--;
        } else if (stones.black > 0) {
            cellscpy[cellid].black += 1;
            if (strcmp(capture_colour, "U") == 0) {
                strncpy(capture_colour, "B", 2);
            }
            stones.black--;
        } else if (stones.red > 0) {
            cellscpy[cellid].red += 1;
            if (strcmp(capture_colour, "U") == 0) {
                strncpy(capture_colour, "R", 2);
            }
            stones.red--;
        }

        cellscpy[cellid].total += 1;
        stones.total--;

        if (capture == 1 && cellid >= first_index && cellid <= last_index) {
            int black_seeds = cellscpy[cellid].black + cellscpy[cellid].special;
            int red_seeds = cellscpy[cellid].red + cellscpy[cellid].special;

            if (strcmp(capture_colour, "R") == 0 && red_seeds >= 2 && red_seeds <= 3) {
                scores_gain += red_seeds;
                cellscpy[cellid].total -= red_seeds;
                cellscpy[cellid].red = 0;
                if (cellscpy[cellid].special > 0) cellscpy[cellid].special = 0;
            } else if (strcmp(capture_colour, "B") == 0 && black_seeds >= 2 && black_seeds <= 3) {
                scores_gain += black_seeds;
                cellscpy[cellid].total -= black_seeds;
                cellscpy[cellid].black = 0;
                if (cellscpy[cellid].special > 0) cellscpy[cellid].special = 0;
            } else if (strcmp(capture_colour, "S") == 0) {
                if ((black_seeds >= 2 && black_seeds <= 3) && (red_seeds >= 2 && red_seeds <= 3)) {
                    strncpy(capture_colour, "S", 2);
                    int total_seeds_capt = black_seeds + red_seeds - 1;
                    cellscpy[cellid].total -= total_seeds_capt;
                    scores_gain += total_seeds_capt;
                    cellscpy[cellid].black = 0;
                    cellscpy[cellid].red = 0;
                    cellscpy[cellid].special = 0;
                }
                else if (black_seeds >= 2 && black_seeds <= 3) {
                    strncpy(capture_colour, "B", 2);
                    cellscpy[cellid].total -= black_seeds;
                    scores_gain += black_seeds;
                    cellscpy[cellid].black = 0;
                    cellscpy[cellid].special = 0;
                }
                else if (red_seeds >= 2 && red_seeds <= 3) {
                    strncpy(capture_colour, "R", 2);
                    cellscpy[cellid].total -= red_seeds;
                    scores_gain += red_seeds;
                    cellscpy[cellid].red = 0;
                    cellscpy[cellid].special = 0;
                } else {
                    capture = -1;
                }
            } else {
                capture = -1;
            }
        } else {
            capture = -1;
        }

        if (stones.total == 0) {
            break;
        }
    }
    if (scores_gain > 0 && newPos.player == 0) {
        newPos.seeds_computer += scores_gain;
    } else if (scores_gain > 0 && newPos.player == 1) {
        newPos.seeds_player += scores_gain;
    }

    memcpy(newPos.cells, cellscpy, sizeof(Cell)*12);
    return newPos;
}