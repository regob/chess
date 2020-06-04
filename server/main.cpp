///
///  main.cpp
///  A szerver indítását végzi.
///

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include "server.hpp"

int main(int argc, char* argv[]){

    // daemonként fut, a jelenlegi könyvtárat megtartja, mert ott hozza létre az adatfájlt
    if(daemon(1, 0) < 0){
        perror("daemon");
        return EXIT_FAILURE;
    }

    openlog("chess_srv", LOG_PID, LOG_DAEMON);

    start_server();
}
