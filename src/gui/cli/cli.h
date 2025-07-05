#ifndef UNTITLED2_TETRIS_FRONTEND_H
#define UNTITLED2_TETRIS_FRONTEND_H

const int *const *getFieldMatrix(const GameInfo_t *gs) static void initNcurses(
    void);
UserAction_t getInput(int user_input);
static void initColors(void);
static void createWindows(WINDOW **field_win, WINDOW **info_win);
static void drawFixedBlocks(WINDOW *field_win, const GameInfo_t *gs);
static void drawNextAndStats(WINDOW *uwin, const GameInfo_t *gs);
static void drawFrame(WINDOW *fwin, WINDOW *uwin, const GameInfo_t *gs);
static void cleanup(WINDOW *fwin, WINDOW *uwin);
int main();

#endif
