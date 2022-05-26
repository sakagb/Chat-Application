#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <sqlite3.h>
#include <string> 
#include <sstream>
#include <iostream>
     
#define TRUE   1 
#define FALSE  0 
#define PORT 8888 

static int callback(void* data, int argc, char** argv, char** azColName)
{
    int i;
    fprintf(stderr, "%s: ", (const char*)data);
  
    for (i = 0; i < argc; i++) {
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    }
  
    printf("\n");
    return 0;
}

int main()  
{  
    int opt = TRUE;  
    int master_socket , addrlen , new_socket , client_socket[30] , max_clients = 30 , activity, i , valread , sd, outSock, count;  
    int max_sd;  
    struct sockaddr_in address;  
         
    char buffer[1025];  //data buffer of 1K 
         
    //set of socket descriptors 
    fd_set readfds;  
         
    //a message 
    std::string message = "Welcome to chat!\r\n";  

    // Database connection
    sqlite3* DB;
    int exit_sql = 0;
    exit_sql = sqlite3_open("chatapp.db", &DB);
  
    if (exit_sql) {
        std::cerr << "Error open DB " << sqlite3_errmsg(DB) << std::endl;
        return (-1);
    }
    else
        std::cout << "Opened Database Successfully!" << std::endl;
    sqlite3_close(DB);

    // Create Table
    std::string sql = "CREATE TABLE CHATS("
                    "ID INT PRIMARY KEY     NOT NULL, "
                    "USER           TEXT    NOT NULL, "
                    "MESSAGE          TEXT     NOT NULL);";
    exit_sql = sqlite3_open("chatapp.db", &DB);
    char* messaggeError;
    exit_sql = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
  
    if (exit_sql != SQLITE_OK) {
        std::cerr << "Error Create Table" << std::endl;
        sqlite3_free(messaggeError);
    }
    else
        std::cout << "Table created Successfully" << std::endl;
    sqlite3_close(DB);
     
    //initialise all client_socket[] to 0 so not checked 
    for (i = 0; i < max_clients; i++)  
    {  
        client_socket[i] = 0;  
    }  
         
    //create a master socket 
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)  
    {  
        perror("socket failed");  
        exit(EXIT_FAILURE);  
    }  
     
    //set master socket to allow multiple connections
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )  
    {  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    }  
     
    //type of socket created 
    address.sin_family = AF_INET;  
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons( PORT );  
         
    //bind the socket to localhost port 8888 
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)  
    {  
        perror("bind failed");  
        exit(EXIT_FAILURE);  
    }  
    printf("Listener on port %d \n", PORT);  
         
    //try to specify maximum of 3 pending connections for the master socket 
    if (listen(master_socket, 3) < 0)  
    {  
        perror("listen");  
        exit(EXIT_FAILURE);  
    }  
         
    //accept the incoming connection 
    addrlen = sizeof(address);  
    puts("Waiting for connections ...");  
         
    while(TRUE)  
    {  
        //clear the socket set 
        FD_ZERO(&readfds);  
     
        //add master socket to set 
        FD_SET(master_socket, &readfds);  
        max_sd = master_socket;  
             
        //add child sockets to set 
        for ( i = 0 ; i < max_clients ; i++)  
        {  
            //socket descriptor 
            sd = client_socket[i];  
                 
            //if valid socket descriptor then add to read list 
            if(sd > 0)  
                FD_SET( sd , &readfds);  
                 
            //highest file descriptor number, need it for the select function 
            if(sd > max_sd)  
                max_sd = sd;  
        }  
     
        //wait for an activity on one of the sockets , timeout is NULL , 
        //so wait indefinitely 
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  
       
        if ((activity < 0) && (errno!=EINTR))  
        {  
            printf("select error");  
        }  
             
        //If something happened on the master socket , 
        //then its an incoming connection 
        if (FD_ISSET(master_socket, &readfds))  
        {  
            if ((new_socket = accept(master_socket,(struct sockaddr *)&address, (socklen_t*)&addrlen))<0)  
            {  
                perror("accept");  
                exit(EXIT_FAILURE);  
            }  
             
            //inform user of socket number - used in send and receive commands 
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));  
           
            //send new connection greeting message 
            if( send(new_socket, message.c_str(), message.size(), 0) != message.size() )  
            {  
                perror("send");  
            }  
            
            // Broadcast a message informing about the connection
            for (int i = 0; i < max_clients; i++)
            {
                outSock = client_socket[i];  
                if (outSock != master_socket && outSock != new_socket)
                {
                    std::ostringstream smessage;
                    smessage << "USER #" << new_socket << " has entered the chat!\r\n";
                    std::string strOut = smessage.str();
                    send(outSock, strOut.c_str(), strOut.size(), 0); 
                } 
            }
     
            puts("Welcome message sent successfully");  
                 
            //add new socket to array of sockets 
            for (i = 0; i < max_clients; i++)  
            {  
                //if position is empty 
                if( client_socket[i] == 0 )  
                {  
                    client_socket[i] = new_socket;  
                    printf("Adding to list of sockets as %d\n" , i);  
                         
                    break;  
                }  
            }  
        }  
             
        //else its some IO operation on some other socket
        for (i = 0; i < max_clients; i++)  
        {  
            sd = client_socket[i];
                 
            if (FD_ISSET( sd , &readfds))  
            {  
                //Check if it was for closing , and also read the 
                //incoming message 
                if ((valread = read( sd , buffer, 1024)) == 0)  
                {  
                    //Somebody disconnected , get his details and print 
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);  
                    printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 

                    // Broadcast a message informing about the disconnection
                    for (int i = 0; i < max_clients; i++)
                    {
                        outSock = client_socket[i];  
                        if (outSock != master_socket && outSock != sd)
                        {
                            std::ostringstream smessage;
                            smessage << "USER #" << sd << " has left the chat!\r\n";
                            std::string strOut = smessage.str();
                            send(outSock, strOut.c_str(), strOut.size(), 0); 
                        } 
                    } 
                         
                    //Close the socket and mark as 0 in list for reuse 
                    close( sd );  
                    client_socket[i] = 0;    
                }  
                     
                else 
                {  
                    //Broadcast the message that came in 
                    for (int i = 0; i < max_clients; i++)
                    {
                        outSock = client_socket[i]; 
                        std::ostringstream ss;
                        if (outSock != master_socket && outSock != sd)
                        {
                            ss << "USER #" << sd << ": " << buffer << "\r\n";
                        } 
                        if (outSock == sd)
                        {
                            ss << "YOU" << ": " << buffer << "\r\n";
                        }
                        std::string strOut = ss.str();
                        buffer[valread] = '\0';  
                        send(outSock, strOut.c_str(), strOut.size() + 1, 0); 
                    }

                    exit_sql = sqlite3_open("chatapp.db", &DB);

                    // Insert user and message information to the DB
                    std::ostringstream sqlmessage;
                    sqlmessage << "INSERT INTO CHATS VALUES(" << count << ", 'USER #" << sd << "', '" << buffer << "');";
                    std::string strsql = sqlmessage.str();
                    std::string sql(strsql);
  
                    exit_sql = sqlite3_exec(DB, sql.c_str(), NULL, 0, &messaggeError);
                    if (exit_sql != SQLITE_OK) {
                        std::cerr << "Error Insert" << std::endl;
                        sqlite3_free(messaggeError);
                    }
                    
                    // Display the chatDB state in the server console
                    std::string query = "SELECT * FROM CHATS;";
                    std::cout << "STATE OF CHATDB" << std::endl;
                    sqlite3_exec(DB, query.c_str(), callback, NULL, NULL);

                    count++;
                    sqlite3_close(DB);
                }  
            }  
        }  
    }  
         
    return 0;  
}  