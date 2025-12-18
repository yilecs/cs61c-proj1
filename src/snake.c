#include <stdio.h>
#include <string.h>

#include "snake_utils.h"
#include "game.h"

int main(int argc, char *argv[]) {
    bool io_stdin = false;
    char *in_filename = NULL;
    char *out_filename = NULL;
    game_t *game = NULL;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-i") == 0 && i < argc - 1) {
            if (io_stdin) {
                fprintf(stderr, "Usage: %s [-i filename | --stdin] [-o filename]\n", argv[0]);
                return 1;
            }
            in_filename = argv[i + 1];
            i++;
            continue;
        } else if (strcmp(argv[i], "--stdin") == 0) {
            if (in_filename != NULL) {
                fprintf(stderr, "Usage: %s [-i filename | --stdin] [-o filename]\n", argv[0]);
                return 1;
            }
            io_stdin = true;
            continue;
        }
        if (strcmp(argv[i], "-o") == 0 && i < argc - 1) {
            out_filename = argv[i + 1];
            i++;
            continue;
        }
        fprintf(stderr, "Usage: %s [-i filename | --stdin] [-o filename]\n", argv[0]);
        return 1;
    }

    // Do not modify anything above this line.

    /* Task 7 */

    // Read board from file, or create default board
    if (in_filename != NULL) {
        // Load the board from in_filename
        FILE *fp = fopen(in_filename, "r");
        // If the file doesn't exist, return -1
        if (fp == NULL) {
            return -1;
        }
        game = load_board(fp);
        // Then call initialize_snakes on the game you made
        game = initialize_snakes(game);
        // close file pointer
        fclose(fp);
    } else if (io_stdin) {
        // Load the board from stdin
        game = load_board(stdin);
        // Then call initialize_snakes on the game you made
        game = initialize_snakes(game);
    } else {
        game = create_default_game();
    }

    // Update game. Use the deterministic_food function
    // (already implemented in snake_utils.h) to add food.
    update_game(game, &deterministic_food);

    // Write updated board to file or stdout
    if (out_filename != NULL) {
        // Save the board to out_filename
        save_board(game, out_filename);
    } else {
        // Print the board to stdout
        print_board(game, stdout);
    }

    // Free the game
    free_game(game);

    return 0;
}
