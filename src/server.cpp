#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>
#include "ini_handler.h"

constexpr int PORT = 12345;
std::mutex mtx;

// Function to handle client requests
void handle_client(int client_socket) {
    char buffer[1024] = {0};
    int valread = read(client_socket, buffer, 1024);
    if (valread > 0) {
        std::string request(buffer, valread);

        if (request.find("LOAD") == 0) { // not-safe concurrent operation
            std::lock_guard<std::mutex> guard(mtx);
            std::string path = request.substr(5); // Remove "LOAD " from the request
            const auto result = load_resource(path);
            std::string response = std::to_string(result) + "\n";
            send(client_socket, response.c_str(), response.size(), 0);
        } 
        else if(request.find("GET") == 0) // safe concurrent operation
        {
            std::string key = request.substr(4); // Remove "GET" from the request
            std::string value;
            const auto result = get_value(key,value);
            std::stringstream ss;
            ss << std::to_string(result) << " " << value << std::endl;
            send(client_socket, ss.str().c_str(), ss.str().size(), 0);
        }
        else if(request.find("SET") == 0) // not-safe concurrent operation
        {
            std::lock_guard<std::mutex> guard(mtx);
            std::istringstream iss(request);
            std::string command, key, value;
            iss >> command >> key >> value;
            const auto result = set_value(key, value);
            std::string response = std::to_string(result) + "\n";
            send(client_socket, response.c_str(), response.size(), 0);
        }
        else {
            std::string response = "127\n";
            send(client_socket, response.c_str(), response.size(), 0);
        }
    }
    close(client_socket);
}

int main() {
    int server_socket, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    // create a new thread for each request
    while (true) {
        if ((new_socket = accept(server_socket, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        std::thread(handle_client, new_socket).detach();
    }

    return 0;
}
