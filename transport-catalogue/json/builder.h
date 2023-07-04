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
  explicit BaseContext(Builder &builder);

protected:
  Builder &builder_;
};

class StartContainersContext : public BaseContext {
public:
  explicit StartContainersContext(Builder &builder);

public:
  ArrayContext &StartArray();
  DictContext &StartDict();
};

class KeyContext : public StartContainersContext {
public:
  explicit KeyContext(Builder &builder);

public:
  ValueContext Value(Node::Value value);
};

class ValueContext : public BaseContext {
public:
  explicit ValueContext(Builder &builder);

public:
  KeyContext &Key(std::string key);
  Builder &EndDict();
};

class DictContext : public BaseContext {
public:
  explicit DictContext(Builder &builder);

public:
  KeyContext &Key(std::string key);
  Builder &EndDict();
};

class ArrayContext : public StartContainersContext {
public:
  explicit ArrayContext(Builder &builder);

public:
  ArrayContext &Value(Node::Value value);
  Builder &EndArray();
};

class Builder final : virtual public KeyContext,
                      virtual public ValueContext,
                      virtual public DictContext,
                      virtual public ArrayContext {
public:
  Builder();

public:
  KeyContext &Key(std::string key);
  Builder &Value(Node::Value value);

  DictContext &StartDict();
  Builder &EndDict();

  ArrayContext &StartArray();
  Builder &EndArray();

  Node &Build();

private:
  [[nodiscard]] bool CouldAddNode() const;

  void AddNode(Node top_node);

private:
  Node root_{nullptr};
  std::vector<std::unique_ptr<Node>> nodes_stack_;
};
} // namespace json