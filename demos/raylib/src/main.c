#include "board.h"
#include "chess.h"
#include "square.h"
#include <raylib.h>
#include <stdint.h>
#include <stdio.h>

#define WINDOW_SIZE_HD 900

#define SQUARE_LIGHT ((Color){.r = 0xBF, .g = 0xC2, .b = 0xBD, .a = 0xFF})
#define SQUARE_DARK ((Color){.r = 0x43, .g = 0x57, .b = 0x47, .a = 0xFF})

#define DRAG_HIGHLIGHT ((Color){.r = 0xE8, .g = 0xFF, .b = 0xEF, .a = 0x62})
#define PICKUP_HIGHLIGHT ((Color){.r = 0xAA, .g = 0x90, .b = 0x6E, .a = 0xA0})
#define MOVE_HIGHLIGHT ((Color){.r = 0xA1, .g = 0x43, .b = 0x5E, .a = 0x80})

static Texture piece_textures[12] = {{0}};
static chess_t chess;
static square_t selection = {0xFF, 0xFF};

static square_t get_hovered_square(void);

/* Draw the checkerboard background */
static void draw_checkerboard(void) {
    float ppx = (float)GetScreenWidth() / 8.0f;
    float ppy = (float)GetScreenHeight() / 8.0f;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            int dark  = (x + y) % 2;
            Color col = dark ? SQUARE_DARK : SQUARE_LIGHT;

            DrawRectangle((int)(x * ppx),
                (int)(y * ppy),
                (int)((x + 1) * ppx) - (int)(x * ppx),
                (int)((y + 1) * ppy) - (int)(y * ppy),
                col);
        }
    }
}

/* Get a texture from a piece */
static Texture texture_from_piece(piece_t piece) {
    int color = PIECE_COLOR(piece);
    int type  = PIECE_TYPE(piece);

    if (type >= PIECE_TYPE_NONE) return (Texture){.width = 0};
    return piece_textures[type + (color == PIECE_COLOR_BLACK ? 6 : 0)];
}

/* Draw the pieces currently on the board */
static void draw_pieces(void) {
    float ppx = (float)GetScreenWidth() / 8.0f;
    float ppy = (float)GetScreenHeight() / 8.0f;

    for (int x = 0; x < 8; x++) {
        for (int y = 0; y < 8; y++) {
            if (x == selection.col && y == selection.row) continue;
            square_t square = (square_t){x, y};
            piece_t piece   = board_piece_at(&chess.board, square);
            Texture texture = texture_from_piece(piece);
            if (!IsTextureValid(texture)) continue;
            Rectangle source = (Rectangle){
                0, 0, (float)texture.width, (float)texture.height};
            Rectangle dest = {
                .x = x * ppx, .y = (7 - y) * ppy, .width = ppx, .height = ppy};
            Vector2 origin = (Vector2){0, 0};
            DrawTexturePro(texture, source, dest, origin, 0.f, WHITE);
        }
    }
}

/* Draw the currently selected square at the mouse */
static void draw_selection(void) {
    if (selection.col == 0xFF) return;
    piece_t piece   = board_piece_at(&chess.board, selection);
    Texture texture = texture_from_piece(piece);
    if (!IsTextureValid(texture)) return;

    float ppx   = (float)GetScreenWidth() / 8.0f;
    float ppy   = (float)GetScreenHeight() / 8.0f;
    Vector2 pos = GetMousePosition();

    Rectangle source = (Rectangle){
        0, 0, (float)texture.width, (float)texture.height};
    Rectangle dest = {.x = pos.x - ppx / 2.0f,
        .y               = pos.y - ppy / 2.0f,
        .width           = ppx,
        .height          = ppy};
    Vector2 origin = (Vector2){0, 0};
    DrawTexturePro(texture, source, dest, origin, 0.f, WHITE);
}

