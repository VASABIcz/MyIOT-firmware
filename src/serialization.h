//
// Created by vasabi on 29.1.23.
//

#ifndef EMBEDED_SERIALIZATION_H
#define EMBEDED_SERIALIZATION_H

#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <cstring>

#define derefType(data, type, offset) *((type*)(data+offset))

class Serializer {
public:
    virtual bool writeBool(bool it) = 0;
    virtual bool writeInt(int it) = 0;
    virtual bool writeString(const char* it, int size) = 0;
    virtual bool writeFloat(float it) = 0;
};

class Deserializer {
public:
    virtual bool readBool(bool& err) = 0;
    virtual int readInt(bool& err) = 0;
    virtual std::string* readString(bool& err) = 0;
    virtual float readFloat(bool& err) = 0;
};

enum ClassDescription {
    Int,
    Float,
    Bool,
    String,
    ListStart,
    ListEnd
};

template <typename T>
class Serializable {
public:
    T* data;
    std::vector<ClassDescription> description;

    void swap(T* newPtr) {
        auto oldPtr = data;
        data = newPtr;
        delete oldPtr;
    }

    explicit Serializable(std::vector<ClassDescription> desc) {
        data = new T;
        description = std::move(desc);
    }
};

class MockDeserializer: public Deserializer {
    int readInt(bool& err) override {
        return 10;
    }
    bool readBool(bool& err) override {
        return false;
    }
    float readFloat(bool& err) override {
        return 10;
    }
    std::string* readString(bool& err) override {
        return new std::string{"UwU"};
    }
};

class MockSerializer: public Serializer {
    bool writeBool(bool it) override {
        std::cout << "writing bool " << it << std::endl;
    }

    bool writeFloat(float it) override {
        std::cout << "writing float " << it << std::endl;
    }

    bool writeInt(int it) override {
        std::cout << "writing int " << it << std::endl;
    }

    bool writeString(const char *it, int size) override {
        std::cout << "writing string " << it << std::endl;
    }
};

class BinarySerializer: public Serializer {
public:
    int dataOffset = 0;
    char* data = nullptr;

    bool writeBool(bool it) override {
        // std::cout << "writing bool " << it << std::endl;
        Serial.println("writing bool internal");
        data = (char*)realloc(data, dataOffset+sizeof it);
        std::memcpy(data+dataOffset, &it, sizeof it);
        dataOffset += sizeof it;
        Serial.println("done writing bool internal");
        return true;
    }

    bool writeFloat(float it) override {
        // std::cout << "writing float " << it << std::endl;
        data = (char*)realloc(data, dataOffset+sizeof it);
        std::memcpy(data+dataOffset, &it, sizeof it);
        dataOffset += sizeof it;
        return true;
    }

    bool writeInt(int it) override {
        // std::cout << "writing int " << it << std::endl;
        data = (char*)realloc(data, dataOffset+sizeof it);
        std::memcpy(data+dataOffset, &it, sizeof it);
        dataOffset += sizeof it;
        return true;
    }

    bool writeString(const char *it, int size) override {
        writeInt(size);
        // std::cout << "writing string " << *it << std::endl;
        data = (char*)realloc(data, dataOffset+size);
        std::memcpy(data+dataOffset, it, size);
        dataOffset += size;
        return true;
    }
};

class BinaryDeserializer: public Deserializer {
    char* data;
    int index = 0;
public:
    int readInt(bool& err) override {
        auto v = derefType(data, int, index);
        // std::cout << "reading int " << v << std::endl;
        index += sizeof(int);
        return v;
    }
    bool readBool(bool& err) override {
        auto v = derefType(data, bool , index);
        // std::cout << "reading bool " << v << std::endl;
        index += sizeof(bool);
        return v;
    }
    float readFloat(bool& err) override {
        auto v = derefType(data, float , index);
        // std::cout << "reading float " << v << std::endl;
        index += sizeof(float);
        return v;
    }
    std::string* readString(bool& err) override {
        auto size = readInt(err);
        // FIXME error == TRUE
        // FIXME memory leaks
        Serial.println("why");
        Serial.println(size);
        auto str = new std::string;
        str->append(data+index, size);
        // std::cout << "reading string " << *str << std::endl;
        index += size;
        return str;
    }

