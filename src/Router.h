//
// Created by vasabi on 29.1.23.
//

#ifndef EMBEDED_ROUTER_H
#define EMBEDED_ROUTER_H

/*
 * packet
 * int - size
 * data
 *
 * capability-packet
 * int - size
 * string - name
 * string - description
 * string - route
 *
 * set-value-packet
 * int - size
 * string - route
 * string - datatype
 * string - data
 */

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <stdexcept>
#include "serialization.h"
#include "connection.h"

class MyResponse {
public:
    void* data;
    std::vector<ClassDescription>* description;
};

class BinaryReference {
public:
    int size;
    void* data;
};

class Capability {
public:
    std::string name;
    std::string route;
    std::string description;
    std::string dataType;

    virtual void input(Deserializer* deserializer) = 0;

    virtual MyResponse output() = 0;
};

template <typename T> class CapabilitySerializable: public Capability {
public:
    Serializable<T>* inner;
    MyResponse output() override {
        return MyResponse{inner->data, &inner->description};
    }
    void input(Deserializer* deserializer) override {
        bool err;
        deserializer->readString(err);
        auto res = deserialize(deserializer, inner->description);
        Serial.println("ahhheeeeeeeeeeeeeeeeeeeeeee");
        Serial.println((byte) (*(byte*)res));
        inner->swap(static_cast<T*>(res));
    }

    explicit CapabilitySerializable(std::string name, std::string route, std::string description, std::string dataType, Serializable<T>* serializable) {
        inner = serializable;
        this->name = name;
        this->route = route;
        this->description = description;
        this->dataType = dataType;
    }
};

class Router {
public:
    std::map<std::string, Capability*> capatibilites;
    std::map<ConnectionType, Connection*> connections;

    static Router* getInstance() {
        static Router* instance;
        if (instance == nullptr) {
            instance = new Router;
        }
        return instance;
    }

    BinaryReference handleRequest(void* data, int length) {
        bool err;
        auto des = BinaryDeserializer(data);
        // set, get, capabilities, ...
        std::unique_ptr<std::string> topLevel(des.readString(err));
        if (strcmp(topLevel->c_str(), "capabilities") == 0) {
            return getCapabilities();
        }
        else if (strcmp(topLevel->c_str(), "get") == 0) {
            auto capatibilityRoute = std::unique_ptr<std::string>(des.readString(err));
            return capabilityGet(*capatibilityRoute);
        }
        else if (strcmp(topLevel->c_str(), "set") == 0) {
            auto capatibilityRoute = std::unique_ptr<std::string>(des.readString(err));
            auto res = capatibilites[*capatibilityRoute];
            res->input(&des);
            return BinaryReference{};
        }
        return BinaryReference{};
    }

    template<typename T>
    void createCapability(std::string name, std::string route, std::string description, std::string dataType, Serializable<T> *data) {
        auto cap = new CapabilitySerializable<T>(name, route, description, dataType, data);
        capatibilites[route] = cap;
    }

    BinaryReference capabilityGet(const std::string& route) {
        auto res = capatibilites.find(route)->second;
        Serial.println("capabiliters");
        Serial.println(capatibilites.size());
        Serial.println((unsigned long) res);
        Serial.println("got capability");
        auto output = res->output();
        Serial.println("capability returned response");
        auto ser = BinarySerializer();
        Serial.println("created serializer");
        Serial.println((unsigned long )output.data);
        for (auto a : *output.description) {
            Serial.println(a);
        }
        ser.writeString(res->dataType.c_str(), res->dataType.size());
        serialize((char*)output.data, *output.description, &ser);
        Serial.println("successfully serialized");
        return BinaryReference{ser.dataOffset, ser.data};
    }

    void capabilitySet(const std::string& route, void* data) {
        auto res = capatibilites[route];
        auto des = BinaryDeserializer(data);
        res->input(&des);
    }

    void registerConnection(Connection* conn) {
        connections[conn->type] = conn;
    }

    void handle() {
        for (auto &c : connections) {
            c.second->handle();
        }
    }

    void enableConnection(ConnectionType connectionType) {
        auto con = connections.find(connectionType);
        if (con != connections.end()) {
            con->second->enable(this);
        }
    }

    void disableConnection(ConnectionType connectionType) {
        auto con = connections.find(connectionType);
        if (con != connections.end()) {
            con->second->disable();
        }
    }

    BinaryReference getCapabilities() {
        auto ser = BinarySerializer();
        ser.writeInt(capatibilites.size());

        for (const auto& pair : capatibilites) {
            auto cap = pair.second;
            ser.writeString(cap->route.c_str(), cap->route.size());
            ser.writeString(cap->name.c_str(), cap->name.size());
            ser.writeString(cap->description.c_str(), cap->description.size());
            ser.writeString(cap->dataType.c_str(), cap->dataType.size());
        }
        return BinaryReference{ser.dataOffset, ser.data};
    }
};

#endif //EMBEDED_ROUTER_H
