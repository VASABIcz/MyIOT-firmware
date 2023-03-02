//
// Created by vasabi on 29.1.23.
//

#include <Arduino.h>
#include "Router.h"
#include "Arduino.h"
#include "connection.h"
#include "util.h"

#ifndef EMBEDED_MYHTTP_H
#define EMBEDED_MYHTTP_H

WebServer server(80);

class SyncHttpConnection: public Connection {
public:
    explicit SyncHttpConnection() : Connection(Http) {}

    void broadCast(void *data, int size) override {}

    void handle() override {
        server.handleClient();
    }

    void enable(Router* router) override {
        Serial.println("enabling SyncHttpConnection");
        server.on("/", [](){
            server.send(200, "text/plain", "UwU");
        });


        server.on("/api/capabilities", []() {
            auto r = Router::getInstance();
            auto ser = BinarySerializer();
            ser.writeInt(r->capatibilites.size());

            for (const auto& pair : r->capatibilites) {
                auto cap = pair.second;
                ser.writeString(cap->route.c_str(), cap->route.size());
                ser.writeString(cap->name.c_str(), cap->name.size());
                ser.writeString(cap->description.c_str(), cap->description.size());
                ser.writeString(cap->dataType.c_str(), cap->dataType.size());
            }

            Serial.println("sending");
            Serial.println(ser.dataOffset);
            auto str = base64_encode((unsigned char*)ser.data, ser.dataOffset);
            server.send(200, "text/plain", str.c_str());
        });

        for (const auto& pair : router->capatibilites) {
            auto cap = pair.second;

            server.on(cap->route.c_str(), HTTP_POST, [cap](){
                auto ser = BinarySerializer();
                Serial.println("received post request");

                auto body = server.arg("plain");
                auto bytes = base64_decode(body.c_str());
                auto r = Router::getInstance();

                auto des = BinaryDeserializer(bytes.data());
                bool err;
                des.readString(err);

                r->capabilitySet(cap->route, des);
                r->capabilityGet(cap->route, ser);

                auto str = base64_encode((unsigned char*)ser.data, ser.dataOffset);

                server.send(200, "text/plain", str.c_str());
            });
            server.on(cap->route.c_str(), HTTP_GET, [cap](){
                auto ser = BinarySerializer();
                Serial.println("received get request");
                auto r = Router::getInstance();

                r->capabilityGet(cap->route, ser);


                auto str = base64_encode((unsigned char*)ser.data, ser.dataOffset);

                server.send(200, "text/plain", str.c_str());
            });
        }
        server.begin();
    }

    void disable() override {
        server.close();
        Serial.println("disabling SyncHttpConnection");
    }
};

#endif //EMBEDED_MYHTTP_H
