#include "json.h"

using namespace std;

namespace transport_catalogue {
namespace detail {
namespace json {
namespace {
    
Node load_node(istream& input);
 
std::string load_literal(std::istream& input) {
    std::string str;
    
    while (std::isalpha(input.peek())) {
        str.push_back(static_cast<char>(input.get()));
    }
    return str;
}
 
Node load_array(std::istream& input) {
    std::vector<Node> array;
    
    for (char ch; input >> ch && ch != ']';) {
        if (ch != ',') {
            input.putback(ch);
        }
        
        array.push_back(load_node(input));
    }

    if (!input) {
        throw ParsingError("unable to parse array"s);
    }

    return Node(array);
}
 
Node load_null(std::istream& input) {
    if (auto literal = load_literal(input); literal == "null"sv) {
        return Node(nullptr);
    } else {
        throw ParsingError("unable to parse '"s + literal + "' as null"s);
    }
}
 
Node load_bool(std::istream& input) {
    const auto str = load_literal(input);

    if (str == "true"sv) {
        return Node(true);
    } else if (str == "false"sv) {
        return Node(false);
    } else {
        throw ParsingError("unable to parse '"s + str + "' as bool"s);
    }
}
 
Node load_number(std::istream& input) {
    std::string number;

    auto read_char = [&number, &input] {
        number += static_cast<char>(input.get());
        
        if (!input) {
            throw ParsingError("unable to read number"s);
        }
    };

    auto read_digits = [&input, read_char] {
        
        if (!std::isdigit(input.peek())) {
            throw ParsingError("digit expected"s);
        } else {
            while (std::isdigit(input.peek())) {
                read_char();
            }   
        }
    };

    if (input.peek() == '-') {
        read_char();
    }

    if (input.peek() == '0') {
        read_char();
    } else {
        read_digits();
    }

    bool is_int = true;
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }

        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            try {
                return Node(std::stoi(number));
                
            } catch (...) {}
        }
        return Node(std::stod(number));
        
    } catch (...) {
        throw ParsingError("unable to convert "s + number + " to number"s);
    }
}
 
Node load_string(std::istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string str;
    
    while (true) {
        if (it == end) {
            throw ParsingError("unable to parse string");
        }
        
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
            
        } else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("unable to parse string");
            }
            
            const char esc_ch = *(it);
            switch (esc_ch) {
                case 'n':
                    str.push_back('\n');
                    break;
                case 't':
                    str.push_back('\t');
                    break;
                case 'r':
                    str.push_back('\r');
                    break;
                case '"':
                    str.push_back('"');
                    break;
                case '\\':
                    str.push_back('\\');
                    break;
                default:
                    throw ParsingError("invalid esc \\"s + esc_ch);
            }
            
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("invalid line end"s);
        } else {
            str.push_back(ch);
        }
        
        ++it;
    }
    
    return Node(str);
}
    
Node load_dictionary(std::istream& input) {
    Dict dictionary;

    for (char ch; input >> ch && ch != '}';) {
        
        if (ch == '"') {
            std::string key = load_string(input).as_string();

            if (input >> ch && ch == ':') {
                
                if (dictionary.find(key) != dictionary.end()) {
                    throw ParsingError("duplicate key '"s + key + "'found");
                }

                dictionary.emplace(std::move(key), load_node(input));
                
            } else {
                throw ParsingError(": expected. but '"s + ch + "' found"s);
            }
            
        } else if (ch != ',') {
            throw ParsingError("',' expected. but '"s + ch + "' found"s);
        }
    }

    if (!input) {
        throw ParsingError("unable to parse dictionary"s);
    } else {
        return Node(dictionary);
    }
    
}
  
Node load_node(std::istream& input) {
    char ch;
    
    if (!(input >> ch)) {
        throw ParsingError(""s);
    } else {
        switch (ch) {
        case '[':
            return load_array(input);
        case '{':
            return load_dictionary(input);
        case '"':
            return load_string(input);
        case 't': case 'f':
            input.putback(ch);
            return load_bool(input);
        case 'n':
            input.putback(ch);
            return load_null(input);
        default:
            input.putback(ch);
            return load_number(input);
        }
    }
} 
    
}//end namespace
   
Node::Node(Array array) : value_(std::move(array)) {}    
Node::Node(std::nullptr_t) : Node() {}
Node::Node(bool value) : value_(value) {}
Node::Node(Dict dict) : value_(std::move(dict)) {}
Node::Node(int value) : value_(value) {}
Node::Node(string value) : value_(std::move(value)) {}
Node::Node(double value) : value_(value) {}
    
