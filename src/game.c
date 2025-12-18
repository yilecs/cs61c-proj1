#include "game.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "snake_utils.h"

/* Helper function definitions */
static void set_board_at(game_t *game, unsigned int row, unsigned int col, char ch);

static bool is_tail(char c);

static bool is_head(char c);

static bool is_snake(char c);

static char body_to_tail(char c);

static char head_to_body(char c);

static unsigned int get_next_row(unsigned int cur_row, char c);

static unsigned int get_next_col(unsigned int cur_col, char c);

static void find_head(game_t *game, unsigned int snum);

static char next_square(game_t *game, unsigned int snum);

static void update_tail(game_t *game, unsigned int snum);

static void update_head(game_t *game, unsigned int snum);

/* Task 1 */
game_t *create_default_game() {
    game_t *game = malloc(sizeof(game_t));
    game->num_rows = 18;
    game->board = malloc(sizeof(char *) * game->num_rows);
    char *header = "####################\n";
    char *body = "#                  #\n";
    for (int i = 0; i < game->num_rows; i++) {
        game->board[i] = malloc(sizeof(char) * 22);
    }
    for (int i = 0; i < game->num_rows; i++) {
        if (i == 0 || i == game->num_rows - 1) {
            strcpy(game->board[i], header);
        } else {
            strcpy(game->board[i], body);
        }
    }
    set_board_at(game, 2, 9, '*');

    game->num_snakes = 1;
    game->snakes = malloc(sizeof(snake_t));
    game->snakes[0].tail_row = 2;
    game->snakes[0].tail_col = 2;
    game->snakes[0].head_row = 2;
    game->snakes[0].head_col = 4;
    game->snakes[0].live = true;
    set_board_at(game, 2, 2, 'd');
    set_board_at(game, 2, 3, '>');
    set_board_at(game, 2, 4, 'D');

    return game;
}

/* Task 2 */
void free_game(game_t *game) {
    for (int i = 0; i < game->num_rows; i++) {
        free(game->board[i]);
    }
    free(game->board);
    free(game->snakes);
    free(game);
}

/* Task 3 */
void print_board(game_t *game, FILE *fp) {
    for (int i = 0; i < game->num_rows; i++) {
        fprintf(fp, "%s", game->board[i]);
    }
}

/*
  Saves the current game into filename. Does not modify the game object.
  (already implemented for you).
*/
void save_board(game_t *game, char *filename) {
    FILE *f = fopen(filename, "w");
    print_board(game, f);
    fclose(f);
}

/* Task 4.1 */

/*
  Helper function to get a character from the board
  (already implemented for you).
*/
char get_board_at(game_t *game, unsigned int row, unsigned int col) { return game->board[row][col]; }

/*
  Helper function to set a character on the board
  (already implemented for you).
*/
static void set_board_at(game_t *game, unsigned int row, unsigned int col, char ch) {
    game->board[row][col] = ch;
}

/*
  Returns true if c is part of the snake's tail.
  The snake consists of these characters: "wasd"
  Returns false otherwise.
*/
static bool is_tail(char c) {
    bool tail = false;
    switch (c) {
        case 'w':
        case 'a':
        case 's':
        case 'd':
            tail = true;
            break;
        default:
            tail = false;
            break;
    }
    return tail;
}

/*
  Returns true if c is part of the snake's head.
  The snake consists of these characters: "WASDx"
  Returns false otherwise.
*/
static bool is_head(char c) {
    bool head = false;
    switch (c) {
        case 'W':
        case 'A':
        case 'S':
        case 'D':
        case 'x':
            head = true;
            break;
        default:
            head = false;
            break;
    }
    return head;
}

/*
  Returns true if c is part of the snake.
  The snake consists of these characters: "wasd^<v>WASDx"
*/
static bool is_snake(char c) {
    char *snake_chars = "wasd^<v>WASDx";
    char *pos = strchr(snake_chars, c);
    if (pos == NULL) {
        return false;
    } else {
        return true;
    }
}

