# MyNetcat program

In this project we;ve created our own netcat program.  
It's open communications in differetn kinds of protocols.  
We can route our input and outout also between users, be oppening clients and servers.

## Flags
1. -e, executes a shell command
2. -i, get input from a socket
3. -o, refering the output to a socket
4. -b, input and output go to the same socket
5. -t, in **UDP** protocol it sets timeout for the execution of the program

## Protocols
This program supports 4 kind of protocols:
1. **TCP**
2. **UDP**
3. **UNIX Domain Socket Stream**
4. **UNIX Domain Socket Datagram**

we can also mix the protocols, opening a **UDP** server, and connecting with **TCP** client.

> NOTE: All datagram sockets (UDP and UDS datagram) will be wait for a dummy input from the client for accepting the connection, and then start the execution of the program.

## How to use
First clone the repository, here the command
```bash
git clone https://github.com/chanan-hash/OS-Ex2.git
```
Each file has it's own ```makefile``` for each step. the final one is in ```Q6``` folder. It has all the protocols
we have also a recursive ```makefile``` if needed.

### sketch for the commands
to use the socket we will send them after the flag, in this format:
1. TCP server: `TCPS<PORT>`
2. TCP client: `TCPC<HOST>,<PORT>`
3. UDP server: `UDPS<PORT>`
4. UDP client: `UDPC<HOST>,<PORT>`
5. UDS stream server: `UDSSS<PATH>`
6. UDS stream client: `UDSCS<PATH>`
7. UDS datagram server: `UDSSD<PATH>`
8. UDS datagram client: `UDSCD<PATH>`

## Checking
To check and comunicate with our program, we;ve used also linux netcat, to save as wrting more clients and server for each protocol for checkups.
The list of command and how to use them are in ```Q6``` folder under ***work_list.txt***

### -e flag
in all of the examples below, when we use the -e flag, we will execute the command a tik tak toe game, that we've implemented, this is in ```ttt.c``` file in each folder.  

executing the command without -e flag, will just refer the communication between side without special program, like a chat.

## Command exmpales:
Oppening a TCP server and wait for a input from a client. output go to stdout
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
