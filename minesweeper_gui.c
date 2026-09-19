#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ROWS 10
#define COLS 10
#define MINES 12
#define CELL_SIZE 40

// --- Game States for the Menu System ---
typedef enum GameScreen { MENU = 0, INPUT_NAME, GAMEPLAY, INSTRUCTIONS, HIGHSCORE } GameScreen;
GameScreen currentScreen = MENU;

// --- Structures ---
typedef struct {
    int isMine;       
    int isRevealed;   
    int adjacent;     
} Cell;

// Structure for sorting high scores
typedef struct {
    char name[50];
    int score;
} ScoreRecord;

// --- Global Variables ---
Cell grid[ROWS][COLS];
int gameOver = 0;
int gameWon = 0;
int currentScore = 0;

char playerName[50] = "\0";
int letterCount = 0;

// [AUDIO] New Audio Variables
Sound fxClick;
Sound fxBoom;
Sound fxWin;

// --- Function Prototypes ---
void InitGame();
int CountMines(int r, int c);
void RevealCell(int r, int c);
void SaveScore();
void DrawMenu();
void DrawInstructions();
void DrawHighScores();
void DrawGameplay();

int main() {
    InitWindow(COLS * CELL_SIZE, ROWS * CELL_SIZE + 60, "Minesweeper Pro: 10x10");
    
    // [AUDIO] 1. Initialize Audio Device and Load Sounds
    InitAudioDevice(); 
    fxClick = LoadSound("click.wav");
    fxBoom = LoadSound("boom.wav");
    fxWin = LoadSound("win.wav");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        
        // ==========================================
        // 1. UPDATE LOGIC (Decision Making)
        // ==========================================
        switch(currentScreen) {
            case MENU:
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mousePoint = GetMousePosition();
                    
                    if (CheckCollisionPointRec(mousePoint, (Rectangle){100, 100, 200, 40})) {
                        PlaySound(fxClick); // [AUDIO] Menu click
                        currentScreen = INPUT_NAME;
                    }
                    if (CheckCollisionPointRec(mousePoint, (Rectangle){100, 160, 200, 40})) {
                        PlaySound(fxClick); // [AUDIO] Menu click
                        currentScreen = INSTRUCTIONS;
                    }
                    if (CheckCollisionPointRec(mousePoint, (Rectangle){100, 220, 200, 40})) {
                        PlaySound(fxClick); // [AUDIO] Menu click
                        currentScreen = HIGHSCORE;
                    }
                    if (CheckCollisionPointRec(mousePoint, (Rectangle){100, 280, 200, 40})) {
                        PlaySound(fxClick); // [AUDIO] Menu click
                        break; // Exit Game sequence will trigger below
                    }
                }
                break;

            case INPUT_NAME:
                // Keyboard Input Logic for Player Name
                {
                    int key = GetCharPressed();
                    while (key > 0) {
                        if ((key >= 32) && (key <= 125) && (letterCount < 49)) {
                            playerName[letterCount] = (char)key;
                            playerName[letterCount+1] = '\0';
                            letterCount++;
                        }
                        key = GetCharPressed();
                    }

                    if (IsKeyPressed(KEY_BACKSPACE)) {
                        letterCount--;
                        if (letterCount < 0) letterCount = 0;
                        playerName[letterCount] = '\0';
                    }

                    if (IsKeyPressed(KEY_ENTER) && letterCount > 0) {
                        PlaySound(fxClick); // [AUDIO] Enter game click
                        InitGame();
                        currentScreen = GAMEPLAY;
                    }
                }
                break;

            case GAMEPLAY:
                if (!gameOver && !gameWon) {
                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        int col = GetMouseX() / CELL_SIZE;
                        int row = (GetMouseY() - 60) / CELL_SIZE; 

                        if (row >= 0 && row < ROWS && col >= 0 && col < COLS) {
                            if (grid[row][col].isMine) {
                                gameOver = 1; 
                                PlaySound(fxBoom); // [AUDIO] Mine Explosion
                                grid[row][col].isRevealed = 1;
                                SaveScore(); // Save score on loss
                            } else {
                                PlaySound(fxClick); // [AUDIO] Safe Tile Click
                                RevealCell(row, col);
                                
                                // [AUDIO] Check if that reveal triggered a win
                                if (gameWon) {
                                    PlaySound(fxWin);
                                }
                            }
                        }
                    }
                } else {
                    if (IsKeyPressed(KEY_R)) {
                        PlaySound(fxClick); // [AUDIO] Reset game click
                        currentScreen = MENU; 
                    }
                }
                break;

            case INSTRUCTIONS:
            case HIGHSCORE:
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mousePoint = GetMousePosition();
                    if (CheckCollisionPointRec(mousePoint, (Rectangle){100, 400, 200, 40})) {
                        PlaySound(fxClick); // [AUDIO] Back button click
                        currentScreen = MENU;
                    }
                }
                break;
        }

        // ==========================================
        // 2. RENDERING (Drawing the GUI)
        // ==========================================
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch(currentScreen) {
            case MENU:
                DrawMenu();
                break;
            case INPUT_NAME:
                DrawText("ENTER YOUR NAME:", 100, 150, 20, DARKGRAY);
                DrawRectangle(100, 180, 200, 40, LIGHTGRAY);
                DrawRectangleLines(100, 180, 200, 40, DARKGRAY);
                DrawText(playerName, 110, 190, 20, MAROON);
                DrawText("Press ENTER to Start", 100, 250, 15, GRAY);
                break;
            case GAMEPLAY:
                DrawGameplay();
                break;
            case INSTRUCTIONS:
                DrawInstructions();
                break;
            case HIGHSCORE:
                DrawHighScores();
                break;
        }

        EndDrawing();
    }

    // [AUDIO] 3. Unload Audio and Close Device
    UnloadSound(fxClick);
    UnloadSound(fxBoom);
    UnloadSound(fxWin);
    CloseAudioDevice(); 

    CloseWindow();
    return 0;
}

