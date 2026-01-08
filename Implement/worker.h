#ifndef WORKER_H
#define WORKER_H

#include <iostream>
#include <cstdint>
#include <string>
#include <data.h>
#include <memory>

class Worker {
private:
    uint64_t data_id;
    const Data* data;
    Worker* next; 
public:
    Worker(uint64_t data_id, const Data* data): data_id(data_id), data(data), next(nullptr) {};
    ~Worker() {}
    inline uint64_t getID() const { return data_id; }
    inline const Data* getData() const { return data; }
    inline const bool updateData(const Data* new_data) { 
        data = new_data; 
        return true;
    }
    inline Worker* getNext() const { return next; }
    inline void setNext(Worker* n) { next = n; }
    inline bool hasData() const { return data != nullptr; }
    inline bool deleteData() { 
        if (data != nullptr) {
            data = nullptr;
            return true;
        }
        return false;
    }
    
};

#endif // WORKER_H

