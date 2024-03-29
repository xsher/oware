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
Hole requestMove(int player, Cell * cells, int first_player);
void requestSpecialSeed(Pos position, int first_player, int player);

Move minimaxAlphaBeta(FILE * f, Pos position, bool maximisingPlayer, int alpha,
    int beta, int depth, int maxDepth, int parent_idx, int * count, int * move_count);
Pos generatePosition(Pos position, int hole, int col, int spos,
    bool maximisingPlayer, int parent_idx, int * move_count);
Pos computeComputerMove(FILE * f, Pos initial, int maxDepth, int * move_cnt);


void printBoard(Cell * b, int seeds_computer, int seeds_player, int first_player) {
    if (first_player == 0) {
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
    } else {
        printf("\nPlayer side\n");
        printf("             1           2            3            4             5           6\n");
        printf("-------------------------------------------------------------------------------------------\n");
        printf("|%5d|%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%5s|\n", seeds_player,
                b[6].red, b[6].black, b[6].special, b[7].red, b[7].black, b[7].special, b[8].red, b[8].black, b[8].special,
                b[9].red, b[9].black, b[9].special, b[10].red, b[10].black, b[10].special, b[11].red, b[11].black, b[11].special, "");
        printf("       -----------------------------------------------------------------------------      \n");
        printf("|%5s|%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%2dR %2dB %2dS |%5d|\n", "", 
                b[5].red, b[5].black, b[5].special, b[4].red, b[4].black, b[4].special, b[3].red, b[3].black, b[3].special,
                b[2].red, b[2].black, b[2].special, b[1].red, b[1].black, b[1].special, b[0].red, b[0].black, b[0].special,
                seeds_computer);
        printf("-------------------------------------------------------------------------------------------\n");
        printf("           12            11           10           9             8           7\n");
        printf("Computer side\n\n");
    }
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
    requestSpecialSeed(position, first_player, position.player);
    printf("Board status after placing special seed.\n");
    printBoard(position.cells, position.seeds_computer, position.seeds_player, first_player);

    printf("Player %d ", (position.player == 0) ? 1 : 0);
    requestSpecialSeed(position, first_player, (position.player == 0) ? 1: 0 );
    printf("Board status after placing special seed.\n");
    printBoard(position.cells, position.seeds_computer, position.seeds_player, first_player);

    startGame(position, maxDepth, first_player);
}

