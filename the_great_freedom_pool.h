//
// Created by Micael Cossa on 29/08/2025.
//

#ifndef CORE_MINESERVER_THE_GREAT_FREEDOM_POOL_H
#define CORE_MINESERVER_THE_GREAT_FREEDOM_POOL_H

#include <vector>
#include <mutex>
#include <functional>

template<typename T>
class ObjectPool {

private:
    using Factory = std::function<T()>;

    Factory creator;
    std::mutex mutex;
    std::vector<T> pool;
    bool isarray;
public:


    explicit ObjectPool(size_t size, bool array, Factory creator) : isarray(array), creator(creator) {
        pool.reserve(size);
        for (size_t begin  = 0; begin < size; begin++) {
            pool.emplace_back(creator());
        }
    }

    ~ObjectPool() {
        for (const auto &item: pool) {

            if(isarray)
                delete[] item;
            else
                delete item;
        }
    }

    T acquire() {
        std::lock_guard<std::mutex> lock(mutex);
        if(pool.empty()) {
            return creator();
        }

        T object = pool.back();
        pool.pop_back();
        return object;
    }

    void returnToPool(T object) {
        std::lock_guard<std::mutex> lock(mutex);
        pool.push_back(object);
    }
};


#endif //CORE_MINESERVER_THE_GREAT_FREEDOM_POOL_H
