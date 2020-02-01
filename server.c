#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>

#define TRUE 1
#define FALSE 0

struct peer {
    int peer_id; // !0 -> online, 0 - > offline
    char ip[20];
    int port;
};


struct login {
    int online;
    char username[1024];
    char password[1024];
    struct peer addrs_info;
};

void showDatabase(struct login database[], int database_counter) {
    // Displays the database of username and password for different clients.

    printf("---------------------------------------------\n");
    if(database_counter == -1) {
        printf("Database is Empty!\n");
        return;
    }
    printf("DATABASE:\n\n");
    for(int i=0; i<=database_counter; i++)
        printf("%d. STATUS: %d USERNAME: %s PASSWORD: %s\n",i+1, database[i].online,
         database[i].username, database[i].password);
    printf("---------------------------------------------\n");
}

void usersOnlineDB(struct login database[], int database_counter) {
    // Shows the online users in the database.

    printf("---------------------------------------------\n");
    if(database_counter == -1) {
        printf("Database is Empty!\n");
        return;
    }
    printf("ONLINE USERS IN DATABASE:\n\n");
    for(int i=0; i<database_counter; i++) {
        if(database[i].online == 1)
            printf("%d. USERNAME: %s PASSWORD: %s\n",i+1,
             database[i].username, database[i].password);
    }
    printf("---------------------------------------------\n");

}


int userNameTaken(struct login user, struct login database[], int database_counter) {
    // Determines if the username is already taken or not

    for(int i=0; i<=database_counter; i++)
        if(strcmp(database[i].username, user.username) == 0)
            return 1;
    return 0;
}


void showOnlineClients(struct peer online_clients[]) {
    // Shows the list of all online clients

    printf("---------------------------------------------\n");
    printf("CLIENTS ONLINE:\n\n");
    int count = 1;
    for(int i=0; i<10; i++)
        if(online_clients[i].peer_id != 0)
            printf("%d. TCP_PEER_ID: %d. IP: %s PORT: %d\n",count++,
             online_clients[i].peer_id, online_clients[i].ip, online_clients[i].port);
    printf("---------------------------------------------\n");
        
}


int authenticate_login(struct login user, struct login *database, int database_counter) {
    // Authenticates the login details provided by the user.
    
    for(int i=0; i<=database_counter; i++) 
        if(strcmp(database[i].username, user.username) == 0 && 
           strcmp(database[i].password, user.password) == 0) {
            if(database[i].online == 0) {
                return i;
            }
            else 
                return -2;
        }
    return -1;
}

void broadcast(struct peer online_clients[]) {
    // Broadcasts the new user's ip and port to all online users

    printf("Initiating Broadcasting:\n");
    int send_id;
    for(int i=0; i<10; i++) {
        if(online_clients[i].peer_id != 0) {
            send_id = send(online_clients[i].peer_id, online_clients, 10*sizeof(struct peer), 0);
            if(send_id == -1) {
                printf("Problem in broadcast!\n");
                exit(1);
            } 
        }
    }
}

