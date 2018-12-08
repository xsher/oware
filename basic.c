#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int * generateTree(int initial, int maxDepth);

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
    printArray(cells, 6);

    int * newcells = (int*)malloc(sizeof(int)*12);

    memcpy(&newcells[6], &cells[0], sizeof(int)*6);
    printArray(newcells, 12);

    int maxDepth = 3;
    int sizeOfArray = pow(6, maxDepth + 1) / 5;
    int * nodes = (int*)malloc(sizeof(int)*sizeOfArray);

    nodes = generateTree(999, maxDepth);
    printArray(nodes, sizeOfArray);
}

int * generateTree(int initial, int maxDepth) {
    int sizeOfArray = pow(6, maxDepth + 1) / 5;

    
    printf("Size of array: %d\n", sizeOfArray);

    // allocate memory for all the nodes
    int * nodes = (int*)malloc(sizeof(int)*sizeOfArray);
    
    // mapping from parent to child = idx * 6^0 (+0, +1 , +2, +3, +4, +5)
    // mapping from child to parent = roundup(idx/6) - 1
    nodes[0] = initial;
    for (int i = 1; i < sizeOfArray; i++) {
        nodes[i] = ceil(i / 6.0) - 1;
    }
    return nodes;
}