CC       := gcc
CFLAGS   := -Wall -Wextra -std=c11 -g
LDFLAGS  := -lncurses
PROJECT  := brick_game

UNAME_S := $(shell uname)
COVERAGE_FLAGS := -fprofile-arcs -ftest-coverage

ifeq ($(UNAME_S),Darwin)
 OPEN_CMD = open
else
 OPEN_CMD = xdg-open
endif

ifeq ($(MAKECMDGOALS),gcov_report)
  CFLAGS += $(COVERAGE_FLAGS)
endif

SRC_ROOT := .
SRC_DIRS := brick_game/tetris gui/cli .
BUILD_DIR := build
BIN_DIR := bin
BIN := $(BIN_DIR)/$(PROJECT)

SRCS := $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.c))
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))

TEST_DIR := tests
TEST_SRC := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS := $(patsubst ../tests/%.c,$(BUILD_DIR)/tests/%.o,$(TEST_SRC))
TEST_BIN := $(BUILD_DIR)/tests/test_runner

CHECK_CFLAGS := $(shell brew --prefix check 2>/dev/null)/include
CHECK_LIBS := $(shell brew --prefix check 2>/dev/null)/lib

ifeq ($(CHECK_LIBS),)
    CHECK_LIBS = -lcheck -lm -lpthread
endif

LIB := tetris.a

all: $(BIN) $(LIB)

$(LIB): $(OBJS)
	ar rcs $@ $^

$(BIN): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(filter-out $(BUILD_DIR)/gui/cli/cli.o,$(OBJS)) $(TEST_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -I$(CHECK_CFLAGS) -o $@ $^ -L$(CHECK_LIBS) -lcheck

$(BUILD_DIR)/tests/%.o: ../tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(COVERAGE_FLAGS) -I$(CHECK_CFLAGS) -c $< -o $@

ifeq ($(UNAME_S),Darwin)
PREFIX := $(HOME)/tetris
else
PREFIX := /usr/local/tetris
endif

install: $(BIN) $(LIB)
	@mkdir -p $(PREFIX)/bin
	@mkdir -p $(PREFIX)/lib
	install -m 0755 $(BIN) "$(PREFIX)/bin/tetris"
	install -m 644 $(LIB) "$(PREFIX)/lib/tetris.a"

uninstall:
	rm -rf $(PREFIX)/bin $(PREFIX)/lib

clean:
	rm -rf $(BUILD_DIR) bin coverage.html *.css *.html report html latex
	rm -f */*/*.gcda */*/*.gcno *.gcda *.gcno */*.gcno */*/*.info coverage.html record.txt *.info *.a
	rm -f *.tar.gz *.txt

gcov_report: clean test
	@lcov --directory $(BUILD_DIR) --capture --output-file coverage.info
	@genhtml --output-directory report --legend coverage.info
	@$(OPEN_CMD) report/index.html || true

clean-dist:
	rm -rf $(PREFIX)/dist
	mkdir -p $(PREFIX)/dist

dist: clean-dist
	@mkdir -p $(PREFIX)/tmp
	cp -pr . $(PREFIX)/tmp

	tar czf $(PREFIX)/dist/tetris.tar.gz -C $(PREFIX)/tmp .
	rm -rf $(PREFIX)/tmp

dvi:
	doxygen Doxyfile 2>/dev/null
	$(OPEN_CMD) html/index.html