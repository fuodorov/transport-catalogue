#include "json_builder.h"

namespace json {
    BaseContext::BaseContext(Builder& builder) : builder_(builder) {}

    StartContainersContext::StartContainersContext(Builder& builder) : BaseContext(builder) {}

    ArrayContext& StartContainersContext::StartArray() {return builder_.StartArray();}

    DictContext& StartContainersContext::StartDict() {return builder_.StartDict();}

    KeyContext::KeyContext(Builder& builder) : StartContainersContext(builder) {}

    ValueContext KeyContext::Value(Node::Value value) {return builder_.Value(std::move(value));}

    ValueContext::ValueContext(Builder& builder) : BaseContext(builder) {}

    KeyContext& ValueContext::Key(std::string key) {return builder_.Key(std::move(key));}

    Builder& ValueContext::EndDict() {return builder_.EndDict();}

    DictContext::DictContext(Builder& builder) : BaseContext(builder) {}

    KeyContext& DictContext::Key(std::string key) {return builder_.Key(std::move(key));}

    Builder& DictContext::EndDict() {return builder_.EndDict();}

    ArrayContext::ArrayContext(Builder& builder) : StartContainersContext(builder) {}

    ArrayContext& ArrayContext::Value(Node::Value value) {return builder_.Value(std::move(value));}

    Builder& ArrayContext::EndArray() {return builder_.EndArray();}

    Builder::Builder() : KeyContext(*this), ValueContext(*this), DictContext(*this), ArrayContext(*this) {}

    KeyContext& Builder::Key(std::string key) {
        if (!root_.IsNull() || stack_.empty() || !stack_.back()->IsDict()){
            throw std::logic_error("Incorrect attempt to add key :" + key);
        } else {
            stack_.emplace_back(std::make_unique<Node>(std::move(key)));
            return *this;
        }
    }

    Builder& Builder::Value(Node::Value value) {
        if (!root_.IsNull() || !CouldAddNode()) {
            throw std::logic_error("Incorrect attempt to add Value");
        } else {
            std::visit([this](auto&& v) { stack_.emplace_back(std::make_unique<Node>(v)); }, value);
            AddNode(*stack_.back().release());
            return *this;
        }
    }

    DictContext& Builder::StartDict() {
        if (!root_.IsNull() || !CouldAddNode()){
            throw std::logic_error("Incorrect attempt to start Dict()");
        } else {
            stack_.emplace_back(std::make_unique<Node>(Dict()));
            return *this;
        }
    }

    Builder& Builder::EndDict() {
        if (!root_.IsNull() || stack_.empty() || !stack_.back()->IsDict()) {
            throw std::logic_error("Incorrect attempt to end Dict()");
        } else {
            AddNode(*stack_.back().release());
            return *this;
        }
    }

    ArrayContext& Builder::StartArray() {
        if (!root_.IsNull() || !CouldAddNode()){
            throw std::logic_error("Incorrect attempt to start Array()");
        } else {
            stack_.emplace_back(std::make_unique<Node>(Array()));
            return *this;
        }
    }

    Builder& Builder::EndArray() {
        if (!root_.IsNull() || stack_.empty() || !stack_.back()->IsArray()) {
            throw std::logic_error("Incorrect attempt to end Array()");
        } else {
            AddNode(*stack_.back().release());
            return *this;
        }
    }

    const Node& Builder::Build() const {
        if (root_.IsNull() || !stack_.empty()) {
            throw std::logic_error("Could not build JSON");
        } else {
            return root_;
        }
    }

    bool Builder::CouldAddNode() const {
        return stack_.empty() || stack_.back()->IsArray() || stack_.back()->IsString();
    }

    void Builder::AddNode(Node top_node) {
        stack_.pop_back();

        if (stack_.empty()) {
            root_ = top_node;
        } else if (stack_.back()->IsArray()) {
            std::get<Array>(stack_.back()->GetValue()).emplace_back(std::move(top_node));
        } else if (stack_.back()->IsString()) {
            std::string key = std::get<std::string>(stack_.back()->GetValue());
            stack_.pop_back();
            std::get<Dict>(stack_.back()->GetValue()).emplace(std::move(key), std::move(top_node));
        } else {
            if (top_node.IsDict()) {
                stack_.back()->GetValue() = Dict();
            } else if (top_node.IsArray()) {
                stack_.back()->GetValue() = Array();
            }
        }
    }

}  // namespace json
