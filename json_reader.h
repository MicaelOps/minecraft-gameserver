//
// Created by Micael Cossa on 29/07/2025.
//

#ifndef MINECRAFTSERVER_JSON_READER_H
#define MINECRAFTSERVER_JSON_READER_H

#include <unordered_map>
#include <string>

class json_node {

public:

    virtual std::string toString() = 0;
    virtual json_node* proccessSegment(std::string& data) = 0;
};



class json_object : public json_node {

};
class json_element : public json_node {

};
class json_array : public json_node {

};
class json_entry : public json_node {

private:
    std::string& name;
    json_element& entry_value;
public:
    json_entry(std::string& name, json_element& entry_value) : name(name) , entry_value(entry_value) {}
    std::string toString() override;
    json_node* proccessSegment(std::string& data) override;
};


class json_document : public json_node {

private:
    std::unordered_map<std::string, json_node> data;

public:
    void insertKeyValue(std::string& key, json_element& value);
    json_node retrieveValue(std::string& key);

    std::string toString() override;
    json_node* proccessSegment(std::string& data) override;
};


#endif //MINECRAFTSERVER_JSON_READER_H