void requestSpecialSeed(Pos position, int start_player, int player) {
    int special_hole;

    do {
        printf("\nWhere would you like to place the special seed?  ");
        scanf("%d", &special_hole);
        special_hole -= 1;
    } while (!(special_hole >= 0 && special_hole <= 11));

    if (player == start_player && player == 1) {
        special_hole += 6;
    } else if (player != start_player && player == 0) {
        special_hole -= 6;
    }

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
            position.move = requestMove(position.player, position.cells, first_player);
            position = move(position);
            fprintf(f, "Player has played the move on hole %d, colour %s and special seed at %d.\n",
                position.move.hole + 1, position.move.colour, position.move.spos);
        } else {
            struct timeval start, end;
            gettimeofday(&start, NULL);

            position = computeComputerMove(f, position, maxDepth, &move_counter);
            gettimeofday(&end, NULL);
            double elapsed = (end.tv_sec - start.tv_sec) +
              ((end.tv_usec - start.tv_usec)/1000000.0);
            int pos_moved = position.move.hole + 1;
            if (first_player != 0) pos_moved += 6;
            printf("Computer has played the move on hole %d, colour %s and special seed at %d.\n",
                pos_moved, position.move.colour, position.move.spos);
            fprintf(f, "Computer has played the move on hole %d, colour %s and special seed at %d.\n",
                pos_moved, position.move.colour, position.move.spos);
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

Pos computeComputerMove(FILE * f, Pos initial, int maxDepth, int * move_cnt) {
    // Initial move is a move by player 1
    initial.player = 1;

    Move nextMove;
    bool maximisingPlayer = true;

    // the first move that is feeded is always the move that has been done by the player
    int counter = 0;
    int * move_counter = move_cnt;
    fprintf(f, "Calling minimaxAlphaBeta\n");
    nextMove = minimaxAlphaBeta(f, initial, maximisingPlayer, -76, 76,
                                maxDepth, maxDepth, 0, &counter, move_counter);

    printf("Number of nodes traversed: %d\n", counter);
    fprintf(f, "Number of nodes traversed: %d\n", counter);
    fprintf(f, "Score for the move: %d\n", nextMove.score);
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
    p2.move.spos = p1.move.spos;
    p2.seeds_diff = p1.seeds_diff;
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

Move minimaxAlphaBeta(FILE * f, Pos position, bool maximisingPlayer, int alpha, int beta,
                int depth, int maxDepth, int parent_idx, int * count, int * move_count) {
    *count += 1;
    int * counter = count;
    int * move_counter = move_count;
    // printf("2");
    // if (*move_count % 1000 == 0 ) printf("1000");
    if (depth == 0) {
        bool game_ending = (76 - position.seeds_player - position.seeds_computer < 25) ? true : false;

        Move move;
        position.evaluation = evaluate_score(position);
        move.score = (game_ending == 1) ? position.seeds_diff : position.evaluation;
        move.position = copyPos(position);
        move.leaf = copyPos(position);

        fprintf(f, "Depth is %d, game ending is %d, score %d, position.move.hole %d, position.move.colour %s position.move.spos %d\n",
            depth, game_ending, move.score, position.move.hole, position.move.colour, position.move.spos);
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

                fprintf(f, "Depth is %d, parent_idx is %d, current_idx %d, score %d,  newPos.move.hole %d, newPos.move.colour %s newPos score %d movespos %d l %d\n",
                    depth, parent_idx, newPos.current_idx, newPos.evaluation, newPos.move.hole, newPos.move.colour, newPos.evaluation, newPos.move.spos, l);

                if (newPos.valid_move == 1) {

                    Move childPos;
                    Pos feedPos = copyPos(newPos);

                    bool toMax = maximisingPlayer ? false : true;
                    childPos = minimaxAlphaBeta(f, feedPos, toMax, alpha, beta, newDepth, maxDepth,
                                index + idx, counter, move_counter);

                    if (first_valid == 0) {
                        // initialization of value if not given
                        bestMove.score = childPos.score;
                        bestMove.position = (newPos.current_idx == 0) ? copyPos(childPos.position) : copyPos(newPos);
                        bestMove.leaf = (newDepth == 0) ? copyPos(childPos.position) : childPos.leaf;
                        first_valid = 1;
                        fprintf(f, "Initializing the first valid depth %d bestMove.leaf pos %d, bestMove.pos %d, bestMove.score %d\n",
                            depth, bestMove.leaf.move.hole, bestMove.position.move.hole, bestMove.score);

                    } else if ((maximisingPlayer && childPos.score > bestMove.score) ||
                            (!maximisingPlayer && childPos.score < bestMove.score)) {
                        bestMove.found_better = 1;
                        bestMove.score = childPos.score;
                        bestMove.position = (newPos.current_idx == 0) ? copyPos(childPos.position) : copyPos(newPos);
                        if (newDepth == 0 ) bestMove.leaf = copyPos(childPos.position);

                        fprintf(f, "Found better move at depth %d bestMove.leaf pos %d, bestMove.pos %d, bestMove.score %d\n",
                            depth, bestMove.leaf.move.hole, bestMove.position.move.hole, bestMove.score);

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

        fprintf(f, "SPOS now when trying to maximis %d\n", position.move.spos);
        Pos bstPos = generatePosition(position, minhole, 1, position.move.spos,
                        maximisingPlayer, parent_idx, move_counter);
        if (bstPos.valid_move == 1) {
            bestMove.position = bstPos;
            bestMove.score = bstPos.evaluation;
        }
        fprintf(f, "No bestMove found, random iniaitlization score is %d\n", bestMove.score); 

    }

    fprintf(f, "BestMove returned position %d, colour %s, spos %d, score %d\n",
        bestMove.position.move.hole, bestMove.position.move.colour, bestMove.position.move.spos, bestMove.score);
    return bestMove;
}

Hole requestMove(int player, Cell * cells, int first_player) {
    Hole request;

    int check_idx;
    if (player == first_player) {
        do {
            printf("Which position to sow? Choices: ");
            for (int i=0; i < 6; i++) {
                int idx = (player == 0) ? i : i+6;
                if (cells[idx].total > 0) printf("%d ", i+1);
            }
            printf("\t");
            scanf("%d", &request.hole);
            request.hole -= 1;
            check_idx = (player == 0) ? request.hole : request.hole + 6;
            printf("request hole is %d, idx %d, idx total %d", request.hole, check_idx, cells[check_idx].total);
        } while(!(request.hole >= 0 && request.hole <= 5) || cells[check_idx].total == 0);
    } else {
        do {
            printf("Which position to sow? Choices: ");
            for (int i=6; i < 12; i++) {
                int idx = (player == 1) ? i : i-6;
                if (cells[idx].total > 0) printf("%d ", i+1);
            }
            scanf("%d", &request.hole);
            request.hole -= 1;
            check_idx = (player == 1) ? request.hole : request.hole - 6;
        } while(!(request.hole >= 6 && request.hole <= 11) || cells[check_idx].total == 0);
    }

    if (player == first_player && player == 1) {
        request.hole += 6;
    } else if (player != first_player && player == 0) {
        request.hole -= 6;
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
    // printf("Comes into move\n");
    // skip original position- change it to while loop
    Pos newPos;
    newPos = copyPos(position);

    Cell * cellscpy = (Cell *)malloc(sizeof(Cell)*12);
    memcpy(cellscpy, position.cells, sizeof(Cell)*12);

    Cell stones = cellscpy[newPos.move.hole];
    Cell empty = {0, 0 ,0};
    cellscpy[position.move.hole] = empty;
    int original_total_stones = stones.total;
    int looped = (original_total_stones > 11) ? 1 : 0;

    // the capture colour should be the last color being stored
    char capture_colour[2];
    if (strcmp(newPos.move.colour, "R") == 0) {
        if (stones.black > 0) {
            strncpy(capture_colour, "B", 2);
        } else {
            strncpy(capture_colour, "R", 2);
        }
    } else if (strcmp(newPos.move.colour, "B") == 0) {
        if (stones.red > 0) {
            strncpy(capture_colour, "R", 2);
        } else {
            strncpy(capture_colour, "B", 2);
        }
    }

    int cellid = newPos.move.hole;
    int idx = 1;
    int pos_id;

    // quick hack to make sure that special seed does not disappear
    if (stones.special > 0 && newPos.move.spos <= 0) {
        newPos.move.spos = 1;
    }
    int special_pos = newPos.move.spos;

    while (stones.total > 0) {
        cellid++; // in order to get final index at the end
        pos_id = cellid % 12;

        if (pos_id == newPos.move.hole) continue;
        if (idx == special_pos && stones.special > 0) {
            cellscpy[pos_id].special += 1;
            stones.special--;
            special_pos++; // just in case if there is 2 special seeds
            // if special is the last seed then overwrite it
            if (stones.total == 1) {
                strncpy(capture_colour, "S", 2);
            }
        } else if (strcmp(newPos.move.colour, "R") == 0 && stones.red > 0) {
            cellscpy[pos_id].red += 1;
            stones.red--;
        } else if (strcmp(newPos.move.colour, "B") == 0 && stones.black > 0) {
            cellscpy[pos_id].black += 1;
            stones.black--;
        } else if (stones.black > 0) {
            cellscpy[pos_id].black += 1;
            stones.black--;
        } else if (stones.red > 0) {
            cellscpy[pos_id].red += 1;
            stones.red--;
        }

        // double check if idx is added correctly
        idx++;
        cellscpy[pos_id].total += 1;
        stones.total--;
    }
    // store the final index
    newPos.last_pos = cellid % 12;

    // write capture here
    int first_index = (newPos.player == 0) ? 6 : 0;
    int last_index = (newPos.player == 0) ? 11 : 5;
    // only if there is at least more than 6 seeds then it could spill over to the other side

    int capture = 1;
    int capt_index = newPos.last_pos;
    if (newPos.last_pos > last_index) {
        if (original_total_stones > 6 || looped == 1) {
            capt_index = last_index;
        } else {
            capture = 0;
        }
    }

    int scores_gain = 0;
    while (capture == 1 && capt_index >= first_index) {
        int black_seeds = cellscpy[capt_index].black + cellscpy[capt_index].special;
        int red_seeds = cellscpy[capt_index].red + cellscpy[capt_index].special;

        int capture_red = (red_seeds >= 2 && red_seeds <= 3) ? 1 : 0;
        int capture_black = (black_seeds >= 2 && black_seeds <= 3) ? 1 : 0;

        if (strcmp(capture_colour, "S") == 0) {
            if (capture_black && capture_red) {
                strncpy(capture_colour, "S", 2);
                int total_seeds_capt = black_seeds + red_seeds - cellscpy[capt_index].special;
                cellscpy[capt_index].total -= total_seeds_capt;
                scores_gain += total_seeds_capt;
                cellscpy[capt_index].black = 0;
                cellscpy[capt_index].red = 0;
                cellscpy[capt_index].special = 0;
            }
            else if (capture_black) {
                strncpy(capture_colour, "B", 2);
                cellscpy[capt_index].total -= black_seeds;
                scores_gain += black_seeds;
                cellscpy[capt_index].black = 0;
                cellscpy[capt_index].special = 0;
            }
            else if (capture_red) {
                strncpy(capture_colour, "R", 2);
                cellscpy[capt_index].total -= red_seeds;
                scores_gain += red_seeds;
                cellscpy[capt_index].red = 0;
                cellscpy[capt_index].special = 0;
            } else {
                capture = 0;
            }
        } else if (strcmp(capture_colour, "R") == 0 && capture_red) {
            scores_gain += red_seeds;
            cellscpy[capt_index].total -= red_seeds;
            cellscpy[capt_index].red = 0;
            if (cellscpy[capt_index].special > 0) cellscpy[capt_index].special = 0;
        } else if (strcmp(capture_colour, "B") == 0 && capture_black) {
            scores_gain += black_seeds;
            cellscpy[capt_index].total -= black_seeds;
            cellscpy[capt_index].black = 0;
            if (cellscpy[capt_index].special > 0) cellscpy[capt_index].special = 0;
        } else {
            capture = 0;
        }
        capt_index--;
    }

    if (scores_gain > 0 && newPos.player == 0) {
        newPos.seeds_computer += scores_gain;
    } else if (scores_gain > 0 && newPos.player == 1) {
        newPos.seeds_player += scores_gain;
    }

    memcpy(newPos.cells, cellscpy, sizeof(Cell)*12);
    return newPos;
}