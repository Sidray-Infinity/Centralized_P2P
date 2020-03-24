# Centralized Peer-to-Peer architecture for File Sharing

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
        gcc server.c -o server.out mysql_config --cflags --libs

> ## This is a header.
> 
> 1.   This is the first list item.
> 2.   This is the second list item.
> 
> Here's some example code:
> 
>     return shell_exec("echo $input | $markdown_script");

### TASKS
- Handle the problem of 0 byte files.
- Files seems to be merging properly, but with some additional chars. Handle it.
- Retentering username after entering used one is buggy. Probably due to use of goto.

### Finalization
- Remove the clear table commands that is executed, everytime the server starts.