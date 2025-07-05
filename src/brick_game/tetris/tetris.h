#ifndef TETRIS_H
#define TETRIS_H

#define HEIGHT 20
#define WIDTH 10

#include <stdbool.h>
#include <time.h>

typedef struct {
  int shape[4][4];
  int x, y;
  int type;
  int rotation;
} Figure;

typedef enum { START, SPAWN, MOVE, GAME_OVER } GameState;

typedef struct {
  int field[HEIGHT][WIDTH];
  Figure current;
  Figure next;
  int score;
  int level;
  int high_score;
  int speed;
  bool paused;
  GameState state;
  clock_t last_tick;
} TetrisGame;

TetrisGame* get_game_instance();
void init_game(TetrisGame* game);
void spawn_figure(TetrisGame* game);
void rotate_figure(TetrisGame* game);
void move_figure(TetrisGame* game, int dx);
void drop_figure(TetrisGame* game);
bool check_collision(TetrisGame* game, Figure* figure);
void fix_figure(TetrisGame* game);
int clear_lines(TetrisGame* game);
void update_level_speed(TetrisGame* game);
void game_tick(TetrisGame* game);
int** convert_field_to_dynamic(const int src[HEIGHT][WIDTH]);
int** convert_to_dynamic(const int src[4][4]);
void hard_drop(TetrisGame* game);
#endif