#include "json_builder.h"
namespace catalogue
{
    namespace json
    {
        Builder::Builder() : root_(nullptr), nodes_stack_{&root_} {}

        Node Builder::Build()
        {
            if (!nodes_stack_.empty())
            {
                throw std::logic_error("not all objects are completed");
            }
            return std::move(root_);
        }

        Builder::KeyItemContext Builder::Key(std::string key)
        {
            if (nodes_stack_.empty())
            {
                throw std::logic_error("attempt to modify completed objects");
            }

            Node::Value &last_value = nodes_stack_.back()->GetValue();
            if (!std::holds_alternative<Dict>(last_value))
            {
                throw std::logic_error("to add a key, you need to create a dictionary");
            }

            nodes_stack_.push_back(&std::get<Dict>(last_value)[std::move(key)]);
            return *this;
        }

        Builder &Builder::Value(Node::Value value)
        {
            if (nodes_stack_.empty())
            {
                throw std::logic_error("attempt to modify completed objects");
            }

            Node::Value &last_value = nodes_stack_.back()->GetValue();
            if (std::holds_alternative<std::nullptr_t>(last_value))
            {
                last_value = std::move(value);
                nodes_stack_.pop_back();
            }
            else
            {
                AddObject(value, false);
            }

            return *this;
        }

        Builder::DictItemContext Builder::StartDict()
        {
            AddObject(Dict{}, true);
            return *this;
        }

        Builder::ArrayItemContext Builder::StartArray()
        {
            AddObject(Array{}, true);
            return *this;
        }

        Builder &Builder::EndDict()
        {
            if (nodes_stack_.empty())
            {
                throw std::logic_error("closing an object that was not created");
            }

            Node::Value &last_value = nodes_stack_.back()->GetValue();
            if (!std::holds_alternative<Dict>(last_value))
            {
                throw std::logic_error("closing an object that was not created");
            }
            nodes_stack_.pop_back();
            return *this;
        }

        Builder &Builder::EndArray()
        {
            if (nodes_stack_.empty())
            {
                throw std::logic_error("closing an object that was not created");
            }

            Node::Value &last_value = nodes_stack_.back()->GetValue();
            if (!std::holds_alternative<Array>(last_value))
            {
                throw std::logic_error("closing an object that was not created");
            }
            nodes_stack_.pop_back();
            return *this;
        }

        void Builder::AddObject(Node::Value value, bool one_shot)
        {
            if (nodes_stack_.empty())
            {
                throw std::logic_error("attempt to modify completed objects");
            }

            Node::Value &last_value = nodes_stack_.back()->GetValue();
            if (std::holds_alternative<Array>(last_value))
            {
                Node &node = std::get<Array>(last_value).emplace_back(std::move(value));
                if (one_shot)
                {
                    nodes_stack_.push_back(&node);
                }
            }
            else
            {
                if (!std::holds_alternative<std::nullptr_t>(last_value))
                {
                    throw std::logic_error("New object in wrong context");
                }
                last_value = std::move(value);
                if (!one_shot)
                {
                    nodes_stack_.pop_back();
                }
            }
        }
    } // namespace json
} // namespace catalogue
