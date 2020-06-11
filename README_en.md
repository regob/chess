# Client-server chess application
[_in Hungarian_](./README.md)

# Description
The task is to create a chess application with a client-server architecture under Linux.

The functionalities of the server:
- store registered users
- handle multiple concurrent games
- store game states, and validate the moves
- search for random opponent

functionalities of the client:
- connect to, and communicate with the server over TCP connection
- display the game, and UI

## The network protocol
The messages: (all of them end in '\n')
| Sender | Content | Description |
|:-----:|:---:|:---:|
|client| REGISTER \<name\> \<password\> | register a new user |
|client| LOGIN \<name\> \<password\> | login as a user |
|client| START | start a new game against a random opponent |
|client| MOVE \<x\> \<y\> | move with a piece on the board |
|server| OK | the client's message was successful|
|server| ERROR \<error message\> | reply with an error to the client's message |
|server| GAME \<white\> \<black\> \<time_white\> \<time_black\> \<result\> | Game status update, also this reports the end of  a game|
|server| MOVED \<x\> \<y\> | the other player's move |

The server replies with an OK, or an ERROR message on each message of the client. When a game is running, a GAME message is sent periodically, and a MOVED message when the opponent made a legal move.

# Dependencies
GNU make, g++, qmake (qt5-qmake, qmake-qt5), Qt5 library (with headers)

# Run
To compile the project run the **make** command at the root directory of the project. The **server/bin/server** executable runs the server application. It needs a write permission on the current working directory. The client application is located at **client/bin/client**.

**URL of the video presenting the application (unfortunately in Hungarian):**
https://youtu.be/HGdWDMJ7Pv0
