#include "Core/TCPConnection.hpp"

#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

namespace Core {
    namespace Network {
        TCPServer::TCPServer(int port) {
            server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd == -1) {
                success = false;
                return;
            }
            
            addr = {};
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(port);

            if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
                success = false;
                return;
            }

            success = true;
        }

        TCPServer::~TCPServer() {
            if (client_fd > 0) close(client_fd);
            if (server_fd > 0) close(server_fd);
            client_fd = -1;
            server_fd = -1;
        }

        std::string TCPServer::getHostName() {
            struct in_addr host_id;
            host_id.s_addr = gethostid();
            if (host_id.s_addr == INADDR_NONE)
                return "";
            return std::string(inet_ntoa(host_id));
        }

        bool TCPServer::waitConnection() {
            if (listen(server_fd, 1) < 0)
                return false;
            client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd < 0)
                return false;
            return true;
        }

        bool TCPServer::send(void *data, size_t size) {
            return size == write(client_fd, data, size);
        }

        bool TCPServer::recv(void *buffer, size_t size) {
            size_t len = read(client_fd, buffer, size);
            return len == size;
        }
    }
}