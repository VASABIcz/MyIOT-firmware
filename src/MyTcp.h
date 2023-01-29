//
// Created by vasabi on 29.1.23.
//

#ifndef EMBEDED_XD_MYTCP_H
#define EMBEDED_XD_MYTCP_H

WiFiServer tcpServer(420);

void tcpSetup() {
    tcpServer.begin();
}

void tcpHandle() {
    auto r = Router::getInstance();

    auto client = tcpServer.available();
    if (!client.available()) return;
    Serial.println("handling client / waiting");

    int msgSize;
    client.readBytes((char*)&msgSize, sizeof msgSize);
    char buffer[msgSize];
    auto size = client.readBytes(buffer, msgSize);
    if (size < 4) {
        Serial.println("warning ignoring tcp message");
        return;
    }

    auto res = r->handleRequest(buffer, size);
    client.write((char*)&res.size, sizeof res.size);
    client.write((char*)res.data, res.size);
}

#endif //EMBEDED_XD_MYTCP_H
