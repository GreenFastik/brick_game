/**

@file game_interfaces.c

@brief Интерфейс между GUI и логикой игры. Принимает ввод и возвращает
состояние.
*/

#include "game_interfaces.h"

#include "brick_game/tetris/tetris.h"

/**
 * @brief Обрабатывает пользовательский ввод и передаёт его в backend.
 * @param action Действие игрока.
 * @param hold true если кнопка удерживается (не используется).
 */
void userInput(UserAction_t action, bool hold) {
  (void)hold;
  TetrisGame* game = get_game_instance();
  if (action == Start) {
    init_game(game);
  } else if (action == Terminate) {
    game->state = GAME_OVER;
  } else if (action == Pause) {
    game->paused = !game->paused;
  }
  if (!(game->state == GAME_OVER || game->paused)) {
    if (action == Left) {
      move_figure(game, -1);
    } else if (action == Right) {
      move_figure(game, 1);
    } else if (action == Down) {
      hard_drop(game);
    } else if (action == Action) {
      rotate_figure(game);
    } else if (action == Up) {
      drop_figure(game);
    }
  }
}

/**
 * @brief Обновляет и возвращает текущее состояние игры.
 * @return Структура GameInfo_t с актуальными данными поля, очков, скорости и
 * т.д.
 */
GameInfo_t updateCurrentState() {
  TetrisGame* game = get_game_instance();
  game_tick(game);
  if (!game->paused && game->state != GAME_OVER) {
    game_tick(game);
  }

  GameInfo_t info;
  int** dynamic_field = convert_field_to_dynamic(game->field);
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      if (game->current.shape[i][j]) {
        int x = game->current.x + j;
        int y = game->current.y + i;
        if (y >= 0 && y < HEIGHT && x >= 0 && x < WIDTH) {
          dynamic_field[y][x] = 1;
        }
      }
    }
  }
  info.field = dynamic_field;
  info.next = convert_to_dynamic(game->next.shape);
  info.score = game->score;
  info.high_score = game->high_score;
  info.level = game->level;
  info.speed = game->speed;
  info.pause = game->paused ? 1 : (game->state == GAME_OVER ? 2 : 0);
  ;

  return info;
}