int main(int args, char *argv[]) {

    if(args != 2) {
        printf("Server port number needed!\n");
        exit(1);
    }

    const int SERVER_PORT = atoi(argv[1]);

    int sock_id = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_id == -1) {
        printf("Cannot create socket!\n");
        exit(1);
    }

    struct sockaddr_in addr, store; int len_store = sizeof(store);
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);

    int bind_id = bind(sock_id, (struct sockaddr*)&addr, sizeof(addr));
    if(bind_id == -1) {
        printf("Cannot bind!\n");
        exit(1);
    }

    int listen_id = listen(sock_id, 10);
    if(listen_id == -1) {
        printf("Cannot listen!\n");
        exit(1);
    }
    
    int maxfds = sock_id;
    char buff[1024];
    fd_set fd_arr;

    struct peer online_clients[10]; // Maximum 10 peers can be online
    for(int i=0; i<10; i++)
        online_clients[i].peer_id = 0;  // Initially, all the peers are offline.

    struct login database[100]; int database_counter = -1;   // Record the details of all peer login

    printf("Waiting for connections..\n");

    while(TRUE) {

        FD_ZERO(&fd_arr);
        FD_SET(sock_id, &fd_arr);
        FD_SET(0, &fd_arr);

        int auth_stat;

        for(int i=0; i<10; i++) {
            if(online_clients[i].peer_id > 0)
                FD_SET(online_clients[i].peer_id, &fd_arr);
            if(online_clients[i].peer_id > maxfds)
                maxfds = online_clients[i].peer_id;
        }

        int select_id = select(maxfds+1, &fd_arr, NULL, NULL, NULL);
        if(select_id == -1) {
            printf("Cannot select!\n");
            exit(1);
        }

        if(FD_ISSET(sock_id, &fd_arr)) {
            int isNewClient = 0;

            printf("A client is trying to connect. Check the login details.\n");
            int client_id = accept(sock_id, (struct sockaddr*)&store, &len_store);
            if(client_id == -1) {
                printf("Cannot accept!\n");
                exit(1);
            }

            int send_id = send(client_id, "New User? y or n: ", strlen("New User? y or n: "), 0);
            if(send_id == -1) {
                printf("Cannot send 'New User' confirmation message!\n");
                exit(1);
            }
            
            bzero(buff, 1024);
            int recv_id = recv(client_id, buff, sizeof(buff), 0);
            if(recv_id == -1) {
                printf("Cannot recieve 'New user' reply!\n");
                exit(1);
            }

            printf("FROM CLIENT: %s\n", buff);;
            if(strcmp(buff, "y") == 0)
                isNewClient = TRUE;
            else if(strcmp(buff, "n") != 0) {
                printf("Wrong input from client!\n");
                continue;
            }

            struct peer new_user; // New user storage strucutre.
            struct login new_user_login;

            recv_id = recv(client_id, &new_user_login, sizeof(new_user_login), 0);
            if(recv_id == -1) {
                printf("Cannot recieve login details!\n");
                exit(1);
            }

            if(isNewClient) {
                // Need to add it to all_clients_ever, online client and assign a user name.
                // Add to the all_clients list (historical record of clients)

                if(userNameTaken(new_user_login, database, database_counter) == TRUE) {
                    printf("Used Username entered by the client.\n");
                    send_id = send(client_id, "Username taken.", strlen("Username taken."), 0);
                    if(send_id == 0) {
                        printf("Cannot send username taken message!\n");
                        exit(1);
                    }
                }   
                else {
                    printf("Registering the username and password to the database..\n");
                    database_counter++;
                    strcpy(database[database_counter].username, new_user_login.username);
                    strcpy(database[database_counter].password, new_user_login.password);
                    database[database_counter].online = 1;

                    int send_id = send(client_id, "Registration successful",
                                         strlen("Registration successful"), 0);
                    if(send_id == -1) {
                        printf("Cannot send successful registration msg\n");
                        exit(1);
                    }
                }
            }
            else { // User had a connection with the server in the past.
                auth_stat = authenticate_login(new_user_login, database, database_counter);
                if(auth_stat >= 0) {
                    printf("Client login succesfull! Sending the greeting msg..\n");
                    int send_id = send(client_id, "Login Successful",
                                         strlen("Login Successful"), 0);
                    if(send_id == -1) {
                        printf("Cannot send the greeting msg!\n");
                        exit(1);
                    }
                    database[auth_stat].online = 1;
                    usersOnlineDB(database, database_counter);

                }
                else if(auth_stat == -2) {
                    printf("User already loged in!\n");
                    send_id = send(client_id, "Already logged in.", 
                                    strlen("Already logged in."), 0);
                    if(send_id == -1) {
                        printf("Cannot send already logged in message!\n");
                        exit(1);
                    }
                }
                else {  // user login failed
                    printf("Authentication failed. Sending the msg..\n");
                    int send_id = send(client_id, "Login Failed", strlen("Login Failed"), 0);
                    if(send_id == -1) {
                        printf("Cannot send the failed login msg!\n");
                        exit(1);
                    }
                    close(client_id);
                    continue;   
                }
            }

            // Continuation means, either a new user or successful login.
            struct sockaddr_in udp_id; // To store the UPD socket details of the client.
            int recv_udpid = recv(client_id, &udp_id, sizeof(udp_id), 0);
            if(recv_udpid == -1) {
                printf("Cannot recieve the UDP socket details of the client!\n");
                exit(1);
            }

            printf("Recieved UDP info for the client %d.\n", client_id);
            // new_user.peer_id = client_id;
            // strcpy(new_user.ip, inet_ntoa(udp_id.sin_addr));
            // new_user.port = ntohs(udp_id.sin_port);

            // for(int i=0; i<10; i++) { // Add the client list to online client_list.
            //     if(online_clients[i].peer_id == 0) {
            //         online_clients[i] = new_user;
            //         break;
            //     }
            //     if(i == 9) {
            //         printf("Online clients limit reached!\n");
            //         exit(0);
            //     }
            // }
          

            new_user_login.addrs_info.peer_id = client_id;
            strcpy(new_user_login.addrs_info.ip, inet_ntoa(udp_id.sin_addr));
            new_user_login.addrs_info.port = ntohs(udp_id.sin_port);

            for(int i=0; i<10; i++) { // Add the client list to online client_list.
                if(online_clients[i].peer_id == 0) {
                    online_clients[i] = new_user_login.addrs_info;
                    break;
                }
                if(i == 9) {
                    printf("Online clients limit reached!\n");
                    exit(0);
                }
            }

            if(isNewClient) { 
                database[database_counter].addrs_info.peer_id = client_id;
                strcpy(database[database_counter].addrs_info.ip, inet_ntoa(udp_id.sin_addr));
                database[database_counter].addrs_info.port = ntohs(udp_id.sin_port);
            }
            else {
                database[auth_stat].addrs_info.peer_id = client_id;
                strcpy(database[auth_stat].addrs_info.ip, inet_ntoa(udp_id.sin_addr));
                database[auth_stat].addrs_info.port = ntohs(udp_id.sin_port);
            }
          
            broadcast(online_clients);

            showDatabase(database, database_counter);
            showOnlineClients(online_clients);
        }

        for(int i=0; i<10; i++) {
            if(online_clients[i].peer_id != 0 && FD_ISSET(online_clients[i].peer_id, &fd_arr)) {
                printf("Recieving a message from the client:\n");
                bzero(buff, 1024);
                int recv_id = recv(online_clients[i].peer_id, buff, sizeof(buff), 0);
                if(recv_id == -1) {
                    printf("Cannot recieve message from client!\n");
                    exit(1);
                }
                //buff[strlen(buff)-1] = '\0';
                printf("FROM CLIENT: %s\n", buff);

                if(strcmp(buff, "exit") == 0) {
                    printf("Connection close request recieve. Processing...\n");
                    int temp = online_clients[i].peer_id, j;
                    for(j=0; j<=database_counter; j++) {
                        printf("%d\n", database[j].addrs_info.peer_id);
                        if(database[j].addrs_info.peer_id == temp)
                            break;
                    }
                    printf("%d %d\n", online_clients[i].peer_id, database[j].addrs_info.peer_id);
                    online_clients[i].peer_id = 0; // Changing the status of the client to offline
                    database[j].online = 0;

                    close(temp);
                    
                    broadcast(online_clients); // Broadcast the updated client list.

                    showDatabase(database, database_counter);
                    showOnlineClients(online_clients);
                }
            }
        }
    }


    close(sock_id);
    return 0;
}