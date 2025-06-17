# CS425  Assignment 1

## Group Members:
- **Mohammed Sibtain Ansari** (210616)
- **Avinash Shukla** (210236)
- **Mantapuram Shreeja** (210592)


## Features Implemented
- User authentication
- Private messaging
- Broadcast messaging
- Group creation and management
- Group messaging

## Features Not Implemented
- Handling multiple login sessions for a single user
- Persistent message storage (messages are not retained after server shutdown).
- Secure communication (messages are transmitted in plain text).

## Design Decisions

### 1. Multithreading
We chose to create a new thread for each client connection. This allows for concurrent handling of multiple clients without blocking the main server thread.

Reason: This approach provides better scalability and responsiveness compared to a single-threaded or process-per-connection model, especially for a chat server that needs to handle multiple simultaneous connections.

### 2. Data Structures
We use the following data structures to manage clients, users, and groups:

- `std::unordered_map<int, std::string> clients`: Maps client sockets to usernames
- `std::unordered_map<std::string, std::string> users`: Maps usernames to passwords
- `std::unordered_map<std::string, std::unordered_set<int>> groups`: Maps group names to sets of client sockets

Reason: These data structures provide O(1) average time complexity for lookups, insertions, and deletions, which is crucial for efficient message routing and group management.

### 3. Synchronization
Used `std::mutex` with `std::lock_guard` for thread-safe access to shared resources (clients and groups maps).

Reason: This prevents data races and ensures consistent access to shared data across multiple threads.

### 4. User Authentication
User credentials are stored in a `users.txt` file and loaded into memory at server startup. This allows for quick authentication without needing to access the file for each login attempt.

### 5. Message Parsing

The server uses string parsing to interpret client commands. It checks for command prefixes (e.g., "/msg", "/broadcast") and extracts relevant information to execute the appropriate
actions.

### 6. Error Handling

The server implements basic error handling, such as checking for the existence of users or groups before performing operations. It sends appropriate error messages back to clients when operations cannot be completed.

## Compilation Instructions

To compile the server and client, use the following makefile command:

make -f Makefile

This will generate two executables: `server_grp` and `client_grp`.

### Running the Server

To start the server, run:

./server_grp

The server will start listening on port 12345 by default.

### Running a Client

To connect a client to the server, run:

./client_grp

Follow the prompts to enter your username and password.

## Supported Commands

- `/msg <username> <message>`: Send a private message to a user
- `/broadcast <message>`: Send a message to all users
- `/create_group <group_name>`: Create a new group
- `/join_group <group_name>`: Join an existing group
- `/group_msg <group_name> <message>`: Send a message to a group
- `/leave_group <group_name>`: Leave a group

## Implementation Details

### High-level Function Overview
- `handle_client()`: Manages the entire lifecycle of a client connection
- `authenticate()`: Verifies user credentials
- `broadcast_message()`: Sends a message to all connected clients
- `private_message()`: Sends a message to a specific user
- `group_message()`: Sends a message to all members of a group
- `join_group()`: Adds a client to a group
- `leave_group()`: Removes a client from a group



### Authentication
- The `load_users()` function reads user credentials from `users.txt` at server startup.
- The `authenticate()` function checks provided credentials against the loaded user data.

### Message Handling
- The `broadcast_message()` function sends a message to all connected clients except the sender.
- The `private_message()` function sends a message to a specific user.
- The `group_message()` function sends a message to all members of a specified group.

### Group Management
- The `join_group()` function adds a client to a group.
- The `leave_group()` function removes a client from a group.
- Group creation is handled in the main client handling loop.

### Client Handling
- The `handle_client()` function manages the entire lifecycle of a client connection, from authentication to command processing and disconnection.


---

## Code Flow Overview

### Initialization
- **Load Users:**  
  - Read `users.txt` to populate the `users` map (username → password).
- **Global Structures:**  
  - `clients`: Maps client socket to username.  
  - `groups`: Maps group name to a set of client sockets.  
- **Thread Safety:**  
  - Use `clients_mutex` and `groups_mutex` with `std::lock_guard` for synchronized access.

### Socket Setup & Connection
- **Create & Bind Socket:**  
  - Set up a TCP socket bound to port `12345`.
- **Listen & Accept:**  
  - Listen for incoming connections.
  - For each accepted connection, spawn a new thread running `handle_client()`.

### Client Handling (`handle_client()`)
- **Authentication:**  
  - Prompt client for username and password.
  - Validate credentials using the `authenticate()` function.
  - On success:
    - Send welcome message.
    - Add client to `clients`.
    - Broadcast a join message to other clients.
- **Command Processing:**  
  - **Private Message (`/msg`):**  
    - Send a direct message to a specified user.
  - **Broadcast (`/broadcast`):**  
    - Send a message to all connected clients.
  - **Group Management:**
    - **Create Group (`/create_group`):**  
      - Create a new group if it doesn’t exist.
    - **Join Group (`/join_group`):**  
      - Add client to an existing group.
    - **Group Message (`/group_msg`):**  
      - Parse group names (including spaces) and send messages to group members.
    - **Leave Group (`/leave_group`):**  
      - Remove client from the group.
  - **Exit (`/exit`):**  
    - Broadcast a leaving message, then clean up the client.
  
- **Cleanup:**  
  - Remove the client from the `clients` map and close the socket when the client disconnects.

---


## Testing

### Correctness Testing
- Tested each command individually to ensure proper functionality
- Tested multiple clients connecting simultaneously
- Verified group creation, joining, and messaging

### Stress Testing
- Connected multiple clients to evaluate concurrency and synchronization.  
- Transmitted large messages to assess buffer limitations and handling.  
- Simulated random client disconnections to verify server stability.

## Restrictions
- Maximum number of clients: Constrained by system and network capabilities.
- Maximum number of groups: No fixed limit, but dependent on available memory.
- Maximum group size: No defined limit, but performance may decline with a high number of users in a group.
- Maximum message size: MAX_BUFFER is configured to allow 1024 bytes per message.

## Challenges Faced
1. Ensuring thread-safety for shared resources
   Solution: Implemented proper mutex locking
2. Handling disconnections gracefully
   Solution: Added error checking and cleanup procedures

## Member Contributions
- **Mohammed Sibtain Ansari** (210616) (33.33%): Implemented each functions and multi-threading
- **Avinash Shukla** (210236) (33.33%): Updated and debugged the main code to add additional features.
- **Mantapuram Shreeja** (210592) (33.33%): Did stress testing and created README file.

## Sources Referred
- **Beej’s Guide to Network Programming** – A comprehensive guide covering network programming concepts, primarily in C.  
- **C++ Documentation (cppreference.com)** – A detailed reference for C++ language features, including networking libraries.  
- **Stack Overflow Discussions on Socket Programming** – Community-driven Q&A platform for troubleshooting and best practices in socket programming.



## Declaration
We declare that we have not engaged in any form of plagiarism in the completion of this assignment.


