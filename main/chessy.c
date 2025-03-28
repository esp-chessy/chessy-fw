#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "board.h"
#include "moves.h"

#define HALL_COL_SW1 GPIO_NUM_39
#define HALL_COL_SW2 GPIO_NUM_40
#define HALL_COL_SW3 GPIO_NUM_41
#define HALL_COL_SW4 GPIO_NUM_42
#define HALL_COL_SW5 GPIO_NUM_4
#define HALL_COL_SW6 GPIO_NUM_5
#define HALL_COL_SW7 GPIO_NUM_6
#define HALL_COL_SW8 GPIO_NUM_7
#define HALL_ROW1 GPIO_NUM_14
#define HALL_ROW2 GPIO_NUM_13
#define HALL_ROW3 GPIO_NUM_21
#define HALL_ROW4 GPIO_NUM_8
#define HALL_ROW5 GPIO_NUM_15
#define HALL_ROW6 GPIO_NUM_16
#define HALL_ROW7 GPIO_NUM_17
#define HALL_ROW8 GPIO_NUM_18
#define LED_DATA_IN1 GPIO_NUM_38

#define COL_NUM 8
#define ROW_NUM 8
#define LED_DELAY_MS 500

static led_strip_handle_t led_strip;
static const char *TAG = "CHESSY";

// LED colors for different states
#define COLOR_EMPTY 0x000000    // Black
#define COLOR_SELECTED 0xFFFF00  // Yellow
#define COLOR_VALID_MOVE 0x00FF00 // Green
#define COLOR_INVALID_MOVE 0xFF0000 // Red
#define COLOR_WHITE_PIECE 0xFFFFFF  // White
#define COLOR_BLACK_PIECE 0x808080  // Gray
#define COLOR_ERROR 0xFF0000    // Red for errors

static uint8_t led_get(int x, int y)
{
    // Leds are in zigzag pattern
    if (y % 2 == 0) {
        return y * 8 + x;
    } else {
        return y * 8 + (7 - x);
    }
}

static void led_set(int x, int y, uint32_t color)
{
    uint32_t red = (color >> 16) & 0xFF;
    uint32_t green = (color >> 8) & 0xFF;
    uint32_t blue = color & 0xFF;
    led_strip_set_pixel(led_strip, led_get(x, y), red, green, blue);
}

static void led_clear(void)
{
    led_strip_clear(led_strip);
    led_strip_refresh(led_strip);
}

static void led_refresh(void)
{
    led_strip_refresh(led_strip);
}


static void configure_led(void)
{
    ESP_LOGI(TAG, "Configuring LED strip");
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_DATA_IN1,
        .max_leds = 64,
    };
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_clear();
}

void hall_init(void)
{
    ESP_LOGI(TAG, "Initializing hall effect sensors");
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL << HALL_COL_SW1) | (1ULL << HALL_COL_SW2) | 
                       (1ULL << HALL_COL_SW3) | (1ULL << HALL_COL_SW4) |
                       (1ULL << HALL_COL_SW5) | (1ULL << HALL_COL_SW6) |
                       (1ULL << HALL_COL_SW7) | (1ULL << HALL_COL_SW8),
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << HALL_ROW1) | (1ULL << HALL_ROW2) |
                          (1ULL << HALL_ROW3) | (1ULL << HALL_ROW4) |
                          (1ULL << HALL_ROW5) | (1ULL << HALL_ROW6) |
                          (1ULL << HALL_ROW7) | (1ULL << HALL_ROW8);
    ESP_ERROR_CHECK(gpio_config(&io_conf));
}

// Read hall effect sensors
void hall_read(uint8_t matrix[ROW_NUM][COL_NUM])
{
    static uint8_t col_pins[] = {
        HALL_COL_SW1, HALL_COL_SW2, HALL_COL_SW3, HALL_COL_SW4,
        HALL_COL_SW5, HALL_COL_SW6, HALL_COL_SW7, HALL_COL_SW8
    };
    
    static uint8_t row_pins[] = {
        HALL_ROW1, HALL_ROW2, HALL_ROW3, HALL_ROW4,
        HALL_ROW5, HALL_ROW6, HALL_ROW7, HALL_ROW8
    };

    // Clear matrix first
    memset(matrix, 0, ROW_NUM * COL_NUM);

    // Scan each column
    for (int col = 0; col < COL_NUM; col++) {
        gpio_set_level(col_pins[col], 1);
        vTaskDelay(pdMS_TO_TICKS(10));
        
        // Read each row
        for (int row = 0; row < ROW_NUM; row++) {
            matrix[row][col] = gpio_get_level(row_pins[row]);
        }
        
        gpio_set_level(col_pins[col], 0);
    }
}

// Verify that the physical board matches the expected state
static bool verify_board_state(const char board[8][8], const uint8_t matrix[ROW_NUM][COL_NUM])
{
    for (int row = 0; row < ROW_NUM; row++) {
        for (int col = 0; col < COL_NUM; col++) {
            // If there's a piece in the board array, there should be a magnet detected
            if (board[row][col] != ' ' && !matrix[row][col]) {
                printf("Error: Missing piece at %c%d\n", 'a' + col, 8 - row);
                led_set(col, row, COLOR_ERROR);
                led_refresh();
                return false;
            }
            // If there's no piece in the board array, there should be no magnet detected
            if (board[row][col] == ' ' && matrix[row][col]) {
                printf("Error: Extra piece at %c%d\n", 'a' + col, 8 - row);
                led_set(col, row, COLOR_ERROR);
                led_refresh();
                return false;
            }
        }
    }
    return true;
}

