//
// Created by Micael Cossa on 26/09/2025.
//
#include "world_handler.h"
#include <thread>


void WORLD_THREAD_TICK_EVENT(const std::stop_token& token, World* world) {



}
World createFlatWorld(std::string_view name) {
    std::unique_ptr<World> world = std::make_unique<World>();

    std::jthread worldThread(WORLD_THREAD_TICK_EVENT, world.get());

}