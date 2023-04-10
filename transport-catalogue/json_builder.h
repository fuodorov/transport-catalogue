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

    class StartContainersContext : public BaseContext {
        public:
            explicit StartContainersContext(Builder& builder);

            ArrayContext& StartArray();
            DictContext& StartDict();
    };

    class KeyContext : public StartContainersContext {
        public:
            explicit KeyContext(Builder& builder);

            ValueContext Value(Node::Value value);
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

    class ArrayContext : public StartContainersContext {
        public:
            explicit ArrayContext(Builder& builder);

            ArrayContext& Value(Node::Value value);
            Builder& EndArray();
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