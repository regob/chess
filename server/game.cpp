///
///  game.cpp
///  Game osztály implementációja.
///

#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "game.hpp"
#include "server.hpp"
#include "../message.hpp"


std::map<User*, Game*> user_game;
User *queued_user = NULL;
pthread_mutex_t game_mutex = PTHREAD_MUTEX_INITIALIZER;

Game::Game(User* _white, User *_black, int max_time)
    :white(_white), black(_black), time_w(max_time), time_b(max_time)
{
    current_player = WHITE;
    pthread_mutex_init(&mutex, NULL);
    game_ended = 0;
    board = new Board();
}

Game::~Game(){
    if(board) delete board;
    
    if(pthread_mutex_destroy(&mutex)){
        syslog(LOG_ERR, "Game::mutex_destroy: %s", strerror(errno));
    }
}

// elindítja a játékot új szálon
void Game::start(){
    pthread_t game_thread;
    if(pthread_create(&game_thread, NULL, Game::start_game, (void*)this)){
        syslog(LOG_ERR, "Game::start: %s", strerror(errno));
        return;
    }

    // háttérszálat csinálunk a szálból, így nem kell majd joinnal megvárni ha végez
    if(pthread_detach(game_thread)){
        syslog(LOG_ERR, "Game::start, detach: %s", strerror(errno));
    }
}

// A statikus szál függvény. Ezt új szálon futtatva elindul a paraméterként kapott játék 
void* Game::start_game(void *data){
    Game *g = (Game*)data;
    g->start_run();
    pthread_exit(0);
}

// A játék futását kezelő privát függvény, ezt külön szálon kell futtatni
void Game::start_run(){
    
    // szétküldi a játék indulását a felhasználóknak
    out_msg.push(new GameMessage(white->name, black->name, time_w, time_b, game_ended));
    
    struct timespec tick, rem;
    tick.tv_sec = 0;
    tick.tv_nsec = TICK_ns;

    int result;
    int tick_counter = 0; 
    while(1){
        result = nanosleep(&tick, &rem);
        while(result){
            syslog(LOG_INFO, "nanosleep interrupted, remaining time : %ld", rem.tv_nsec);
            if(errno == EINTR) result = nanosleep(&rem, &rem);
            else {
                syslog(LOG_ERR, "Game::start_run, nanosleep: %s", strerror(errno));
                pthread_exit(0);
            }
        }

        pthread_mutex_lock(&mutex);

        // egy TICK idő eltelt, csökken a játékos ideje
        if(current_player & WHITE) time_w -= TICK_ms;
        else time_b -= TICK_ms;

        // ha még nincs vége, és lejárt az idő valamelyik játékosnak, akkor veszített
        if(!game_ended){
            if(time_w <= 0) game_ended |= BLACK_WON;
            if(time_b <= 0) game_ended |= WHITE_WON;
        }
        
        if(game_ended){
            pthread_mutex_lock(&game_mutex);
            user_game.erase(white);
            user_game.erase(black);
            pthread_mutex_unlock(&game_mutex);
            pthread_mutex_unlock(&mutex);
            goto suicide;
        }

        pthread_mutex_unlock(&mutex);

        // ha kb. eltelt egy MESSAGE_PERIOD, akkor frissítő üzenetet küld a játékosoknak
        if((++tick_counter % (MESSAGE_PERIOD_ms/TICK_ms)) == 0)
            out_msg.push(new GameMessage(white->name, black->name,
                                         time_w, time_b, game_ended));
    }

// ide akkor ugrik a függvény, ha vége a játéknak.
// itt várunk még (30 sec-et), hogy a szerveren belül elkésett üzenetek ne törölt objektumra
// referáljanak. 1-2 ms is elég lenne, mert az új üzenetek már úgy látják, hogy nem létezik a játék.
suicide:
    out_msg.push(new GameMessage(white->name, black->name, time_w, time_b, game_ended));
    tick.tv_sec = 30;
    result = nanosleep(&tick, &rem);
    while(result){
        if(errno == EINTR) result = nanosleep(&rem, &rem);
        else {
            syslog(LOG_ERR, "Game::start_run, nanosleep: %s", strerror(errno));
            pthread_exit(0);
        }
    }
    
    // jatek vege, objektum torlese, itt már nem létezhet referencia az objektumra máshol
    delete this;
}

// A játékosok lépését kezeli
void Game::move(User *user, int from, int to){
    pthread_mutex_lock(&mutex);

    if(game_ended){
        pthread_mutex_unlock(&mutex);
        out_msg.push(new ErrorMessage("Game has ended.", (void*) user));
        return;
    }
    
    int user_color = 0;
    if(user == white) user_color = WHITE;
    else if(user == black) user_color = BLACK;
    else {
        pthread_mutex_unlock(&mutex);
        out_msg.push(new ErrorMessage("Not your game.", (void*) user));
        return;
    }

    if((user_color & current_player) == 0){
        pthread_mutex_unlock(&mutex);
        out_msg.push(new ErrorMessage("Not your turn.", (void*) user));
        return;
    }

    // a lépés átváltása [0,63] intervallumról {x,y} tábla koordinátákba
    int ax = from/8;
    int ay = from % 8;
    int bx = to/8;
    int by = to % 8;

    if(ax < 0 || ax >= 8 || ay < 0 || ay >= 8 || bx < 0 || bx >= 8 || by < 0 || by >= 8){
        pthread_mutex_unlock(&mutex);
        out_msg.push(new ErrorMessage("Invalid coordinates.", (void*) user));
        return;
    }

    if(!(board->board[ax][ay] & current_player)){
        pthread_mutex_unlock(&mutex);
        out_msg.push(new ErrorMessage("Not your piece.", (void*) user));
        return;
    }

    if(!board->valid_move(ax, ay, bx, by)){
        pthread_mutex_unlock(&mutex);
        out_msg.push(new ErrorMessage("Invalid move.", (void*) user));
        return;
    }

    // a lépés itt már biztosan szabályos
    board->move_piece(ax, ay, bx, by);

    // elküldjük az értesítő üzeneteket
    out_msg.push(new MovedMessage(from, to, (user == white) ? black : white));
    out_msg.push(new OKMessage((void*) ((user == white) ? white : black)));

    // jóváíródik a plusz idő
    if(current_player & WHITE) time_w += INCREMENT_ms;
    else time_b += INCREMENT_ms;

    // ellenkező játékos jön
    current_player ^= (BLACK | WHITE);
    
    // meg kell nézni vége-e a játéknak
    game_ended = board->game_state(current_player);
    pthread_mutex_unlock(&mutex);
}

