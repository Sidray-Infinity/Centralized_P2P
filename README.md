# Centralized Peer-to-Peer architecture for File Sharing

The aim of this project is to provide a file sharing system, among the peers connected to the network, administered by a centralized server.

### Requirements
* Linux based Operating System.
* Internet Connection

### Installation Process

* Download the server.c and client.c files.

- #### Server side
* Installing MySQL
    * Execute the following commands on Ubuntu:
        `sudo apt-get update`
        `sudo apt-get install mysql-server`

* Create a new database 'p2p' and four tables to it, namely:
    * login_details

| Field     | Type       | Null | Key | Default | Extra |
|-----------|------------|------|-----|---------|-------|
| log_id    | int        | NO   | PRI | NULL    |       |
| user_name | text       | YES  |     | NULL    |       |
| password  | text       | YES  |     | NULL    |       |
| isOnline  | tinyint(1) | YES  |     | NULL    |       |

|A|B|C|AA|
| --- | --- | --- | --- |
|Q|W   |    E | RR     |

* online_clients
* files
* blocks




### Server Roles
* To provide registration and authentication services 


### TASKS
- Handle the problem of 0 byte files.
- Files seems to be merging properly, but with some additional chars. Handle it.
- Retentering username after entering used one is buggy. Probably due to use of goto.