// Update LED display based on current board state and selected piece
static void update_led_display(const char board[8][8], const uint8_t matrix[ROW_NUM][COL_NUM], 
                             const Position_t *selected_pos,
                             const Position_t *valid_moves,
                             int valid_move_count)
{
    led_clear();

    // First, show all pieces on the board
    for (int row = 0; row < ROW_NUM; row++) {
        for (int col = 0; col < COL_NUM; col++) {
            if (matrix[row][col]) {
                char piece = board[row][col];
                uint32_t color = (piece >= 'a' && piece <= 'z') ? 
                               COLOR_BLACK_PIECE : COLOR_WHITE_PIECE;
                led_set(col, row, color);
            }
        }
    }

    // Highlight selected piece
    if (selected_pos) {
        led_set(selected_pos->y, selected_pos->x, COLOR_SELECTED);
    }

    // Show valid moves
    for (int i = 0; i < valid_move_count; i++) {
        led_set(valid_moves[i].y, valid_moves[i].x, COLOR_VALID_MOVE);
    }

    led_refresh();
}

// Returns true if a piece movement was detected and stores the position in pos
bool detect_piece_movement(Position_t *pos)
{
    static uint8_t prev_matrix[ROW_NUM][COL_NUM] = {0};
    uint8_t curr_matrix[ROW_NUM][COL_NUM] = {0};
    bool movement_detected = false;

    // Read current state
    hall_read(curr_matrix);

    // Compare with previous state to find piece movement
    for (int i = 0; i < ROW_NUM; i++) {
        for (int j = 0; j < COL_NUM; j++) {
            if (prev_matrix[i][j] != curr_matrix[i][j]) {
                pos->x = i;
                pos->y = j;
                movement_detected = true;
                break;
            }
        }
        if (movement_detected) break;
    }

    // Update previous state
    memcpy(prev_matrix, curr_matrix, sizeof(curr_matrix));

    return movement_detected;
}

Move_t get_user_move(const char board[8][8])
{
    Position_t start = {-1, -1}, end = {-1, -1};
    Position_t valid_moves[32];
    int valid_move_count = 0;
    bool move_completed = false;
    Position_t current_pos;

    while (!move_completed) {
        // Wait for piece selection
        while (!detect_piece_movement(&current_pos)) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        start = current_pos;

        // Check if the move is valid
        if (board[start.x][start.y] == ' ') {
            printf("Error: No piece at that position\n");
            start.x = start.y = -1;
            continue;
        }

        // Get and display valid moves
        valid_move_count = get_available_moves(board, start.x, start.y, valid_moves);
        uint8_t curr_matrix[ROW_NUM][COL_NUM];
        hall_read(curr_matrix);
        update_led_display(board, curr_matrix, &start, valid_moves, valid_move_count);

        // Wait for move completion
        while (!detect_piece_movement(&current_pos)) {
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        end = current_pos;

        // Validate the move
        if (is_valid_move(end, valid_moves, valid_move_count)) {
            move_completed = true;
            // Show success feedback
            led_set(end.y, end.x, COLOR_VALID_MOVE);
            led_refresh();
            vTaskDelay(pdMS_TO_TICKS(LED_DELAY_MS));
        } else {
            printf("Invalid move\n");
            // Show error feedback
            led_set(end.y, end.x, COLOR_INVALID_MOVE);
            led_refresh();
            vTaskDelay(pdMS_TO_TICKS(LED_DELAY_MS));
            start.x = start.y = -1;
        }
    }

    return (Move_t) { start, end };
}

Move_t move_list[100];  // TODO: make this dynamic
unsigned int move_count = 0;

void print_move_list()
{
    for (int i = 0; i < move_count; i++) {
        printf("%d. %c%d->%c%d\n", i + 1, 'a' + move_list[i].start.y, 8 - move_list[i].start.x, 'a' + move_list[i].end.y, 8 - move_list[i].end.x);
    }
}

void add_move(char board[8][8], Move_t move)
{
    // Add the move to the move list and update the board
    move_list[move_count] = move;
    move_count++;
    printf("moving %c from %d%d to %d%d\n", board[move.start.x][move.start.y], move.start.y, move.start.x, move.end.y, move.end.x);
    board[move.end.x][move.end.y] = board[move.start.x][move.start.y];
    board[move.start.x][move.start.y] = ' ';
}

int app_main(int argc, char *argv[])
{
    // Initialize hardware
    hall_init();
    configure_led();
    
    // Initialize game
    char board[8][8];
    init_board(board);
    print_board(board);

    // Wait for initial board setup
    uint8_t matrix[ROW_NUM][COL_NUM];
    bool board_ready = false;

    while (!board_ready) {
        printf("Please set up the board according to the displayed state...\n");
        hall_read(matrix);
        
        if (verify_board_state(board, matrix)) {
            board_ready = true;
            printf("Board setup verified!\n");
            led_clear();
        } else {
            printf("Board setup incorrect. Please fix the highlighted positions.\n");
            vTaskDelay(pdMS_TO_TICKS(LED_DELAY_MS));
        }
    }

    while (1) {
        // Get and process the move
        Move_t move = get_user_move(board);
        add_move(board, move);
        print_board(board);
    }
}