// --- Gameplay & Logic Functions ---

void InitGame() {
    gameOver = 0;
    gameWon = 0;
    currentScore = 0;
    int placedMines = 0;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            grid[r][c].isMine = 0;
            grid[r][c].isRevealed = 0;
            grid[r][c].adjacent = 0;
        }
    }

    srand(time(NULL));
    while (placedMines < MINES) {
        int r = rand() % ROWS;
        int c = rand() % COLS;
        if (grid[r][c].isMine == 0) {
            grid[r][c].isMine = 1;
            placedMines++;
        }
    }

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (!grid[r][c].isMine) {
                grid[r][c].adjacent = CountMines(r, c);
            }
        }
    }
}

int CountMines(int r, int c) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            int newRow = r + i;
            int newCol = c + j;
            if (newRow >= 0 && newRow < ROWS && newCol >= 0 && newCol < COLS) {
                if (grid[newRow][newCol].isMine) count++;
            }
        }
    }
    return count;
}

void RevealCell(int r, int c) {
    if (r < 0 || r >= ROWS || c < 0 || c >= COLS) return;
    if (grid[r][c].isRevealed) return;

    grid[r][c].isRevealed = 1;
    currentScore += 10; // 10 points per safe tile

    int unrevealedSafe = 0;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            if (!grid[i][j].isMine && !grid[i][j].isRevealed) unrevealedSafe++;
        }
    }
    if (unrevealedSafe == 0) {
        gameWon = 1;
        SaveScore(); // Save score on win
    }

    if (grid[r][c].adjacent == 0 && !grid[r][c].isMine) {
        for (int i = -1; i <= 1; i++) {
            for (int j = -1; j <= 1; j++) {
                RevealCell(r + i, c + j);
            }
        }
    }
}

// --- File Handling Functions ---

void SaveScore() {
    FILE *file = fopen("highscore.txt", "a");
    if (file != NULL) {
        fprintf(file, "%s %d\n", playerName, currentScore);
        fclose(file);
    }
}

