#include "json_builder.h"


namespace json {

    Builder::KeyContext Builder::Key(std::string key) {
        if (!root_.IsNull() || nodes_stack_.empty() || !nodes_stack_.back()->IsDict())
            throw std::logic_error("can't add key");
        nodes_stack_.emplace_back(std::make_unique<Node>(key));
        return *this;
    }

    Builder::BaseContext Builder::Value(Node::Value value) {
        if (!root_.IsNull() || !(nodes_stack_.empty() || nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsString()))
            throw std::logic_error("can't add Value");

        if (nodes_stack_.empty()) {
            root_ = Node(move(value));
        }
        else if (nodes_stack_.back()->IsArray()) {
            std::get<Array>(nodes_stack_.back()->GetValue()).emplace_back(move(value));
            // return ArrayItemContext(*this);
        }
        else if (nodes_stack_.back()->IsString()) {
            std::string key = std::get<std::string>(nodes_stack_.back()->GetValue());
            nodes_stack_.pop_back();
            std::get<Dict>(nodes_stack_.back()->GetValue()).emplace(std::move(key), std::move(value));
            //return DictItemContext(*this);
        }

        else
        {

        }
        //return BaseContext(*this);
        return *this;
    }

    Builder::DictItemContext Builder::StartDict() {
        if (!root_.IsNull() || !(nodes_stack_.empty() || nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsString()))
            throw std::logic_error("can't add Dict");
        nodes_stack_.emplace_back(std::make_unique<Node>(Dict()));
        return *this;
    }


    Builder::ArrayItemContext Builder::StartArray() {
        if (!root_.IsNull() || !(nodes_stack_.empty() || nodes_stack_.back()->IsArray() || nodes_stack_.back()->IsString()))
            throw std::logic_error("can't add Array");
        nodes_stack_.emplace_back(std::make_unique<Node>(Array()));
        return *this;
    }

    Builder::BaseContext Builder::EndDict() {
        if (!root_.IsNull() || nodes_stack_.empty() || !nodes_stack_.back()->IsDict())
            throw std::logic_error("can't complete Dict");
        Node::Value val = nodes_stack_.back().release()->GetValue();
        nodes_stack_.pop_back();
        Value(move(val));
        return *this;
    }


    Builder::BaseContext Builder::EndArray() {
        if (!root_.IsNull() || nodes_stack_.empty() || !nodes_stack_.back()->IsArray())
            throw std::logic_error("can't complete Array");
        Node::Value val = nodes_stack_.back().release()->GetValue();
        nodes_stack_.pop_back();
        Value(move(val));
        return *this;
    }


    Node& Builder::Build() {
        if (root_.IsNull() || !nodes_stack_.empty())
            throw std::logic_error("error Build()");
        return root_;
    }
}