#pragma once

#include <variant>
#include "JsonException.h"
#include "Json.h"
#include <functional>

class JsonValue final {
public:

    explicit JsonValue(std::nullptr_t) : _val(nullptr) { }
    explicit JsonValue(bool val) : _val(val) { }
    explicit JsonValue(double val) : _val(val) { }
    explicit JsonValue(const std::string& val) : _val(val) { }
    explicit JsonValue(const Json::array& val) : _val(val) { }
    explicit JsonValue(const Json::object& val) : _val(val) { }
    explicit JsonValue(std::string& val) : _val(std::move(val)) { }
    explicit JsonValue(Json::array&& val) : _val(std::move(val)) { }
    explicit JsonValue(Json::object&& val) : _val(std::move(val)) { }
    ~JsonValue() = default;

    JsonType getType() const;
    size_t size() const;
    const Json& operator[](size_t) const;
    Json& operator[](size_t);
    const Json& operator[](const std::string&) const;
    Json& operator[](const std::string&);

    std::nullptr_t toNull() const;
    bool toBool() const;
    double toNumber() const;
    const std::string& toString() const;
    const Json::array& toArray() const;
    const Json::object& toObject() const;

private:
    std::variant<std::nullptr_t, bool, double, 
        std::string, Json::array, Json::object> _val;
    
};