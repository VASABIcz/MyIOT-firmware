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
public:

    void enable(Router *router) override {
        Serial.println("enabling tcp");
        isEnabled = true;
        this->router = router;
        tcpServer.begin();
    }

    void disable() override {
        tcpServer.stop();
    }

    void handle() override {
        if (!isEnabled) return;

        auto client = tcpServer.available();
        if (!client.available()) return;
        Serial.println("handling client / waiting");

        int msgSize;
        auto size = client.readBytes((char*)&msgSize, sizeof msgSize);
        msgSize = reverseBytes(msgSize);

        printf("msg size %d\n", msgSize);

        if (size < 4) {
            Serial.printf("read %d required 4 (msg len)\n", size);
            return;
        }


        char buffer[msgSize];
        size = client.readBytes(buffer, msgSize);

        if (size < msgSize) {
            Serial.printf("read %d required %d (msg data)\n", size, msgSize);
            return;
        }

        auto res = router->handleRequest(buffer, size);
        int rawSize;

        if (isBigEndian()) {
            rawSize = reverseBytes(res.size);
        }
        else {
            rawSize = res.size;
        }

        printf("sending tcp %d\n", res.size);

        client.write((char*)&rawSize, sizeof res.size);
        client.write((char*)res.data, res.size);
    }

    void broadCast(void *data, int size) override {
        // TODO
        // tcpServer.availableForWrite();
    }

    TcpConnection() : Connection(Tcp) {}
};

#endif //EMBEDED_XD_MYTCP_H
