//
// Created by Micael Cossa on 29/07/2025.
//

#include "json_reader.h"

json_node* json_document::proccessSegment(std::string& data) {
    return nullptr;
}

std::string json_document::toString() {
    return "{}";
}

std::string json_entry::toString() {
    return std::string();
}

json_node *json_entry::proccessSegment(std::string &data) {
    return nullptr;
}
