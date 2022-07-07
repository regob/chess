///
///  server.hpp
///

#ifndef SERVER_HPP
#define SERVER_HPP

#include "../message.hpp"

// a szerver szolgáltatás default portja
#define PORT "25000"

// a poll() hívás timeout ideje ms-ban a kliensek kezelésekor
#define POLL_WAIT 300

// a szerver felé, és a kifelé irányuló üzenetek FIFO tárolója
extern MessageQueue in_msg, out_msg;

// elindítja a szerver futását
void start_server(const char*);

// a kliens socketeket figyelő, és olvasó szálfüggvény
void* handle_clients(void*);

#endif
