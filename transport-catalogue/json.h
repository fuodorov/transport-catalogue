#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

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

            bool IsInt() const {return std::holds_alternative<int>(*this);}
            bool IsPureDouble() const {return std::holds_alternative<double>(*this);}
            bool IsDouble() const {return IsInt() || IsPureDouble();}
            bool IsBool() const {return std::holds_alternative<bool>(*this);}
            bool IsNull() const {return std::holds_alternative<std::nullptr_t>(*this);}
            bool IsArray() const {return std::holds_alternative<Array>(*this);}
            bool IsString() const {return std::holds_alternative<std::string>(*this);}
            bool IsDict() const {return std::holds_alternative<Dict>(*this);}

            int AsInt() const {
                if (IsInt()) {
                    return std::get<int>(*this);
                } else if (IsPureDouble()) {
                    return std::get<double>(*this);
                } else {
                    throw std::logic_error("Not an int");
                }
            }

            double AsDouble() const {
                if (IsInt()) {
                    return AsInt();
                } else if (IsPureDouble()) {
                    return std::get<double>(*this);
                } else {
                    throw std::logic_error("Not a double");
                }
            }

            bool AsBool() const {
                if (IsBool()) {
                    return std::get<bool>(*this);
                } else {
                    throw std::logic_error("Not a bool");
                }
            }
            
            const Array& AsArray() const {
                if (IsArray()) {
                    return std::get<Array>(*this);
                } else {
                    throw std::logic_error("Not an array");
                }
            }

            const std::string& AsString() const {
                if (IsString()) {
                    return std::get<std::string>(*this);
                } else {
                    throw std::logic_error("Not a string");
                }
            }
            
            const Dict& AsDict() const {
                if (IsDict()) {
                    return std::get<Dict>(*this);
                } else {
                    throw std::logic_error("Not a dict");
                }
            }

            bool operator==(const Node& rhs) const {return GetValue() == rhs.GetValue();}

            const Value& GetValue() const {return *this;}

            Value& GetValue() {return *this;}
    };

    inline bool operator!=(const Node& lhs, const Node& rhs) {return !(lhs == rhs);}

    class Document {
        public:
            explicit Document(Node root) : root_(std::move(root)) {}

            const Node& GetRoot() const {return root_;}

        private:
            Node root_;
    };

    inline bool operator==(const Document& lhs, const Document& rhs) {return lhs.GetRoot() == rhs.GetRoot();}

    inline bool operator!=(const Document& lhs, const Document& rhs) {return !(lhs == rhs);}

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json