void DrawHighScores() {
    DrawText("--- HIGH SCORES ---", 100, 30, 20, DARKGRAY);
    
    FILE *file = fopen("highscore.txt", "r");
    ScoreRecord records[100]; // Array to hold up to 100 scores
    int count = 0;
    
    if (file != NULL) {
        // 1. Read all scores from the file into the array
        while (fscanf(file, "%s %d", records[count].name, &records[count].score) != EOF) {
            count++;
            if (count >= 100) break; // Prevent array overflow
        }
        fclose(file);
        
        // 2. Bubble Sort algorithm (Descending order: Highest to Lowest)
        for (int i = 0; i < count - 1; i++) {
            for (int j = 0; j < count - i - 1; j++) {
                if (records[j].score < records[j + 1].score) { 
                    ScoreRecord temp = records[j];
                    records[j] = records[j + 1];
                    records[j + 1] = temp;
                }
            }
        }
        
        // 3. Display the sorted scores
        int yPos = 80;
        for (int i = 0; i < count; i++) {
            DrawText(TextFormat("%d. %s | Score: %d", i + 1, records[i].name, records[i].score), 50, yPos, 20, BLACK);
            yPos += 30;
            if (yPos > 350) break; // Limit to the top 10 or so that fit on screen
        }
    } else {
        DrawText("No high scores yet!", 100, 100, 20, RED);
    }

    DrawRectangle(100, 400, 200, 40, LIGHTGRAY);
    DrawText("BACK TO MENU", 135, 410, 20, BLACK);
}

// --- Visual GUI Functions ---

void DrawMenu() {
    DrawText("MAIN MENU", 130, 40, 30, DARKBLUE);
    
    DrawRectangle(100, 100, 200, 40, LIGHTGRAY);
    DrawText("1. Start Game", 120, 110, 20, BLACK);

    DrawRectangle(100, 160, 200, 40, LIGHTGRAY);
    DrawText("2. Instructions", 120, 170, 20, BLACK);

    DrawRectangle(100, 220, 200, 40, LIGHTGRAY);
    DrawText("3. High Score", 120, 230, 20, BLACK);

    DrawRectangle(100, 280, 200, 40, RED);
    DrawText("4. Exit", 120, 290, 20, WHITE);
}

void DrawInstructions() {
    DrawText("INSTRUCTIONS", 100, 50, 30, DARKBLUE);
    DrawText("- Click a tile to reveal it.", 20, 120, 15, BLACK);
    DrawText("- Numbers show adjacent mines.", 20, 150, 15, BLACK);
    DrawText("- Earn 10 points for every safe tile.", 20, 180, 15, BLACK);
    DrawText("- Avoid the red mines (*).", 20, 210, 15, BLACK);
    
    DrawRectangle(100, 400, 200, 40, LIGHTGRAY);
    DrawText("BACK TO MENU", 135, 410, 20, BLACK);
}

void DrawGameplay() {
    DrawText(TextFormat("Player: %s | Score: %d", playerName, currentScore), 10, 10, 20, DARKGRAY);
    
    if (gameOver) DrawText("BOOM! Press 'R' for Menu", 10, 35, 20, RED);
    else if (gameWon) DrawText("YOU WIN! Press 'R' for Menu", 10, 35, 20, GREEN);
    else DrawText("Click to reveal tiles", 10, 35, 20, GRAY);

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int x = c * CELL_SIZE;
            int y = r * CELL_SIZE + 60; 

            if (grid[r][c].isRevealed) {
                DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, LIGHTGRAY);
                DrawRectangleLines(x, y, CELL_SIZE, CELL_SIZE, GRAY);
                
                if (grid[r][c].isMine) {
                    DrawCircle(x + CELL_SIZE/2, y + CELL_SIZE/2, 10, RED);
                } else if (grid[r][c].adjacent > 0) {
                    DrawText(TextFormat("%d", grid[r][c].adjacent), x + 14, y + 10, 20, DARKBLUE);
                }
            } else {
                DrawRectangle(x, y, CELL_SIZE, CELL_SIZE, GRAY);
                DrawRectangleLines(x, y, CELL_SIZE, CELL_SIZE, DARKGRAY);
            }
        }
    }
}