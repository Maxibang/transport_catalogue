#include <vector>
#include "json.h"


namespace json {
       

class DictItemContext;
class KeyItemContext;
class AferStartArrayContext;

class Builder {
    
public:
    Builder() {
        nodes_stack_.push_back(&root_);
    }
    
    Builder& Value(Node::Value&& value);
    const Node& Build() const;
    KeyItemContext Key(std::string);
    DictItemContext StartDict();
    Builder& EndDict();
    AferStartArrayContext StartArray();
    Builder& EndArray();
    
private:
    Node root_ = nullptr;
    std::vector<Node*> nodes_stack_;
    std::string key_;
    bool key_used_ = true;
};


// Parent class for all SomthingItemContext classes
class ItemContext {

public:
    ItemContext(Builder& b) : builder_(b) {}

    DictItemContext StartDict();
    AferStartArrayContext StartArray();

protected:
    Builder& builder_;
};


// Class using to call only Key() и EndDict() after StartDict()
class DictItemContext {

public:
    DictItemContext(Builder& b) : builder_(b) {}
    
    KeyItemContext Key(std::string);
    Builder& EndDict();

private:
    Builder& builder_;
};


// Class using to call only Value() и StartDict() or StartArray() after Key()
class KeyItemContext : public ItemContext {

public:
    KeyItemContext(Builder& b) : ItemContext(b) {}

    DictItemContext Value(Node::Value&&);

private:
    // ....//
};


// Class using to call only Value() or StartDict() or StartArray() or EndArray() after StartArray()
class AferStartArrayContext : public  ItemContext {

public:
    AferStartArrayContext(Builder& b) : ItemContext(b) {}

    AferStartArrayContext& Value(Node::Value&& value);
    Builder& EndArray();

private:
    // ....//
}; 


} // end of namespace json