/**

@file tetris.c

@brief Реализация логики игры Tetris: фигуры, поле, обработка столкновений и
уровней.
*/

#include "tetris.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @brief Возвращает singleton-инстанс структуры игры.
 * @return Указатель на глобальный TetrisGame.
 */
TetrisGame* get_game_instance() {
  static TetrisGame instance;
  static bool initialized = false;

  if (!initialized) {
    srand(time(NULL));
    initialized = true;
  }
  return &instance;
}

/**
 * @brief Возвращает матрицу формы фигуры по её типу.
 * @param type Тип фигуры (0–6).
 * @return Указатель на 4x4-массив, описывающий форму фигуры.
 */
const int (*get_shape(int type))[4] {
  static const int shapes[7][4][4] = {
      {{0, 0, 0, 0}, {1, 1, 1, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 1, 1, 0}, {1, 1, 0, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {1, 0, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 0, 1, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 1, 0, 0}, {1, 1, 1, 0}, {0, 0, 0, 0}},
      {{0, 0, 0, 0}, {0, 1, 1, 0}, {0, 1, 1, 0}, {0, 0, 0, 0}}};
  return shapes[type];
}

/**
 * @brief Инициализация новой игры: обнуление поля, загрузка рекорда, генерация
 * первых фигур.
 * @param game Указатель на структуру игры.
 */
void init_game(TetrisGame* game) {
  memset(game->field, 0, sizeof(game->field));
  game->score = 0;
  game->level = 1;
  game->speed = 500;
  game->paused = false;
  game->state = START;
  game->next.type = rand() % 7;
  memcpy(game->next.shape, get_shape(game->next.type),
         sizeof(game->next.shape));
  game->next.x = WIDTH / 2 - 2;
  game->next.y = 0;
  FILE* f = fopen("high_score.txt", "r");
  if (f) {
    fscanf(f, "%d", &game->high_score);
    fclose(f);
  } else {
    game->high_score = 0;
  }
  spawn_figure(game);
  game->state = MOVE;
  game->last_tick = clock();
}

/**
 * @brief Генерирует новую фигуру на поле.
 * Если она сталкивается — завершает игру.
 * @param game Структура игры.
 */
void spawn_figure(TetrisGame* game) {
  game->current = game->next;
  game->next.type = rand() % 7;
  memcpy(game->next.shape, get_shape(game->next.type),
         sizeof(game->next.shape));
  game->next.x = WIDTH / 2 - 2;
  game->next.y = 0;

  if (check_collision(game, &game->current)) {
    game->state = GAME_OVER;
  }
  game->last_tick = clock();
}

/**
 * @brief Поворачивает текущую фигуру, если это допустимо.
 * @param game Указатель на игру.
 */
void rotate_figure(TetrisGame* game) {
  Figure tmp = game->current;
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      tmp.shape[i][j] = game->current.shape[3 - j][i];
    }
  }
  if (!check_collision(game, &tmp)) {
    game->current = tmp;
  }
}

/**
 * @brief Перемещает фигуру по горизонтали.
 * @param game Указатель на игру.
 * @param dx Смещение: -1 влево, 1 вправо.
 */
void move_figure(TetrisGame* game, int dx) {
  game->current.x += dx;
  if (check_collision(game, &game->current)) {
    game->current.x -= dx;
  }
}

/**
 * @brief Смещает фигуру вниз на 1 клетку. Если невозможно — фиксирует.
 * @param game Игра.
 */
void drop_figure(TetrisGame* game) {
  game->current.y++;
  if (check_collision(game, &game->current)) {
    game->current.y--;
    fix_figure(game);
  }
}

/**
 * @brief Проверяет, будет ли коллизия у фигуры.
 * @param game Указатель на игру.
 * @param figure Фигура для проверки.
 * @return true если будет коллизия.
 */
