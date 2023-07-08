#pragma once
#include "json.h"
#include <optional>
#include <memory>
#include <variant>


namespace json {

    class Builder {

    private:
        Node root_ = nullptr;
        std::vector<std::unique_ptr<Node>> nodes_stack_;

        class BaseContext;
        class EndContext;
        class KeyContext;
        class DictItemContext;
        class ArrayItemContext;


        class BaseContext { 

        protected:
            Builder& builder_;

        public:
            BaseContext(Builder& builder) :builder_(builder) {}

            BaseContext EndArray() {
                return builder_.EndArray();
            }

            BaseContext EndDict() {
                return builder_.EndDict();
            }

            KeyContext Key(std::string key) {
                return builder_.Key(key);
            }

            ArrayItemContext StartArray() {
                return builder_.StartArray();
            }

            DictItemContext StartDict() {
                return builder_.StartDict();
            }
            BaseContext Value(Node::Value value) {
                return builder_.Value(value);
            }

            Node& Build() {
                return builder_.Build();
            }
        };


        class EndContext : public BaseContext {
            EndContext(Builder& builder) : BaseContext(builder) {}
            BaseContext StartArray() = delete;
            BaseContext StartDict() = delete;
            BaseContext EndArray() = delete;
            BaseContext EndDict() = delete;
            BaseContext Key(std::string key) = delete;

        };


        class KeyContext : public BaseContext {
        public:
            KeyContext(Builder& builder) : BaseContext(builder) {}
            BaseContext EndArray() = delete;
            BaseContext EndDict() = delete;
            BaseContext Key(std::string key) = delete;
            Node& Build() = delete;
            DictItemContext Value(Node::Value value) {
                std::string key = std::get<std::string>(builder_.nodes_stack_.back()->GetValue());
                builder_.nodes_stack_.pop_back();
                std::get<Dict>(builder_.nodes_stack_.back()->GetValue()).emplace(std::move(key), std::move(value));
                return builder_;
            }
        };


        class DictItemContext :public BaseContext {
        public:
            DictItemContext(Builder& builder) : BaseContext(builder) {}
            BaseContext EndArray() = delete;
            BaseContext StartArray() = delete;
            BaseContext StartDict() = delete;
            BaseContext Value(Node::Value value) = delete;
            Node& Build() = delete;

        };


        class ArrayItemContext : public BaseContext {
        public:
            ArrayItemContext(Builder& builder) : BaseContext(builder) {}
            BaseContext Key(std::string key) = delete;
            BaseContext EndDict() = delete;
            Node& Build() = delete;
            ArrayItemContext Value(Node::Value value){
                std::get<Array>(builder_.nodes_stack_.back()->GetValue()).emplace_back(move(value));
                return builder_;
            }
        };


    public:


        Builder() {}

        KeyContext Key(std::string key);

        BaseContext Value(Node::Value value);

        DictItemContext StartDict();

        ArrayItemContext StartArray();

        BaseContext EndDict();

        BaseContext EndArray();

        Node& Build();
    };
}