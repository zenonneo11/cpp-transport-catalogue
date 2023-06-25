#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    using namespace std::literals;

    class Node;

    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    using Node_Value = std::variant<std::nullptr_t, Array, Dict, std::string, int, double, bool>;

    inline const Node_Value null_node = nullptr;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node : private Node_Value {

    public:
        using std::variant<std::nullptr_t, Array, Dict, std::string, int, double, bool>::variant;

        bool IsInt() const;
        bool IsDouble() const;
        bool IsPureDouble() const;
        bool IsBool() const;
        bool IsString() const;
        bool IsNull() const;
        bool IsArray() const;
        bool IsMap() const;

        int AsInt() const;
        double AsDouble() const;
        const std::string& AsString() const;
        bool AsBool() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        const Node_Value& GetValue() const { return *this; }
    private:

    };

    bool operator==(const Node& lhs, const Node& rhs);

    bool operator!=(const Node& lhs, const Node& rhs);


    void PrintNode(const Node& node, std::ostream& out);

    struct NodePrinter {
        std::ostream& out;
        void operator()(std::nullptr_t) const {
            out << "null"s;
        }
        void operator()(std::string str) const {
            out << '\"';
            for (auto c : str) {
                if (c == '\"')
                    out << '\\' << '\"';
                else if (c == '\r')
                    out << '\\' << 'r';
                else if (c == '\n')
                    out << '\\' << 'n';
                else if (c == '\\')
                    out << '\\' << '\\';
                else
                    out << (c);
            }
            out << '\"';
        }
        template <typename Val>
        void operator()(Val value) const {
            out << value;
        }
        void operator()(bool b) const {
            out << std::boolalpha << b;
        }

        void operator()(Array arr) const {
            out << '[';
            bool first = true;
            for (const auto& obj : arr) {
                if (first) {
                    PrintNode(obj, out);
                    first = false;
                }
                else {
                    out << ',';
                    PrintNode(obj, out);
                }
            }
            out << ']';
        }

        void operator()(Dict dict) const {
            out << '{';
            bool first = true;
            for (const auto& [key, val] : dict) {
                if (first) {
                    out << '\"' << key << '\"' << ':';
                    PrintNode(val, out);
                    first = false;
                }
                else {
                    out << ',';
                    out << '\"' << key << '\"' << ':';
                    PrintNode(val, out);
                }
            }
            out << '}';
        }
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

    private:
        Node root_;
    };
    bool operator == (const Document& lhs, const Document& rhs);
    bool operator != (const Document& lhs, const Document& rhs);

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

}  // namespace json