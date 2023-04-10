#pragma once

#include <memory>
#include <optional>
#include <queue>

#include "json.h"

namespace json {

    class Builder;

    class DictContext;

    class ArrayContext;
    
    class ValueContext;

    class BaseContext {
        public: 
            explicit BaseContext(Builder& builder);

        protected:
            Builder& builder_;
    };

    class KeyContext : public BaseContext {
        public:
            explicit KeyContext(Builder& builder);

            ValueContext Value(Node::Value value);
            ArrayContext& StartArray();
            DictContext& StartDict();
    };

    class ValueContext : public BaseContext {
        public:
            explicit ValueContext(Builder& builder);

            KeyContext& Key(std::string key);
            Builder& EndDict();
    };

    class DictContext : public BaseContext {
        public:
            explicit DictContext(Builder& builder);

            KeyContext& Key(std::string key);
            Builder& EndDict();
    };

    class ArrayContext : public BaseContext {
        public:
            explicit ArrayContext(Builder& builder);

            ArrayContext& Value(Node::Value value);
            Builder& EndArray();
            ArrayContext& StartArray();
            DictContext& StartDict();
    };

    class Builder final : virtual public KeyContext, virtual public ValueContext, virtual public DictContext, virtual public ArrayContext {
        public:
            Builder();

            KeyContext& Key(std::string key);
            DictContext& StartDict();
            ArrayContext& StartArray();
            
            Builder& Value(Node::Value value);
            Builder& EndDict();
            Builder& EndArray();

            const Node& Build() const;

        private:
            [[nodiscard]] bool CouldAddNode() const;
            void AddNode(Node top_node);

            Node root_{nullptr};
            std::vector<std::unique_ptr<Node>> stack_;
    };
}  // namespace json