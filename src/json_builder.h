#pragma once

#include "json.h"
#include <stack>
#include <string>
#include <memory>

namespace transport_catalogue {
namespace detail {
namespace json {
namespace builder {

class KeyContext;
class DictionaryContext;
class ArrayContext;

class Builder {
public:
    Node make_node(const Node::Value& value_);
    void add_node(const Node& node);

    KeyContext key(const std::string& key_);
    Builder& value(const Node::Value& value);
    
    DictionaryContext start_dict();
    Builder& end_dict();
    
    ArrayContext start_array();
    Builder& end_array();

    Node build();

private:
    Node root_;
    std::vector<std::unique_ptr<Node>> nodes_stack_;
};

class BaseContext {
public:
    BaseContext(Builder& builder);

    KeyContext key(const std::string& key);
    Builder& value(const Node::Value& value);
    
    DictionaryContext start_dict();
    Builder& end_dict();
    
    ArrayContext start_array();
    Builder& end_array();

protected:
    Builder& builder_;
};

class KeyContext : public BaseContext {
public:
    KeyContext(Builder& builder);

    KeyContext key(const std::string& key) = delete;

    BaseContext end_dict() = delete;
    BaseContext end_array() = delete;

    DictionaryContext value(const Node::Value& value);
};

class DictionaryContext : public BaseContext {
public:
    DictionaryContext(Builder& builder);

    DictionaryContext start_dict() = delete;

    ArrayContext start_array() = delete;
    Builder& end_array() = delete;

    Builder& value(const Node::Value& value) = delete;
};

class ArrayContext : public BaseContext {
public:
    ArrayContext(Builder& builder);

    KeyContext key(const std::string& key) = delete;

    Builder& end_dict() = delete;

    ArrayContext value(const Node::Value& value);
};

}//end namespace builder
}//end namespace json
}//end namespace detail
}//end namespace transport_catalogue