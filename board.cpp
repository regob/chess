#include <string.h>
#include <vector>
#include <utility>
#include "board.hpp"

#include <stdio.h>

// pálya inicializálása a kezdőállapottal
Board::Board(){
    memset((void*)board, 0, sizeof(board));
    for(int i = 0; i < 2; i++)
        for(int j = 0; j < 8; j++){
            board[i][j] = BLACK;
            board[7-i][j] = WHITE;
        }

    for(int i = 0; i < 8; i++){
        board[1][i] |= PAWN;
        board[6][i] |= PAWN;
    }

    int elso_sor[8] = {ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK};

    for(int i = 0; i < 8; i++){
        board[0][i] |= elso_sor[i];
        board[7][i] |= elso_sor[i];
    }

    last_move_from = {-1,-1};
    last_move_to = {-1,-1};

    memset((void*)moved, 0, sizeof(moved));
}


// huszár lehetséges lépései
const vector<pair<int, int>> knight_move = {{-2,-1}, {-2,1}, {-1,-2}, {-1,2},
                                            {1,-2}, {1,2}, {2,-1}, {2,1}};

// fekete gyalog lépések 
const vector<pair<int,int>> black_pawn_move = {{1,0}, {1,-1}, {1, 1}};

// fehér gyalog lépések 
const vector<pair<int,int>> white_pawn_move = {{-1,0}, {-1,-1}, {-1, 1}};

// a többi bábu lépésének lehetséges irányai:

const vector<pair<int,int>> queen_direction = {{-1,1}, {-1,-1}, {1,1}, {1,-1},
                                             {0,1}, {0,-1}, {-1,0}, {1,0}};

const vector<pair<int,int>> rook_direction = {{0,1}, {0,-1}, {-1,0}, {1,0}};
const vector<pair<int,int>> bishop_direction = {{-1,1}, {-1,-1}, {1,1}, {1,-1}};



