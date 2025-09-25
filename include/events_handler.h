//
// Created by Micael Cossa on 21/09/2025.
//

#ifndef CORE_MINESERVER_EVENTS_HANDLER_H
#define CORE_MINESERVER_EVENTS_HANDLER_H

#define EVENT_ID short
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <memory>
#include <vector>
#include <algorithm>

class EventHandler {

private:

    struct BaseEvent {
        int priority;
        explicit BaseEvent(int p) : priority(p) {}
        virtual ~BaseEvent() = default;
    };

    struct EventPriorityComparator {
        bool operator()(const std::unique_ptr<BaseEvent>& a,
                        const std::unique_ptr<BaseEvent>& b) const {
            return a->priority < b->priority;
        }
    };

    template<typename... Args>
    struct FunctionEvent : BaseEvent {
        std::function<bool(Args...)> func;

        explicit FunctionEvent(int priority, std::function<bool(Args...)> f)
                : BaseEvent(priority), func(std::move(f)) {}

        bool execute(Args... args) {
            return func(args...);
        }
    };

    std::unordered_map<EVENT_ID, std::vector<std::unique_ptr<BaseEvent>>> event_callbacks;

public:
    EventHandler() = default;

    template<typename ...Args>
    void registerEvent(EVENT_ID event_id, std::function<bool(Args...)> eventFunction, int priority) {
        auto& callbacks = event_callbacks[event_id];

        callbacks.push_back(std::make_unique<FunctionEvent<Args...>>(priority, std::move(eventFunction)));

        std::sort(callbacks.begin(), callbacks.end(), EventPriorityComparator{});
    }

    template<typename ...Args>
    bool callEvent(EVENT_ID event_id, Args&&... args) {
        auto it = event_callbacks.find(event_id);
        bool cancelled = false;

        if (it != event_callbacks.end()) {
            for (const auto &item: it->second) {

                auto* functionevent = static_cast<FunctionEvent<Args..., bool>*>(item.get());

                if (!functionevent)
                    continue;

                if (functionevent->execute(std::forward<Args>(args)..., cancelled)) {
                    cancelled = true;
                }
            }
        }
        return cancelled;
    }
};

#endif //CORE_MINESERVER_EVENTS_HANDLER_H