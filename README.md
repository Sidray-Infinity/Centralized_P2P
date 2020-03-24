# Centralized Peer-to-Peer architecture for File Sharing
<center><img src="images/CP2P_logo.png" alt="logo" width=200
            height=200></center>

The aim of this project is to provide a file sharing system, among the peers connected to the network, administered by a centralized server.

## Requirements
* Linux based Operating System.
* Internet Connection

## Installation Process

* Download the server.c and client.c files.

#### Server side
* Installing MySQL
    * Execute the following commands on Ubuntu: <br />
        `sudo apt-get update` <br />
        `sudo apt-get install mysql-server`

* Create a new database 'p2p' and four tables to it, namely:
    * login_details

        | Field     | Type       | Null | Key | Default | Extra |
        |-----------|------------|------|-----|---------|-------|
        | log_id    | int        | NO   | PRI | NULL    |       |
        | user_name | text       | YES  |     | NULL    |       |
        | password  | text       | YES  |     | NULL    |       |
        | isOnline  | tinyint(1) | YES  |     | NULL    |       |

    * online_clients

        | Field  | Type | Null | Key | Default | Extra |
        |--------|------|------|-----|---------|-------|
        | p_id   | int  | NO   | PRI | NULL    |       |
        | IP     | text | NO   |     | NULL    |       |
        | port   | int  | NO   |     | NULL    |       |
        | log_id | int  | NO   | MUL | NULL    |       |

    * files

        | Field    | Type | Null | Key | Default | Extra |
        |----------|------|------|-----|---------|-------|
        | f_id     | int  | NO   | PRI | NULL    |       |
        | filename | text | NO   |     | NULL    |       |
        | p_id     | int  | NO   |     | NULL    |       |

    * blocks

        | Field     | Type | Null | Key | Default | Extra |
        |-----------|------|------|-----|---------|-------|
        | blockname | text | NO   |     | NULL    |       |
        | dst_IP    | text | NO   |     | NULL    |       |
        | dst_port  | int  | NO   |     | NULL    |       |
        | f_id      | int  | NO   | MUL | NULL    |       |

* Create a new user and grant all privileges to it.

## Compilation
* Compile the server side code using the following command : <br />
>        gcc server.c -o server.out `mysql_config --cflags --libs`
* Compile the client side code using the following command : <br />
>        gcc client.c -o client.out 

## Execution
* First start the server side program using `./server.out` and provide the following details: <br />
    * Server Port Number
    * User id and password of the MySQL client

* Start the client side program using `./client.out` and provide the following details: <br />
    * Server IP address and port number
    * UDP socket port number

* Once connection with server is established, choose whether the client is a new user or not.
* The client enters it's user name and password, which is registered/auhtenticated by the server.
  
  

### BUGS
- Handle the problem of 0 byte files.
- Files seems to be merging properly, but with some additional chars. Handle it.
- Re-entering username after entering used one is buggy. Probably due to use of goto.

### Finalization
- Remove the clear table commands that is executed, everytime the server starts.