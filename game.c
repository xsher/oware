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
    int score;
    Pos position;
    Pos leaf;
    int found_better;
} Move;

void startGame(Pos position, int maxDepth);
Pos move(Pos position);
int capture(Hole move, int last_pos, int looped, int player, Cell * cells);
int evaluate(Pos position, int * move_count);
Pos copyPos(Pos p1);
Hole requestMove(int player, Cell * cells);
void requestSpecialSeed(Pos position);

Move minimax(Pos position, bool maximisingPlayer,
    int depth, int parent_idx, int * count);
Move minimaxAlphaBeta(Pos position, bool maximisingPlayer, int alpha,
    int beta, int depth, int maxDepth, int parent_idx, int * count, int * move_count);
Pos generatePosition(Pos position, int hole, int col, int spos,
    bool maximisingPlayer, int parent_idx, int * move_count);
Pos computeComputerMove(Pos initial, int maxDepth, int * move_cnt);


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

    printf("Player %d", position.player);
    requestSpecialSeed(position);

    printf("Player %d ", (position.player == 0) ? 1 : 0);
    requestSpecialSeed(position);

    startGame(position, maxDepth);
}

void requestSpecialSeed(Pos position) {
    int special_hole;

    do {
        printf("\nWhere would you like to place the special seed?  ");
        scanf("%d", &special_hole);
    } while (!(special_hole >= 0 && special_hole <= 11));

    position.cells[special_hole].special += 1;
    position.cells[special_hole].total += 1;
    printf("Board status after placing special seed.\n");
    printBoard(position.cells, position.seeds_computer, position.seeds_player);
}

void startGame(Pos position, int maxDepth) {
    int scores_gain;
    int maxSeeds = 74;    // 12 holes, 6 seeds each and 2 special seeds
    int move_counter = 0;

    // while it is not game over yet
    while (position.seeds_computer <= maxSeeds/2 ||
            position.seeds_player <= maxSeeds/2) {
        printf("\nPlayer %d turn\n", position.player);

        if (position.player == 1) {
            position.move = requestMove(position.player, position.cells);
            position = move(position);
        } else {
            struct timeval start, end;
            gettimeofday(&start, NULL);

            position = computeComputerMove(position, maxDepth, &move_counter);
            gettimeofday(&end, NULL);
            double elapsed = (end.tv_sec - start.tv_sec) +
              ((end.tv_usec - start.tv_usec)/1000000.0);
            printf("Computer has played the move on hole %d, colour %s and special seed at %d.\n",
                position.move.hole, position.move.colour);
            printf("Time taken for computer %f\n\n", elapsed);
        }

        move_counter += 1;
        printf("Current board status:\n");
        printBoard(position.cells, position.seeds_computer, position.seeds_player);
        position.player = (position.player == 1) ? 0 : 1;
    }

    printf("GAME OVER.\n");
}

Pos computeComputerMove(Pos initial, int maxDepth, int * move_cnt) {
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

int evaluate(Pos position, int * move_count) {
    return position.seeds_computer - position.seeds_player;
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
    newPos.evaluation = evaluate(position, move_counter);
    newPos.parent_idx = parent_idx;

    return newPos;
}

Move minimaxAlphaBeta(Pos position, bool maximisingPlayer, int alpha, int beta,
                int depth, int maxDepth, int parent_idx, int * count, int * move_count) {
    *count += 1;
    int * counter = count;
    int * move_counter = move_count;
    if (depth == 0) {
        Move move;
        move.score = evaluate(position, move_counter);

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
        Pos bstPos = generatePosition(position, minhole, 1, position.move.spos,
                                maximisingPlayer, parent_idx, move_counter);
        bestMove.position = bstPos;
        bestMove.score = bstPos.evaluation;
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

    Cell * cellscpy = (Cell *)malloc(sizeof(Cell)*12);
    memcpy(cellscpy, position.cells, sizeof(Cell)*12);

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
    capt_index = (looped == 1) ? last_index : newPos.last_pos % 12;

    // breaks out when there is no stones left
    for (int j = newPos.last_pos; ; j--) {
        int cellid = j % 12;

        if (cellid == capt_index) capture = true;
        if (cellid == newPos.move.hole) {
            capture = false;
            continue;
        }

        char seed_placed[2];

        if (j == special_seed_pos & stones.special > 0) {
            cellscpy[cellid].special += 1;
            strncpy(seed_placed, "S", 2);
            stones.special--;
        } else if (strcmp(newPos.move.colour, "R") == 0 && stones.black > 0) {
            cellscpy[cellid].black += 1;
            strncpy(seed_placed, "B", 2);
            stones.black--;
        } else if (strcmp(newPos.move.colour, "B") == 0 && stones.red > 0) {
            cellscpy[cellid].red += 1;
            strncpy(seed_placed, "R", 2);
            stones.red--;
        } else if (stones.black > 0) {
            cellscpy[cellid].black += 1;
            strncpy(seed_placed, "B", 2);
            stones.black--;
        } else if (stones.red > 0) {
            cellscpy[cellid].red += 1;
            strncpy(seed_placed, "R", 2);
            stones.red--;
        }

        cellscpy[cellid].total += 1;
        stones.total--;

        if (capture && cellid >= first_index && cellid <= last_index) {
            if (strcmp(seed_placed, "R") == 0 && cellscpy[cellid].red >= 2 && cellscpy[cellid].red <= 3) {
                scores_gain += cellscpy[cellid].red;
                cellscpy[cellid].total -= cellscpy[cellid].red;
                cellscpy[cellid].red = 0;
            } else if (strcmp(seed_placed, "B") == 0 && cellscpy[cellid].black >= 2 && cellscpy[cellid].black <= 3) {
                scores_gain += cellscpy[cellid].black;
                cellscpy[cellid].total -= cellscpy[cellid].black;
                cellscpy[cellid].black = 0;
            } else if (strcmp(seed_placed, "S") == 0) {
                int black_seeds = cellscpy[cellid].black + cellscpy[cellid].special;
                int red_seeds = cellscpy[cellid].red + cellscpy[cellid].special;
                int gained_special = 0;

                if (black_seeds >= 2 && black_seeds <= 3) {
                    cellscpy[cellid].total -= cellscpy[cellid].black;
                    scores_gain += black_seeds;
                    cellscpy[cellid].black = 0;
                    cellscpy[cellid].special = 0;
                    gained_special = 1;
                }
                if (red_seeds >= 2 && red_seeds <= 3) {
                    cellscpy[cellid].total -= cellscpy[cellid].red;
                    scores_gain += red_seeds;
                    cellscpy[cellid].red = 0;
                    cellscpy[cellid].special = 0;
                }
                if (gained_special == 1) cellscpy[cellid].total--; // minus special seed only once
            } else {
                capture = false;
            }
        } else {
            capture = false;
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