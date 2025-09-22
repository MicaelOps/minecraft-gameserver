//
// Created by Micael Cossa on 21/09/2025.
//

#include "events_handler.h"


bool Event::isCancelled() const noexcept {
    return cancelled;
}

void Event::cancelEvent() noexcept {
    cancelled = true;
}