const Array& Node::as_array() const {
    using namespace std::literals;
    
    if (!is_array()) {
        throw std::logic_error("value is not an array"s);
    } else {
        return std::get<Array>(value_); 
    }
}

const Dict& Node::as_dict() const {
    using namespace std::literals;
    
    if (!is_dict()) {
        throw std::logic_error("value is not a dictionary"s);
    } else {
        return std::get<Dict>(value_);  
    }
}

const std::string& Node::as_string() const {
    using namespace std::literals;
    
    if (!is_string()) {
        throw std::logic_error("value is not a string"s);
    } else {
        return std::get<std::string>(value_);        
    }
}
    
int Node::as_int() const {
    using namespace std::literals; 
    
    if (!is_int()) {
        throw std::logic_error("value is not an int"s);
    } else {
        return std::get<int>(value_);    
    }
}

double Node::as_double() const {
    using namespace std::literals;
    
    if (!is_double()) {
        throw std::logic_error("value is not a double"s);
    } else if (is_real_double()) {
        return std::get<double>(value_);
    } else {
        return as_int();
    }   
}

bool Node::as_bool() const {
    using namespace std::literals;
    
    if (!is_bool()) {
        throw std::logic_error("value is not a bool"s);
    } else {
        return std::get<bool>(value_);   
    }
}
    
bool Node::is_null() const {return std::holds_alternative<std::nullptr_t>(value_);}
bool Node::is_int() const {return std::holds_alternative<int>(value_);}
bool Node::is_double() const {return is_real_double() || is_int();}  
bool Node::is_real_double() const {return std::holds_alternative<double>(value_);}
bool Node::is_bool() const {return std::holds_alternative<bool>(value_);}
bool Node::is_string() const {return std::holds_alternative<std::string>(value_);}
bool Node::is_array() const {return std::holds_alternative<Array>(value_);}
bool Node::is_dict() const {return std::holds_alternative<Dict>(value_);}

const Node::Value& Node::get_value() const {return value_;}
   
Document::Document(Node root) : root_(std::move(root)) {}
const Node& Document::get_root() const {return root_;}
    
Document load(istream& input) {return Document(load_node(input));}

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void print_indent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    [[nodiscard]] PrintContext indented() const {
        return {out, 
                indent_step, 
                indent_step + indent};
    }
};
 
void print_node(const Node& node, const PrintContext& context);
 
void print_string(const std::string& value, std::ostream& out) {
    out.put('"');
    
    for (const char ch : value) {
        switch (ch) {
            case '\r':
                out << R"(\r)";
                break;
            case '\n':
                out << R"(\n)";
                break;
            case '"':
                out << R"(\")";
                break;
            case '\\':
                out << R"(\\)";
                break;
            default:
                out.put(ch);
                break;
        }
    }
    
    out.put('"');
}

template <typename Value>
void print_value(const Value& value, const PrintContext& context) {
    context.out << value;
}    

template <>
void print_value<std::string>(const std::string& value, const PrintContext& context) {
    print_string(value, context.out);
}
 
void print_value(const std::nullptr_t&, const PrintContext& context) {
    context.out << "null"s;
}
 
void print_value(bool value, const PrintContext& context) {
    context.out << std::boolalpha << value;
}
 
[[maybe_unused]] void print_value(Array nodes, const PrintContext& context) {
    std::ostream& out = context.out;
    out << "[\n"sv;
    bool first = true;
    auto inner_context = context.indented();
    
    for (const Node& node : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }

        inner_context.print_indent();
        print_node(node, inner_context);
    }

    out.put('\n');
    context.print_indent();
    out.put(']');
}
 
[[maybe_unused]] void print_value(Dict nodes, const PrintContext& context) {
    std::ostream& out = context.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_context = context.indented();
    
    for (const auto& [key, node] : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }

        inner_context.print_indent();
        print_string(key, context.out);
        out << ": "sv;
        print_node(node, inner_context);
    }

    out.put('\n');
    context.print_indent();
    out.put('}');
}
 
void print_node(const Node& node, const PrintContext& context) {
    std::visit([&context](const auto& value) {
            print_value(value, context);
        }, node.get_value());
}
    
void print(const Document& document, std::ostream& output) {
    print_node(document.get_root(), PrintContext{output});
}

}//end namespace json
}//end namespace detail
}//end namespace transport_catalogue