# Chat-Application
Chat application for a computer room that allows communication between devices in the room

## Features:
 - Communication between devices using sockets
 - View sent and received messages
 - Store conversation history in an embedded database

## How to use:

### Server side: 
Having opened a Terminal window from the project folder, type in the following command to build the server
```
g++ mserver.cpp -l sqlite3
```
Then, type in the following command to run the server
```
./a.out
```
### Client side:
Having opened a Terminal window from the project folder, type in the following to build the client
```
g++ mclient.cpp
```
Then, type in the following command to run the client
```
./mclient
```

## Notes:
- This project was developed and tested in macOS. Theoretically, it should work for any UNIX OS as the libraries and the socket definitions are relatively the same. For Windows, the code would have to be re-writen, specifically the sections related to the sockets operation.
- For the embedded database, SQLite was used. If necessary, it should be downloaded and installed on the server's device.
