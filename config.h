#include <stdio.h>
#include <stdbool.h> // Include for boolean data type
#include "API.h"

// Define constants
#define ROWS 16
#define COLS 16
#define END1 7
#define END2 8
#define NORTH 0
#define EAST 1
#define SOUTH 2
#define WEST 3

// Define cell offset in directions
const int adjacentX[] = {0, 1, 0, -1}; // N E S W
const int adjacentY[] = {1, 0, -1, 0};

// Structure to represent a cell in the grid
typedef struct
{
    int manhattanDistance; // Manhattan distance to the end
    bool hasNorthWall;
    bool hasEastWall;
    bool hasSouthWall;
    bool hasWestWall;
    // unsigned char walls; // Using bitfields to represent walls
    bool visited;
    bool isDeadEnd;
} Cell;

// Global variables
int posX = 0;
int posY = 0;
int direction = NORTH;
Cell grid[ROWS][COLS]; // Use the Cell structure for the grid
int divergingPathsQueue[ROWS * COLS * 2][2];
int nextSpot[2] = {7, 7};
int last = 0;

// MMS SIMULATOR
char logMessageBuffer[50]; // MMS: To simply log some info and errors. Adjust the size as needed
// MMS: Actual logging function
void logMessage(char *text)
{
    fprintf(stderr, "%s\n", text);
    fflush(stderr);
}
// MMS: Function to convert integer to drawable string
void intToString(int num, char *buffer)
{
    if (num == 0)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    int i = 0;
    while (num != 0)
    {
        int digit = num % 10;
        buffer[i++] = digit + '0';
        num /= 10;
    }
    buffer[i] = '\0';
    int length = i;
    for (i = 0; i < length / 2; i++)
    {
        char temp = buffer[i];
        buffer[i] = buffer[length - i - 1];
        buffer[length - i - 1] = temp;
    }
}
// MMS: Function to draw the start and end
void drawZones()
{
    API_setColor(0, 0, 'R');
    API_setColor(7, 7, 'G');
    API_setColor(7, 8, 'G');
    API_setColor(8, 7, 'G');
    API_setColor(8, 8, 'G');
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            if (grid[i][j].isDeadEnd)
            {
                API_setColor(i, j, 'o');
            }
        }
    }
    API_setColor(nextSpot[0], nextSpot[1], 'w');
    for(int n = 0; n < last; n++)
    {
        API_setColor(divergingPathsQueue[n][0], divergingPathsQueue[n][1], 'c');
    }
}
// MMS: Function to print the grid
void drawGrid()
{
    char gridDistance[3]; // Assuming gridDistance will not exceed 3 digits
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            if (grid[i][j].manhattanDistance != -1)
            { // Print only calculated cells
                intToString(grid[i][j].manhattanDistance, gridDistance);
                API_setText(i, j, gridDistance);
            }
            if (grid[i][j].hasNorthWall == true)
            {
                API_setWall(i, j, 'n');
            }
            if (grid[i][j].hasSouthWall == true)
            {
                API_setWall(i, j, 's');
            }
            if (grid[i][j].hasEastWall == true)
            {
                API_setWall(i, j, 'e');
            }
            if (grid[i][j].hasWestWall == true)
            {
                API_setWall(i, j, 'w');
            }
            if (grid[i][j].visited == true && grid[i][j].manhattanDistance < 245)
            {
                API_setColor(i, j, 'C');
            }
        }
    }
}

