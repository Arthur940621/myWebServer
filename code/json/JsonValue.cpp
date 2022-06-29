#include "JsonValue.h"

JsonType JsonValue::getType() const {
    if (std::holds_alternative<std::nullptr_t>(_val)) {
        return JsonType::Null;
    } else if (std::holds_alternative<bool>(_val)) {
        return JsonType::Bool;
    } else if (std::holds_alternative<double>(_val)) {
        return JsonType::Number;
    } else if (std::holds_alternative<std::string>(_val)) {
        return JsonType::String;
    } else if (std::holds_alternative<Json::array>(_val)) {
        return JsonType::Array;
    } else {
        return JsonType::Object;
    }
}

size_t JsonValue::size() const {
    if (std::holds_alternative<Json::array>(_val)) {
        return std::get<Json::array>(_val).size();
    } else if (std::holds_alternative<Json::object>(_val)) {
        return std::get<Json::object>(_val).size();
    } else {
        throw JsonException("not a array or object");
    }
}

const Json& JsonValue::operator[](size_t pos) const {
    if (std::holds_alternative<Json::array>(_val)) {
        return std::get<Json::array>(_val)[pos];
    } else {
        throw JsonException("not a array");
    }
}

Json& JsonValue::operator[](size_t pos) {
    return const_cast<Json&>(static_cast<const JsonValue&>(*this)[pos]);
}


const Json& JsonValue::operator[](const std::string& key) const {
    if (std::holds_alternative<Json::object>(_val)) {
        return std::get<Json::object>(_val).at(key);
    } else {
        throw JsonException("not a object");
    }
}

Json& JsonValue::operator[](const std::string& key) {
    return const_cast<Json&>(static_cast<const JsonValue&>(*this)[key]);
}

std::nullptr_t JsonValue::toNull() const {
    if (this->getType() == JsonType::Null) {
        return std::get<std::nullptr_t>(_val);
    } else {
        throw JsonException("not a null");
    }
}

bool JsonValue::toBool() const {
    if (this->getType() == JsonType::Bool) {
        return std::get<bool>(_val);
    } else {
        throw JsonException("not a bool");
    }
}

double JsonValue::toNumber() const {
    if (this->getType() == JsonType::Number) {
        return std::get<double>(_val);
    } else {
        throw JsonException("not a number");
    }
}

const std::string& JsonValue::toString() const {
    if (this->getType() == JsonType::String) {
        return std::get<std::string>(_val);
    } else {
        throw JsonException("not a string");
    }
}

const Json::array& JsonValue::toArray() const {
    if (this->getType() == JsonType::Array) {
        return std::get<Json::array>(_val);
    } else {
        throw JsonException("not a array");
    }
}

const Json::object& JsonValue::toObject() const {
    if (this->getType() == JsonType::Object) {
        return std::get<Json::object>(_val);
    } else {
        throw JsonException("not a object");
    }
}