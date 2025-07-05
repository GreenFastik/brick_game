/**

@file cli.c

@brief Реализация консольного интерфейса Tetris через ncurses.
*/

#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>

#include "../../game_interfaces.h"

#define FIGURE_SIZE 4
#define GRID_W 10
#define GRID_H 20
#define CELL_TEXT "  "
#define FRAME_DELAY 50

const int *const *getFieldMatrix(const GameInfo_t *gs) {
  return (const int *const *)gs->field;
}

static const int *const *fetch_matrix(const GameInfo_t *st) {
  return getFieldMatrix(st);
}

/**
 * @brief Настраивает ncurses: без эха, без курсора, неблокирующий ввод.
 */
static void setup_screen(void) {
  setlocale(LC_ALL, "");
  initscr();
  noecho();
  curs_set(0);
  keypad(stdscr, TRUE);
  nodelay(stdscr, TRUE);
}

/**
 * @brief Инициализирует цветовые пары для фигур и фона.
 */
static void setup_colors(void) {
  start_color();
  init_pair(1, COLOR_GREEN, COLOR_GREEN);
  init_pair(2, COLOR_BLACK, COLOR_WHITE);
  init_pair(3, COLOR_WHITE, COLOR_BLACK);
}

/**
 * @brief Создаёт игровое окно и панель информации.
 * @param[out] play_win Окно поля.
 * @param[out] info_win Окно информации.
 */
static void build_ui(WINDOW **play_win, WINDOW **info_win) {
  *play_win = newwin(GRID_H + 2, GRID_W * 2 + 2, 0, 0);
  wbkgd(*play_win, COLOR_PAIR(3));
  wattron(*play_win, COLOR_PAIR(2));
  box(*play_win, 0, 0);
  wattroff(*play_win, COLOR_PAIR(2));
  wrefresh(*play_win);

  *info_win = newwin(GRID_H + 2, 20, 0, GRID_W * 2 + 3);
  wbkgd(*info_win, COLOR_PAIR(3));
  wattron(*info_win, COLOR_PAIR(3));
  box(*info_win, 0, 0);
  mvwprintw(*info_win, 0, 2, " INFO ");
  wattroff(*info_win, COLOR_PAIR(3));
  wrefresh(*info_win);
}

/**
 * @brief Преобразует нажатую клавишу в игровое действие.
 * @param c Символ клавиши.
 * @return Значение перечисления UserAction_t.
 */
static UserAction_t decode_key(int c) {
  UserAction_t action;
  if (c == ' ' || c == '\n' || c == KEY_ENTER) {
    action = Start;
  } else if (c == KEY_LEFT) {
    action = Left;
  } else if (c == KEY_RIGHT) {
    action = Right;
  } else if (c == KEY_DOWN) {
    action = Down;
  } else if (c == 'z' || c == 'Z') {
    action = Action;
  } else if (c == 'p' || c == 'P') {
    action = Pause;
  } else if (c == 27) {
    action = Terminate;
  } else {
    action = Up;
  }
  return action;
}

/**
 * @brief Отрисовывает игровые блоки на поле.
 * @param w Окно поля.
 * @param st Структура GameInfo_t.
 */
static void paint_blocks(WINDOW *w, const GameInfo_t *st) {
  const int *const *m = fetch_matrix(st);
  wattron(w, COLOR_PAIR(1));
  for (int y = 0; y < GRID_H; ++y) {
    for (int x = 0; x < GRID_W; ++x) {
      if (m[y][x]) {
        mvwaddstr(w, y + 1, x * 2 + 1, CELL_TEXT);
      }
    }
  }
  wattroff(w, COLOR_PAIR(1));
}

/**
 * @brief Отрисовывает боковую панель: очки, уровень, следующую фигуру.
 * @param w Окно информации.
 * @param st Структура состояния игры.
 */
static void paint_sidebar(WINDOW *w, const GameInfo_t *st) {
  wattron(w, COLOR_PAIR(3));
  mvwprintw(w, 2, 2, "Next:");
  wattroff(w, COLOR_PAIR(3));

  for (int i = 0; i < FIGURE_SIZE; ++i) {
    for (int j = 0; j < FIGURE_SIZE; ++j) {
      if (st->next[i][j]) {
        wattron(w, COLOR_PAIR(1));
        mvwaddstr(w, 3 + i, 2 + j * 2, CELL_TEXT);
        wattroff(w, COLOR_PAIR(1));
      }
    }
  }

  mvwprintw(w, 8, 2, "Score:      %4d", st->score);
  mvwprintw(w, 9, 2, "High Score: %4d", st->high_score);
  mvwprintw(w, 11, 2, "Level:      %2d", st->level);
}

/**
 * @brief Полная перерисовка интерфейса.
 * @param play Окно поля.
 * @param info Окно информации.
 * @param st Статус игры.
 */
static void refresh_screen(WINDOW *play, WINDOW *info, const GameInfo_t *st) {
  werase(play);
  wattron(play, COLOR_PAIR(2));
  box(play, ACS_VLINE, ACS_HLINE);
  wattroff(play, COLOR_PAIR(2));

  paint_blocks(play, st);

  if (st->pause == 1) {
    wattron(play, A_BOLD | COLOR_PAIR(2));
    mvwprintw(play, GRID_H / 2 + 1, (GRID_W * 2 - 5) / 2 + 1, "PAUSE");
    wattroff(play, A_BOLD | COLOR_PAIR(2));
  } else if (st->pause == 2) {
    wattron(play, A_BOLD | COLOR_PAIR(2));
    mvwprintw(play, GRID_H / 2 + 1, (GRID_W * 2 - 9) / 2 + 1, "GAME OVER");
    wattroff(play, A_BOLD | COLOR_PAIR(2));
  }
  wrefresh(play);
  werase(info);
  wattron(info, COLOR_PAIR(2));
  box(info, ACS_VLINE, ACS_HLINE);
  mvwprintw(info, 0, 2, " INFO ");
  wattroff(info, COLOR_PAIR(2));
  paint_sidebar(info, st);
  wrefresh(info);
}

/**
 * @brief Основной игровой цикл: ввод, обновление, отрисовка.
 * @param play Окно поля.
 * @param info Окно информации.
 */
static void play_cycle(WINDOW *play, WINDOW *info) {
  userInput(Start, false);
  GameInfo_t st = updateCurrentState();
  int timer = 0;
  bool fl = true;
  while (fl) {
    int ch = getch();
    UserAction_t act = decode_key(ch);
    if (act != Up) userInput(act, false);
    if (act == Terminate) fl = false;

    timer += FRAME_DELAY;
    if (timer >= st.speed) {
      userInput(Up, false);
      timer = 0;
    }

    st = updateCurrentState();
    refresh_screen(play, info, &st);
    napms(FRAME_DELAY);
  }
}

/**
 * @brief Завершение и освобождение ncurses.
 */
static void teardown(WINDOW *play, WINDOW *info) {
  delwin(play);
  delwin(info);
  endwin();
}

/**
 * @brief Основной запуск программы.
 */
int main(void) {
  srand((unsigned)time(NULL));
  setup_screen();
  setup_colors();

  WINDOW *play_w, *info_w;
  build_ui(&play_w, &info_w);

  play_cycle(play_w, info_w);
  teardown(play_w, info_w);
  return EXIT_SUCCESS;
}