    explicit BinaryDeserializer(void* data) {
        this->data = (char*)data;
    }
};

void* deserialize(Deserializer* source, std::vector<ClassDescription> description) {
    auto size = 0;
    auto dataIndex = 0;
    bool err = false;

    for (auto item : description) {
        switch (item) {
            case Int:
                size += sizeof(int);
                break;
            case Float:
                size += sizeof(float);
                break;
            case Bool:
                size += sizeof(bool);
                break;
            case String:
                size += sizeof(std::string);
                break;
            case ListStart:
                size += sizeof(std::vector<int>);
                break;
            case ListEnd:
                break;
        }
    }

    auto dataPtr = new char[size]();

    for (auto n = 0; n < description.size(); n++) {
        float f;
        int i;
        bool b;
        std::string* s;
        auto* a = new std::vector<void*>;
        std::vector<ClassDescription> buf;
        int listIgnore = 0;
        if (err) {
            return nullptr;
            delete[] dataPtr;
        }

        switch (description[n]) {
            case Int:
                i = source->readInt(err);
                memcpy(dataPtr+dataIndex, &i, sizeof i);
                dataIndex += sizeof i;
                break;
            case Float:
                f = source->readFloat(err);
                memcpy(dataPtr+dataIndex, &f, sizeof f);
                dataIndex += sizeof f;
                break;
            case Bool:
                b = source->readBool(err);
                memcpy(dataPtr+dataIndex, &b, sizeof b);
                dataIndex += sizeof b;
                break;
            case String:
                s = source->readString(err);
                memcpy(dataPtr+dataIndex, &s, sizeof s);
                dataIndex += sizeof s;
                break;
            case ListStart:
                n++;
                while (description[n] != ListEnd && listIgnore == 0) {
                    if (description[n] == ListStart) {
                        listIgnore++;
                    }
                    else if (description[n] == ListEnd) {
                        listIgnore--;
                    }
                    buf.push_back(description[n]);
                    n++;
                }

                i = source->readInt(err);
                for (int j = 0; j < i; j++) {
                    a->push_back(deserialize(source, buf));
                }
                memcpy(dataPtr+dataIndex, &a, sizeof a);
                dataIndex += sizeof a;
                buf.clear();
                break;
            case ListEnd:
                break;
        }
    }
    return dataPtr;
}

void serialize(const char* data, std::vector<ClassDescription> description, Serializer* serializer) {
    auto dataIndex = 0;

    for (auto n = 0; n < description.size(); n++) {
        std::string* s;
        std::vector<void*>* a;
        std::vector<ClassDescription> buf;
        int listIgnore = 0;
        Serial.println("ahhh");
        switch (description[n]) {
            case Int:
                serializer->writeInt(derefType(data, int, dataIndex));
                dataIndex += sizeof(int);
                break;
            case Float:
                serializer->writeFloat(derefType(data, float, dataIndex));
                dataIndex += sizeof(float);
                break;
            case Bool:
                Serial.println("writing bool");
                serializer->writeBool(derefType(data, bool, dataIndex));
                dataIndex += sizeof(bool);
                Serial.println("i am here");
                break;
            case String:
                s = derefType(data, std::string*, dataIndex);
                serializer->writeString(const_cast<char *>(s->c_str()), s->length());
                dataIndex += sizeof(std::string*);
                break;
            case ListStart:
                n++;
                while (description[n] != ListEnd && listIgnore == 0) {
                    if (description[n] == ListStart) {
                        listIgnore++;
                    }
                    else if (description[n] == ListEnd) {
                        listIgnore--;
                    }
                    buf.push_back(description[n]);
                    n++;
                }

                a = (std::vector<void*>*) *((std::vector<void*>**)(data+dataIndex));
                serializer->writeInt(a->size());
                for (auto item : *a) {
                    serialize((char*)item, buf, serializer);
                }
                dataIndex += sizeof a;
                buf.clear();
                break;
            case ListEnd:
                break;
        }
    }
    Serial.println("returning");
}

#endif //EMBEDED_SERIALIZATION_H