// az {ax, ay} figurát lehetséges-e {bx, by}-ra léptetni
bool Board::can_move(int ax, int ay, int bx, int by){
    if(ax < 0 || ax > 7 || bx < 0 || bx > 7 ||
       ay < 0 || ay > 7 || by < 0 || by > 7)
        return false;

    // ugyanolyan színű bábut biztosan nem üthet le ...
    if((board[ax][ay] & (WHITE | BLACK)) & (board[bx][by] & (WHITE | BLACK)))
        return false;

    // nem is áll ott semmi 
    if(board[ax][ay] == 0) return false;

    int color = board[ax][ay] & (WHITE | BLACK);
    int color_enemy = color ^ (WHITE | BLACK);
    
    if(board[ax][ay] & KING){
        for(pair<int,int> p : queen_direction){
            if((ax + p.first == bx) && (ay + p.second == by)) return true;
        }

        // sáncolás is lehetséges lépés, ha a király, és a bástya sem lépett még
        
        if((board[ax][ay] & WHITE) && (ax != 7 || ay != 4)) return false;
        if((board[ax][ay] & BLACK) && (ax != 0 || ay != 4)) return false;
        if(bx != ax) return false;
        int rook_place;

        if(by == 2) rook_place = 0;
        else if(by == 6) rook_place = 7;
        else return false;
        
        // a király, és a bástya között nem állhat bábu, és sakkban sem lehet
        if(in_check(ax, ay, color_enemy)) return false;
        if(rook_place == 0)
            for(int j = ay - 1; j > 0; j--){
                if(board[ax][j] || in_check(ax, j, color_enemy)) return false;
            }
        if(rook_place == 7)
            for(int j = ay + 1; j < 7; j++){
                if(board[ax][j] || in_check(ax, j, color_enemy)) return false;
            }

        // már csak azt kell megnézni, a bástya helyén van-e és nem lépett-e még

        if(!(board[ax][rook_place] & board[ax][ay])) return false; // nem egyforma színűek
        if(moved[ax][rook_place]) return false; // már lépett
        if(!(board[ax][rook_place] & ROOK)) return false; // nem bástya áll ott
            
        return true;
    }

    if(board[ax][ay] & PAWN){
        const vector<pair<int,int>> *steps;
        if(board[ax][ay] & WHITE) steps = &white_pawn_move;
        else steps = &black_pawn_move;

        // előre lépés
        if(ax + (*steps)[0].first == bx && ay + (*steps)[0].second == by){
            return (!(board[bx][by])); // akkor léphet, ha nem áll ott semmi
        }

        // dupla előre lépés kezdéskor
        if(ax + 2*(*steps)[0].first == bx && ay + (*steps)[0].second == by){

            // nem szabad hogy előtte levő két mező foglalt legyen, és hogy már lépett előtte
            return(!moved[ax][ay] && !(board[bx][by]) &&
                   !board[ax + (*steps)[0].first][by]);
        }

        // átlósan akkor léphet, ha ellenséges figura áll ott
        for(unsigned i = 1; i < steps->size(); i++){
            if(ax + (*steps)[i].first == bx && ay + (*steps)[i].second == by){
                if((board[bx][by] & (WHITE | BLACK)) & (board[ax][ay] & (WHITE | BLACK)))
                    return false;

                // itt már vagy ellenséges, vagy üres a mező.

                // Üres mező esetén is lehet en-passant lépés
                if(!(board[bx][by])){
                    if(last_move_from.second != by) return false;
                    if(board[ax][ay] & WHITE){
                        if(ax != 3 || last_move_from.first != 1 ||
                           last_move_to.first != 3 || last_move_to.second != by) return false;
                        return ((board[last_move_to.first][last_move_to.second] & (BLACK | PAWN)) == (BLACK | PAWN));
                    } else {
                        if(ax != 4 || last_move_from.first != 6 ||
                           last_move_to.first != 4 || last_move_to.second != by) return false;
                        return ((board[last_move_to.first][last_move_to.second] & (WHITE | PAWN)) == (WHITE | PAWN));
                    }
                }

                return true;
            }
        }

        return false;
    }

    
    if(board[ax][ay] & KNIGHT){
        for(pair<int,int> p : knight_move){
            if(ax + p.first == bx && ay + p.second == by){
                return !((board[ax][ay] & (WHITE | BLACK)) & (board[bx][by] & (WHITE | BLACK)));
            }
        }
        return false;
    }

    // a maradék a QUEEN, ROOK, BISHOP.
    // QUEEN mind a 8 irányba léphet, BISHOP az első 4-be (átlósan),
    // ROOK pedig az utolsó 4 irányba
    const vector<pair<int,int>> *dir;
    if(board[ax][ay] & ROOK) dir = &rook_direction;
    else if(board[ax][ay] & BISHOP) dir = &bishop_direction;
    else dir = &queen_direction;
    
    for(pair<int,int> p : *dir){
        int i = ax + p.first;
        int j = ay + p.second;
        while(i >= 0 && i < 8 && j >= 0 && j < 8){
            // ha ugyanolyan színű bábu áll a mezőn, ebben az irányban már nem jó
            if((board[ax][ay] & (WHITE | BLACK)) & (board[i][j] & (WHITE | BLACK))) break;

            // ha elértük a célmezőt, akkor jó a lépés
            if(bx == i && by == j) return true;

            // ha állt valami a célmezőn, nem mehetünk tovább ebbe az irányba
            if(board[i][j]) break;

            i += p.first;
            j += p.second;
        }
    }

    return false;
}

// az {ax, ay} mezőt sakkban tartja-e a "player" színű játékos
bool Board::in_check(int ax, int ay, int player){
    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++){
            if(!(board[i][j] & player)) continue; // nem a megfelelő színű a bábu
            if(can_move(i, j, ax, ay)) return true; 
        }
    return false;
}


// {ax,ay} -ról {bx,by} -ra lép a bábu.
void Board::move_piece(int ax, int ay, int bx, int by){
    if(en_passant(ax, ay, bx, by)){
        if(board[ax][ay] & BLACK) board[bx - 1][by] = 0;
        else board[bx + 1][by] = 0;
    }
    else if(is_castling(ax, ay, bx, by)){
        if(by < ay){
            board[ax][by + 1] = board[ax][0];
            board[ax][0] = 0;
        } else {
            board[ax][by - 1] = board[ax][7];
            board[ax][7] = 0;
        }
    }

    moved[ax][ay] = true;
    moved[bx][by] = true;
    board[bx][by] = board[ax][ay];
    board[ax][ay] = 0;

    // ha a gyalog beért az alapvonalra, átváltozik
    if(board[bx][by] & PAWN){
        if((board[bx][by] & WHITE && bx == 0) || (board[bx][by] & BLACK && bx == 7)){
            board[bx][by] ^= PAWN | QUEEN;
        }
    }
    
    last_move_from = {ax, ay};
    last_move_to = {bx, by};
}