bool check_collision(TetrisGame* game, Figure* figure) {
  bool fl = false;
  for (int i = 0; i < 4 && fl == false; ++i) {
    for (int j = 0; j < 4 && fl == false; ++j) {
      if (figure->shape[i][j]) {
        int x = figure->x + j;
        int y = figure->y + i;
        if (x < 0 || x >= WIDTH || y >= HEIGHT ||
            (y >= 0 && game->field[y][x])) {
          fl = true;
        }
      }
    }
  }
  return fl;
}

/**
 * @brief Фиксирует текущую фигуру в поле. После чего генерирует новую.
 * @param game Игра.
 */
void fix_figure(TetrisGame* game) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      if (game->current.shape[i][j]) {
        int x = game->current.x + j;
        int y = game->current.y + i;
        if (y >= 0 && y < HEIGHT && x >= 0 && x < WIDTH) {
          game->field[y][x] = 1;
        }
      }
    }
  }
  clear_lines(game);
  spawn_figure(game);
}

/**
 * @brief Проверяет линии на заполненность, удаляет их и смещает поле.
 * @param game Игра.
 * @return Количество очищенных линий.
 */
int clear_lines(TetrisGame* game) {
  int cleared = 0;
  for (int i = 0; i < HEIGHT; ++i) {
    int full = 1;
    for (int j = 0; j < WIDTH && full == 1; ++j) {
      if (!game->field[i][j]) {
        full = 0;
      }
    }
    if (full) {
      for (int k = i; k > 0; --k) {
        memcpy(game->field[k], game->field[k - 1], sizeof(game->field[0]));
      }
      memset(game->field[0], 0, sizeof(game->field[0]));
      cleared++;
    }
  }
  if (cleared > 0) {
    const int points[] = {0, 100, 300, 700, 1500};
    game->score += points[cleared];
    update_level_speed(game);
  }
  if (game->score > game->high_score) {
    game->high_score = game->score;
    FILE* f = fopen("high_score.txt", "w");
    if (f) {
      fprintf(f, "%d", game->high_score);
      fclose(f);
    }
  }

  return cleared;
}

/**
 * @brief Обновляет уровень и скорость падения в зависимости от очков.
 * @param game Игра.
 */
void update_level_speed(TetrisGame* game) {
  if (game->level < 10) game->level = game->score / 600 + 1;
  game->speed = 500 - (game->level - 1) * 45;
  if (game->speed < 100) game->speed = 100;
}

/**
 * @brief Таймер игры: проверка на падение по времени.
 * @param game Игра.
 */
void game_tick(TetrisGame* game) {
  if (!(game->paused || game->state != MOVE)) {
    clock_t now = clock();
    double elapsed = (double)(now - game->last_tick) * 1000 / CLOCKS_PER_SEC;
    if (elapsed >= game->speed) {
      drop_figure(game);
      game->last_tick = now;
    }
  }
}

/**
 * @brief Мгновенно опускает фигуру до упора (hard drop).
 * @param game Игра.
 */
void hard_drop(TetrisGame* game) {
  while (!check_collision(game, &game->current)) {
    game->current.y++;
  }
  game->current.y--;
  fix_figure(game);
}

/**
 * @brief Преобразует поле из статического в динамический формат.
 * @param src Статическое поле.
 * @return Динамический массив.
 */
int** convert_field_to_dynamic(const int src[HEIGHT][WIDTH]) {
  int** dst = malloc(HEIGHT * sizeof(int*));
  for (int i = 0; i < HEIGHT; ++i) {
    dst[i] = malloc(WIDTH * sizeof(int));
    memcpy(dst[i], src[i], WIDTH * sizeof(int));
  }
  return dst;
}

/**
 * @brief Преобразует 4x4 фигуру в динамический массив.
 * @param src Исходный массив.
 * @return Динамический 4x4-массив.
 */
int** convert_to_dynamic(const int src[4][4]) {
  int** dst = malloc(4 * sizeof(int*));
  for (int i = 0; i < 4; ++i) {
    dst[i] = malloc(4 * sizeof(int));
    memcpy(dst[i], src[i], 4 * sizeof(int));
  }
  return dst;
}
