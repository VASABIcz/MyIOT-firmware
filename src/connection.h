//
// Created by vasabi on 29.1.23.
//

#ifndef EMBEDED_XD_CONNECTION_H
#define EMBEDED_XD_CONNECTION_H

enum ConnectionType {
    Tcp,
    Ws,
    Http,
    BtLow
};

class Connection {
    ConnectionType type;

    virtual void disable() = 0;
    virtual void enable() = 0;

    virtual void handle() = 0;
    virtual void sendAll(void* data, int size) = 0;

    explicit Connection(ConnectionType type) {
        this->type = type;
    }
};

#endif //EMBEDED_XD_CONNECTION_H
