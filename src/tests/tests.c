#include <check.h>
#include <stdlib.h>
#include <string.h>

#include "../brick_game/tetris/tetris.h"
#include "../game_interfaces.h"

START_TEST(test_game_init_defaults) {
  TetrisGame* game = get_game_instance();
  init_game(game);
  ck_assert_int_eq(game->score, 0);
  ck_assert_int_eq(game->level, 1);
  ck_assert_int_eq(game->paused, false);
  ck_assert_int_eq(game->state, MOVE);
}
END_TEST

START_TEST(test_spawn_does_not_overlap) {
  TetrisGame* game = get_game_instance();
  init_game(game);
  ck_assert_int_eq(check_collision(game, &game->current), false);
}
END_TEST

START_TEST(test_move_left_right) {
  TetrisGame* game = get_game_instance();
  init_game(game);
  int original_x = game->current.x;

  move_figure(game, -1);
  ck_assert_int_eq(game->current.x, original_x - 1);

  move_figure(game, 1);
  move_figure(game, 1);
  ck_assert_int_eq(game->current.x, original_x + 1);
}
END_TEST

START_TEST(test_drop_and_fix) {
  TetrisGame* game = get_game_instance();
  init_game(game);
  for (int i = 0; i < HEIGHT; ++i) {
    drop_figure(game);
  }
  bool fixed = false;
  for (int i = 0; i < HEIGHT && !fixed; ++i)
    for (int j = 0; j < WIDTH && !fixed; ++j)
      if (game->field[i][j]) fixed = true;

  ck_assert_msg(fixed, "Figure did not fix to the field");
}
END_TEST

START_TEST(test_hard_drop) {
  TetrisGame* game = get_game_instance();
  init_game(game);
  hard_drop(game);

  bool placed = false;
  for (int i = 0; i < HEIGHT && !placed; ++i)
    for (int j = 0; j < WIDTH && !placed; ++j)
      if (game->field[i][j]) placed = true;

  ck_assert_msg(placed, "Hard drop did not place the piece");
}
END_TEST

START_TEST(test_clear_lines) {
  TetrisGame* game = get_game_instance();
  init_game(game);
  for (int i = 0; i < WIDTH; ++i) game->field[HEIGHT - 1][i] = 1;

  int cleared = clear_lines(game);
  ck_assert_int_eq(cleared, 1);
}
END_TEST

START_TEST(test_update_current_state_adds_current_figure) {
  init_game(get_game_instance());
  GameInfo_t info = updateCurrentState();

  bool found = false;
  for (int i = 0; i < HEIGHT && !found; ++i)
    for (int j = 0; j < WIDTH && !found; ++j)
      if (info.field[i][j]) found = true;

  ck_assert_msg(found, "Current figure not present in GameInfo field");
}
END_TEST

START_TEST(test_user_input_pause_toggle) {
  TetrisGame* game = get_game_instance();
  init_game(game);
  userInput(Pause, false);
  ck_assert(game->paused);
  userInput(Pause, false);
  ck_assert(!game->paused);
}
END_TEST

START_TEST(test_user_input_start_resets_game) {
  TetrisGame* game = get_game_instance();
  game->score = 999;
  userInput(Start, false);
  ck_assert_int_eq(game->score, 0);
  ck_assert(game->state == MOVE);
}
END_TEST

Suite* tetris_suite(void) {
  Suite* s = suite_create("Tetris");
  TCase* tc_core = tcase_create("Core");

  tcase_add_test(tc_core, test_game_init_defaults);
  tcase_add_test(tc_core, test_spawn_does_not_overlap);
  tcase_add_test(tc_core, test_move_left_right);
  tcase_add_test(tc_core, test_drop_and_fix);
  tcase_add_test(tc_core, test_hard_drop);
  tcase_add_test(tc_core, test_clear_lines);
  tcase_add_test(tc_core, test_update_current_state_adds_current_figure);
  tcase_add_test(tc_core, test_user_input_pause_toggle);
  tcase_add_test(tc_core, test_user_input_start_resets_game);

  suite_add_tcase(s, tc_core);
  return s;
}

int main(void) {
  int failed;
  Suite* s = tetris_suite();
  SRunner* runner = srunner_create(s);
  srunner_run_all(runner, CK_VERBOSE);
  failed = srunner_ntests_failed(runner);
  srunner_free(runner);
  return (failed == 0) ? 0 : 1;
}
