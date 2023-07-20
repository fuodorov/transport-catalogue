#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace transport_catalogue {
namespace detail {
namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:
    using variant::variant;
    using Value = variant;

    Node() = default;
    Node(bool value);
    Node(Array array);
    Node(Dict dict);
    Node(int value);
    Node(std::string value);
    Node(std::nullptr_t);
    Node(double value);

    const Array& as_array() const;
    const Dict& as_dict() const;
    int as_int() const;
    double as_double() const;
    bool as_bool() const;
    const std::string& as_string() const;

    bool is_null() const;
    bool is_int() const;
    bool is_double() const;
    bool is_real_double() const;
    bool is_bool() const;
    bool is_string() const;
    bool is_array() const;
    bool is_dict() const;

    const Value& get_value() const;

private:
    Value value_;
};

inline bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.get_value() == rhs.get_value();
}
inline bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

class Document {
public:
    Document() = default;
    explicit Document(Node root);
    const Node& get_root() const;

private:
    Node root_;
};

inline bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.get_root() == rhs.get_root();
}
inline bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

Document load(std::istream& input);
void print(const Document& document, std::ostream& output);

}  // end namespace json
}  // end namespace detail
}  // end namespace transport_catalogue