// megnézi a lépés en-passant-e
bool Board::en_passant(int ax, int ay, int bx, int by){
    return (board[ax][ay] & PAWN && (board[bx][by] == 0) &&
            (ax != bx) && (ay != by));
}

// megnézi a lépés sáncolás-e
bool Board::is_castling(int ax, int ay, int bx, int by){
    return(board[ax][ay] & KING && (ay - by > 1 || ay - by < -1));
}


// az {ax, ay} -> {bx, by} lépés szabályos-e
bool Board::valid_move(int ax, int ay, int bx, int by){

    // ha a bábu nem képes oda lépni biztosan nem jó
    if(!can_move(ax, ay, bx, by)) return false;

    // elmentjük a lépés előtti állapotot, hogy vissza lehessen majd állítani
    int saved = board[bx][by];
    int moved_from_saved = moved[ax][ay];
    int moved_to_saved = moved[bx][by];
    pair<int,int> saved_from = last_move_from;
    pair<int,int> saved_to = last_move_to;

    int color = board[ax][ay] & (WHITE | BLACK);
    int enemy_color = color ^ (WHITE | BLACK);

    // a speciális lépéseket megvizsgáljuk, hogy vissza lehessen állítani
    bool castling = is_castling(ax, ay, bx, by);
    bool enpassant = en_passant(ax, ay, bx, by);
    bool pawn_to_queen = (board[ax][ay] & PAWN && (color & WHITE ? bx == 0 : bx == 7));

    // elvégezzük a lépést
    move_piece(ax, ay, bx, by);

    // megkeressük a játékos királyát
    int king_x, king_y;
    for(king_x = 0; king_x < 8; king_x++)
        for(king_y = 0; king_y < 8; king_y++)
            if((board[king_x][king_y] & KING) && (board[king_x][king_y] & color)) goto found;

found:

    // ha a lépő játékos királya sakkban van akkor nem szabályos
    bool valid = !in_check(king_x, king_y, enemy_color);

    // vissza kell állítani az állapotot
    board[ax][ay] = board[bx][by];
    board[bx][by] = saved;
    moved[ax][ay] = moved_from_saved;
    moved[bx][by] = moved_to_saved;
    last_move_from = saved_from;
    last_move_to = saved_to;

    if(castling){
        if(by > ay){
            board[ax][7] = board[bx][by - 1];
            board[bx][by - 1] = 0;
        } else {
            board[ax][0] = board[bx][by + 1];
            board[bx][by + 1] = 0;
        }
    }

    if(enpassant){
        if(color & WHITE) board[bx + 1][by] = BLACK | PAWN;
        else board[bx - 1][by] = WHITE | PAWN;
    }

    if(pawn_to_queen) board[ax][ay] ^= PAWN | QUEEN;

    return valid;
}

// megnézi vége-e a játéknak, ha a "color" van soron
int Board::game_state(int color){
    
    // ha csak királyok maradtak, döntetlen
    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++)
            if(board[i][j] && !(board[i][j] & KING)) goto not_tie;
    return TIE;
    
not_tie:
    
    for(int i = 0; i < 8; i++)
        for(int j = 0; j < 8; j++)
            if(board[i][j] & color) 
                for(int ci = 0; ci < 8; ci++)
                    for(int cj = 0; cj < 8; cj++)
                        if(valid_move(i, j, ci, cj))
                            return 0; // létezik lépés
                        
    // ha a playernek nincs lépése: játék vége, és az eredmény attól függ sakkban áll-e
    // meg kell keresni a királyt
    int king_x, king_y;
    for(king_x = 0; king_x < 8; king_x++)
        for(king_y = 0; king_y < 8; king_y++)
            if(board[king_x][king_y] & KING && board[king_x][king_y] & color) goto found;

found:

    // megvizsgáljuk sakkban van-e a király
    bool checkmate = in_check(king_x, king_y, color ^ (WHITE | BLACK));

    if(checkmate){
        if(color & BLACK) return WHITE_WON;
        else return BLACK_WON;
    }

    // nincs lépés, és sakkban sincs, tehát döntetlen
    return TIE;
}



