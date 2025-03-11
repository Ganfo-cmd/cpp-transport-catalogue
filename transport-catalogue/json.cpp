#include "json.h"

namespace catalogue
{
    namespace json
    {
        namespace
        {
            using namespace std::literals;
            std::string ParseWord(std::istream &input)
            {
                std::string str;

                while (std::isalpha(input.peek()))
                {
                    str.push_back((input.get()));
                }
                return str;
            }

            Node LoadNode(std::istream &input);

            Node LoadArray(std::istream &input)
            {
                Array result;

                for (char c; input >> c && c != ']';)
                {
                    if (c != ',')
                    {
                        input.putback(c);
                    }
                    result.push_back(LoadNode(input));
                }

                if (!input)
                {
                    throw ParsingError("Сan't process the map");
                }

                return Node(move(result));
            }

            Node LoadNumber(std::istream &input)
            {
                std::string parsed_num;

                // Считывает в parsed_num очередной символ из input
                auto read_char = [&parsed_num, &input]
                {
                    parsed_num += static_cast<char>(input.get());
                    if (!input)
                    {
                        throw ParsingError("Failed to read number from stream"s);
                    }
                };

                // Считывает одну или более цифр в parsed_num из input
                auto read_digits = [&input, read_char]
                {
                    if (!std::isdigit(input.peek()))
                    {
                        throw ParsingError("A digit is expected"s);
                    }
                    while (std::isdigit(input.peek()))
                    {
                        read_char();
                    }
                };

                if (input.peek() == '-')
                {
                    read_char();
                }
                // Парсим целую часть числа
                if (input.peek() == '0')
                {
                    read_char();
                    // После 0 в JSON не могут идти другие цифры
                }
                else
                {
                    read_digits();
                }

                bool is_int = true;
                // Парсим дробную часть числа
                if (input.peek() == '.')
                {
                    read_char();
                    read_digits();
                    is_int = false;
                }

                // Парсим экспоненциальную часть числа
                if (int ch = input.peek(); ch == 'e' || ch == 'E')
                {
                    read_char();
                    if (ch = input.peek(); ch == '+' || ch == '-')
                    {
                        read_char();
                    }
                    read_digits();
                    is_int = false;
                }

                try
                {
                    if (is_int)
                    {
                        // Сначала пробуем преобразовать строку в int
                        try
                        {
                            return Node(std::stoi(parsed_num));
                        }
                        catch (...)
                        {
                            // В случае неудачи, например, при переполнении,
                            // код ниже попробует преобразовать строку в double
                        }
                    }
                    return Node(std::stod(parsed_num));
                }
                catch (...)
                {
                    throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
                }
            }

            Node LoadString(std::istream &input)
            {
                auto it = std::istreambuf_iterator<char>(input);
                auto end = std::istreambuf_iterator<char>();
                std::string s;
                while (true)
                {
                    if (it == end)
                    {
                        throw ParsingError("String parsing error");
                    }
                    const char ch = *it;
                    if (ch == '"')
                    {
                        ++it;
                        break;
                    }
                    else if (ch == '\\')
                    {
                        ++it;
                        if (it == end)
                        {
                            throw ParsingError("String parsing error");
                        }
                        const char escaped_char = *(it);
                        switch (escaped_char)
                        {
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
                            throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
                        }
                    }
                    else if (ch == '\n' || ch == '\r')
                    {
                        throw ParsingError("Unexpected end of line"s);
                    }
                    else
                    {
                        s.push_back(ch);
                    }
                    ++it;
                }

                return Node(s);
            }

            Node LoadDict(std::istream &input)
            {
                Dict result;

                for (char c; input >> c && c != '}';)
                {
                    if (c == ',')
                    {
                        input >> c;
                    }

                    std::string key = LoadString(input).AsString();
                    input >> c;
                    result.insert({std::move(key), LoadNode(input)});
                }

                if (!input)
                {
                    throw ParsingError("Сan't process the dictionary");
                }

                return Node(move(result));
            }

            Node LoadBool(std::istream &input)
            {
                std::string str = ParseWord(input);

                if (str == "false"sv)
                {
                    return Node(false);
                }
                else if (str == "true"sv)
                {
                    return Node(true);
                }
                else
                {
                    throw ParsingError("Сan't process the bool value");
                }
            }

            Node LoadNull(std::istream &input)
            {
                std::string str = ParseWord(input);
                if (str == "null"sv)
                {
                    return Node(nullptr);
                }

                throw ParsingError("Сan't process the input");
            }

            Node LoadNode(std::istream &input)
            {
                char c;
                input >> c;

                if (c == '[')
                {
                    return LoadArray(input);
                }
                else if (c == '{')
                {
                    return LoadDict(input);
                }
                else if (c == '"')
                {
                    return LoadString(input);
                }
                else if (isdigit(c) || c == '-')
                {
                    input.putback(c);
                    return LoadNumber(input);
                }
                else if (c == 'f' || c == 't')
                {
                    input.putback(c);
                    return LoadBool(input);
                }
                else
                {
                    input.putback(c);
                    return LoadNull(input);
                }
            }

        } // namespace

        //------------Node------------

        Node::Node(Value value) : variant(std::move(value)) {}

