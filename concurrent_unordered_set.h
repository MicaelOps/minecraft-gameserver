//
// Created by Micael Cossa on 19/07/2025.
//

#ifndef MINECRAFTSERVER_CONCURRENT_UNORDERED_SET_H
#define MINECRAFTSERVER_CONCURRENT_UNORDERED_SET_H

#include <unordered_set>
#include <mutex>

template<typename Type>
class concurrent_unordered_set {
    public:
        void insert(Type data);
        void remove(Type data);
        void clear();

        template<typename Func>
        void for_each(Func func);

    private:
        std::mutex setMutex;
        std::unordered_set<Type> original_set;
};
template<typename Type>
template<typename Func>
void concurrent_unordered_set<Type>::for_each(Func func) {
    std::lock_guard<std::mutex> lock(setMutex);
    for (const auto &item: original_set) {
        func(item);
    }
}

#endif //MINECRAFTSERVER_CONCURRENT_UNORDERED_SET_H
