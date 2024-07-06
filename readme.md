# MyNetcat program

## Introduction

In this project, we developed our own netcat program. This program supports open communications across various protocols, enabling routing of standard input (stdin) and standard output (stdout) through socket file descriptors based on the specified flags. This allows us to facilitate communication between users by opening clients and servers.

## Authors

This project was developed collaboratively by:
- [Gal Ben Ami](https://github.com/benami171)
- [Chanan Helman](https://github.com/chanan-hash)

## Flags
1. -e: Executes a shell command
2. -i: Gets input from a socket.
3. -o: Directs the output to a socket.
4. -b: Sends input and output to the same socket.
5. -t: Sets a timeout for the execution of the program when using the **UDP** protocol.

## Protocols
This program supports four types of protocols:

1. **TCP**
2. **UDP**
3. **UNIX Domain Socket Stream**
4. **UNIX Domain Socket Datagram**

You can also mix protocols, for example, by opening a **UDP** server and connecting with a **TCP** client.

> Note: All datagram sockets (UDP and UDS datagram) will wait for dummy input from the client to accept the connection before starting the execution of the program.


## How to use
First, clone the repository using the following command:

```bash
git clone https://github.com/chanan-hash/OS-Ex2.git
```
Each folder has its own `Makefile` for each step. The final version is in the `Q6` folder, which includes all the protocols. A recursive `Makefile` is also available if needed.

### sketch for the commands
To use the sockets, follow this format after the flag:

1. TCP server: `TCPS<PORT>`
2. TCP client: `TCPC<HOST>,<PORT>`
3. UDP server: `UDPS<PORT>`
4. UDP client: `UDPC<HOST>,<PORT>`
5. UDS stream server: `UDSSS<PATH>`
6. UDS stream client: `UDSCS<PATH>`
7. UDS datagram server: `UDSSD<PATH>`
8. UDS datagram client: `UDSCD<PATH>`

## Testing
To test and communicate with our program, we used Linux netcat to avoid writing separate clients and servers for each protocol. The list of commands and usage instructions are in the ```Q6``` folder under ***work_list.txt***.

### -e flag
In the examples below, the `-e`  flag is used to execute a command for a tic-tac-toe game implemented in the ```ttt.c```  file in each folder. Executing the command without the `-e` flag will simply facilitate commuincation between two ends, like a chat.

## Command exmpales:
Opening a TCP server and waiting for input from a client. Output goes to stdout:

```bash
# server
./mync -e "./ttt 123456789" -i TCPS9876
# client
nc localhost 9876
```

Oppening a TCP client and wait for a input from a server. output go to the client

```bash
#server
nc -l -p 9876
#client
/mync -e "./ttt 123456789" -o TCPC127.0.0.1,9876
```
For the whole command list you can check in
[Q6-worklist](./Q6/works_list.txt)
