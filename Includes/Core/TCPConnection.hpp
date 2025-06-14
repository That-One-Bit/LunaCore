#pragma once

#include <string>

#include <stddef.h>

#include <netinet/in.h>

namespace Core {
    namespace Network {
        class TCPServer {
            int server_fd = -1;
            int client_fd = -1;
            sockaddr_in addr;
            bool success = true;
            
            public:
                TCPServer(int port);
                ~TCPServer();

                std::string getHostName();

                bool waitConnection();

                bool send(void *data, size_t size);

                bool recv(void *buffer, size_t size);
        };
    }
}