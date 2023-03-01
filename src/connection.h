//
// Created by vasabi on 29.1.23.
//

#ifndef EMBEDED_XD_CONNECTION_H
#define EMBEDED_XD_CONNECTION_H

#include "Router.h"

class Router;

enum ConnectionType {
    Tcp,
    Ws,
    Http,
    BtLow
};

class Connection {
public:
    ConnectionType type;

    virtual void disable() = 0;
    virtual void enable(Router* router) = 0;

    virtual void handle() = 0;
    virtual void broadCast(void* data, int size) = 0;

    Connection(ConnectionType type) {
        this->type = type;
    }
};

#endif //EMBEDED_XD_CONNECTION_H