        bool Node::operator==(const Node &rhs) const
        {
            return GetValue() == rhs.GetValue();
        }

        bool Node::operator!=(const Node &rhs) const
        {
            return !(*this == rhs);
        }

        bool Node::IsInt() const
        {
            return std::holds_alternative<int>(*this);
        }

        bool Node::IsPureDouble() const
        {
            return std::holds_alternative<double>(*this);
        }

        bool Node::IsDouble() const
        {
            return (IsPureDouble() || IsInt());
        }

        bool Node::IsBool() const
        {
            return std::holds_alternative<bool>(*this);
        }

        bool Node::IsString() const
        {
            return std::holds_alternative<std::string>(*this);
        }

        bool Node::IsNull() const
        {
            return std::holds_alternative<std::nullptr_t>(*this);
        }

        bool Node::IsArray() const
        {
            return std::holds_alternative<Array>(*this);
        }

        bool Node::IsMap() const
        {
            return std::holds_alternative<Dict>(*this);
        }

        int Node::AsInt() const
        {
            if (IsInt())
            {
                return std::get<int>(*this);
            }
            throw std::logic_error("logic error");
        }

        bool Node::AsBool() const
        {
            if (IsBool())
            {
                return std::get<bool>(*this);
            }
            throw std::logic_error("logic error");
        }

        double Node::AsDouble() const
        {
            if (IsDouble())
            {
                return IsInt() ? std::get<int>(*this) : std::get<double>(*this);
            }

            throw std::logic_error("logic error");
        }

        const std::string &Node::AsString() const
        {
            if (IsString())
            {
                return std::get<std::string>(*this);
            }
            throw std::logic_error("logic error");
        }

        const Array &Node::AsArray() const
        {
            if (IsArray())
            {
                return std::get<Array>(*this);
            }
            throw std::logic_error("logic error");
        }

        const Dict &Node::AsMap() const
        {
            if (IsMap())
            {
                return std::get<Dict>(*this);
            }
            throw std::logic_error("logic error");
        }

        const Node::Value &Node::GetValue() const
        {
            return *this;
        }

        Node::Value &Node::GetValue()
        {
            return *this;
        }

        //------------Document------------

        Document::Document(Node root)
            : root_(std::move(root))
        {
        }

        const Node &Document::GetRoot() const
        {
            return root_;
        }

        bool Document::operator==(const Document &rhs) const
        {
            return GetRoot() == rhs.GetRoot();
        }

        bool Document::operator!=(const Document &rhs) const
        {
            return !(*this == rhs);
        }

        Document Load(std::istream &input)
        {
            return Document{LoadNode(input)};
        }

        //------------Print------------

        void PrintIndent(std::ostream &out, int indent_count)
        {
            for (int i = 0; i < indent_count * 4; ++i)
            {
                out << " ";
            }
        }

        template <typename Value>
        void PrintValue(const Value &value, std::ostream &out, [[maybe_unused]] int indent_count)
        {
            out << value;
        }

        void PrintValue(std::nullptr_t, std::ostream &out, [[maybe_unused]] int indent_count)
        {
            out << "null"sv;
        }

        void PrintValue(bool value, std::ostream &out, [[maybe_unused]] int indent_count)
        {
            out << (value ? "true" : "false");
        }

        void PrintValue(const std::string &value, std::ostream &out, [[maybe_unused]] int indent_count)
        {
            out << '"';
            for (const auto &ch : value)
            {

                if (ch == '\\' || ch == '"')
                {
                    out << '\\';
                }

                if (ch == '\r')
                {
                    out << "\\r";
                    continue;
                }

                if (ch == '\n')
                {
                    out << "\\n";
                    continue;
                }

                if (ch == '\t')
                {
                    out << "\\t";
                    continue;
                }

                out << ch;
            }
            out << '"';
        }

        void PrintValue(const Array &array, std::ostream &out, int indent_count)
        {
            bool is_first = true;
            out << "[\n";
            for (const auto &elem : array)
            {
                if (!is_first)
                {
                    out << ",\n";
                }

                PrintIndent(out, indent_count + 1);
                std::visit([&out, &indent_count](const auto &val)
                           { PrintValue(val, out, indent_count + 1); }, elem.GetValue());

                is_first = false;
            }
            out << "\n";
            PrintIndent(out, indent_count);
            out << "]";
        }

        void PrintValue(const Dict &dict, std::ostream &out, int indent_count)
        {
            bool is_first = true;
            out << "{\n";
            for (const auto &[key, value] : dict)
            {
                if (!is_first)
                {
                    out << ",\n";
                }
                PrintIndent(out, indent_count + 1);
                out << '"' << key << "\": ";
                std::visit([&out, &indent_count](const auto &val)
                           { PrintValue(val, out, indent_count + 1); }, value.GetValue());

                is_first = false;
            }
            out << "\n";
            PrintIndent(out, indent_count);
            out << "}";
        }

        void Print(const Document &doc, std::ostream &output)
        {
            int indent_count = 0;
            std::visit([&output, &indent_count](const auto &value)
                       { PrintValue(value, output, indent_count); }, doc.GetRoot().GetValue());
        }

    } // namespace json
} // namespace catalogue