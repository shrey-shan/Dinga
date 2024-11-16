#include "config.h"
// to calc with finish as 0
void calcMazeValues()
{
    logMessage("Initialize Grid Manhattan Distance Values...");
    initializeGridValues();
    logMessage("Fill Numbers...");
    floodFillNumbers();
}
// to calc with any position as 0
void calcNextSpotValues()
{
    logMessage("Initialize Grid Manhattan Distance Values...");
    initializeGridValues();
    logMessage("Fill Numbers...");
    floodFillNextSpot();
}
// to calc with any position as 0
void calcResetValues()
{
    logMessage("Initialize Grid Manhattan Distance Values...");
    initializeGridValues();
    logMessage("Fill Numbers...");
    floodFillStart();
}
// to draw calculated values
void drawMazeValues()
{
    logMessage("Draw in UI...");
    drawGrid();
    logMessage("Overwrite end and start...");
    drawZones();
}
// to go towards green
void moveToEnd()
{
    while (grid[posX][posY].manhattanDistance != 0) // As we have to move to the end of maze
    {
        logMessage("\n\nFinding a Solution...");
        calcMazeValues();
        sprintf(logMessageBuffer, "Current Position... (%d, %d)", posX, posY);
        logMessage(logMessageBuffer);

        // Initialize variables to store the Manhattan distance of adjacent cells
        int manhattanNorth = 100;
        int manhattanEast = 100;
        int manhattanSouth = 100;
        int manhattanWest = 100;
        // update the values of accessible adjacent cells
        checkAccessibleNeighbours(&manhattanNorth, &manhattanEast, &manhattanSouth, &manhattanWest);

        // Choose the direction with the minimum Manhattan distance
        int count = 0;
        int minManhattan = minOrMax(manhattanNorth, manhattanEast, manhattanSouth, manhattanWest, &count, false);//false for min
        sprintf(logMessageBuffer, "*************minManhattan from %d,%d in %d direction is %d", posX, posY, direction, minManhattan);
        logMessage(logMessageBuffer);
        sprintf(logMessageBuffer, "*************Manhattan values were %d, %d, %d, %d repeated %d times", manhattanNorth, manhattanEast, manhattanSouth, manhattanWest, count);
        logMessage(logMessageBuffer);
        (void)count;
        // Rotate towards the direction with the minimum distance
        if (minManhattan == manhattanWest)
        {
            rotateToDesiredDirection(WEST);
        }
        else if (minManhattan == manhattanSouth)
        {
            rotateToDesiredDirection(SOUTH);
        }
        else if (minManhattan == manhattanEast)
        {
            rotateToDesiredDirection(EAST);
        }
        else if (minManhattan == manhattanNorth)
        {
            rotateToDesiredDirection(NORTH);
        }

        // Move forward
        moveForward();
        sprintf(logMessageBuffer, "***Moving towards %d", direction);
        logMessage(logMessageBuffer);
        logMessage("MarkingWalls");
        checkAndMarkWalls();
        logMessage("Filling numbers");
        drawMazeValues();
    }
}
// to explore green
void exploreFinish()
{
    moveForward();
    checkAndMarkWalls();
    calcMazeValues();
    drawMazeValues();
    if (!API_wallRight())
    {
        for (int s = 0; s < 3; s++)
        {
            turnRight();
            moveForward();
            checkAndMarkWalls();
            calcMazeValues();
            drawMazeValues();
        }
        turnLeft();
    }
    else
    {
        for (int s = 0; s < 3; s++)
        {
            turnLeft();
            moveForward();
            checkAndMarkWalls();
            calcMazeValues();
            drawMazeValues();
        }
        turnRight();
    }
    grid[7][7].visited = true;
    grid[7][8].visited = true;
    grid[8][8].visited = true;
    grid[8][7].visited = true;
    killDeadEndPaths();
    for (int n = 0; n < last; n++)
    {
        sprintf(logMessageBuffer, "Diverged paths = %d, %d", divergingPathsQueue[n][0], divergingPathsQueue[n][1]);
        logMessage(logMessageBuffer);
    }
    pickNextSpot();
}
// EXPLORE MORE
void exploreMore()
{
    // Initialize variables to store the Manhattan distance of adjacent cells
    int manhattanNorth = 100;
    int manhattanEast = 100;
    int manhattanSouth = 100;
    int manhattanWest = 100;
    calcNextSpotValues();
    drawMazeValues();
    if(posX == 7 && posY == 7)// this needs to work in all mazes :-(
    {
        moveForward(); // to make sure we are not standing on 0 and getting stuck in a while loop
    }
    while (grid[posX][posY].manhattanDistance != 0) // As we have to move to the next spot
    {
        if(grid[posX][posY].manhattanDistance == 1 && !hasUnvisitedCell())
        {//in rare cases the maze can also have an unvisited spot with distance 1. :-(
            break; //example 2 in mms
        }
        manhattanNorth = 100;
        manhattanEast = 100;
        manhattanSouth = 100;
        manhattanWest = 100;
        logMessage("\n\nMoved to the Next Spot");
        sprintf(logMessageBuffer, "Current Position... (%d, %d)", posX, posY);
        logMessage(logMessageBuffer);
        calcNextSpotValues();

        checkAccessibleNeighbours(&manhattanNorth, &manhattanEast, &manhattanSouth, &manhattanWest);

        // Choose the direction with the minimum Manhattan distance
        int count = 0;
        int minManhattan = minOrMax(manhattanNorth, manhattanEast, manhattanSouth, manhattanWest, &count, false);//false for min
        sprintf(logMessageBuffer, "*************minManhattan from %d,%d in %d direction is %d", posX, posY, direction, minManhattan);
        logMessage(logMessageBuffer);
        sprintf(logMessageBuffer, "*************Manhattan values were %d, %d, %d, %d repeated %d times", manhattanNorth, manhattanEast, manhattanSouth, manhattanWest, count);
        logMessage(logMessageBuffer);
        (void)count;
        // Rotate towards the direction with the minimum distance
        if (minManhattan == manhattanWest)
        {
            rotateToDesiredDirection(WEST);
        }
        else if (minManhattan == manhattanSouth)
        {
            rotateToDesiredDirection(SOUTH);
        }
        else if (minManhattan == manhattanEast)
        {
            rotateToDesiredDirection(EAST);
        }
        else if (minManhattan == manhattanNorth)
        {
            rotateToDesiredDirection(NORTH);
        }
        // Move forward
        moveForward();
        sprintf(logMessageBuffer, "***Moving towards %d", direction);
        logMessage(logMessageBuffer);
        logMessage("MarkingWalls");
        checkAndMarkWalls();
        logMessage("Filling numbers");
        drawMazeValues();
    }
}
// to go back to start
void moveInUnexplored()
{
    bool moveAgain = true;
    while(moveAgain)
    {
        moveAgain = false;
        logMessage("\n\nMoving in unexplored path");
        sprintf(logMessageBuffer, "Current Position... (%d, %d)", posX, posY);
        logMessage(logMessageBuffer);
        calcMazeValues();
        // Initialize variables to store the Manhattan distance of adjacent cells
        int manhattanNorth = 0;
        int manhattanEast = 0;
        int manhattanSouth = 0;
        int manhattanWest = 0;
        checkAccessibleNeighbours(&manhattanNorth, &manhattanEast, &manhattanSouth, &manhattanWest);
        // Check if the cell to the north is unvisited
        if (!grid[posX][posY].hasNorthWall && posY + 1 < COLS)
        {
            if (manhattanNorth > 245)
            {
                manhattanNorth = 0;
            }
            if (grid[posX][posY + 1].visited == false)
            {
                manhattanNorth = manhattanNorth + 10;
                moveAgain = true;
            }
        }
        // Check if the cell to the east is unvisited
        if (!grid[posX][posY].hasEastWall && posX + 1 < ROWS)
        {
            if (manhattanEast > 245)
            {
                manhattanEast = 0;
            }
            if (grid[posX + 1][posY].visited == false)
            {
                manhattanEast = manhattanEast + 10;
                moveAgain = true;
            }
        }
        // Check if the cell to the south is unvisited
        if (!grid[posX][posY].hasSouthWall && posY - 1 >= 0)
        {
            if (manhattanSouth > 245)
            {
                manhattanSouth = 0;
            }
            if (grid[posX][posY - 1].visited == false)
            {
                manhattanSouth = manhattanSouth + 10;
                moveAgain = true;
            }
        }
        // Check if the cell to the west is unvisited
        if (!grid[posX][posY].hasWestWall && posX - 1 >= 0)
        {
            if (manhattanWest > 245)
            {
                manhattanWest = 0;
            }
            if (grid[posX - 1][posY].visited == false)
            {
                manhattanWest = manhattanWest + 10;
                moveAgain = true;
            }
        }
        // Choose the direction with the maximum Manhattan distance
        int count = 0;
        int maxManhattan = minOrMax(manhattanNorth, manhattanEast, manhattanSouth, manhattanWest, &count, true);//true for max
        sprintf(logMessageBuffer, "*************maxManhattan from %d,%d in %d direction is %d", posX, posY, direction, maxManhattan);
        logMessage(logMessageBuffer);
        sprintf(logMessageBuffer, "*************Manhattan values were %d, %d, %d, %d repeated %d times", manhattanNorth, manhattanEast, manhattanSouth, manhattanWest, count);
        logMessage(logMessageBuffer);
        (void)count;
        // Rotate towards the direction with the minimum distance
        if (maxManhattan == manhattanWest)
        {
            rotateToDesiredDirection(WEST);
        }
        else if (maxManhattan == manhattanSouth)
        {
            rotateToDesiredDirection(SOUTH);
        }
        else if (maxManhattan == manhattanEast)
        {
            rotateToDesiredDirection(EAST);
        }
        else if (maxManhattan == manhattanNorth)
        {
            rotateToDesiredDirection(NORTH);
        }

        // Move forward
        moveForward();
        sprintf(logMessageBuffer, "***Moving towards %d", direction);
        logMessage(logMessageBuffer);
        logMessage(logMessageBuffer);
        logMessage("MarkingWalls");
        checkAndMarkWalls();
        logMessage("Filling numbers");
        drawMazeValues();
    }
    for (int n = 0; n < last; n++)
    {
        sprintf(logMessageBuffer, "Diverged paths = %d, %d", divergingPathsQueue[n][0], divergingPathsQueue[n][1]);
        logMessage(logMessageBuffer);
    }
    pickNextSpot();
}
// get ready for speed run
void moveToStart()
{
    while (!(posX == 0 && posY == 0)) // As we have to move to the end of maze
    {
        logMessage("\n\nGoing to the start for a speed run");
        calcResetValues();
        sprintf(logMessageBuffer, "Current Position... (%d, %d)", posX, posY);
        logMessage(logMessageBuffer);

        // Initialize variables to store the Manhattan distance of adjacent cells
        int manhattanNorth = 100;
        int manhattanEast = 100;
        int manhattanSouth = 100;
        int manhattanWest = 100;
        // update the values of accessible adjacent cells
        checkAccessibleNeighbours(&manhattanNorth, &manhattanEast, &manhattanSouth, &manhattanWest);

        // Choose the direction with the minimum Manhattan distance
        int count = 0;
        int minManhattan = minOrMax(manhattanNorth, manhattanEast, manhattanSouth, manhattanWest, &count, false);//false for min
        sprintf(logMessageBuffer, "*************minManhattan from %d,%d in %d direction is %d", posX, posY, direction, minManhattan);
        logMessage(logMessageBuffer);
        sprintf(logMessageBuffer, "*************Manhattan values were %d, %d, %d, %d repeated %d times", manhattanNorth, manhattanEast, manhattanSouth, manhattanWest, count);
        logMessage(logMessageBuffer);
        (void)count;
        // Rotate towards the direction with the minimum distance
        if (minManhattan == manhattanWest)
        {
            rotateToDesiredDirection(WEST);
        }
        else if (minManhattan == manhattanSouth)
        {
            rotateToDesiredDirection(SOUTH);
        }
        else if (minManhattan == manhattanEast)
        {
            rotateToDesiredDirection(EAST);
        }
        else if (minManhattan == manhattanNorth)
        {
            rotateToDesiredDirection(NORTH);
        }

        // Move forward
        moveForward();
        sprintf(logMessageBuffer, "***Moving towards %d", direction);
        logMessage(logMessageBuffer);
        logMessage("MarkingWalls");
        checkAndMarkWalls();
        logMessage("Filling numbers");
        drawMazeValues();
    }
}


// DINGA!!!!!!!!!
int main(int argc, char *argv[])
{
    logMessage("Cranking");
    initializeGrid();
    drawGrid();
    markOuterWalls();
    checkAndMarkWalls();
    logMessage("Launch Control");
    moveToEnd();
    exploreFinish();
    /*
    while(hasUnvisitedCell())
    {
        exploreMore();
        moveInUnexplored();
    }
    */
    logMessage("ALL KNOWING!!!");
    moveToStart();
    logMessage("SpeedRun");
}
