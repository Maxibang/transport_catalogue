#include "json_builder.h"
#include <stdexcept>
#include <iostream>

namespace json {
    

// -----  Class Builder  ----- //

// Process input value
Builder& Builder::Value(Node::Value&& value) {
    if (nodes_stack_.size() == 0) {
        throw std::logic_error("Call Value() after ready object");
    }

    Node node;
    if (std::holds_alternative<nullptr_t>(value)) {
        node = Node(std::get<0>(value));
    } else if (std::holds_alternative<Array>(value)) {
        node = Node(std::get<1>(value));
    } else if (std::holds_alternative<Dict>(value)) {
        node = Node(std::get<2>(value));
    } else if (std::holds_alternative<bool>(value)) {
        node = Node(std::get<3>(value));
    } else if (std::holds_alternative<int>(value)) {
        node = Node(std::get<4>(value));
    } else if(std::holds_alternative<double>(value)) {
        node = Node(std::get<5>(value));
    } else if (std::holds_alternative<std::string>(value)) {
        node = Node(std::get<6>(value));
    }
    
    auto &curr_node = nodes_stack_.back(); 
    if (curr_node->IsArray()) {
        std::get<1>(curr_node->GetRefValue()).push_back(node);
        if (node.IsArray()|| node.IsMap()) {
            nodes_stack_.push_back(&std::get<1>(curr_node->GetRefValue()).back());
        }
    } else if (curr_node->IsMap()) {
        if (key_used_) {
            throw std::logic_error("Invalid use of Value()");
        }
        std::get<2>(curr_node->GetRefValue())[key_] = node;
        key_used_ = true;
        if (node.IsArray() || node.IsMap()) {
            nodes_stack_.push_back(&std::get<2>(curr_node->GetRefValue()).at(key_));
        }
        key_.clear();

    } else {
        root_ = node;
        if (!root_.IsArray() && !root_.IsMap()) {
            nodes_stack_.pop_back();
        }

    }
    return *this;
}
    
    
// Complete json construction
const Node& Builder::Build() const {
    if (nodes_stack_.size() > 0 && !root_.IsArray() && !root_.IsMap()) {
        throw std::logic_error("Invalid use of Build()");
    }
    return root_;
}
  
    
// Get a key for Dict  
KeyItemContext Builder::Key(std::string key) {
    if (nodes_stack_.size() == 0) {
        throw std::logic_error("Call Key() after ready object");
    }
    if (!nodes_stack_.back()->IsMap()) {
        throw std::logic_error("Call Key() outside of Dict");
    }
    if (!key_used_) {
        throw std::logic_error("Call Key() after Key()");
    }
    key_used_ = false;
    key_ = std::move(key);
    return KeyItemContext (*this);
}    
    
    
// Add Dict for filling
DictItemContext Builder::StartDict() {
    if (nodes_stack_.size() == 0) {
        throw std::logic_error("Call StartDict() after ready object");
    }
    this->Value(Dict{});
    return DictItemContext(*this);
}


// End filling Dict
Builder& Builder::EndDict() {
    if (!nodes_stack_.empty()) {
        if (nodes_stack_.back()->IsMap()) {
            nodes_stack_.pop_back();
            return *this;
        }
    }
    throw std::logic_error("Invalid use of EndDict()"); 
}
  
    
// Add Array for filling
AferStartArrayContext Builder::StartArray() {
    if (nodes_stack_.size() == 0) {
        throw std::logic_error("Call StartArray() after ready object");
    }
    this->Value(Array{});
    return AferStartArrayContext(*this);
}
    

// End filling Array
Builder& Builder::EndArray() {
    if (!nodes_stack_.empty()) {
        if (nodes_stack_.back()->IsArray()) {
            nodes_stack_.pop_back();
            return *this;
        }
    }
    throw std::logic_error("Invalid use of EndArray()"); 
}


// -----  Class DictItemContext  ----- //

KeyItemContext DictItemContext::Key(std::string key) {
    builder_.Key(move(key));
    return KeyItemContext(builder_);
}


Builder& DictItemContext::EndDict() {
    return builder_.EndDict();
}


// -----  Class ItemContext  ----- //

DictItemContext ItemContext::StartDict() {
    return builder_.StartDict();
}

AferStartArrayContext ItemContext::StartArray() {
    return builder_.StartArray();
}


// -----  Class KeyItemContext  ----- //

DictItemContext KeyItemContext::Value(Node::Value&& value) {
    builder_.Value(move(value));
    return DictItemContext(builder_);
}


// -----  Class AferStartArrayContext  ----- //

AferStartArrayContext& AferStartArrayContext::Value(Node::Value&& value) {
    builder_.Value(move(value));
    return *this;
}


Builder& AferStartArrayContext::EndArray() {
    return builder_.EndArray();
}


} // end of namespace json