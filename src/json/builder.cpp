#include "builder.h"

namespace transport_catalogue {
namespace detail {
namespace json {
namespace builder {

BaseContext::BaseContext(Builder& builder) : builder_(builder) {}

KeyContext BaseContext::key(const std::string& key) {return builder_.key(key);}
Builder& BaseContext::value(const Node::Value& value) {return builder_.value(value);}

DictionaryContext BaseContext::start_dict() {return DictionaryContext(builder_.start_dict());}
Builder& BaseContext::end_dict() {return builder_.end_dict();}

ArrayContext BaseContext::start_array() {return ArrayContext(builder_.start_array());}
Builder& BaseContext::end_array() {return builder_.end_array();}

KeyContext::KeyContext(Builder& builder) : BaseContext(builder) {}

DictionaryContext KeyContext::value(const Node::Value& value) {return BaseContext::value(std::move(value));}

DictionaryContext::DictionaryContext(Builder& builder) : BaseContext(builder) {}

ArrayContext::ArrayContext(Builder& builder) : BaseContext(builder) {}

ArrayContext ArrayContext::value(const Node::Value& value) {return BaseContext::value(std::move(value));}

Node Builder::make_node(const Node::Value& value_) {
    Node node;

    if (std::holds_alternative<bool>(value_)) {
        bool bol = std::get<bool>(value_);
        node = Node(bol);

    } else if (std::holds_alternative<int>(value_)) {
        int intt = std::get<int>(value_);
        node = Node(intt);

    } else if (std::holds_alternative<double>(value_)) {
        double doble = std::get<double>(value_);
        node = Node(doble);

    } else if (std::holds_alternative<std::string>(value_)) {
        std::string str = std::get<std::string>(value_);
        node = Node(std::move(str));

    } else if (std::holds_alternative<Array>(value_)) {
        Array arr = std::get<Array>(value_);
        node = Node(std::move(arr));

    } else if (std::holds_alternative<Dict>(value_)) {
        Dict dictionary = std::get<Dict>(value_);
        node = Node(std::move(dictionary));

    } else {
        node = Node();
    }

    return node;
}

void Builder::add_node(const Node& node) {
    if (nodes_stack_.empty()) {

        if (!root_.is_null()) {
            throw std::logic_error("root has been added");
        }

        root_ = node;
        return;

    } else {

        if (!nodes_stack_.back()->is_array()
            && !nodes_stack_.back()->is_string()) {

            throw std::logic_error("unable to create node");
        }

        if (nodes_stack_.back()->is_array()) {
            Array arr = nodes_stack_.back()->as_array();
            arr.emplace_back(node);

            nodes_stack_.pop_back();
            auto arr_ptr = std::make_unique<Node>(arr);
            nodes_stack_.emplace_back(std::move(arr_ptr));

            return;
        }

        if (nodes_stack_.back()->is_string()) {
            std::string str = nodes_stack_.back()->as_string();
            nodes_stack_.pop_back();

            if (nodes_stack_.back()->is_dict()) {
                Dict dictionary = nodes_stack_.back()->as_dict();
                dictionary.emplace(std::move(str), node);

                nodes_stack_.pop_back();
                auto dictionary_ptr = std::make_unique<Node>(dictionary);
                nodes_stack_.emplace_back(std::move(dictionary_ptr));
            }

            return;
        }
    }
}

KeyContext Builder::key(const std::string& key_) {
    if (nodes_stack_.empty()) {
        throw std::logic_error("unable to create key");
    }

    auto key_ptr = std::make_unique<Node>(key_);

    if (nodes_stack_.back()->is_dict()) {
        nodes_stack_.emplace_back(std::move(key_ptr));
    }

    return KeyContext(*this);
}

Builder& Builder::value(const Node::Value& value_) {
    add_node(make_node(value_));

    return *this;
}

DictionaryContext Builder::start_dict() {
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Dict())));

    return DictionaryContext(*this);
}

Builder& Builder::end_dict() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("unable to close as without opening");
    }

    Node node = *nodes_stack_.back();

    if (!node.is_dict()) {
        throw std::logic_error("object isn't dictionary");
    }

    nodes_stack_.pop_back();
    add_node(node);

    return *this;
}

ArrayContext Builder::start_array() {
    nodes_stack_.emplace_back(std::move(std::make_unique<Node>(Array())));

    return ArrayContext(*this);
}

Builder& Builder::end_array() {
    if (nodes_stack_.empty()) {
        throw std::logic_error("unable to close without opening");
    }

    Node node = *nodes_stack_.back();

    if (!node.is_array()) {
        throw std::logic_error("object isn't array");
    }

    nodes_stack_.pop_back();
    add_node(node);

    return *this;
}

Node Builder::build() {
    if (root_.is_null()) {
        throw std::logic_error("empty json");
    }

    if (!nodes_stack_.empty()) {
        throw std::logic_error("invalid json");
    }

    return root_;
}

}//end namespace builder
}//end namespace json
}//end namespace detail
}//end namespace transport_catalogue