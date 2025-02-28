#include <stdio.h>
#include <string.h>
#include "linenoise/linenoise.h"
#include "esp_console.h"
#include "board.h"
#include "moves.h"

Move_t move_list[100];  // TODO: make this dynamic
unsigned int move_count = 0;

void print_move_list()
{
    for (int i = 0; i < move_count; i++) {
        printf("%d. %c%d->%c%d\n", i + 1, 'a' + move_list[i].start.y, 8 - move_list[i].start.x, 'a' + move_list[i].end.y, 8 - move_list[i].end.x);
    }
}

/*
 * Handle user input
 * Returns true if the input is a valid move, false otherwise
 */
bool handle_user_input(Position_t *pos, char *prompt)
{
    char *line = linenoise(prompt);
    if (!line) {
        printf("Error: Failed to read input\n");
        return false;
    }
    if (strcmp(line, "h") == 0) {
        printf("Commands:\n");
        printf("  'p' - Print the current board\n");
        printf("  'm' - Print move history\n");
        printf("  'r' - Reset the board\n");
        printf("  'q' - Quit\n");
        printf("  Or enter a position (e.g. 'e2')\n");
        linenoiseFree(line);
        return false;
    }

    if (strcmp(line, "p") == 0) {
        print_board();
        linenoiseFree(line);
        return false;
    } else if (strcmp(line, "m") == 0) {
        print_move_list();
        linenoiseFree(line);
        return false;
    } else if (strcmp(line, "r") == 0) {
        init_board();
        print_board();
        linenoiseFree(line);
        return false;
    } else if (strcmp(line, "q") == 0) {
        linenoiseFree(line);
        exit(0);
    }
    if (strlen(line) != 2) {
        printf("Error: Invalid input format. Please enter a letter followed by a number (e.g. 'e2')\n");
        linenoiseFree(line);
        return false;
    }

    if (line[0] < 'a' || line[0] > 'h') {
        printf("Error: Column must be between 'a' and 'h'\n");
        linenoiseFree(line);
        return false;
    }

    if (line[1] < '1' || line[1] > '8') {
        printf("Error: Row must be between 1 and 8\n");
        linenoiseFree(line);
        return false;
    }

    pos->x = 7 - (line[1] - '1');  // Convert number to 0-7, flipped to have 1 at bottom
    pos->y = line[0] - 'a';        // Convert letter to 0-7 for columns
    linenoiseFree(line);
    return true;
}

Move_t get_user_move()
{
    // TODO: add check for switching players if the move is valid
    if (move_count % 2 == 0) {
        printf("White's turn\n");
    } else {
        printf("Black's turn\n");
    }

    Position_t start, end;

    do {
        if (!handle_user_input(&start, "Enter the starting position (e.g., 'e2'): ")) {
            continue;
        }
        if (board[start.x][start.y] == ' ') {
            printf("Error: No piece at that position\n");
            continue;
        }
        break;
    } while (1);
    printf("start: %c%d idx: %d, %d piece: %c\n", 'a' + start.y, 8 - start.x, start.x, start.y, board[start.x][start.y]);

    // Show the available moves for the starting position
    Position_t moves[32];
    int move_count = get_available_moves(start.x, start.y, moves);
    printf("Available moves: ");
    // Print all non-empty moves
    for (int i = 0; i < move_count; i++) {
        printf("%c%d, ", 'a' + moves[i].y, 8 - moves[i].x);
    }
    printf("\n");

    do {
        if (!handle_user_input(&end, "Enter the ending position (e.g., 'e4'): ")) {
            continue;
        }
        break;
    } while (1);
    printf("end: %c%d idx: %d, %d piece: %c\n", 'a' + end.y, 8 - end.x, end.x, end.y, board[end.x][end.y]);
    if (!is_valid_move(end, moves)) {
        printf("Invalid move\n");
        return get_user_move();
    }

    // TODO: add a check for end game checkmate etc.

    return (Move_t) {
        start, end
    };
}

void add_move(Move_t move)
{
    // Add the move to the move list and update the board
    move_list[move_count] = move;
    move_count++;
    printf("moving %c from %d%d to %d%d\n", board[move.start.x][move.start.y], move.start.y, move.start.x, move.end.y, move.end.x);
    board[move.end.x][move.end.y] = board[move.start.x][move.start.y];
    board[move.start.x][move.start.y] = ' ';
}

void setup_console()
{
    // Setup console
    // TODO: add a setup for UART as well based on kconfig
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    esp_console_dev_usb_serial_jtag_config_t usbjtag_config = ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_console_new_repl_usb_serial_jtag(&usbjtag_config, &repl_config, &repl));
}

int app_main(int argc, char *argv[])
{
    setup_console();
    init_board();
    print_board();
    while (1) {
        Move_t move = get_user_move();
        add_move(move);
        print_board();
    }
}
