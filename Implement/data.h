#ifndef DATA_H
#define DATA_H

#include <cstdint>
#include <string>

class Data {
private:
    int record_id;
    std::string data;
public:
    Data(int id, const std::string& data) : record_id(id), data(data) {}
    inline int getRecordID() const { return record_id; }
    inline std::string getData() const { return data; }
};

#endif // DATA_H

