#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <sstream>

namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

   
class Node {
public:
    
    /* Реализуйте Node, используя std::variant */
    // std::variant<...>
    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    
    Node() : value_(nullptr) {}
    Node(nullptr_t ptr) : value_(ptr) {}
    

    Node(Array array);
    Node(Dict map);

    explicit Node(bool value) : value_(value) {};
    Node(int value) : value_(value) {};
    Node(double value) : value_(value) {};
    Node(std::string value);
    
    // std::variant<...>
    const Value& GetValue() const;
    bool IsInt() const;
    bool IsDouble() const; // Returns true, if in Node holds int or double.
    bool IsPureDouble() const; // Returns true, if in Node holds double. 
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    
    bool operator== (const Node &rhs) const {
        return value_ == rhs.value_;
    }
    
    bool operator!= (const Node &rhs) const {
        return !(value_ == rhs.value_);
    }
    
    Value& GetRefValue() {
        return value_;
    }
    
private:
    Value value_;
    
}; // End of class Node

    
class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;
    
    bool operator== (const Document& rhs) const {
        return root_ == rhs.root_;
    }
    
    bool operator!= (const Document& rhs) const {
        return root_ != rhs.root_;
    }
    
private:
    Node root_;
    
}; // End of class Document

    
Document Load(std::istream& input);
    
void Print(const Document& doc, std::ostream& output);
    
Document LoadJSON(const std::string& s); 

std::string Print(const Node& node);

}  // namespace json