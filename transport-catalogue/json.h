#pragma once
#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace catalogue
{
    namespace json
    {

        class Node;
        using Dict = std::map<std::string, Node>;
        using Array = std::vector<Node>;

        class ParsingError : public std::runtime_error
        {
        public:
            using runtime_error::runtime_error;
        };

        class Node final : std::variant<std::nullptr_t, int, double, bool, std::string, Array, Dict>
        {
        public:
            using variant::variant;
            using Value = variant;

            Node(Value value);

            bool operator==(const Node &rhs) const;
            bool operator!=(const Node &rhs) const;

            bool IsInt() const;
            bool IsPureDouble() const;
            bool IsDouble() const;
            bool IsBool() const;
            bool IsString() const;
            bool IsNull() const;
            bool IsArray() const;
            bool IsMap() const;

            // методы, которые возвращают хранящееся внутри Node значение заданного типа
            int AsInt() const;
            bool AsBool() const;
            double AsDouble() const;
            const std::string &AsString() const;
            const Array &AsArray() const;
            const Dict &AsMap() const;

            const Value &GetValue() const;

            Value &GetValue();
        };

        class Document
        {
        public:
            Document() = default;
            explicit Document(Node root);

            bool operator==(const Document &rhs) const;
            bool operator!=(const Document &rhs) const;

            const Node &GetRoot() const;

        private:
            Node root_;
        };

        Document Load(std::istream &input);

        void Print(const Document &doc, std::ostream &output);

    } // namespace json
} // namespace catalogue