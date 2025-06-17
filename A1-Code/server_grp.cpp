#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 12345
#define BUFFER_SIZE 1024

// Mapping from client socket to username
std::unordered_map<int, std::string> clients; 
// Mapping from username to password (loaded from users.txt)
std::unordered_map<std::string, std::string> users; 
// Mapping from group name to set of client sockets
std::unordered_map<std::string, std::unordered_set<int>> groups; 
std::mutex clients_mutex, groups_mutex;

void load_users() {
    std::ifstream file("users.txt");
    std::string line, username, password;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        if (std::getline(iss, username, ':') && std::getline(iss, password)) {
            users[username] = password;
        }
    }
}

bool authenticate(int client_socket, const std::string& username, const std::string& password) {
    auto it = users.find(username);
    return (it != users.end() && it->second == password);
}

void broadcast_message(int sender_socket, const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& client : clients) {
        if (client.first != sender_socket) {
            send(client.first, message.c_str(), message.length(), 0);
        }
    }
}

void private_message(int sender_socket, const std::string& recipient, const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& client : clients) {
        if (client.second == recipient) {
            std::string formatted_message = "[" + clients[sender_socket] + "]: " + message;
            send(client.first, formatted_message.c_str(), formatted_message.length(), 0);
            return;
        }
    }
    send(sender_socket, "ERROR: User not found or not online.", 36, 0);
}

void join_group(int client_socket, const std::string& group_name) {
    std::lock_guard<std::mutex> lock(groups_mutex);
    if (groups.find(group_name) != groups.end()) {
        groups[group_name].insert(client_socket);
        std::string reply = "You joined the group " + group_name + ".";
        send(client_socket, reply.c_str(), reply.length(), 0);
    } else {
        send(client_socket, "ERROR: Group does not exist.", 29, 0);
    }
}

void leave_group(int client_socket, const std::string& group_name) {
    std::lock_guard<std::mutex> lock(groups_mutex);
    if (groups.find(group_name) != groups.end()) {
        if(groups[group_name].find(client_socket)==groups[group_name].end()){
            send(client_socket,"ERROR: Group not joined",23,0);
            return;
        }
        groups[group_name].erase(client_socket);
        send(client_socket, "Left group successfully.", 25, 0);
    } else {
        send(client_socket, "ERROR: Group does not exist.", 29, 0);
    }
}

