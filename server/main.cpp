///
///  main.cpp
///  A szerver indítását végzi.
///

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include "server.hpp"



int main(int argc, char* argv[]){

    // egyetlen argument lehet, a portszám
    if (argc > 2) {
        printf("Too many arguments, the only (optional) argument is the port number to run on.\n");
        return 2;
    }

    const char *port = NULL;
    if (argc == 2) {
        port = argv[1];
        int port_number = atoi(port);
        if (strlen(port) > 6 || port_number <= 0 || port_number >= (1 << 16)){
            printf("Invalid port number provided.\n");
            return 3;
        }
    }
            

    // daemonként fut, a jelenlegi könyvtárat megtartja, mert ott hozza létre az adatfájlt
    if(daemon(1, 0) < 0){
        perror("daemon");
        return EXIT_FAILURE;
    }

    openlog("chess_srv", LOG_PID, LOG_DAEMON);

    start_server(port);
}
