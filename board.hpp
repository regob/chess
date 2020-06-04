#ifndef BOARD_HPP
#define BOARD_HPP
#include <vector>
#include <utility>

// figura, és játékos kódok a bitmapekhez
#define WHITE (1 << 0)
#define BLACK (1 << 1)
#define PAWN (1 << 2)
#define BISHOP (1 << 3)
#define KNIGHT (1 << 4)
#define ROOK (1 << 5)
#define QUEEN (1 << 6)
#define KING (1 << 7)

// játékállás bitek az állapot eldöntéséhez
#define TIE (1 << 8)
#define WHITE_WON (1 << 9)
#define BLACK_WON (1 << 10)

using std::pair;
using std::vector;

struct Board {
    pair<int, int> last_move_from;
    pair<int, int> last_move_to;

    bool moved[8][8];
    int board[8][8];

    Board();



    // az {ax, ay} mezőt sakkban tartja-e a "player" színű játékos
    bool in_check(int ax, int ay, int player);

    // {ax,ay} -ról {bx,by} -ra lép a bábu. 
    void move_piece(int ax, int ay, int bx, int by);

    // az {ax, ay} -> {bx, by} lépés szabályos-e
    bool valid_move(int ax, int ay, int bx, int by);

    // megnézi vége-e a játéknak, ha a "player" van soron
    int game_state(int player);

private:
    // az {ax, ay} figurát lehetséges-e {bx, by}-ra léptetni
    bool can_move(int ax, int ay, int bx, int by);

    // egy szabályos lépésről eldönti, en-passant lépés-e
    bool en_passant(int ax, int ay, int bx, int by);

    // egy szabályos lépésről eldönti, sáncolás-e
    bool is_castling(int ax, int ay, int bx, int by);
};


#endif