void group_message(int sender_socket, const std::string& group_name, const std::string& message) {
    std::lock_guard<std::mutex> lock(groups_mutex);
    if (groups.find(group_name) != groups.end()) {
        std::string formatted_message = "[Group " + group_name + "][" + clients[sender_socket] + "]: " + message;
        if(groups[group_name].find(sender_socket)==groups[group_name].end()){
            send(sender_socket,"ERROR: Group not joined",23,0);
            return;
        }
        for (int client_socket : groups[group_name]) {
            if (client_socket != sender_socket) {
                send(client_socket, formatted_message.c_str(), formatted_message.length(), 0);
            }
        }
    } else {
        send(sender_socket, "ERROR: Group does not exist.", 29, 0);
    }
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    std::string username, password;

    // --- Authentication ---
    send(client_socket, "Enter username: ", 16, 0);
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    username = std::string(buffer);
    // Remove any trailing newline characters
    username.erase(username.find_last_not_of("\n\r") + 1);

    send(client_socket, "Enter password: ", 16, 0);
    memset(buffer, 0, BUFFER_SIZE);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    password = std::string(buffer);
    password.erase(password.find_last_not_of("\n\r") + 1);

    if (!authenticate(client_socket, username, password)) {
        send(client_socket, "Authentication failed.", 22, 0);
        close(client_socket);
        return;
    }

    // Send welcome message to the newly authenticated client.
    send(client_socket, "Welcome to the chat server!", 27, 0);

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients[client_socket] = username;
    }

    // Broadcast to all other clients that this user has joined.
    broadcast_message(client_socket, username + " has joined the chat.");

    // --- Main loop to process client commands ---
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            break;
        }

        std::string message(buffer);

        if (message.starts_with("/msg")) {
            // Private message: /msg <username> <message>
            size_t space1 = message.find(' ');
            size_t space2 = message.find(' ', space1 + 1);
            if (space1 != std::string::npos && space2 != std::string::npos) {
                std::string recipient = message.substr(space1 + 1, space2 - space1 - 1);
                std::string msg = message.substr(space2 + 1);
                private_message(client_socket, recipient, msg);
            }
        } else if (message.starts_with("/broadcast")) {
            // Broadcast message: /broadcast <message>
            std::string msg = message.substr(11); // Skip "/broadcast " (10 characters + space)
            broadcast_message(client_socket, "[" + username + "]: " + msg);
        } else if (message.starts_with("/create_group")) {
            // Create a new group: /create_group <groupname>
            size_t pos = message.find(' ');
            if (pos != std::string::npos) {
                std::string group_name = message.substr(pos + 1);
                group_name.erase(group_name.find_last_not_of("\n\r") + 1);
                std::lock_guard<std::mutex> lock(groups_mutex);
                if (groups.find(group_name) != groups.end()) {
                    send(client_socket, "Group already exists.", 23, 0);
                } else {
                    groups[group_name] = std::unordered_set<int>();
                    std::string response = "Group " + group_name + " created.";
                    send(client_socket, response.c_str(), response.length(), 0);
                }
            } else {
                send(client_socket, "Invalid command format for group creation.", 45, 0);
            }
        } else if (message.starts_with("/join_group")) {
            // Join an existing group: /join_group <groupname>
            std::string group_name = message.substr(12); // Skip "/join_group " (11 characters + space)
            group_name.erase(group_name.find_last_not_of("\n\r") + 1);
            join_group(client_socket, group_name);
        } else if (message.starts_with("/group_msg")) {
            // Send a message to a group: /group_msg <group name> <message>
            // We need to allow group names with spaces.
            // First, remove the command prefix.
            std::string args = message.substr(10); // "/group_msg" is 10 characters.
            // Trim any leading spaces.
            while (!args.empty() && args.front() == ' ') {
                args.erase(0, 1);
            }
            if (args.empty()) {
                send(client_socket, "No group specified.", 21, 0);
                continue;
            }
            // Find the longest matching group name that is a prefix of args.
            std::string matched_group = "";
            {
                std::lock_guard<std::mutex> lock(groups_mutex);
                for (const auto &entry : groups) {
                    const std::string &group_name = entry.first;
                    if (args.size() >= group_name.size() &&
                        args.compare(0, group_name.size(), group_name) == 0) {
                        // Ensure the group name is a full token (either args equals the group name,
                        // or the character following it is a space)
                        if (args.size() == group_name.size() || args[group_name.size()] == ' ') {
                            if (group_name.size() > matched_group.size()) {
                                matched_group = group_name;
                            }
                        }
                    }
                }
            }
            if (matched_group.empty()) {
                send(client_socket, "Group does not exist.", 22, 0);
                continue;
            }
            // Extract the group message: skip the matched group and any following space(s).
            std::string group_msg = "";
            if (args.size() > matched_group.size()) {
                group_msg = args.substr(matched_group.size());
                while (!group_msg.empty() && group_msg.front() == ' ')
                    group_msg.erase(0, 1);
            }
            if (group_msg.empty()) {
                send(client_socket, "No message provided for group message.", 40, 0);
                continue;
            }
            group_message(client_socket, matched_group, group_msg);
        } else if (message.starts_with("/leave_group")) {
            // Leave a group: /leave_group <groupname>
            std::string group_name = message.substr(13); // Skip "/leave_group " (12 characters + space)
            group_name.erase(group_name.find_last_not_of("\n\r") + 1);
            leave_group(client_socket, group_name);
        }
        else if(message.starts_with("/exit")){
            // std::lock_guard<std::mutex> lock(clients_mutex);
            std::string left_message = clients[client_socket] + " has left the chat.";
            broadcast_message(client_socket, left_message);
            // clients.erase(client_socket);
            // close(client_socket);
            // return;
        }
        
        // (Additional commands can be added here.)
    }

    // Cleanup on disconnect: remove client and broadcast leaving message.
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        // std::string left_message = clients[client_socket] + " has left the chat.";
        clients.erase(client_socket);
        // broadcast_message(client_socket, left_message);
    }
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    load_users();

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Error creating server socket." << std::endl;
        return 1;
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error binding server socket." << std::endl;
        return 1;
    }
    if (listen(server_socket, 5) < 0) {
        std::cerr << "Error listening on server socket." << std::endl;
        return 1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            std::cerr << "Error accepting client connection." << std::endl;
            continue;
        }
        std::thread(handle_client, client_socket).detach();
    }

    close(server_socket);
    return 0;
}
