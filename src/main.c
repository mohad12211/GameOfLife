#include <assert.h>
#include <ctype.h>
#include <float.h>
#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define WIDTH 1600
#define HEIGHT 900
#define CELL_LEN 25
#define CELL_COUNT (COLUMN_COUNT * ROW_COUNT)
#define ZOOM_INCREMENT 0.050f
#define MINIMUM_ZOOM 0.250f
#define CELLS_INIT_CAPACITY 256
#define STEP_DELTA 0.1f

typedef struct {
  int x;
  int y;
} Cell;

typedef struct {
  Cell key;
} CellEntry;

CellEntry *parseRLE(const char *str);
void nextGeneration(CellEntry **aliveCellsPtr, CellEntry **nextAliveCellsPtr);

CellEntry *parseRLE(const char *str) {
  CellEntry *aliveCells = NULL;
  int n, x, y = 0;
  for (size_t i = 0; i < strlen(str); i++) {
    char c = str[i];
    if (isdigit(c)) {
      n = n * 10 + (c - '0');
    } else if (c == '$') {
      y++;
      x = 0;
      n = 0;
    } else if (c == 'b') {
      x += n != 0 ? n : 1;
      n = 0;
    } else if (c == 'o') {
      n = n != 0 ? n : 1;
      for (int j = 0; j < n; j++, x++) {
        CellEntry cellEntry = {(Cell){x, y}};
        hmputs(aliveCells, cellEntry);
      }
      n = 0;
    }
  }
  return aliveCells;
}

void nextGeneration(CellEntry **aliveCellsPtr, CellEntry **nextAliveCellsPtr) {
  CellEntry *aliveCells = *aliveCellsPtr;
  CellEntry *nextAliveCells = *nextAliveCellsPtr;

  Vector3 min = {FLT_MAX, FLT_MAX, 0};
  Vector3 max = {FLT_MIN, FLT_MIN, 0};
  for (int i = 0; i < hmlen(aliveCells); i++) {
    min = Vector3Min(min, (Vector3){aliveCells[i].key.x, aliveCells[i].key.y, 0});
    max = Vector3Max(max, (Vector3){aliveCells[i].key.x, aliveCells[i].key.y, 0});
  }
  for (int x = min.x - 1; x <= max.x + 1; x++) {
    for (int y = min.y - 1; y <= max.y + 1; y++) {
      int aliveNeighbours = 0;
      for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
          if (dy == 0 && dx == 0) {
            continue;
          }
          Cell neighbourCell = {x + dx, y + dy};
          if (hmgeti(aliveCells, neighbourCell) >= 0) {
            aliveNeighbours++;
          }
        }
      }
      CellEntry cellEntry = (CellEntry){(Cell){x, y}};
      bool isAlive = hmgeti(aliveCells, cellEntry.key) >= 0;
      if (isAlive && (aliveNeighbours == 2 || aliveNeighbours == 3)) {
        hmputs(nextAliveCells, cellEntry);
      } else if (!isAlive && aliveNeighbours == 3) {
        hmputs(nextAliveCells, cellEntry);
      }
    }
  }

  // PERFORMANCE: Find a way to not free the hashmap (somehow implement hmclear)
  hmfree(aliveCells);
  *aliveCellsPtr = nextAliveCells;
  *nextAliveCellsPtr = NULL;
}

int main(int argc, char *argv[]) {

  SetTraceLogLevel(LOG_WARNING);
  InitWindow(WIDTH, HEIGHT, "GameOfLife");
  SetTargetFPS(60);

  CellEntry *aliveCells = NULL;
  CellEntry *nextAliveCells = NULL;
  Camera2D camera = {0};
  camera.zoom = 1.0f;
  bool shouldDrawGrid = true;
  bool shouldRunSimluation = false;
  float time = 0.0f;

  while (!WindowShouldClose()) {
    // Zoom
    float wheel = GetMouseWheelMove();
    if (wheel != 0) {
      Vector2 mouseWorldPosition = GetScreenToWorld2D(GetMousePosition(), camera);
      camera.target = mouseWorldPosition;
      camera.offset = GetMousePosition();
      camera.zoom += (wheel * ZOOM_INCREMENT);
      if (camera.zoom < MINIMUM_ZOOM)
        camera.zoom = MINIMUM_ZOOM;
    }

    // Drag
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
      Vector2 delta = GetMouseDelta();
      delta = Vector2Scale(delta, -1.0f / camera.zoom);
      camera.target = Vector2Add(camera.target, delta);
    }

    // Edit Cell (Toggle)
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      Vector2 mouseWorldPosition = GetScreenToWorld2D(GetMousePosition(), camera);
      int y = floor(mouseWorldPosition.y / CELL_LEN);
      int x = floor(mouseWorldPosition.x / CELL_LEN);
      CellEntry entry = (CellEntry){(Cell){x, y}};
      if (hmgeti(aliveCells, entry.key) >= 0) {
        assert(hmdel(aliveCells, entry.key) == 1);
      } else {
        hmputs(aliveCells, entry);
      }
    }

    if (IsKeyPressed(KEY_SPACE)) {
      nextGeneration(&aliveCells, &nextAliveCells);
    }

    if (IsKeyPressed(KEY_G)) {
      shouldDrawGrid ^= true;
    }

    if (IsKeyPressed(KEY_S)) {
      shouldRunSimluation ^= true;
      time = STEP_DELTA;
    }

    if (IsKeyPressed(KEY_L)) {
      const char *clipboardText = GetClipboardText();
      hmfree(aliveCells);
      aliveCells = parseRLE(clipboardText);
#if defined(PLATFORM_WEB)
      free(clipboardText);
#endif
    }

    // Run simluation every STEP_DELTA seconds
    if (shouldRunSimluation) {
      if (time < STEP_DELTA) {
        time += GetFrameTime();
      } else if (hmlen(aliveCells) > 0) {
        time = 0.0f;
        nextGeneration(&aliveCells, &nextAliveCells);
      }
    }

    BeginDrawing();
    ClearBackground(WHITE);
    BeginMode2D(camera);

    // Draw cells
    for (int i = 0; i < hmlen(aliveCells); i++) {
      DrawRectangle(aliveCells[i].key.x * CELL_LEN, aliveCells[i].key.y * CELL_LEN, CELL_LEN, CELL_LEN, BLACK);
    }

    // Draw Grid Lines
    if (shouldDrawGrid) {
      Vector2 topLeft = Vector2Scale(GetScreenToWorld2D(Vector2Zero(), camera), 1.0f / CELL_LEN);
      Vector2 bottomRight = Vector2Scale(GetScreenToWorld2D((Vector2){WIDTH, HEIGHT}, camera), 1.0f / CELL_LEN);
      for (int y = topLeft.y - 1; y <= bottomRight.y; y++) {
        for (int x = topLeft.x - 1; x <= bottomRight.x; x++) {
          Rectangle rec = {x * CELL_LEN, y * CELL_LEN, CELL_LEN, CELL_LEN};
          DrawRectangleLinesEx(rec, 1, GRAY);
        }
      }
    }

    EndMode2D();
    EndDrawing();
  }

  hmfree(aliveCells);
  hmfree(nextAliveCells);
  CloseWindow();
  return 0;
}
