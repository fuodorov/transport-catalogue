#pragma once

#include <memory>
#include <stack>
#include <string>

#include "json.h"

namespace transport_catalogue::json::builder {

class KeyContext;
class DictionaryContext;
class ArrayContext;

class Builder {
 public:
  Node MakeNode(const Node::Value& value_);
  void AddNode(const Node& node);

  KeyContext Key(const std::string& key_);
  Builder& Value(const Node::Value& value);

  DictionaryContext StartDict();
  Builder& EndDict();

  ArrayContext StartArray();
  Builder& EndArray();

  Node Build();

 private:
  Node root_;
  std::vector<std::unique_ptr<Node>> nodes_stack_;
};

class BaseContext {
 public:
  BaseContext(Builder& builder);

  KeyContext Key(const std::string& key);
  Builder& Value(const Node::Value& value);

  DictionaryContext StartDict();
  Builder& EndDict();

  ArrayContext StartArray();
  Builder& EndArray();

 protected:
  Builder& builder_;
};

class KeyContext : public BaseContext {
 public:
  KeyContext(Builder& builder);

  KeyContext Key(const std::string& key) = delete;

  BaseContext EndDict() = delete;
  BaseContext EndArray() = delete;

  DictionaryContext Value(const Node::Value& value);
};

class DictionaryContext : public BaseContext {
 public:
  DictionaryContext(Builder& builder);

  DictionaryContext StartDict() = delete;

  ArrayContext StartArray() = delete;
  Builder& EndArray() = delete;

  Builder& Value(const Node::Value& value) = delete;
};

class ArrayContext : public BaseContext {
 public:
  ArrayContext(Builder& builder);

  KeyContext Key(const std::string& key) = delete;

  Builder& EndDict() = delete;

  ArrayContext Value(const Node::Value& value);
};

}  // end namespace transport_catalogue::json::builder