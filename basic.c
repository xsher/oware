#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void generateTree(int * nodes, int initial, int depth, int parent_idx);

void printArray(int * cells, int size) {
    for (int i = 0; i<size; i++) {
        printf("Cell %d, value %d\n", i, cells[i]);
    }
    printf("\n");
}

int main(void) {
  
    int * cells = (int*)malloc(sizeof(int)*6);

    for (int i = 0; i < 6; i++) {
        cells[i] = i;
    }
    //printArray(cells, 6);

    int * newcells = (int*)malloc(sizeof(int)*12);

    memcpy(&newcells[6], &cells[0], sizeof(int)*6);
    //printArray(newcells, 12);

    int maxDepth = 3;
    int sizeOfArray = pow(6, maxDepth + 1) / 5;
    int * nodes = (int*)malloc(sizeof(int)*sizeOfArray);

    nodes[0] = 999;
    generateTree(nodes, 999, maxDepth, 0);
    printArray(nodes, sizeOfArray);
}

void generateTree(int * nodes, int initial, int depth, int parent_idx) {

    if (depth == 0) {
        return;
    }

    int possib_moves[6];
    for (int i = 0; i < 6; i++) {
        // for now let's store the parent id
        possib_moves[i] = parent_idx;
    }

    // mapping from parent to child = idx * 6 + 1 for the first child
    // mapping from child to parent = roundup(idx/6) - 1 
    // get the child index based on the parent index
    int index = (parent_idx * 6) + 1;
    memcpy(&nodes[index], &possib_moves[0], sizeof(int)*6);

    int newDepth = depth - 1;
    if (newDepth > 0) {
        for (int i = 0; i < 6; i++) {
            // printf("parent Index %d \n", parentIndex);
            generateTree(nodes, initial, newDepth, index + i);
        }

    }
}

/*
0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42
n 0 0 0 0 0 0 1 1 1  1  1  1  2  2  2  2  2  2  3  3  3  3  3  3  4  4  4  4  4  4  5  5  5  5  5  5  6  6  6  6  6  6
*/