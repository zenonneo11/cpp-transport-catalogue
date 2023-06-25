#include "json.h"

using namespace std;

namespace json {

    namespace {

        Node LoadNode(istream& input);

        Node LoadArray(istream& input) {
            Array result;
            bool closing_bracket = false;
            for (char c; input >> c;) {
                if (c == ']') {
                    closing_bracket = true;
                    break;
                }
                if (c != ',') {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
            if (closing_bracket)
                return Node(move(result));
            else
                throw ParsingError("no closing bracket"s);
        }

        using Number = std::variant<int, double>;

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            // Считывает в parsed_num очередной символ из input
            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream"s);
                }
            };

            // Считывает одну или более цифр в parsed_num из input
            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
                    read_char();
                }
            };

            if (input.peek() == '-') {
                read_char();
            }
            // Парсим целую часть числа
            if (input.peek() == '0') {
                read_char();
                // После 0 в JSON не могут идти другие цифры
            }
            else {
                read_digits();
            }

            bool is_int = true;
            // Парсим дробную часть числа
            if (input.peek() == '.') {
                read_char();
                read_digits();
                is_int = false;
            }

            // Парсим экспоненциальную часть числа
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
                    // Сначала пробуем преобразовать строку в int
                    try {
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                        // В случае неудачи, например, при переполнении,
                        // код ниже попробует преобразовать строку в double
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        // Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
        std::string LoadString(std::istream& input) {
            using namespace std::literals;

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
            while (true) {
                if (it == end) {
                    // Поток закончился до того, как встретили закрывающую кавычку?
                    throw ParsingError("String parsing error");
                }
                const char ch = *it;
                if (ch == '"') {
                    // Встретили закрывающую кавычку
                    ++it;
                    break;
                }
                else if (ch == '\\') {
                    // Встретили начало escape-последовательности
                    ++it;
                    if (it == end) {
                        // Поток завершился сразу после символа обратной косой черты
                        throw ParsingError("String parsing error");
                    }
                    const char escaped_char = *(it);
                    // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
                    switch (escaped_char) {
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        // Встретили неизвестную escape-последовательность
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                    }
                }
                else if (ch == '\n' || ch == '\r') {
                    // Строковый литерал внутри- JSON не может прерываться символами \r или \n
                    throw ParsingError("Unexpected end of line"s);
                }
                else {
                    // Просто считываем очередной символ и помещаем его в результирующую строку
                    s.push_back(ch);
                }
                ++it;
            }

            return s;
        }

        Node LoadNum(istream& input) {
            Node res;
            std::visit([&](auto value) {
                res = Node(value);
                }, LoadNumber(input));
            return res;
        }

        Node LoadStr(istream& input) {
            return Node(LoadString(input));
        }

        Node LoadDict(istream& input) {
            Dict result;
            bool closing_bracket = false;
            for (char c; input >> c;) {
                if (c == '}') {
                    closing_bracket = true;
                    break;
                }

                if (c == ',') {
                    input >> c;
                }

                string key = LoadStr(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
            if (closing_bracket) {
                return Node(move(result));
            }
            else
            {
                throw ParsingError("no closing bracket"s);
            }
        }

        bool IsInInputStream(const string& s, istream& input) {
            string res = "";
            char c;
            for (size_t i = 0; i < s.size() && input >> c; ++i) {
                res += c;
            }
            return  res == s;
        }

        Node LoadNode(istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                return LoadDict(input);
            }
            else if (c == '"') {
                return LoadStr(input);

            }
            else if (c == 'n') {
                input.putback(c);
                if (IsInInputStream("null", input))
                    return Node(nullptr);
                else
                    throw ParsingError("");
            }
            else if (c == 'f') {
                input.putback(c);
                if (IsInInputStream("false", input))
                    return Node(false);
                else
                    throw ParsingError("");
            }
            else if (c == 't') {
                input.putback(c);
                if (IsInInputStream("true", input))
                    return Node(true);
                else
                    throw ParsingError("");
            }
            else {
                input.putback(c);
                return LoadNum(input);
            }
        }

    }  // namespace

    bool Node::IsInt() const {
        return std::holds_alternative<int>(*this);
    }

    bool Node::IsDouble() const {
        return std::holds_alternative<double>(*this) || IsInt();
    }

    bool Node::IsPureDouble() const {
        return std::holds_alternative<double>(*this);
    }
    bool Node::IsBool() const {
        return std::holds_alternative<bool>(*this);
    }

    bool Node::IsString() const {
        return std::holds_alternative<string>(*this);
    }

    bool Node::IsNull() const {
        return std::holds_alternative<nullptr_t>(*this);
    }

    bool Node::IsArray() const {
        return std::holds_alternative<Array>(*this);
    }

    bool Node::IsMap() const {
        return std::holds_alternative<Dict>(*this);
    }

    int Node::AsInt() const {
        if (IsInt())
            return get<int>(*this);
        else
            throw logic_error("not int");
    }

    double Node::AsDouble() const {
        if (IsDouble()) {
            if (IsPureDouble())
                return get<double>(*this);
            else
                return static_cast<double>(get<int>(*this));
        }
        else
            throw logic_error("not double");
    }
    const string& Node::AsString() const {
        if (IsString())
            return get<string>(*this);
        else
            throw logic_error("not string");
    }

    bool Node::AsBool() const {
        if (IsBool())
            return get<bool>(*this);
        else
            throw logic_error("not bool");
    }

    const Array& Node::AsArray() const {
        if (std::holds_alternative<Array>(*this))
            return get<Array>(*this);
        else
            throw logic_error("not array");
    }

    const Dict& Node::AsMap() const {
        if (IsMap())
            return get <Dict>(*this);
        else
            throw logic_error("not dictionary");
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    bool operator == (const Document& lhs, const Document& rhs) {
        return lhs.GetRoot() == rhs.GetRoot();
    }

    bool operator != (const Document& lhs, const Document& rhs) {
        return !(lhs == rhs);
    }

    Document Load(istream& input) {
        return Document{ LoadNode(input) };
    }

    void PrintNode(const Node& node, std::ostream& out) {
        std::visit(NodePrinter{ out }, node.GetValue());
    }

    void Print(const Document& doc, std::ostream& output) {
        PrintNode(doc.GetRoot(), output);
    }



    bool operator==(const Node& lhs, const Node& rhs) {
        return std::is_same_v<decltype(lhs), decltype(rhs)>&& lhs.GetValue() == rhs.GetValue();
    }

    bool operator!=(const Node& lhs, const Node& rhs) {
        return !(lhs == rhs);
    }



}  // namespace json