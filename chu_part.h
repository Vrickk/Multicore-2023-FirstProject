#include <math.h>
#include <ctime>
#include <iostream>
using namespace std;

void generateArrays(int arrays[][200], int numArrays, int arraySize) {
    srand(time(NULL)); // seed the random number generator with the current time
    for (int i = 0; i < numArrays; i++) {
        for (int j = 0; j < arraySize; j++) {
            arrays[i][j] = rand() % 4 + 1; // generate random number between 1 and 4
        }
    }
}

bool no_way(int coordination[2], int** maze) {
    int count;
    if (maze[coordination[0]][coordination[1] + 1] == '#')
        count++;
    if (maze[coordination[0]][coordination[1] - 1] == '#')
        count++;
    if (maze[coordination[0] + 1][coordination[1]] == '#')
        count++;
    if (maze[coordination[0] - 1][coordination[1]] == '3')
        count++;

    if (count == 3)
        return true;
    else
        return false;
}

int Evaluate(int suitability, int coordination[2], int next_coordination[2], int** maze, int End_coordination[2]) {
    int now_length = pow((End_coordination[0] - coordination[0]), 2) + pow((End_coordination[1] - coordination[1]), 2);
    int next_length = pow((End_coordination[0] - next_coordination[0]), 2) + pow((End_coordination[1] - next_coordination[1]), 2);
    if (maze[next_coordination[0]][next_coordination[1]] == 'V')
        suitability -= 2;
    else if (now_length > next_length)
        suitability -= 1;
    else if (now_length < next_length)
        suitability += 1;
    else if (no_way(next_coordination, maze))
        suitability -= 10;
    else if (next_coordination == End_coordination)
        suitability += 30;
}