#ifndef DATA_H
#define DATA_H

#include <cstdint>
#include <string>

class Data {
private:
    uint64_t key;
    std::string data;
public:
    Data(uint64_t key, const std::string& data) : key(key), data(data) {}
    inline uint64_t getKey() const { return key; }
    inline const std::string& getData() const { return data; }
};

#endif // DATA_H

