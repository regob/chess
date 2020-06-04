///
///  game.hpp
///  Sakk játék memória struktúrája.
///

#ifndef GAME_HPP
#define GAME_HPP

#include <map>
#include <string.h>
#include <pthread.h>
#include "user.hpp"
#include "../board.hpp"

// a játék időmérésének egységei
// TICK időnként küld frissítést a szerver az állapotról
#define TICK_ms 100
#define TICK_ns TICK_ms*1000*1000

// egy játék hossza felhasználónként
#define GAME_LENGTH_ms 1000*60*5

// plusz idő minden lépés után
#define INCREMENT_ms 5000

// játékidő frissítő üzenetek periódusa, nagyobb egyenlő mint a TICK
#define MESSAGE_PERIOD_ms 1000 

// Két felhasználó közötti sakk játszma.
struct Game {
    User *white;
    User *black;

    // a hátralévő idő ms-ban mérve
    int time_w;
    int time_b;

    int current_player; // jelenlegi felhasználó, WHITE, vagy BLACK
    int game_ended; // játék végeredményét tárolja. Ha még nincs vége 0.

    Board *board;
    pthread_mutex_t mutex;

    Game(User *_white, User *_black, int max_time);
    ~Game();

    void start();
    static void* start_game(void*);
    void move(User *user, int from, int to);
private:
    void end();
    void start_run();
};


extern std::map<User*, Game*> user_game; // a felhasználóhoz tartozó játék megkeresésére szolgáló stuktúra
extern User *queued_user; // a jelenleg játékra várakozó játékos
extern pthread_mutex_t game_mutex; // az előző struktúrákat védő mutex

#endif
