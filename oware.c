#include <stdbool.h>

/**
 * Start with a simple Oware Game without any rules about starving
 * Oware Game:
 * n = 6 holes per player
 * 4 seeds per hole
 * 1 color
 * no special
 *
 * Step 1: Generate the Tree
 * Step 2: Add validity check and also capture
 * Step 3: Add minMax
 * Step 4: Add Alpha-Beta
 * Step 5: Transition table? Remove duplicates?
 * Step 6: Can we think about parallelizing it?
*/

struct Position {
    int cells_player[6]; // each cell contains a certain number of seeds
    int cells_computer[6];
    bool computer_play; // boolean true if the computer has to play and false otherwise
    int seeds_player; // seeds taken by the player
    int seeds_computer; // seeds taken by the computer
};

int minMaxValue(Position* pos_current, computer_play, depth, depthMax) {

    // computer_play is true if the computer has to play and false otherwise
    int tab_values[6];

    Position pos_next; // In C : created on the stack: = very fast

    if (finalPosition(pos_current, computer_play, depth)) {
        // WRITE the code: returns VALMAX (=96) if the computer wins, -96 if it loses; 0 if draw
    }

    if (depth == depthMax) {
        return evaluation(pos_current, computer_play, depth);
        // the simplest evealution fucntion is the difference of the taken seeds
    }

    for(int i = 0; i < 6; i++) {
        // we play the move i
        // WRITE function validMove(pos_current, computer_play,i)
        // it checks whether we can select the seeds in cell i and play (if there is no seed the function returns false)

        if (validMove(pos_current, computer_play, i)) {
            // WRITE function playMove(&pos_next,pos_current, computer_play,i)
            // we play the move i from pos_current and obtain the new position pos_next
            playMove(&pos_next,pos_current, computer_play, i);

            // pos_next is the new current poisition and we change the player
            tab_values[i]= minMaxValue(&pos_next, !computer_play, depth+1, depthMax);
        } else {
            if (computer_play) tab_values[i] =-100;
            else tab_values[i] =+100;
        }
    }

    int res;

    if (computer_play) {
            // WRITE the code: res contains the MAX of tab_values
    } else {
            // WRITE the code: res contains the MIN of tab_values
    }

    return res;
}