/*
  Converts a character in the snake's body ("^<v>")
  to the matching character representing the snake's
  tail ("wasd").
*/
static char body_to_tail(char c) {
    char *bodys = "^<v>";
    char *tails = "wasd";
    char *pos = strchr(bodys, c);
    if (pos == NULL) {
        return '?';
    } else {
        long int ind = pos - bodys;
        return tails[ind];
    }
}

/*
  Converts a character in the snake's head ("WASD")
  to the matching character representing the snake's
  body ("^<v>").
*/
static char head_to_body(char c) {
    char *bodys = "^<v>";
    char *heads = "WASD";
    char *pos = strchr(heads, c);
    if (pos == NULL) {
        return '?';
    } else {
        long int ind = pos - heads;
        return bodys[ind];
    }
}

/*
  Returns cur_row + 1 if c is 'v' or 's' or 'S'.
  Returns cur_row - 1 if c is '^' or 'w' or 'W'.
  Returns cur_row otherwise.
*/
static unsigned int get_next_row(unsigned int cur_row, char c) {
    if (c == 'v' || c == 's' || c == 'S') {
        return cur_row + 1;
    } else if (c == '^' || c == 'w' || c == 'W') {
        return cur_row - 1;
    } else {
        return cur_row;
    }
}

/*
  Returns cur_col + 1 if c is '>' or 'd' or 'D'.
  Returns cur_col - 1 if c is '<' or 'a' or 'A'.
  Returns cur_col otherwise.
*/
static unsigned int get_next_col(unsigned int cur_col, char c) {
    if (c == '<' || c == 'a' || c == 'A') {
        return cur_col - 1;
    } else if (c == '>' || c == 'd' || c == 'D') {
        return cur_col + 1;
    } else {
        return cur_col;
    }
}

/*
  Task 4.2

  Helper function for update_game. Return the character in the cell the snake is moving into.

  This function should not modify anything.
*/
static char next_square(game_t *game, unsigned int snum) {
    snake_t *snake = &game->snakes[snum];
    unsigned int cur_row = snake->head_row;
    unsigned int cur_col = snake->head_col;
    char c = get_board_at(game, cur_row, cur_col);
    unsigned int next_row = get_next_row(cur_row, c);
    unsigned int next_col = get_next_col(cur_col, c);

    return get_board_at(game, next_row, next_col);
}

/*
  Task 4.3

  Helper function for update_game. Update the head...

  ...on the board: add a character where the snake is moving

  ...in the snake struct: update the row and col of the head

  Note that this function ignores food, walls, and snake bodies when moving the head.
*/
static void update_head(game_t *game, unsigned int snum) {
    snake_t *snake = &game->snakes[snum];
    unsigned int cur_row = snake->head_row;
    unsigned int cur_col = snake->head_col;
    char head = get_board_at(game, cur_row, cur_col);
    char body = head_to_body(head);
    unsigned int next_row = get_next_row(cur_row, head);
    unsigned int next_col = get_next_col(cur_col, head);

    set_board_at(game, cur_row, cur_col, body);
    set_board_at(game, next_row, next_col, head);

    snake->head_row = next_row;
    snake->head_col = next_col;
}

/*
  Task 4.4

  Helper function for update_game. Update the tail...

  ...on the board: blank out the current tail, and change the new
  tail from a body character (^<v>) into a tail character (wasd)

  ...in the snake struct: update the row and col of the tail
*/
static void update_tail(game_t *game, unsigned int snum) {
    snake_t *snake = &game->snakes[snum];
    unsigned int cur_row = snake->tail_row;
    unsigned int cur_col = snake->tail_col;
    char tail = get_board_at(game, cur_row, cur_col);
    unsigned int next_row = get_next_row(cur_row, tail);
    unsigned int next_col = get_next_col(cur_col, tail);
    char body = get_board_at(game, next_row, next_col);
    tail = body_to_tail(body);

    set_board_at(game, cur_row, cur_col, ' ');
    set_board_at(game, next_row, next_col, tail);

    snake->tail_row = next_row;
    snake->tail_col = next_col;
}