/* Draw highlights */
static void draw_highlights(void) {
    float ppx = (float)GetScreenWidth() / 8.0f;
    float ppy = (float)GetScreenHeight() / 8.0f;

    /* Highlight the selected square */
    if (selection.col != 0xFF) {
        DrawRectangle((int)(selection.col * ppx),
            (int)((7 - selection.row) * ppy),
            (int)((selection.col + 1) * ppx) - (int)(selection.col * ppx),
            (int)((8 - selection.row) * ppy) - (int)((7 - selection.row) * ppy),
            PICKUP_HIGHLIGHT);
    }

    /* Highlight legal moves of selected piece */
    if (selection.col != 0xFF) {
        for (size_t i = 0; i < chess.moves_len; i++) {
            move_t move = chess.moves[i];
            if (square_equals(move.from, selection)) {
                DrawRectangle((int)(move.to.col * ppx),
                    (int)((7 - move.to.row) * ppy),
                    (int)((move.to.col + 1) * ppx) - (int)(move.to.col * ppx),
                    (int)((8 - move.to.row) * ppy)
                        - (int)((7 - move.to.row) * ppy),
                    MOVE_HIGHLIGHT);
            }
        }
    }

    /* Outline hovered square while dragging */
    if (selection.col != 0xFF) {
        square_t hovered = get_hovered_square();
        if (hovered.col != 0xFF) {
            Rectangle rect = {.x = (float)((int)(hovered.col * ppx)),
                .y               = (float)((int)((7 - hovered.row) * ppy)),
                .width           = (float)((int)((hovered.col + 1) * ppx)
                                           - (int)(hovered.col * ppx)),
                .height          = (float)((int)((8 - hovered.row) * ppy)
                                           - (int)((7 - hovered.row) * ppy))};

            DrawRectangleLinesEx(rect, ppx / 20.0f, DRAG_HIGHLIGHT);
        }
    }

    /* TODO: Last move highlight */
    /* TODO: Check highlight */
}

/* Load piece textures */
static int load_piece_textures(void) {
    char buf[1024];

    for (int type = PIECE_TYPE_PAWN; type < PIECE_TYPE_NONE; type++) {
        piece_t white   = PIECE(PIECE_COLOR_WHITE, type);
        char char_white = piece_to_char(white);

        snprintf(buf, sizeof(buf), "textures/w%c.png", char_white);
        piece_textures[type] = LoadTexture(buf);

        snprintf(buf, sizeof(buf), "textures/b%c.png", char_white);
        piece_textures[type + 6] = LoadTexture(buf);

        if (!IsTextureValid(piece_textures[type])
            || !IsTextureValid(piece_textures[type + 6])) {
            for (int i = 0; i < type; i++) {
                UnloadTexture(piece_textures[i]);
                UnloadTexture(piece_textures[i + 6]);
            }
            if (IsTextureValid(piece_textures[type]))
                UnloadTexture(piece_textures[type]);
            if (IsTextureValid(piece_textures[type + 6]))
                UnloadTexture(piece_textures[type + 6]);
            return 1;
        }
    }

    return 0;
}

static square_t get_hovered_square(void) {
    float ppx   = (float)GetScreenWidth() / 8.0f;
    float ppy   = (float)GetScreenHeight() / 8.0f;
    Vector2 pos = GetMousePosition();

    if (pos.x < 0 || pos.x >= GetScreenWidth() || pos.y < 0
        || pos.y >= GetScreenHeight()) {
        return (square_t){0xFF, 0xFF};
    }

    return (square_t){
        .col = (uint8_t)(pos.x / ppx), .row = (uint8_t)(7 - (int)(pos.y / ppy))};
}

static void process_input(void) {
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        square_t hovered = get_hovered_square();

        if (hovered.col != 0xFF
            && PIECE_TYPE(board_piece_at(&chess.board, hovered))
                   != PIECE_TYPE_NONE) {
            selection = hovered;
        }
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        if (selection.col != 0xFF) {
            square_t destination = get_hovered_square();

            if (destination.col != 0xFF) {
                chess_make_move(
                    &chess, (move_t){selection, destination, PIECE_TYPE_NONE});
            }
        }

        selection = (square_t){0xFF, 0xFF};
    }
}

static void draw(void) {
    BeginDrawing();
    draw_checkerboard();
    draw_highlights();
    draw_pieces();
    draw_selection();
    EndDrawing();
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WINDOW_SIZE_HD, WINDOW_SIZE_HD, "Raylib chess demo");
    if (!IsWindowReady()) return 1;

    if (load_piece_textures()) {
        CloseWindow();
        return 1;
    }

    SetTargetFPS(60);

    /* Scale window size to display resolution */
    int mon         = GetCurrentMonitor();
    int monw        = GetMonitorWidth(mon);
    int monh        = GetMonitorHeight(mon);
    int max         = monw > monh ? monw : monh;
    int window_size = (int)((float)max / 1920.0f * (float)WINDOW_SIZE_HD);
    SetWindowSize(window_size, window_size);

    /* Create chess game */
    chess = chess_new();

    /* Draw loop */
    while (!WindowShouldClose()) {
        process_input();
        draw();
    }

    /* Cleanup */
    CloseWindow();
    return 0;
}
