# TCP Handshake Implementation using Raw Sockets

## Overview

This project demonstrates a TCP three-way handshake implementation using raw sockets in C++. It simulates the connection establishment phase of TCP by manually crafting packets with specific sequence numbers and TCP flags. The implementation follows the standard TCP handshake protocol but uses predefined sequence numbers for educational purposes.

## TCP Handshake Process

The TCP three-way handshake consists of three steps:

1. **SYN**: Client sends a SYN packet with sequence number 200.
2. **SYN-ACK**: Server responds with a SYN-ACK packet (sequence number 400, acknowledgment 201).
3. **ACK**: Client sends an ACK packet with sequence number 600 and acknowledgment 401.

This handshake establishes a reliable connection between the client and server before any data is transferred.

## Requirements

- Linux operating system (tested on Ubuntu/Debian).
- C++ compiler (g++).
- Root privileges (needed for raw sockets).
- Server and client code files.

## Code Explanation

The client code (`client.cpp`) implements the following functionality:

- Creates a raw socket for sending and receiving TCP packets.
- Manually constructs IP and TCP headers for precise control.
- Sends the initial SYN packet with sequence number 200.
- Listens for and processes the SYN-ACK response from the server.
- Sends the final ACK packet with sequence number 600 and acknowledgment 401.
- Logs the progress of the handshake.

The server implementation listens on port 12345 and follows the standard handshake protocol with predetermined sequence numbers.

## Compilation Instructions

Compile the client code using g++ using MakeFile:
```
make
```


## Running the Application

### Step 1: Start the Server

Open a terminal window and run the server with root privileges:

```
sudo ./server
```

The server will start listening on port 12345 and display a message indicating it's ready to accept connections.

### Step 2: Run the Client

Open another terminal window and run the client with root privileges:

```
sudo ./client
```

### Expected Output

#### Server Terminal
```
[sudo] password for user:
[+] Server listening on port 12345...
[+] TCP Flags:  SYN: 1 ACK: 0 FIN: 0 RST: 0 PSH: 0 SEQ: 200
[+] Received SYN from 127.0.0.1
[+] Sent SYN-ACK
[+] TCP Flags:  SYN: 0 ACK: 1 FIN: 0 RST: 0 PSH: 0 SEQ: 600
[+] Received ACK, handshake complete.
```

#### Client Terminal
```
[sudo] password for user:
[+] Sent SYN with sequence number 200
[+] Received SYN-ACK from 127.0.0.1 with server sequence number 400
[+] Sent ACK with sequence number 600 and acknowledgment 401
[+] Handshake complete.
```

## How the Connection Works

1. **Raw Socket Creation**: The client creates a raw socket with `SOCK_RAW` and `IPPROTO_TCP` to have full control over TCP packet headers.

2. **Initial SYN Packet**:
   - The client constructs a packet with the SYN flag set.
   - Sets sequence number to 200 (predefined value).
   - Sends the packet to the server at `127.0.0.1:12345`.

3. **SYN-ACK Response**:
   - Server receives the SYN packet.
   - Responds with a SYN-ACK packet.
   - Sets its sequence number to 400.
   - Sets the acknowledgment number to client's sequence number + 1 (201).

4. **Final ACK**:
   - Client receives the server's SYN-ACK.
   - Sends a final ACK packet with sequence number 600.
   - Sets acknowledgment to server's sequence number + 1 (401).
   - Handshake is now complete.

## Troubleshooting

If you encounter issues running the program:

1. **Permission Denied**: Ensure you're running with `sudo` or as root (required for raw sockets).
2. **Firewall Issues**: Check if your firewall is blocking the connections.
3. **Port Conflicts**: Make sure port `12345` is not in use by another application.
4. **Packet Filtering**: Some network configurations might filter raw packets.

## Additional Notes

- This implementation is for educational purposes and demonstrates the TCP handshake at a low level.
- It uses predefined sequence numbers for simplicity (`200`, `400`, `600`).
- The code only implements the connection establishment (handshake) without data transfer.
- Raw sockets require root privileges, which is why both programs need to be run with `sudo`.

## Member Contributions
- **Mohammed Sibtain Ansari** (210616) (33.33%): Coding part of the assignment
- **Avinash Shukla** (210236) (33.33%): Debugging and adding comments to the changes made.
- **Mantapuram Shreeja** (210592) (33.33%): Created README file.