/* Task 4.5 */
void update_game(game_t *game, int (*add_food)(game_t *game)) {
    for (unsigned int i = 0; i < game->num_snakes; i++) {
        if (!game->snakes[i].live) {
            continue;
        }

        char next_char = next_square(game, i);
        if (is_snake(next_char) || next_char == '#') {
            game->snakes[i].live = false;
            set_board_at(game, game->snakes[i].head_row, game->snakes[i].head_col, 'x');
        } else if (next_char == '*') {
            update_head(game, i);
            (*add_food)(game);
        } else {
            update_head(game, i);
            update_tail(game, i);
        }
    }
}

/* Task 5.1 */
char *read_line(FILE *fp) {
    if (fp == NULL) {
        return NULL;
    }
    if (feof(fp)) {
        return NULL;
    }

    const int CHUNK_SIZE = 128;
    char buffer[CHUNK_SIZE];
    char *line = NULL;
    size_t total_len = 0;

    // reset buffer
    buffer[0] = '\0';

    while (fgets(buffer, CHUNK_SIZE, fp) != NULL) {
        size_t chunk_len = strlen(buffer);

        char *temp = realloc(line, sizeof(char) *(total_len + chunk_len + 1));
        if (temp == NULL) {
            free(line);
            return NULL;
        }
        line = temp;

        // copy buffer to line end
        strcpy(line + total_len, buffer);
        total_len += chunk_len;

        // check if read to \n or EOF
        if (chunk_len > 0) {
            if (buffer[chunk_len - 1] == '\n' || chunk_len < CHUNK_SIZE - 1) {
                break;
            }
        } else {
            // read nothing
            break;
        }
    }

    return line;
}

/* Task 5.2 */
game_t *load_board(FILE *fp) {
    game_t *game = malloc(sizeof(game_t));

    game->snakes = NULL;
    game->num_snakes = 0;

    unsigned int num_rows = 0;
    unsigned int initial_num_rows = 10;

    char **board = malloc(sizeof(char *) * initial_num_rows);
    char *line;
    while ((line = read_line(fp)) != NULL) {
        if (num_rows == initial_num_rows) {
            initial_num_rows *= 2;
            board = realloc(board, sizeof(char *) * initial_num_rows);
        }
        board[num_rows] = line;
        num_rows++;
    }
    board = realloc(board, sizeof(char *) * num_rows);

    game->board = board;
    game->num_rows = num_rows;

    return game;
}

/*
  Task 6.1

  Helper function for initialize_snakes.
  Given a snake struct with the tail row and col filled in,
  trace through the board to find the head row and col, and
  fill in the head row and col in the struct.
*/
static void find_head(game_t *game, unsigned int snum) {
    snake_t *snake = &game->snakes[snum];
    unsigned int row = snake->tail_row;
    unsigned int col = snake->tail_col;
    char c = get_board_at(game, row, col);
    while (!is_head(c)) {
        row = get_next_row(row, c);
        col = get_next_col(col, c);
        c = get_board_at(game, row, col);
    }
    snake->head_row = row;
    snake->head_col = col;
}

/* Task 6.2 */
game_t *initialize_snakes(game_t *game) {
    unsigned int initial_num_snakes = 10;
    unsigned int num_snakes = 0;
    game->snakes = malloc(sizeof(snake_t) * initial_num_snakes);
    for (unsigned int row = 0; row < game->num_rows; row++) {
        unsigned int cols = strlen(game->board[row]);
        for (unsigned int col = 0; col < cols; col++) {
            if (is_tail(get_board_at(game, row, col))) {
                if (num_snakes == initial_num_snakes) {
                    initial_num_snakes *= 2;
                    game->snakes = realloc(game->snakes, sizeof(snake_t) * initial_num_snakes);
                }
                game->snakes[num_snakes].tail_row = row;
                game->snakes[num_snakes].tail_col = col;
                game->snakes[num_snakes].live = true;

                find_head(game, num_snakes);

                num_snakes++;
            }
        }
    }

    game->snakes = realloc(game->snakes, sizeof(snake_t) * num_snakes);
    game->num_snakes = num_snakes;

    return game;
}
