/*
Ideas:
Portals - Space that transports to a new maze containing items/keys
Infinite mode - allows user to complete maze after maze to compete for a high score
Leaderboard - API call to home server to get the top 10 scores and display them

To Do:
Solving algorithm - make sure every maze is completeable
Time limit - Score based on time limit
Story mode - standard 5 maze gameplay
Items & enemies

Notes:
Hunt-Kill algorithm

*/
#include <iostream>
#include <string>
#include <random>
#include <omp.h> //Timer
#include <conio.h> //_getch()

void movement(char direction);
void menu();
void shuffle();
void initArray();
void freeAll();
bool isWall(int x, int y);
void generate();
void buildMaze();
void outputMaze();
bool winCondition();
void playStory();
void playSpeedrun();
void playStandard();
void generateTiles();

// Defines the mapSize of the maze & produces array
const int mapSize[] = { 13, 17, 21, 25, 27 };
const int arraySize = sizeof(mapSize) / sizeof(mapSize[0]);
int currentLevel;
char*** maze = NULL;
int playerLoc[2];
int endLoc[2];
double startTime;
double levelTime;
double endTime;
int keyCount;
int keyCollected;
int keyLoc[3][2];
int lockLoc[2];

// Directions to be used by the generation algorithm
int directions[4] = { 0,1,2,3 };
const int NSEW[4][2] = {
    { 0, -1 },  // North
    { 0, 1 },   // South
    { 1, 0 },   // East
    { -1, 0 }   // West
};

