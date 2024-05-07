/**
 * @file client.cpp
 * @author Nicola Sabino (nicola.sabino94@gmail.com)
 * @brief Simple client application
 * @version 0.1
 * @date 2024-05-07
 */
#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

/**
 * @brief Send request to server
 * 
 * @param request The request string
 * @return std::string The server response
 */
std::string sendRequest(const std::string &request)
{
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Socket creation error" << std::endl;
        return "";
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(12345); // Server port number

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return "";
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection Failed" << std::endl;
        return "";
    }

    // Send request to server
    send(sock, request.c_str(), request.length(), 0);
    valread = read(sock, buffer, 1024);
    return std::string(buffer, valread);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " --load <file_path>" << std::endl;
        std::cerr << "       " << argv[0] << " --get <key>" << std::endl;
        std::cerr << "       " << argv[0] << " --set <key> <value>" << std::endl;
        return 1;
    }

    std::string command = argv[1];
    std::string arg1 = argv[2];
    std::stringstream ss;

    // we faced some issues with "" quoted parameters
    // in this way we merge the all in one string
    for (int i = 3; i < argc; ++i) ss << argv[i] << " ";
    std::string arg2 = ss.str();

    std::string request;
    if (command == "--load")
    {
        request = "LOAD " + arg1 + "\n";
    }
    else if (command == "--get")
    {
        request = "GET " + arg1 + "\n";
    }
    else if (command == "--set")
    {
        request = "SET " + arg1 + " " + arg2 + "\n";
    }
    else
    {
        std::cerr << "Invalid command" << std::endl;
        return 1;
    }

    std::string response = sendRequest(request);
    std::cout << response << std::endl;

    return 0;
}
