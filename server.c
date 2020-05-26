
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <mysql/mysql.h>
#include "handle_files.c"

#define ONLINE_CLIENTS_LIMIT 50

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
    int num_blocks;
    struct block *block_arr;
};

void finish_with_error(MYSQL *con) {
    fprintf(stderr, "%s\n", mysql_error(con));
    mysql_close(con);
    exit(1);        
}


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


void show_auth_DB(MYSQL *con) {
    // Displays the database of username and password for different clients.

    printf("---------------------------------------------\n");
    printf("DATABASE:\n\n");
    char query[30];
    bzero(query, sizeof(query));

    if(snprintf(query, sizeof(query),
        "SELECT * from login_details") == -1) {
            fprintf(stderr, "Snprintf error!\n");
            exit(1);
        }

        if(mysql_query(con, query))
            finish_with_error(con);

        MYSQL_RES *result = mysql_store_result(con);
        MYSQL_ROW row;

        while ((row = mysql_fetch_row(result)))
            printf("LOG_ID: %d USERNAME: %s PASSWORD: %s STATUS: %d\n", atoi(row[0]), row[1], row[2], atoi(row[3]));

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


int userNameTaken(struct login user, MYSQL *con) {
    // Determines if the username is already taken or not

    char query[50];
    bzero(query, 50);

    if(snprintf(query, sizeof(query), "SELECT * FROM login_details WHERE user_name = \"%s\"", user.username) == -1) {
        printf("Snprintf error!\n");
        exit(1);
    }

    if(mysql_query(con, query))
        finish_with_error(con);

    MYSQL_RES *result = mysql_store_result(con);
    if(mysql_num_rows(result) == 0)
        return FALSE;

    printf("Test\n");


    return TRUE; 
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

void show_online_clients(MYSQL *con) {
    printf("---------------------------------------------\n");
    printf("CLIENTS ONLINE:\n\n");
    char query[30];
    bzero(query, sizeof(query));

    if(snprintf(query, sizeof(query),
        "SELECT * from online_clients") == -1) {
            fprintf(stderr, "Snprintf error!\n");
            exit(1);
        }

        if(mysql_query(con, query))
            finish_with_error(con);

        MYSQL_RES *result = mysql_store_result(con);
        MYSQL_ROW row;

        while ((row = mysql_fetch_row(result)))
            printf("PEER_ID: %d IP: %s PORT: %d LOG_ID: %d\n", atoi(row[0]), row[1], atoi(row[2]), atoi(row[3]));

    printf("---------------------------------------------\n");
        
}


int authenticate_login(struct login user, MYSQL *con) {
    // Authenticates the login details provided by the user.

    char query[1024];
    bzero(query, sizeof(query));

    if(snprintf(query, sizeof(query),
         "SELECT * FROM login_details WHERE user_name = \"%s\" AND password = \"%s\"", user.username, user.password) == -1) {
        fprintf(stderr, "Snprintf error!\n");
        exit(1);
    }

    if(mysql_query(con, query))
        finish_with_error(con);

    MYSQL_RES *result = mysql_store_result(con);
    if(mysql_num_rows(result) == 0)
        return -1;
    else if(mysql_num_rows(result) == 1) {
        MYSQL_ROW row = mysql_fetch_row(result);
        if(strcmp(row[3], "1") == 0) // If the user is already online 
            return -2;
        printf("LOG ID %d\n", atoi(row[0]));
        return atoi(row[0]);
    }
    

    return 1;
}


void braodcast_peerlist(struct peer online_clients[]) {
    // braodcast_peerlists the new user's ip and port to all online users

    printf("Initiating braodcast_peerlisting:\n");
    int send_id, recv_id;
    char reply[1024];

    for(int i=0; i<10; i++) {
        if(online_clients[i].peer_id != 0) {
            send_id = send(online_clients[i].peer_id, "PeerUpdate",
                           strlen("PeerUpdate"), 0);
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

            send_id = send(online_clients[i].peer_id, online_clients,
                           10*sizeof(struct peer), 0);
            if(send_id == -1) {
                printf("Problem in braodcast_peerlist!\n");
                exit(1);
            } 
        }
    }
}


void broadcast_peerlist_sql(MYSQL *con) {
    // braodcast_peerlists the new user's ip and port to all online users

    printf("Initiating braodcast_peerlisting:\n");
    int send_id, recv_id, num_clients;
    char reply[1024], query[100];
    bzero(query, sizeof(query));

    if(snprintf(query, sizeof(query), 
        "SELECT * FROM online_clients") == -1) {
        fprintf(stderr, "Snprintf error!\n");
        exit(1);
    }

    if(mysql_query(con, query))
        finish_with_error(con);

    MYSQL_RES *result = mysql_store_result(con);
    MYSQL_ROW row;
    num_clients = mysql_num_rows(result);

    printf("NUM CLIENTS: %d\n", num_clients);

    struct peer *online_clients = (struct peer *)malloc(num_clients*sizeof(struct peer));
    for(int i=0; i<num_clients; i++) {
        struct peer temp;

        row = mysql_fetch_row(result);
        temp.peer_id = atoi(row[0]);
        strcpy(temp.ip, row[1]);
        temp.port = atoi(row[2]);

        online_clients[i] = temp;
    }

    for(int i=0; i<num_clients; i++) {
        send_id = send(online_clients[i].peer_id, "PeerUpdate",
                           strlen("PeerUpdate"), 0);
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

        send_id = send(online_clients[i].peer_id, &num_clients,
                           sizeof(int), 0);
        if(send_id == -1) {
            printf("Cannot send communication type!\n");
            exit(1);
        } 

        send_id = send(online_clients[i].peer_id, online_clients,
                        10*sizeof(struct peer), 0);
        if(send_id == -1) {
            printf("Problem in braodcast_peerlist!\n");
            exit(1);
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

void show_DHT(struct file_node DHT[]) {
    printf("---------------------------------------------\n");
    printf("BLOCK DISTRIBUTION TABLE\n\n");
    for(int i=0; i<100; i++) {
        if(DHT[i].block_arr != NULL) {
            printf("FILENAME: %s \n", DHT[i].filename);
            for(struct block *q = DHT[i].block_arr; q != NULL; q = q->next)
                printf("\tBLOCKNAME: %s IP: %s PORT: %d\n", q->block_name, q->loc.ip, q->loc.port);
            printf("\n");
           
        }
    }
    printf("---------------------------------------------\n");
}

void send_files_to_client(MYSQL *con, struct peer client) {
    /*  Send the list of files available the client requesting it  */

    char msg[2048], reply[4], *file_name, *orig_name;
    int file_count = 0, send_id, recv_id, index_list[100];
    char query[200];
    bzero(reply, 4);
    bzero(query, sizeof(query));

    if(snprintf(query, sizeof(query),
                "SELECT filename FROM files") == -1) {
        fprintf(stderr, "Snprintf error!\n");
    }

    if(mysql_query(con, query))
        finish_with_error(con);

    MYSQL_RES *res = mysql_store_result(con);
    file_count = mysql_num_rows(res);
    MYSQL_ROW row;


    send_id = send(client.peer_id, &file_count, sizeof(int), 0);
    if(send_id == -1) {
        printf("Cannot send number of files!\n");
        exit(1);
    }

    recv_id = recv(client.peer_id, reply, sizeof(reply), 0);
    if(recv_id == -1) {
        printf("Cannot recieve ACK!\n");
        exit(1);
    }

    while((row = mysql_fetch_row(res))){
        bzero(reply, 4);

        send_id = send(client.peer_id, row[0], strlen(row[0]), 0);
        if(send_id == -1) {
            printf("Cannot send filename!\n");
            exit(1);
        }

        recv_id = recv(client.peer_id, reply, sizeof(reply), 0);
        if(recv_id == -1) {
            printf("Cannot recieve ACK of filename!\n");
            exit(1);
        }

    }
    mysql_free_result(res);
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
    char serv_ip[20];
    bzero(serv_ip, 20);

    struct sockaddr_in addr, store; int len_store = sizeof(store);

    addr.sin_addr.s_addr = htonl(INADDR_ANY);
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
    char query[200];
    fd_set fd_arr;

    struct file_node DHT[100]; // The server can handle at most 100 files
    for(int i=0; i<100; i++)
        DHT[i].block_arr = NULL; // All blocks lists are initially NULL

    struct peer online_clients[10]; // Maximum 10 peers can be online
    for(int i=0; i<10; i++)
        online_clients[i].peer_id = 0;  // Initially, all the peers are offline.

    struct login database[100]; int database_counter = -1;   // Record the details of all peer login

	printf("MySQL client version: %s\n", mysql_get_client_info());
	MYSQL *con = mysql_init(NULL); // Initialized mysql object
	if(con == NULL) {
		fprintf(stderr, "%s\n", mysql_error(con));
		exit(1);
	}

	// Establishes connection to DB p2p
	if (mysql_real_connect(con, "localhost", "sid", "sid", "p2p", 0, NULL, 0) == NULL)
		finish_with_error(con);


    // Remove after completion !!
    if(mysql_query(con, "DELETE FROM online_clients WHERE TRUE"))
        finish_with_error(con);
    if(mysql_query(con, "DELETE FROM login_details WHERE TRUE"))
        finish_with_error(con);
    if(mysql_query(con, "DELETE FROM blocks WHERE TRUE"))
        finish_with_error(con);
    if(mysql_query(con, "DELETE FROM files WHERE TRUE"))
        finish_with_error(con);


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

            // Recieving username and password
            recv_id = recv(client_id, &new_user_login, sizeof(new_user_login), 0);
            if(recv_id == -1) {
                printf("Cannot recieve login details!\n");
                exit(1);
            }

            if(isNewClient) {
                // Need to add it to all_clients_ever, online client and assign a user name.
                // Add to the all_clients list (historical record of clients)

                /*
                if(userNameTaken(new_user_login, database, database_counter) == TRUE) {
                    printf("Used Username entered by the client.\n");
                    send_id = send(client_id, "Username taken.", strlen("Username taken."), 0);
                    if(send_id == 0) {
                        printf("Cannot send username taken message!\n");
                        exit(1);
                    }
                } 
                */

                if(userNameTaken(new_user_login, con) == TRUE) {
                    printf("Used Username entered by the client.\n");
                    send_id = send(client_id, "Username taken.", strlen("Username taken."), 0);
                    if(send_id == 0) {
                        printf("Cannot send username taken message!\n");
                        exit(1);
                    }
                    while(userNameTaken(new_user_login, con) == TRUE) {
                        recv_id = recv(client_id, &new_user_login, sizeof(new_user_login), 0);
                        if(recv_id == -1) {
                            printf("Cannot recieve login details!\n");
                            exit(1);
                        }
                    }

                }   
           
                printf("Registering the username and password to the database..\n");
                database_counter++;
                strcpy(database[database_counter].username, new_user_login.username);
                strcpy(database[database_counter].password, new_user_login.password);
                database[database_counter].online = 1;

                bzero(query, sizeof(query));
                if(snprintf(query, sizeof(query), "INSERT INTO login_details VALUES(%d, \"%s\", \"%s\", 1)",
                                database_counter, new_user_login.username, new_user_login.password) == -1) {
                    fprintf(stderr, "Snprintf error!\n");
                    exit(1);
                }                     
                
                if(mysql_query(con, query))
                    finish_with_error(con);

                int send_id = send(client_id, "Registration successful",
                                        strlen("Registration successful"), 0);
                if(send_id == -1) {
                    printf("Cannot send successful registration msg\n");
                    exit(1);
                }
            
            }
            else { // User had a connection with the server in the past.
                /*
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
                */
                auth_stat = authenticate_login(new_user_login, con);
                if(auth_stat >= 0) {
                    printf("Client login succesfull! Sending the greeting msg..\n");
                    int send_id = send(client_id, "Login Successful",
                                         strlen("Login Successful"), 0);
                    if(send_id == -1) {
                        printf("Cannot send the greeting msg!\n");
                        exit(1);
                    }

                    bzero(query, sizeof(query));
                    if(snprintf(query, sizeof(query), 
                        "UPDATE login_details SET isOnline = 1 WHERE log_id = %d", auth_stat) == -1) {
                            fprintf(stderr, "Snprintf error!\n");
                            exit(1);
                        }
                    
                    if(mysql_query(con, query))
                        finish_with_error(con);

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
                    continue;
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

                bzero(query, sizeof(query));
                if(snprintf(query, sizeof(query),
                    "INSERT INTO online_clients VALUES(%d, \"%s\", %d, %d)", client_id, inet_ntoa(udp_id.sin_addr), ntohs(udp_id.sin_port), database_counter) == -1) {
                    fprintf(stderr, "Snprintf error!\n");
                    exit(1);
                }

                if(mysql_query(con, query))
                    finish_with_error(con);

            }
            else {
                database[auth_stat].addrs_info.peer_id = client_id;
                strcpy(database[auth_stat].addrs_info.ip, inet_ntoa(udp_id.sin_addr));
                database[auth_stat].addrs_info.port = ntohs(udp_id.sin_port);

                bzero(query, sizeof(query));
                if(snprintf(query, sizeof(query),
                    "INSERT INTO online_clients VALUES(%d, \"%s\", %d, %d)", client_id, inet_ntoa(udp_id.sin_addr), ntohs(udp_id.sin_port), auth_stat) == -1) {
                    fprintf(stderr, "Snprintf error!\n");
                    exit(1);
                }

                if(mysql_query(con, query))
                    finish_with_error(con);
            }
          
            //broadcast_peerlist(online_clients);
            // showDatabase(database, database_counter);
            // showOnlineClients(online_clients);
            
            broadcast_peerlist_sql(con);
            show_auth_DB(con);
            show_online_clients(con);
        }

        for(int i=0; i<10; i++) {
            if(online_clients[i].peer_id != 0 && FD_ISSET(online_clients[i].peer_id, &fd_arr)) {
                printf("Recieving a message from the client:\n");
                int client_id = online_clients[i].peer_id;

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

                    // Extract the log_id to be updated
                    bzero(query, sizeof(query));
                    if(snprintf(query, sizeof(query), 
                        "SELECT log_id FROM online_clients WHERE p_id = %d", client_id) == -1) {
                        fprintf(stderr, "Snprintf error!\n");
                        exit(1);
                    }

                    if(mysql_query(con, query))
                        finish_with_error(con);

                    MYSQL_RES *result = mysql_store_result(con);
                    MYSQL_ROW row = mysql_fetch_row(result);
                    int log_id = atoi(row[0]); 

                    // Remove the row from online_clients
                    bzero(query, sizeof(query));
                    if(snprintf(query, sizeof(query), 
                        "DELETE FROM online_clients WHERE p_id = %d", client_id) == -1) {
                        fprintf(stderr, "Snprintf error!\n");
                        exit(1);
                    }

                    if(mysql_query(con, query))
                        finish_with_error(con);

                    // Update the login_details file
                    bzero(query, sizeof(query));
                    if(snprintf(query, sizeof(query), 
                        "UPDATE login_details SET isOnline = 0 WHERE log_id = %d", log_id) == -1) {
                        fprintf(stderr, "Snprintf error!\n");
                        exit(1);
                    }

                    if(mysql_query(con, query))
                        finish_with_error(con);

                    // char c;
                    // fgetc(stdin);

            
                    broadcast_peerlist_sql(con);
                    show_auth_DB(con);
                    show_online_clients(con);

          
                    fgetc(stdin);
                }
                else if(strcmp(buff, "upload") == 0) {
                    printf("Client %d is uploading a file ...\n", online_clients[i].peer_id);
                    char filename[256], final_name[1024],
                         copy_final_name1[1024], copy_final_name2[1024], buffer[BUFSIZ], cmnd[20];
                    int latest_file_id = 0;

                    bzero(filename, 256);
                    bzero(final_name, 1024);
                    bzero(cmnd, 20);

                    recv_id = recv(online_clients[i].peer_id, filename, sizeof(filename), 0);
                    if(recv_id == -1) {
                        printf("Cannot recieve filename!\n");
                        exit(1);
                    }

                    printf("File name recieved: %s\n", filename);

                    // Extracting latest file ID
                    bzero(query, sizeof(query));
                    if(snprintf(query, sizeof(query),
                        "SELECT f_id from files ORDER BY f_id DESC LIMIT 1") == -1) {
                        fprintf(stderr, "Snprintf error!\n");
                        exit(1);
                    }

                    if(mysql_query(con, query))
                        finish_with_error(con);

                    MYSQL_RES * result = mysql_store_result(con);

                    if(mysql_num_rows(result) > 0) {
                        MYSQL_ROW row = mysql_fetch_row(result);
                        latest_file_id = atoi(row[0]);

                        // Updating the files table
                        bzero(query, sizeof(query));
                        if(snprintf(query, sizeof(query),
                            "INSERT INTO files VALUES(%d, \"%s\", %d)",++latest_file_id, filename, client_id) == -1) {
                            fprintf(stderr, "Snprintf error!\n");
                            exit(1);
                        }

                    }
                    else {
                        latest_file_id = 0;

                        // Updating the files table
                        bzero(query, sizeof(query));
                        if(snprintf(query, sizeof(query),
                            "INSERT INTO files VALUES(%d, \"%s\", %d)",0, filename, client_id) == -1) {
                            fprintf(stderr, "Snprintf error!\n");
                            exit(1);
                        }

                    }

                    if(mysql_query(con, query))
                        finish_with_error(con);
        
                    // int send_id = send(online_clients[i].peer_id, "ACK", strlen("ACK"), 0);
                    // if(send_id == -1) {
                    //     printf("Cannot send ACK for file name!\n");
                    //     exit(1);
                    // }

                    snprintf(final_name, sizeof(final_name), 
                                "%d-%s", online_clients[i].peer_id, filename);

                    strcpy(copy_final_name1, final_name);
                    strcpy(copy_final_name2, final_name);
                    // char *file = strtok(copy_final_name1, ".");
                    // char *fileformat = strtok(NULL, ".");

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

                    printf("FINAL NAME: %s\n", final_name);
                    // printf("FINAL NAME LEN: %ld\n", strlen(final_name));
          
                    split_file(final_name, 
                        num_users_online, strlen(final_name));
                    printf("File splitted.\n");
                    printf("LATEST FILE ID: %d\n", latest_file_id);
                    int index = hash_filename(copy_final_name2);
                    strcpy(DHT[index].filename, copy_final_name2);
                    DHT[index].num_blocks = num_users_online;

                    // printf("NUM USERS ONLINE: %d\n", num_users_online);

                    // printf("COPY FINAL NAME 2: %s\n", copy_final_name2);

                    char block_name[1024];

                    // Number of blocks will be equal to the number of online clients.
                    int block_count = 0;
                    for(int i=0; i<10; i++) {
                        if(online_clients[i].peer_id != 0) {
                            bzero(block_name, 1024);
                            
                            if(block_count < 10) {
                                if(snprintf(block_name, sizeof(block_name), "%s0%d",
                                             copy_final_name2, block_count) == -1) {
                                    printf("Snprintf problem!\n");
                                    exit(1);
                                }
                            }
                            else {
                                if(snprintf(block_name, sizeof(block_name), "%s%d", 
                                             copy_final_name2, block_count) == -1) {
                                    printf("Snprintf problem!\n");
                                    exit(1);
                                }
                            }
                            

                            // Updating the blocks table
                            bzero(query, sizeof(query));
                            if(snprintf(query, sizeof(query),
                                "INSERT INTO blocks VALUES(\"%s\", \"%s\", %d, %d)",
                                 block_name, online_clients[i].ip, online_clients[i].port, latest_file_id) == -1) {
                                fprintf(stderr, "Snprintf error!\n");
                                exit(1);
                            }

                            if(mysql_query(con, query))
                                finish_with_error(con);

                            // printf("BLOCK NAME: %s\n", block_name);
                            
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

                            // free(new_block);
                        }
                    }

                    show_DHT(DHT);
                    upload_to_clients(DHT, index);
                    remove_from_server(DHT, index);
                    
                }
                else if(strcmp(buff, "ls") == 0) {
                    send_files_to_client(con, online_clients[i]);
                }
                else if(strcmp(buff, "download") == 0) {
                    char *filename = (char *)malloc(20*sizeof(char)), reply[4];
                    int send_id, recv_id, num_blocks = 0;
                    bzero(reply, 4);

                    // Recieving filename requested to be downloaded
                    recv_id = recv(online_clients[i].peer_id, filename,
                                     sizeof(filename), 0);
                    if(recv_id == -1) {
                        printf("Cannot recieve filename for download!\n");
                        exit(1);
                    }
                    filename[strlen(filename)-1] = '\0';
                    // printf("FILENAME RECIEVED %s\n", filename);


                    bzero(query, sizeof(query));
                    if(snprintf(query, sizeof(query),
                                "SELECT f_id FROM files WHERE filename = \"%s\"", filename) == -1) {
                        fprintf(stderr, "Snprintf errro!\n");
                        exit(1);
                    }

                    if(mysql_query(con, query))
                        finish_with_error(con);

                    MYSQL_RES *res = mysql_store_result(con);
                    MYSQL_ROW row;

                    if(mysql_num_rows(res) > 0) {

                        row = mysql_fetch_row(res);
                        
                        // Extracting blocks info
                        bzero(query, sizeof(query));
                        if(snprintf(query, sizeof(query),
                                    "SELECT blockname, dst_IP, dst_port FROM blocks WHERE f_id = %d", atoi(row[0])) == -1) {
                            fprintf(stderr, "Snprintf errro!\n");
                            exit(1);
                        }

                        if(mysql_query(con, query))
                            finish_with_error(con);
                        
                        res = mysql_store_result(con);
                        num_blocks = mysql_num_rows(res);

                        printf("NUMBLOCKS FOUND: %d\n", num_blocks);

                        // Sending number of blocks found to client
                        send_id = send(online_clients[i].peer_id, &num_blocks, sizeof(int), 0);
                        if(send_id == -1) {
                            printf("Cannot send number of blocks!\n");
                            exit(1);
                        }

                        recv_id = recv(online_clients[i].peer_id, reply, sizeof(reply), 0);
                        if(recv_id == -1) {
                            printf("Cannot recieve ACK for num blocks!\n");
                            exit(1);
                        }

                        while((row = mysql_fetch_row(res))) {
                            bzero(reply, 4);
                            printf("SENDING BLOCKNAME: %s IP: %s PORT: %s\n", row[0], row[1], row[2]);

                            struct block *temp_block = (struct block *)malloc(sizeof(struct block));

                            strcpy(temp_block->block_name, row[0]);
                            strcpy(temp_block->loc.ip, row[1]);
                            temp_block->loc.port = atoi(row[2]);

                            send_id = send(online_clients[i].peer_id, temp_block, sizeof(struct block), 0);
                            if(send_id == -1) {
                                printf("Cannot send block info!\n");
                                exit(1);
                            }

                            recv_id = recv(online_clients[i].peer_id, reply, sizeof(reply), 0);
                            if(recv_id == -1) {
                                printf("Cannot recieve ACK for blocks!\n");
                                exit(1);
                            } 

                            // free(temp_block);
                        }
                        printf("ALL BLOCK DETAILS SENT.\n");

                    }
                    else {
                        // Filename not found
                        send_id = send(online_clients[i].peer_id, &num_blocks, sizeof(int), 0);
                        if(send_id == -1) {
                            printf("Cannot send number of blocks!\n");
                            exit(1);
                        }

                        recv_id = recv(online_clients[i].peer_id, reply, sizeof(reply), 0);
                        if(recv_id == -1) {
                            printf("Cannot recieve ACK for num blocks!\n");
                            exit(1);
                        }

                        continue;
                    }
                }
            }
        }

        if(FD_ISSET(0, &fd_arr)) {
            /* Server side system interaction */

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
            /* BUGGY
            else if(strncmp(buff, "-show ", 6) == 0) {
                printf("TEST\n");
                if(strlen(buff) < 6)
                    printf("Message too small!\n");
                else {
                    char *msg = buff + 5;
                    if(strcmp(msg, "online_clients") == 0)
                        show_online_clients(con);
                    else if(strcmp(msg, "login_database") == 0)
                        show_auth_DB(con);
                }
            }
            */
            
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
    mysql_close(con);
    return 0;
}
