## Banking System using Client-Server socket programming

A simple client-server application for banking system which can be used for checking balance, mini statement, crediting and debiting amount.

* Compile the server and client source files by following commands.
gcc client.c -o client
gcc server.c -o server

* server execute
./server <port_number>  # ./server 6345

* client execute
./client <server_ip_address> <port_number>  # ./client 127.0.0.1 6345

## Features

This application supports 3 types of users with different requirements:
#### Bank_Customer
#### Bank_Admin
#### Police

