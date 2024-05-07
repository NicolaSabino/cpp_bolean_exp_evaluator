/**
 * @file server.cpp
 * @author Nicola Sabino (nicolasabino94@gmail.com)
 * @brief Simple server
 * @version 0.1
 * @date 2024-05-07
 * 
 * @see https://www.bogotobogo.com/cplusplus/sockets_server_client.php
 * @see https://github.com/MartinoMensio/DP1-Labs/blob/master/lab1/lab1es04/lab1es04_server.c
 * 
 */
#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <mutex>
#include <csignal>
#include <cstdlib>
#include <condition_variable>
#include <fcntl.h>
#include <chrono>
#include "ini_handler.h"

constexpr int PORT = 12345;
constexpr auto GET_COMMAND = "GET";
constexpr auto SET_COMMAND = "SET";
constexpr auto LOAD_COMMAND = "LOAD";
constexpr auto ERR_RESPONSE = "127\n";
constexpr auto BACKLOG_QUEUE_SIZE = 3;
std::mutex mtx;
bool terminate_program = false;

/**
 * @brief Singal handler
 * 
 * Quit the server on SIGINT
 * 
 * @param signal The received siganl
 */
void sigintHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "SIGINT received. Terminating application..." << std::endl;
        terminate_program = true;
    }
}

/**
 * @brief Client handler thread body
 * 
 * Each thread wrap a socket connection
 * and interaction with client
 * 
 * It implements the following request type
 *  * LOAD
 *  * GET
 *  * SET
 * 
 * If the request does not contain a valid method
 * we send back code 127.
 * Otherwise the correspondig response processed by
 * the ini handler library is forwarded.
 * 
 * @param clientSocket thread socket descriptor
 */
void handleClient(int clientSocket) {
    char buffer[1024] = {0};
    int valread = read(clientSocket, buffer, 1024);
    if (valread > 0) {
        std::string request(buffer, valread);

        if (request.find(LOAD_COMMAND) == 0) { 
            std::lock_guard<std::mutex> guard(mtx); // not-safe concurrent operation
            std::string path = request.substr(5); // Remove "LOAD " from the request
            const auto result = load_resource(path);
            std::string response = std::to_string(result) + "\n";
            send(clientSocket, response.c_str(), response.size(), 0);
        } 
        else if(request.find(GET_COMMAND) == 0) // safe concurrent operation
        {
            std::string key = request.substr(4); // Remove "GET" from the request
            std::string value;
            const auto result = get_value(key,value);
            std::stringstream ss;
            ss << std::to_string(result) << " " << value << std::endl;
            send(clientSocket, ss.str().c_str(), ss.str().size(), 0);
        }
        else if(request.find(SET_COMMAND) == 0) 
        {
            std::lock_guard<std::mutex> guard(mtx); // not-safe concurrent operation
            std::istringstream iss(request);
            std::string command, key, value;
            iss >> command >> key;
            // Read the rest of the line as the value
            std::getline(iss, value);
            // Remove leading and trailing whitespaces
            value = value.substr(value.find_first_not_of(" \t"));
            const auto result = set_value(key, value);
            std::string response = std::to_string(result) + "\n";
            send(clientSocket, response.c_str(), response.size(), 0);
        }
        else {
            std::string response = ERR_RESPONSE;
            send(clientSocket, response.c_str(), response.size(), 0);
        }
    }
    close(clientSocket);
    pthread_exit(NULL);
}

int main() {
    int server_socket, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // Install SIGINT signal handler
    std::signal(SIGINT, sigintHandler);

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
    
    if (listen(server_socket, BACKLOG_QUEUE_SIZE) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    // Set the server socket to non-blocking mode
    int flags = fcntl(server_socket, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }

    flags |= O_NONBLOCK; // Add non-blocking flag
    if (fcntl(server_socket, F_SETFL, flags) == -1) {
        perror("fcntl");
        exit(EXIT_FAILURE);
    }

    std::vector<std::thread> threads;
    // create a new thread for each request
    while (!terminate_program) {
        if ((new_socket = accept(server_socket, (struct sockaddr *)&address, &addrlen)) < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // No pending connections, sleep for 500ms then continue loop
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
                continue;
            } else {
                perror("accept");
                exit(EXIT_FAILURE);
            }
        }

        std::thread t(handleClient, new_socket);
        threads.push_back(std::move(t));
    }


    for (auto& thread: threads)
    {
        thread.join();
    }

    std::cout << "All threads terminated. Application exiting." << std::endl;
    return 0;
}
