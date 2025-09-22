//
// Created by Micael Cossa on 21/09/2025.
//

#ifndef CORE_MINESERVER_EVENTS_HANDLER_H
#define CORE_MINESERVER_EVENTS_HANDLER_H

#define EVENT_ID short
#include <unordered_map>
#include <functional>
#include <stdexcept>

class Event {

private:
    bool cancelled = false;
public:

    Event() = default;
    virtual ~Event() = default;

    virtual void handleEvent();

    bool isCancelled() const noexcept;

    void cancelEvent() noexcept;
};

class EventHandler {

private:

    std::unordered_map<EVENT_ID, std::function<bool(Event)>> event_callbacks;
public:
    EventHandler() = default;


    template<typename... Args>
    bool callEvent(EVENT_ID event_id, Args...args) {

        auto it = event_callbacks.find(event_id);

        if (it != event_callbacks.end()) {
            return it->second(args...);
        }

        throw std::runtime_error("EventID not found");
    }
};



#endif //CORE_MINESERVER_EVENTS_HANDLER_H
