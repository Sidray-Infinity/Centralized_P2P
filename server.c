#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "handle_files.c"

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

struct block {
    char block_name[1024];
    struct peer loc;
    struct block *next;
};

struct file_node {
    char filename[1024];
    struct block *block_arr;
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

int numUsersOnline(struct peer online_clients[]) {
    // Returns the number of users online

    int num_users_online = 0;
    for(int i=0; i<10; i++) {

        if(online_clients[i].peer_id != 0)
            num_users_online++;
    }

    return num_users_online;
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

void braodcast_peerlist(struct peer online_clients[]) {
    // braodcast_peerlists the new user's ip and port to all online users

    printf("Initiating braodcast_peerlisting:\n");
    int send_id, recv_id;
    char reply[1024];

    for(int i=0; i<10; i++) {
        if(online_clients[i].peer_id != 0) {
            send_id = send(online_clients[i].peer_id, "PeerUpdate", strlen("PeerUpdate"), MSG_DONTWAIT);
            if(send_id == -1) {
                printf("Cannot send communication type!\n");
                exit(1);
            } 

            bzero(reply, 1024);
            recv_id = recv(online_clients[i].peer_id, reply, sizeof(reply), 0);
            if(recv_id == -1) {
                printf("Cannot recieve acknowledgement!\n");
                exit(1);
            }

            send_id = send(online_clients[i].peer_id, online_clients, 10*sizeof(struct peer), MSG_WAITALL);
            if(send_id == -1) {
                printf("Problem in braodcast_peerlist!\n");
                exit(1);
            } 
        }
    }
}

void show_block_locations(struct file_node DHT[], int index) {
    // Displays the location of each block associated with a file

    printf("---------------------------------------------\n");
    printf("FILE: %s\n", DHT[index].filename);
    for(struct block *q = DHT[index].block_arr; q != NULL; q = q->next) {
        printf("BLOCK NAME: %s LOC:", q->block_name);
        printf(" IP: %s PORT: %d\n", q->loc.ip, q->loc.port);
    }
    printf("---------------------------------------------\n");
}

void upload_to_clients(struct file_node DHT[], int index) {
    // Uploads each block to it's respective peer to be stored.

    int send_id, recv_id;
    FILE *f;
    char sendbuffer[BUFSIZ], reply[1024];
    for(struct block *q = DHT[index].block_arr; q != NULL; q = q->next) {
        send_id = send(q->loc.peer_id, "BlockUpload", strlen("BlockUpload"), 0);
        if(send_id == -1) {
                printf("Cannot send communication type!\n");
                exit(1);
        }

        bzero(reply, 1024);
        recv_id = recv(q->loc.peer_id, reply, sizeof(reply), 0);
        if(recv_id == -1) {
            printf("Cannot recieve filename!\n");
            exit(1);
        }

        char filesize[256], sendbuffer[BUFSIZ];
        int sent_bytes = 0;
        bzero(filesize, 256);
        
        send_id = send(q->loc.peer_id, q->block_name, strlen(q->block_name), 0);
        if(send_id == -1) {
            printf("Cannot send file name!\n");
            exit(1);
        }

        FILE *f = fopen(q->block_name, "r ");
        if(f == NULL) {
            printf("Block doesn't exist!\n");
            exit(1);
        }

        while(1) {
            bzero(sendbuffer, BUFSIZ);
            int nread = fread(sendbuffer, 1, BUFSIZ, f);
            if(nread > 0) {
                write(q->loc.peer_id, sendbuffer, nread);
            }
            else if(nread == 0)
                break;
        }

        printf("Block uploaded to IP: %s PORT: %d\n", q->loc.ip, q->loc.port);

    }
}

void remove_from_server(struct file_node DHT[], int index) {
    // Remove the file and it's blocks from the server
    
    int snprint_stat;
    char cmnd[1024];
    bzero(cmnd, 1024);

    snprint_stat = snprintf(cmnd, sizeof(cmnd), "rm %s", DHT[index].filename);
    if(snprint_stat == -1) {
        printf("Snprintf error!\n");
        exit(1);
    }
    if(system(cmnd) == -1) {
        printf("Cannot remove the file!\n");
        exit(1);
    }

    for(struct block *q = DHT[index].block_arr; q != NULL; q = q->next) {
        snprint_stat = snprintf(cmnd, sizeof(cmnd), "rm %s", q->block_name);
        if(snprint_stat == -1) {
            printf("Snprintf error!\n");
            exit(1);
        }
        if(system(cmnd) == -1) {
            printf("Cannot remove the block %s!\n", q->block_name);
            exit(1);
        }
    }
    printf("Files and their blocks removed from server.\n");
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

    struct file_node DHT[100]; // The server can handle at most 100 files

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
          
            braodcast_peerlist(online_clients);

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
                    online_clients[i].peer_id = 0; // Changing the status of the client to offline
                    database[j].online = 0;

                    close(temp);
                    
                    braodcast_peerlist(online_clients); // braodcast_peerlist the updated client list.

                    showDatabase(database, database_counter);
                    showOnlineClients(online_clients);
                }

                if(strcmp(buff, "upload") == 0) {
                    printf("Client %d is uploading a file ...\n", online_clients[i].peer_id);
                    char filename[256], final_name[1024], copy_final_name1[1024], copy_final_name2[1024], buffer[BUFSIZ], cmnd[20];
                    bzero(filename, 256);
                    bzero(final_name, 1024);
                    bzero(cmnd, 20);

                    recv_id = recv(online_clients[i].peer_id, filename, sizeof(filename), 0);
                    if(recv_id == -1) {
                        printf("Cannot recieve filename!\n");
                        exit(1);
                    }

                    snprintf(final_name, sizeof(final_name), "%d_%s", online_clients[i].peer_id, filename);

                    strcpy(copy_final_name1, final_name);
                    strcpy(copy_final_name2, final_name);
                    char *file = strtok(copy_final_name1, ".");
                    char *fileformat = strtok(NULL, ".");

                    FILE* fp = fopen(final_name, "w");
                    if(fp == NULL) {
                        printf("Cannot create file!\n");
                        exit(1);
                    }
                    
                    int recv_bytes = 0;  
    
                    while( (recv_bytes = read(online_clients[i].peer_id, buffer, BUFSIZ))> 0 ) {
                        fwrite(buffer, 1, recv_bytes, fp);
                        if(recv_bytes < BUFSIZ)
                            break;
                    }

                    if (recv_bytes < 0)
                        perror("Receiving");

                    fclose(fp);
                    printf("File recieved.\n");

                    int num_users_online = numUsersOnline(online_clients);
          
                    split_file(final_name, 
                        num_users_online, strlen(final_name));
                    printf("File splitted.\n");

                    int index = hash_filename(copy_final_name2);
                    strcpy(DHT[index].filename, copy_final_name2);
                    DHT[index].block_arr = NULL;

                    char block_name[1024];

                    // Number of blocks will be equal to the number of online clients.
                    int block_count = 0;
                    for(int i=0; i<10; i++) {
                        if(online_clients[i].peer_id != 0) {
                            bzero(block_name, 1024);
                            
                            if(block_count < 10)
                                snprintf(block_name, sizeof(block_name), "%s0%d.%s",file, block_count, fileformat);
                            else
                                snprintf(block_name, sizeof(block_name), "%s%d.%s",file, block_count, fileformat);
                            block_count++;
                            
                            struct block *new_block = (struct block*)malloc(sizeof(struct block));
                            strcpy(new_block->block_name, block_name);
                            new_block->loc = online_clients[i];
                            new_block->next = NULL;

                            if(DHT[index].block_arr == NULL)
                                DHT[index].block_arr = new_block;
                            else {
                                struct block *q = DHT[index].block_arr;
                                while(q->next != NULL)
                                    q = q->next;
                                q->next = new_block;
                            }
                        }
                    }

                    show_block_locations(DHT, index);
                    upload_to_clients(DHT, index);
                    remove_from_server(DHT, index);
                    
                }
            }
        }

        if(FD_ISSET(0, &fd_arr)) {
            bzero(buff, 100);
            fgets(buff, 1024, stdin);

            buff[strlen(buff)-1] = '\0';

            if(strncmp(buff, "-sys ", 5) == 0) {
                if(strlen(buff) < 5)
                    printf("Message too small!\n");
                else {
                    char *msg = buff + 5;
                    if(system(msg) == -1) {
                        printf("Cannot execute system command!\n");
                        exit(0);
                    }
                }
            }
            
            else if(strcmp(buff, "exit") == 0) {
                printf("Goodbye.\n");
                for(int i=0; i<10; i++) {
                    // Close all peer connections before exiting server
                    if(online_clients[i].peer_id != 0)
                        close(online_clients[i].peer_id);
                }
                close(sock_id);
                exit(0);
            }
        }
    }

    close(sock_id);
    
    return 0;
}