//
// Created by vasabi on 29.1.23.
//

#include "util.h"

#ifndef EMBEDED_XD_MYTCP_H
#define EMBEDED_XD_MYTCP_H

WiFiServer tcpServer(420);

class TcpConnection: public Connection {
    Router* router{};
    bool isEnabled = false;
    std::vector<WiFiClient> clients;
public:
    void enable(Router *router) override {
        Serial.println("enabling tcp");
        isEnabled = true;
        this->router = router;
        tcpServer.begin();
    }

    void disable() override {
        Serial.println("disabling tcp");
        isEnabled = false;

        // clear clients
        clients = std::vector<WiFiClient>();

        tcpServer.stop();
    }

    void loadClients() {
        auto client = tcpServer.available();
        clients.push_back(client);
    }

    void pruneClients() {
        // could be better
        std::vector<WiFiClient> aliveClients;

        for (auto c : clients) {
            if (c.connected()) {
                aliveClients.push_back(c);
            }
        }

        clients = aliveClients;
    }

    void handleConnections() {
        for (auto &client : clients) {
            if (client.available() < 4 || !client.connected()) continue;

            Serial.println("handling client");

            int msgSize;
            auto size = client.readBytes((char*)&msgSize, sizeof msgSize);
            if (isBigEndian()) {
                msgSize = reverseBytes(msgSize);
            }

            printf("msg size %d\n", msgSize);

            if (size < 4) {
                Serial.printf("read %d required 4 bytes (msg len)\n", size);
                return;
            }


            char buffer[msgSize];
            size = client.readBytes(buffer, msgSize);

            if (size < msgSize) {
                Serial.printf("read %d required %d bytes (msg data)\n", size, msgSize);
                return;
            }

            auto res = router->handleRequest(buffer, size);

            printf("sending tcp %d\n", res.size);

            writeData(client, res.data, res.size);
        }
    }

    void handle() override {
        if (!isEnabled) return;

        loadClients();
        handleConnections();
        pruneClients();
    }

    void broadCast(void *data, int size) override {
        for (auto &c : clients) {
            writeData(c, data, size);
        }
    }

    static void writeData(WiFiClient& client, void* data, int length) {
        int rawLength;

        if (isBigEndian()) {
            rawLength = reverseBytes(length);
        }
        else {
            rawLength = length;
        }

        client.write((char*)&rawLength, sizeof rawLength);
        client.write((char*)data, length);
    }

    TcpConnection() : Connection(Tcp) {}
};

#endif //EMBEDED_XD_MYTCP_H