// Checks if the coordinate is within the bounds and is a wall
bool isWall(int x, int y) {
    if (x >= 0 && y >= 0 && x <= mapSize[currentLevel] - 1 && y <= mapSize[currentLevel] - 1) {
        if (strcmp(maze[y][x], "[ ]") == 0 || strcmp(maze[y][x], "[L]") == 0) {
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}

// Randomizes the order of the directions array
void shuffle() {
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(std::begin(directions), std::end(directions), rng);
}

// Start of the hunt & Kill algorithm
//http://weblog.jamisbuck.org/2011/1/24/maze-generation-hunt-and-kill-algorithm
void generate() {
    // Selects a random starting point for the Hunt & Kill algorithm
    int x = 1 + 2 * (std::rand() % (mapSize[currentLevel] / 2));
    int y = 1 + 2 * (std::rand() % (mapSize[currentLevel] / 2));
    strcpy_s(maze[y][x], 4, "   ");

    while (true) {
        bool huntPhase = false;
        bool killPhase = false;

        // Kill phase
        while (true) { 
            bool moved = false;
            shuffle();

            for (int i = 0; i < 4; i++) {
                int newX = x + 2 * NSEW[directions[i]][0];
                int newY = y + 2 * NSEW[directions[i]][1];

                if (isWall(newX, newY) == true) {
                    strcpy_s(maze[y + NSEW[directions[i]][1]][x + NSEW[directions[i]][0]], 4, "   ");
                    strcpy_s(maze[newY][newX], 4, "   ");
                    x = newX;
                    y = newY;
                    moved = true;
                    killPhase = true;
                    break;
                }
            }
            if (moved == false) {
                break;

            }
        }

        // Hunt phase
        for (int i = 1; i < mapSize[currentLevel] - 1; i += 2) {
            for (int j = 1; j < mapSize[currentLevel] - 1; j += 2) {
                if (strcmp(maze[i][j], "[ ]") == 0) {
                    for (int k = 0; k < 4; k++) {
                        int newX = j + 2 * NSEW[k][0];
                        int newY = i + 2 * NSEW[k][1];
                        if (newX > 0 && newY > 0 && newX < mapSize[currentLevel] && newY < mapSize[currentLevel]) {
                            if (strcmp(maze[newY][newX], "   ") == 0) {
                                strcpy_s(maze[i + NSEW[k][1]][j + NSEW[k][0]], 4, "   ");
                                strcpy_s(maze[i][j], 4, "   ");
                                x = j;
                                y = i;
                                huntPhase = true;
                                break;
                            }
                        }
                    }
                }
                if (huntPhase == true) {
                    break;
                }
            }
            if (huntPhase == true) {
                break;
            }

        }
        if (killPhase == false && huntPhase == false) {
            break;
        }

    }
}

// Outputs the maze
void outputMaze() {
    std::string mazeOutput;

    int elapsed = static_cast<int>(omp_get_wtime() - levelTime);
    printf("\nLevel - %d | Timer: %d:%02d\n",currentLevel+1, elapsed/60, elapsed%60);

    for (int i = 0; i < mapSize[currentLevel]; i++) {
        for (int j = 0; j < mapSize[currentLevel]; j++) {
            mazeOutput += maze[i][j];
        }
        mazeOutput += "\n";
    }
    std::cout << mazeOutput;
}

// Main function to start construction of the maze
void buildMaze() {
    //Sets all spaces to walls
    for (int i = 0; i < mapSize[currentLevel]; i++) {
        for (int j = 0; j < mapSize[currentLevel]; j++) {
            strcpy_s(maze[i][j], 4, "[ ]");
        }
    }
    generate(); //Runs the Hunt/Kill algorithm
    strcpy_s(maze[1][1], 4, " P ");
    playerLoc[0] = 1;
    playerLoc[1] = 1;
    generateTiles();

}

void movement(char direction) {
    int newX = playerLoc[0], newY = playerLoc[1];

    switch (std::toupper(direction)) {
    case 'W':
        newY -= 1;
        break;
    case 'S':
        newY += 1;
        break;
    case 'A':
        newX -= 1;
        break;
    case 'D':
        newX += 1;
        break;
    default:
        return;
    };

    if (isWall(newX, newY) == false) {
        if (strcmp(maze[newY][newX], " K ") == 0) {
            keyCollected += 1;
            printf("\n\nYou collected a key! (%d/%d)\n", keyCollected, keyCount);

            if (keyCollected == keyCount) {
                printf("You have collected all the keys, the door is now open\n");
                strcpy_s(maze[lockLoc[1]][lockLoc[0]], 4, "   ");
            }
            system("pause");
        }
        strcpy_s(maze[playerLoc[1]][playerLoc[0]], 4, "   ");
        playerLoc[0] = newX;
        playerLoc[1] = newY;
        strcpy_s(maze[playerLoc[1]][playerLoc[0]], 4, " P ");
        outputMaze();
    }
    else {
        printf("\nPlease enter a valid move\n");
    }
}

void menu() {
    while (true) {
        char input;
        printf("1 - Story Mode \n2 - Quick Play \n3 - Speedrun \nOther - Exit \n\n> ");
        std::cin >> input;

        switch (input) {
        default:
            char input;
            printf("\nAre you sure you want to exit? (Y/N) > ");
            std::cin >> input;
            if (std::toupper(input) == 'Y') {
                return;
            }
            else {
                break;
            }

        case '1':
            playStory();
            break;

        case '2':
            playStandard();
            break;

        case '3':
            playSpeedrun();
            break;
        }
    }
}
int main() {
    menu();
}

void initArray() {
    maze = (char***)malloc(mapSize[currentLevel] * sizeof(char**));

    if (maze != NULL) {
        for (int i = 0; i < mapSize[currentLevel]; i++) {
            maze[i] = (char**)malloc(mapSize[currentLevel] * sizeof(char*));

            if (maze[i] != NULL) {
                for (int j = 0; j < mapSize[currentLevel]; j++) {
                    maze[i][j] = (char*)malloc(4 * sizeof(char));

                    if (maze[i][j] == NULL) {
                        printf("Error when allocating memory for maze[i][j]");
                        exit(-1);
                    }
                }
            }
            else {
                printf("Error when allocating memory for maze[i]");
                exit(-1);
            }
        }
    }
    else {
        printf("Error when allocating memory for maze");
        exit(-1);
    }
}

void freeAll() {
    for (int i = 0; i < mapSize[currentLevel]; i++) {
        for (int j = 0; j < mapSize[currentLevel]; j++) {
            free(maze[i][j]);
        }
        free(maze[i]);
    }
    free(maze);
}

void playStory() {
    startTime = omp_get_wtime(); //gets the starting time of the player
    for (int i = 0; i < (sizeof(mapSize)/sizeof(mapSize[0])); i++) {
        currentLevel = i;
        initArray();
        buildMaze();
        outputMaze();

        levelTime = omp_get_wtime();
        while (true) {//movement stuff
            printf("WASD to move > ");
            char WASD = _getch();
            movement(WASD);

            if (winCondition() == true) {
                printf("\n\nMoving to the next level\n\n");
                break;
            }
        }
        freeAll();
    }
    endTime = omp_get_wtime();
    int takenTime = static_cast<int>(endTime - startTime);
    printf("\nYou finished the story mode in %d:%02d!\n", takenTime/60, takenTime%60);
    system("pause");

}

void playStandard() {
    int input;
    printf("\nWhich level would you like to play? (1 - %d) > ", arraySize);
    std::cin >> input;

    // if input is not a number 
    // https://en.cppreference.com/w/cpp/io/basic_istream/ignore
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(10, '\n');
        return;
    }

    if (0 < input && input <= arraySize) {
        currentLevel = (input - 1);
        levelTime = omp_get_wtime();
        initArray();
        buildMaze();
        outputMaze();
        while (true) {//movement stuff
            printf("WASD to move > ");
            char WASD = _getch();
            movement(WASD);

            if (winCondition() == true) {
                endTime = omp_get_wtime();
                int takenTime = static_cast<int>(endTime - levelTime);
                printf("\nYou finished the level in %d:%02d!\n", takenTime / 60, takenTime % 60);
                system("pause");
                break;
            }
        }
        freeAll();
    }
    else {
        printf("\nPlease enter a valid level\n");
    }
}

void playSpeedrun() {
    int input;
    int completions = 0;
    printf("\nWhich level would you like to run? (1 - %d) > ", arraySize);
    std::cin >> input;

    // if input is not a number 
    // https://en.cppreference.com/w/cpp/io/basic_istream/ignore
    if (std::cin.fail()) {
        std::cin.clear();
        std::cin.ignore(10, '\n');
        printf("\nPlease enter a valid level\n");
        return;
    }

    if (1 <= input && input <= (arraySize)) {
        currentLevel = (input - 1);
        printf("You have 2 minutes to complete as many runs as possible \nAre you ready?\n");
        system("pause");
        startTime = omp_get_wtime();
        while (omp_get_wtime() < startTime + 120) {
            initArray();
            buildMaze();
            levelTime = omp_get_wtime();
            outputMaze();
            while (true) {
                printf("WASD to move > ");
                char WASD = _getch();
                movement(WASD);

                if (winCondition() == true) {
                    endTime = omp_get_wtime();
                    int takenTime = static_cast<int>(endTime - levelTime);
                    printf("\nYou finished the level in %d:%02d!\n", takenTime / 60, takenTime % 60);
                    printf("\n\nMoving to the next level\n\n");
                    completions += 1;
                    break;
                }

                if (omp_get_wtime() > startTime + 120) {
                    printf("\n\nTimes up\n");
                    system("pause");
                    break;
                }
            }
            freeAll();
        }
        freeAll();
        printf("You managed to complete %d runs in the time limit, good job!\n", completions);
        system("pause");
    }
    else {
        printf("Please enter a valid level");
    }
}

bool winCondition() {
    if (playerLoc[0] == endLoc[0] && playerLoc[1] == endLoc[1]) {
        return true;
    }
    else {
        return false;
    }
}

void generateTiles() {
    int exitY;
    int exitX;
    keyCount = currentLevel + 1;

    // Checking maze for all dead ends
    int deadLoc[50][2];
    int deadCount = 0;

    for (int y = 0; y < mapSize[currentLevel]; y++) {
        for (int x = 0; x < mapSize[currentLevel]; x++) {
            if (strcmp(maze[y][x], "   ") == 0) {
                int count = 0;

                for (int i = 0; i < 4; i++) {
                    int newX = x + NSEW[i][0];
                    int newY = y + NSEW[i][1];

                    if (strcmp(maze[newY][newX], "   ") == 0) {
                        count += 1;
                        int lockY = newY;
                        int lockX = newX;

                    }
                }

                if (count == 1) {
                    deadLoc[deadCount][0] = y;
                    deadLoc[deadCount][1] = x;
                    deadCount += 1;
                }
            }
        }
    }

    // Generating the exit and the lock
    int exitIndex;

    while (true) {
    exitIndex = std::rand() % deadCount;

        exitY = deadLoc[exitIndex][0];
        exitX = deadLoc[exitIndex][1];

        if (exitY != 1 && exitX != 1) {
            endLoc[1] = exitY;
            endLoc[0] = exitX;
            
            strcpy_s(maze[endLoc[1]][endLoc[0]], 4, " X ");

            if (keyCount != 0) {
                for (int i = 0; i < 4; i++) {
                    int newX = exitX + NSEW[i][0];
                    int newY = exitY + NSEW[i][1];

                    if (strcmp(maze[newY][newX], "   ") == 0) {
                        lockLoc[0] = newX;
                        lockLoc[1] = newY;

                        strcpy_s(maze[lockLoc[1]][lockLoc[0]], 4, "[L]");
                        break;
                    }
                }
            } 
            break;
        }
    }

    //Generate keys throughout the maze
    int placedKeys = 0;
    keyCollected = 0;

    while (placedKeys < keyCount) {
        int keyIndex = std::rand() % deadCount;

        int keyY = deadLoc[keyIndex][0];
        int keyX = deadLoc[keyIndex][1];

        if ((keyY != 1 && keyX != 1) && (keyY != exitY && keyX != exitX) && strcmp(maze[keyY][keyX], " K ") != 0) {
            strcpy_s(maze[keyY][keyX], 4, " K ");
            keyLoc[placedKeys][0] = keyY;
            keyLoc[placedKeys][1] = keyX;

            for (int i = keyIndex; i < deadCount - 1; i++) {
                deadLoc[i][0] = deadLoc[i + 1][0];
                deadLoc[i][1] = deadLoc[i + 1][1];
            }
            deadCount -= 1;
            placedKeys += 1;
        }
    }
}