// TO PORT PATH LOGIC
// Function to find the minimum or maximum value among four integers and return the number of equal items
int minOrMax(int a, int b, int c, int d, int *count, bool findMax)
{
    int result = a;
    *count = 1; // Initialize count to 1, assuming the first value is the minimum or maximum
    if ((findMax ? b > result : b < result))
    {
        result = b;
        *count = 1;
    }
    else if (b == result)
    {
        (*count)++;
    }
    if ((findMax ? c > result : c < result))
    {
        result = c;
        *count = 1;
    }
    else if (c == result)
    {
        (*count)++;
    }
    if ((findMax ? d > result : d < result))
    {
        result = d;
        *count = 1;
    }
    else if (d == result)
    {
        (*count)++;
    }
    return result;
}
// Function to initialize the Walls
void initializeGrid()
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            grid[i][j].manhattanDistance = -1; // Initialize to -1 to indicate unvisited cells
            grid[i][j].hasNorthWall = false;   // Initialize walls to false
            grid[i][j].hasEastWall = false;    // Initialize walls to false
            grid[i][j].hasSouthWall = false;   // Initialize walls to false
            grid[i][j].hasWestWall = false;    // Initialize walls to false
            grid[i][j].visited = false;
            grid[i][j].isDeadEnd = false;
            API_setColor(i, j, 'Y');
        }
    }
    // Mark start and end positions
    grid[0][0].manhattanDistance = 100; // Assuming we will not get a situation worse than 100
    grid[7][7].manhattanDistance = 0;
    // what about other end cells
}
// Function to initialize the grid
void initializeGridValues()
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            if (grid[i][j].manhattanDistance < 245)
            {
                grid[i][j].manhattanDistance = -1; // Initialize to -1 to indicate unvisited cells
            }
        }
    }
}
// Function to initialize visited cells
void initializeVisited()
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            grid[i][j].visited = false;
        }
    }
}
// function to fill numbers in the grid
void floodFillNumbers()
{
    // Initialize the queue
    int queue[ROWS * COLS * 2][2];
    int front = 0, rear = 0;
    // Set end position to 0 and add it to the queue
    grid[END1][END1].manhattanDistance = 0;
    queue[rear][0] = END1;
    queue[rear][1] = END1;
    rear++;
    grid[END1][END2].manhattanDistance = 0;
    queue[rear][0] = END1;
    queue[rear][1] = END2;
    rear++;
    grid[END2][END1].manhattanDistance = 0;
    queue[rear][0] = END2;
    queue[rear][1] = END1;
    rear++;
    grid[END2][END2].manhattanDistance = 0;
    queue[rear][0] = END2;
    queue[rear][1] = END2;
    rear++;

    // Iterate until the queue is empty
    while (front < rear)
    {
        // Dequeue the cell
        int x = queue[front][0];
        int y = queue[front][1];
        front++;

        // Explore adjacent cells
        for (int d = 0; d < 4; d++)
        {
            int newX = x + adjacentX[d];
            int newY = y + adjacentY[d];

            // Check if the adjacent cell is within bounds and unvisited
            if (newX >= 0 && newX < ROWS && newY >= 0 && newY < COLS && grid[newX][newY].manhattanDistance == -1)
            {
                // Check for walls in the current cell
                bool hasWall = false;
                switch (d)
                {
                case NORTH:
                    hasWall = grid[x][y].hasNorthWall;
                    break;
                case EAST:
                    hasWall = grid[x][y].hasEastWall;
                    break;
                case SOUTH:
                    hasWall = grid[x][y].hasSouthWall;
                    break;
                case WEST:
                    hasWall = grid[x][y].hasWestWall;
                    break;
                }
                // If there's no wall, enqueue the adjacent cell
                if (!hasWall)
                {
                    // Set the value of the adjacent cell and enqueue it
                    grid[newX][newY].manhattanDistance = grid[x][y].manhattanDistance + 1;
                    queue[rear][0] = newX;
                    queue[rear][1] = newY;
                    rear++;
                }
            }
        }
    }
    // Overwrite the Manhattan distance of dead-end cells with 250
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            if (grid[i][j].isDeadEnd)
            {
                grid[i][j].manhattanDistance = 250;
            }
        }
    }
}
// function to go back to start
void floodFillStart()
{
    // Initialize the queue
    int queue[ROWS * COLS * 2][2];
    int front = 0, rear = 0;
    // Set end position to 0 and add it to the queue
    grid[0][0].manhattanDistance = 0;
    queue[rear][0] = 0;
    queue[rear][1] = 0;
    rear++;

    // Iterate until the queue is empty
    while (front < rear)
    {
        // Dequeue the cell
        int x = queue[front][0];
        int y = queue[front][1];
        front++;

        // Explore adjacent cells
        for (int d = 0; d < 4; d++)
        {
            int newX = x + adjacentX[d];
            int newY = y + adjacentY[d];

            // Check if the adjacent cell is within bounds and unvisited
            if (newX >= 0 && newX < ROWS && newY >= 0 && newY < COLS && grid[newX][newY].manhattanDistance == -1)
            {
                // Check for walls in the current cell
                bool hasWall = false;
                switch (d)
                {
                case NORTH:
                    hasWall = grid[x][y].hasNorthWall;
                    break;
                case EAST:
                    hasWall = grid[x][y].hasEastWall;
                    break;
                case SOUTH:
                    hasWall = grid[x][y].hasSouthWall;
                    break;
                case WEST:
                    hasWall = grid[x][y].hasWestWall;
                    break;
                }
                // If there's no wall, enqueue the adjacent cell
                if (!hasWall)
                {
                    // Set the value of the adjacent cell and enqueue it
                    grid[newX][newY].manhattanDistance = grid[x][y].manhattanDistance + 1;
                    queue[rear][0] = newX;
                    queue[rear][1] = newY;
                    rear++;
                }
            }
        }
    }
}
// Mark unvisited but solved cells
void checkSolvedCells()
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            if (!grid[i][j].visited)
            {
                // Explore adjacent cells
                int countVisitedNeighbours = 0;
                for (int d = 0; d < 4; d++)
                {
                    int newX = i + adjacentX[d];
                    int newY = j + adjacentY[d];
                    // Check if the adjacent cell is within bounds and visited
                    if (newX >= 0 && newX < ROWS && newY >= 0 && newY < COLS && grid[newX][newY].visited)
                    {
                        countVisitedNeighbours++;
                    }
                }
                if ((i == 0 || i == ROWS - 1 || j == 0 || j == COLS - 1) && countVisitedNeighbours == 3)
                {
                    grid[i][j].visited = true;
                    sprintf(logMessageBuffer, "     Found solved edge cell at (%d, %d)", i, j);
                    logMessage(logMessageBuffer);
                }
                else if ((i == 0 || i == ROWS - 1) && (j == 0 || j == COLS - 1) && countVisitedNeighbours == 2)
                {
                    grid[i][j].visited = true;
                    sprintf(logMessageBuffer, "     Found solved corner cell at (%d, %d)", i, j);
                    logMessage(logMessageBuffer);
                }
                else if (countVisitedNeighbours == 4)
                {
                    grid[i][j].visited = true;
                    sprintf(logMessageBuffer, "     Found solved cell at (%d, %d)", i, j);
                    logMessage(logMessageBuffer);
                }
                if (grid[i][j].visited)
                {
                    int nWalls = 0;
                    nWalls += grid[i][j].hasNorthWall ? 1 : 0;
                    nWalls += grid[i][j].hasEastWall ? 1 : 0;
                    nWalls += grid[i][j].hasSouthWall ? 1 : 0;
                    nWalls += grid[i][j].hasWestWall ? 1 : 0;
                    if (nWalls > 2)
                    {
                        grid[i][j].isDeadEnd = true;
                        grid[i][j].manhattanDistance = 250;
                    }
                }
            }
        }
    }
}
// function to fill dead end paths in the grid
void killDeadEndPaths()
{
    // Initialize the queue
    int queue[ROWS * COLS * 2][2];
    int front = 0, rear = 0;

    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            if (grid[i][j].isDeadEnd == true)
            {
                queue[rear][0] = i;
                queue[rear][1] = j;
                rear++;
                sprintf(logMessageBuffer, "     Found dead end at (%d, %d)", i, j);
                logMessage(logMessageBuffer);
            }
        }
    }

    // Iterate until the queue is empty
    while (front < rear)
    {
        // Dequeue the cell
        int x = queue[front][0];
        int y = queue[front][1];
        sprintf(logMessageBuffer, "     Dequeued (%d, %d)", x, y);
        logMessage(logMessageBuffer);
        front++;

        // Explore adjacent cells
        for (int d = 0; d < 4; d++)
        {
            int newX = x + adjacentX[d];
            int newY = y + adjacentY[d];

            // Check if the adjacent cell is within bounds and unvisited
            if (newX >= 0 && newX < ROWS && newY >= 0 && newY < COLS)
            {
                // Check for walls in the current cell
                bool hasWall = false;
                switch (d)
                {
                case NORTH:
                    hasWall = grid[x][y].hasNorthWall;
                    // if (y + 1 < COLS)
                    //{
                    //     hasWall = grid[x][y + 1].manhattanDistance > 245;
                    // }
                    break;
                case EAST:
                    hasWall = grid[x][y].hasEastWall;
                    // if (x + 1 < ROWS)
                    //{
                    //     hasWall = grid[x + 1][y].manhattanDistance > 245;
                    // }
                    break;
                case SOUTH:
                    hasWall = grid[x][y].hasSouthWall;
                    // if (y - 1 > 0)
                    //{
                    //     hasWall = grid[x][y - 1].manhattanDistance > 245;
                    // }
                    break;
                case WEST:
                    hasWall = grid[x][y].hasWestWall;
                    // if (x - 1 > 0)
                    //{
                    //     hasWall = grid[x - 1][y].manhattanDistance > 245;
                    // }
                    break;
                }
                // Check if the adjacent cell has exactly two walls
                int newCellWallCount = 0;
                newCellWallCount += grid[newX][newY].hasNorthWall ? 1 : 0;
                newCellWallCount += grid[newX][newY].hasEastWall ? 1 : 0;
                newCellWallCount += grid[newX][newY].hasSouthWall ? 1 : 0;
                newCellWallCount += grid[newX][newY].hasWestWall ? 1 : 0;
                // If there's no wall, enqueue the adjacent cell
                if (!hasWall && newCellWallCount == 2)
                {
                    // Set the value of the adjacent cell and enqueue it
                    sprintf(logMessageBuffer, "  2 walled neighbor found at (%d, %d) = %d", newX, newY, grid[newX][newY].manhattanDistance);
                    logMessage(logMessageBuffer);
                    if (grid[newX][newY].manhattanDistance < 249)
                    {
                        grid[newX][newY].manhattanDistance = 249;
                        queue[rear][0] = newX;
                        queue[rear][1] = newY;
                        rear++;
                        sprintf(logMessageBuffer, "     Setting the 2 walled neighbor also as a dead end at (%d, %d)", newX, newY);
                        logMessage(logMessageBuffer);
                        API_setColor(newX, newY, 'o'); // Mark the dead-end cell orange
                    }
                }
            }
        }
    }
    checkSolvedCells();
}
// Function to remember the wall positions
void checkAndMarkWalls()
{
    // Initialize a counter to keep track of the number of walls surrounding the cell
    int wallCount = 0;

    switch (direction)
    {
    case NORTH:
        if (API_wallFront())
        {
            grid[posX][posY].hasNorthWall = true;
            wallCount++;
            sprintf(logMessageBuffer, "Marked North Wall at %d, %d position", posX, posY);
            logMessage(logMessageBuffer);
            if (posY + 1 < COLS)
            {
                grid[posX][posY + 1].hasSouthWall = true;
                sprintf(logMessageBuffer, "Also marked South Wall at %d, %d position", posX, posY + 1);
                logMessage(logMessageBuffer);
            }
        }
        if (API_wallRight())
        {
            grid[posX][posY].hasEastWall = true;
            wallCount++;
            sprintf(logMessageBuffer, "Marked East Wall at %d, %d position", posX, posY);
            logMessage(logMessageBuffer);
            if (posX + 1 < ROWS)
            {
                grid[posX + 1][posY].hasWestWall = true;
                sprintf(logMessageBuffer, "Also marked West Wall at %d, %d position", posX + 1, posY);
                logMessage(logMessageBuffer);
            }
        }
        if (API_wallLeft())
        {
            grid[posX][posY].hasWestWall = true;
            wallCount++;
            sprintf(logMessageBuffer, "Marked West Wall at %d, %d position", posX, posY);
            logMessage(logMessageBuffer);
            if (posX - 1 >= 0)
            {
                grid[posX - 1][posY].hasEastWall = true;
                sprintf(logMessageBuffer, "Also marked East Wall at %d, %d position", posX - 1, posY);
                logMessage(logMessageBuffer);
            }
        }
        break;
    case EAST:
        if (API_wallFront())
        {
            grid[posX][posY].hasEastWall = true;
            wallCount++;
            sprintf(logMessageBuffer, "Marked East Wall at %d, %d position", posX, posY);
            logMessage(logMessageBuffer);
            if (posX + 1 < ROWS)
            {
                grid[posX + 1][posY].hasWestWall = true;
                sprintf(logMessageBuffer, "Also marked West Wall at %d, %d position", posX + 1, posY);
                logMessage(logMessageBuffer);
            }
        }
        if (API_wallRight())
        {
            grid[posX][posY].hasSouthWall = true;
            wallCount++;
            sprintf(logMessageBuffer, "Marked South Wall %d. at %d, %d position", grid[posX][posY].hasSouthWall, posX, posY);
            logMessage(logMessageBuffer);
            if (posY - 1 >= 0)
            {
                grid[posX][posY - 1].hasNorthWall = true;
                sprintf(logMessageBuffer, "Also marked North Wall %d. at %d, %d position", grid[posX][posY - 1].hasNorthWall, posX, posY - 1);
                logMessage(logMessageBuffer);
            }
        }
        if (API_wallLeft())
        {
            grid[posX][posY].hasNorthWall = true;
            wallCount++;
            sprintf(logMessageBuffer, "Marked North Wall %d. at %d, %d position", grid[posX][posY].hasNorthWall, posX, posY);
            logMessage(logMessageBuffer);
            if (posY + 1 < COLS)
            {
                grid[posX][posY + 1].hasSouthWall = true;
                sprintf(logMessageBuffer, "Also marked South Wall %d. at %d, %d position", grid[posX][posY + 1].hasSouthWall, posX, posY + 1);
                logMessage(logMessageBuffer);
            }
        }
        break;
    case SOUTH:
        if (API_wallFront())
        {
            grid[posX][posY].hasSouthWall = true;
            wallCount++;
            sprintf(logMessageBuffer, "Marked South Wall %d. at %d, %d position", grid[posX][posY].hasSouthWall, posX, posY);
            logMessage(logMessageBuffer);
            if (posY - 1 >= 0)
            {
                grid[posX][posY - 1].hasNorthWall = true;
                sprintf(logMessageBuffer, "Also marked North Wall %d. at %d, %d position", grid[posX][posY - 1].hasNorthWall, posX, posY - 1);
                logMessage(logMessageBuffer);
            }
        }
        if (API_wallRight())
        {
            grid[posX][posY].hasWestWall = true;
            wallCount++;
            sprintf(logMessageBuffer, "Marked West Wall at %d, %d position", posX, posY);
            logMessage(logMessageBuffer);
            if (posX - 1 >= 0)
            {
                grid[posX - 1][posY].hasEastWall = true;
                sprintf(logMessageBuffer, "Also marked East Wall at %d, %d position", posX - 1, posY);
                logMessage(logMessageBuffer);
            }
        }
        if (API_wallLeft())
        {
            grid[posX][posY].hasEastWall = true;
            wallCount++;
            sprintf(logMessageBuffer, "Marked East Wall at %d, %d position", posX, posY);
            logMessage(logMessageBuffer);
            if (posX + 1 < ROWS)
            {
                grid[posX + 1][posY].hasWestWall = true;
                sprintf(logMessageBuffer, "Also marked West Wall at %d, %d position", posX + 1, posY);
                logMessage(logMessageBuffer);
            }
        }
        break;
    case WEST:
        if (API_wallFront())
        {
            grid[posX][posY].hasWestWall = true;
            wallCount++;
            sprintf(logMessageBuffer, "Marked West Wall at %d, %d position", posX, posY);
            logMessage(logMessageBuffer);
            if (posX - 1 >= 0)
            {
                grid[posX - 1][posY].hasEastWall = true;
                sprintf(logMessageBuffer, "Also marked East Wall at %d, %d position", posX - 1, posY);
                logMessage(logMessageBuffer);
            }
        }
        if (API_wallRight())
        {
            grid[posX][posY].hasNorthWall = true;
            wallCount++;
            sprintf(logMessageBuffer, "Marked North Wall %d. at %d, %d position", grid[posX][posY].hasNorthWall, posX, posY);
            logMessage(logMessageBuffer);
            if (posY + 1 < COLS)
            {
                grid[posX][posY + 1].hasSouthWall = true;
                sprintf(logMessageBuffer, "Also marked South Wall %d. at %d, %d position", grid[posX][posY + 1].hasSouthWall, posX, posY + 1);
                logMessage(logMessageBuffer);
            }
        }
        if (API_wallLeft())
        {
            grid[posX][posY].hasSouthWall = true;
            wallCount++;
            sprintf(logMessageBuffer, "Marked South Wall %d. at %d, %d position", grid[posX][posY].hasSouthWall, posX, posY);
            logMessage(logMessageBuffer);
            if (posY - 1 >= 0)
            {
                grid[posX][posY - 1].hasNorthWall = true;
                sprintf(logMessageBuffer, "Also marked North Wall %d. at %d, %d position", grid[posX][posY - 1].hasNorthWall, posX, posY - 1);
                logMessage(logMessageBuffer);
            }
        }
        break;
    }
    grid[posX][posY].visited = true;
    // If there are three walls surrounding the cell, mark it as dead end
    if (wallCount < 2)
    {
        divergingPathsQueue[last][0] = posX;
        divergingPathsQueue[last][1] = posY;
        last++;
        sprintf(logMessageBuffer, "***Found Diverging paths at %d, %d", posX, posY);
        logMessage(logMessageBuffer);
        killDeadEndPaths();
    }
    else if (wallCount == 3)
    {
        if(!(posX == 0 && posY == 0))
        {
            grid[posX][posY].isDeadEnd = true;
            sprintf(logMessageBuffer, "***Found Dead End at %d, %d", posX, posY);
            logMessage(logMessageBuffer);
        }
    }
}
// to mark all outer walls
void markOuterWalls()
{
    // Mark top and bottom rows
    for (int j = 0; j < COLS; j++)
    {
        grid[0][j].hasWestWall = true;        // Mark top row
        grid[ROWS - 1][j].hasEastWall = true; // Mark bottom row
    }

    // Mark left and right columns
    for (int i = 0; i < ROWS; i++)
    {
        grid[i][0].hasSouthWall = true;        // Mark left column
        grid[i][COLS - 1].hasNorthWall = true; // Mark right column
    }
}
// function to confirm if the stored points are valid or not.
void pickNextSpot()
{
    int checkX = 0;
    int checkY = 0;
    while (last > 0)
    {
        checkX = divergingPathsQueue[last][0];
        checkY = divergingPathsQueue[last][1];
        last--;
        bool hasUnvisitedAdjacent = false;

        // Iterate over adjacent cells
        for (int d = 0; d < 4; d++)
        {
            int newX = checkX + adjacentX[d];
            int newY = checkY + adjacentY[d];

            // Check if the adjacent cell is within bounds and unvisited
            if (newX >= 0 && newX < ROWS && newY >= 0 && newY < COLS && !grid[newX][newY].visited)
            {
                if (!(grid[checkX][checkY].hasNorthWall && d == NORTH) && // Check if there's a wall to the north
                    !(grid[checkX][checkY].hasEastWall && d == EAST) &&   // Check if there's a wall to the east
                    !(grid[checkX][checkY].hasSouthWall && d == SOUTH) && // Check if there's a wall to the south
                    !(grid[checkX][checkY].hasWestWall && d == WEST))     // Check if there's a wall to the west
                {
                    hasUnvisitedAdjacent = true;
                    break; // Found at least one unvisited adjacent cell, no need to continue checking
                }
            }
        }

        if ((checkX != END1 || checkY != END1) && // Not equal to (7, 7)
            (checkX != END1 || checkY != END2) && // Not equal to (7, 8)
            (checkX != END2 || checkY != END1) && // Not equal to (8, 7)
            (checkX != END2 || checkY != END2) && // Not equal to (8, 8)
            hasUnvisitedAdjacent)                 // At least one unvisited adjacent cell
        {
            nextSpot[0] = checkX;
            nextSpot[1] = checkY;
            sprintf(logMessageBuffer, "@@@@@@Next Spot to Visit = %d, %d", checkX, checkY);
            logMessage(logMessageBuffer);
            break; // Try moving to this position
        }
        else
        {
            logMessage("@@@@@@IN DEEP SHIT");
            for (int i = 0; i < ROWS; i++)
            {
                for (int j = 0; j < COLS; j++)
                {
                    if (!grid[i][j].visited)
                    {
                        nextSpot[0] = i;
                        nextSpot[1] = j;
                    }
                }
            }
        }
    }
}
// function to fill numbers in the grid
void floodFillNextSpot()
{
    // Initialize the queue
    int queue[ROWS * COLS * 2][2];
    int front = 0, rear = 0;
    // Set end position to 0 and add it to the queue
    grid[nextSpot[0]][nextSpot[1]].manhattanDistance = 0;
    queue[rear][0] = nextSpot[0];
    queue[rear][1] = nextSpot[1];
    rear++;

    // Iterate until the queue is empty
    while (front < rear)
    {
        // Dequeue the cell
        int x = queue[front][0];
        int y = queue[front][1];
        front++;

        // Explore adjacent cells
        for (int d = 0; d < 4; d++)
        {
            int newX = x + adjacentX[d];
            int newY = y + adjacentY[d];

            // Check if the adjacent cell is within bounds and unvisited
            if (newX >= 0 && newX < ROWS && newY >= 0 && newY < COLS && grid[newX][newY].manhattanDistance == -1)
            {
                // Check for walls in the current cell
                bool hasWall = false;
                switch (d)
                {
                case NORTH:
                    hasWall = grid[x][y].hasNorthWall;
                    break;
                case EAST:
                    hasWall = grid[x][y].hasEastWall;
                    break;
                case SOUTH:
                    hasWall = grid[x][y].hasSouthWall;
                    break;
                case WEST:
                    hasWall = grid[x][y].hasWestWall;
                    break;
                }
                // If there's no wall, enqueue the adjacent cell
                if (!hasWall)
                {
                    // Set the value of the adjacent cell and enqueue it
                    grid[newX][newY].manhattanDistance = grid[x][y].manhattanDistance + 1;
                    queue[rear][0] = newX;
                    queue[rear][1] = newY;
                    rear++;
                }
            }
        }
    }
    // Overwrite the Manhattan distance of dead-end cells with 250
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            if (grid[i][j].isDeadEnd)
            {
                grid[i][j].manhattanDistance = 250;
            }
        }
    }
}
// function to check accessible neighbours
void checkAccessibleNeighbours(int *manhattanNorth, int *manhattanEast, int *manhattanSouth, int *manhattanWest)
{
    // Check if the cell to the north is accessible
    if (!grid[posX][posY].hasNorthWall && posY + 1 < COLS)
    {
        *manhattanNorth = grid[posX][posY + 1].manhattanDistance;
    }
    // Check if the cell to the east is accessible
    if (!grid[posX][posY].hasEastWall && posX + 1 < ROWS)
    {
        *manhattanEast = grid[posX + 1][posY].manhattanDistance;
    }
    // Check if the cell to the south is accessible
    if (!grid[posX][posY].hasSouthWall && posY - 1 >= 0)
    {
        *manhattanSouth = grid[posX][posY - 1].manhattanDistance;
    }
    // Check if the cell to the west is accessible
    if (!grid[posX][posY].hasWestWall && posX - 1 >= 0)
    {
        *manhattanWest = grid[posX - 1][posY].manhattanDistance;
    }
}
// Function to check if there is at least one unvisited cell in the grid
bool hasUnvisitedCell()
{
    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            if (!grid[i][j].visited)
            {
                return true; // Found an unvisited cell
            }
        }
    }
    return false; // All cells have been visited
}

// TO PORT ROBOT MOVEMENT
//  Function to move the robot forward
void moveForward()
{
    if (API_moveForward())
    {
        switch (direction)
        {
        case NORTH:
            posY++;
            break;
        case EAST:
            posX++;
            break;
        case SOUTH:
            posY--;
            break;
        case WEST:
            posX--;
            break;
        }
    }
    else
    {
        logMessage("\n\nStuck in deep shit+");
    }
}
// Function to turn the robot left
void turnLeft()
{
    direction = (direction + 3) % 4; // Modulo 4 to wrap around
    API_turnLeft();
}
// Function to turn the robot right
void turnRight()
{
    direction = (direction + 1) % 4; // Modulo 4 to wrap around
    API_turnRight();
}
// Function to turn to the desired direction
void rotateToDesiredDirection(int desiredDirection)
{
    while (direction != desiredDirection)
    {
        sprintf(logMessageBuffer, "Rotating towards %d", desiredDirection);
        logMessage(logMessageBuffer);
        turnRight();
    }
}
