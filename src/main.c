#include <raylib.h>
#include <stdbool.h>
#include <string.h>

#define WIDTH 1600
#define HEIGHT 900
#define CELL_LEN 50
#define ROW_COUNT (HEIGHT / CELL_LEN)
#define COLUMN_COUNT (WIDTH / CELL_LEN)
#define CELL_COUNT (COLUMN_COUNT * ROW_COUNT)
#define OFFSET (CELL_LEN / 2.0f)

int main(int argc, char *argv[]) {
  bool grid[ROW_COUNT][COLUMN_COUNT] = {0};

  SetTraceLogLevel(LOG_WARNING);
  InitWindow(WIDTH + OFFSET * 2, HEIGHT + OFFSET * 2, "GameOfLife");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(WHITE);

    // Edit Cell (Toggle)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      int row = (GetMouseY() - OFFSET) / CELL_LEN;
      int column = (GetMouseX() - OFFSET) / CELL_LEN;
      if (row >= 0 && row < ROW_COUNT && column >= 0 && column < COLUMN_COUNT) {
        grid[row][column] ^= true;
      }
    }

    // Step to the next generation
    if (IsKeyPressed(KEY_SPACE)) {
      bool newGrid[ROW_COUNT][COLUMN_COUNT] = {0};
      for (int row = 0; row < ROW_COUNT; row++) {
        for (int column = 0; column < COLUMN_COUNT; column++) {
          int aliveNeighbours = 0;
          for (int dr = -1; dr <= 1; dr++) {
            for (int dc = -1; dc <= 1; dc++) {
              if (dr == 0 && dc == 0) {
                continue;
              }
              int newRow = row + dr;
              int newColumn = column + dc;
              if (newRow < 0 || newRow >= ROW_COUNT || newColumn < 0 || newColumn >= COLUMN_COUNT) {
                continue;
              }
              if (grid[newRow][newColumn]) {
                aliveNeighbours++;
              }
            }
          }
          bool alive = grid[row][column];
          if (alive && (aliveNeighbours == 2 || aliveNeighbours == 3)) {
            newGrid[row][column] = true;
          } else if (!alive && aliveNeighbours == 3) {
            newGrid[row][column] = true;
          } else {
            newGrid[row][column] = false;
          }
        }
      }
      memcpy(grid, newGrid, CELL_COUNT);
    }

    // Draw Cells
    for (int row = 0; row < ROW_COUNT; row++) {
      for (int column = 0; column < COLUMN_COUNT; column++) {
        if (grid[row][column]) {
          DrawRectangle(column * CELL_LEN + OFFSET, row * CELL_LEN + OFFSET, CELL_LEN, CELL_LEN, BLACK);
        }
      }
    }

    // Draw Grid Lines
    for (int row = 0; row < HEIGHT / CELL_LEN; row++) {
      for (int column = 0; column < WIDTH / CELL_LEN; column++) {
        DrawRectangleLines(column * CELL_LEN + OFFSET, row * CELL_LEN + OFFSET, CELL_LEN, CELL_LEN, GRAY);
      }
    }
    DrawRectangleLines(OFFSET - 1, OFFSET - 1, WIDTH + 2, HEIGHT + 2, GRAY);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
