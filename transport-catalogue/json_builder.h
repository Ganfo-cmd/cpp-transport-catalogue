#pragma once

#include "json.h"
namespace catalogue
{
    namespace json
    {

        class Builder
        {
        private:
            class DictItemContext;
            class ArrayItemContext;
            class ValueItemContext;
            class KeyItemContext;

        public:
            Builder();
            Node Build();
            KeyItemContext Key(std::string key);
            Builder &Value(Node::Value value);
            DictItemContext StartDict();
            ArrayItemContext StartArray();
            Builder &EndDict();
            Builder &EndArray();

        private:
            Node root_;
            std::vector<Node *> nodes_stack_;

            void AddObject(Node::Value value, bool one_shot);

            class ItemContext
            {
            public:
                ItemContext(Builder &builder) : builder_(builder) {};

                Node Build()
                {
                    return builder_.Build();
                }

                KeyItemContext Key(std::string key)
                {
                    return builder_.Key(std::move(key));
                }

                Builder &Value(Node::Value value)
                {
                    {
                        return builder_.Value(std::move(value));
                    }
                }

                DictItemContext StartDict()
                {
                    {
                        return builder_.StartDict();
                    }
                }

                ArrayItemContext StartArray()
                {
                    {
                        return builder_.StartArray();
                    }
                }

                Builder &EndDict()
                {
                    {
                        return builder_.EndDict();
                    }
                }

                Builder &EndArray()
                {
                    {
                        return builder_.EndArray();
                    }
                }

            private:
                Builder &builder_;
            };

            class DictItemContext : public ItemContext
            {
            public:
                DictItemContext(Builder &builder) : ItemContext(builder) {}

                Node Build() = delete;
                Builder &Value(Node::Value value) = delete;
                DictItemContext StartDict() = delete;
                ArrayItemContext StartArray() = delete;
                Builder &EndArray() = delete;
            };

            class ArrayItemContext : public ItemContext
            {
            public:
                ArrayItemContext(Builder &builder) : ItemContext(builder) {}

                Node Build() = delete;
                KeyItemContext Key(std::string key) = delete;
                Builder &EndDict() = delete;

                ArrayItemContext Value(Node::Value value)
                {
                    return ItemContext::Value(std::move(value));
                }
            };

            class ValueItemContext : public ItemContext
            {
            public:
                ValueItemContext(Builder &builder) : ItemContext(builder) {}

                Node Build() = delete;
                Builder &Value(Node::Value value) = delete;
                DictItemContext StartDict() = delete;
                ArrayItemContext StartArray() = delete;
                Builder &EndArray() = delete;
            };

            class KeyItemContext : public ItemContext
            {
            public:
                KeyItemContext(Builder &builder) : ItemContext(builder) {}

                Node Build() = delete;
                KeyItemContext Key(std::string key) = delete;
                Builder &EndDict() = delete;
                Builder &EndArray() = delete;

                ValueItemContext Value(Node::Value value)
                {
                    return ItemContext::Value(std::move(value));
                }
            };
        };
    } // namespace json
} // namespace catalogue
