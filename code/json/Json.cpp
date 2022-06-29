#include <cstdio>
#include "Json.h"
#include "JsonValue.h"
#include "JsonParser.h"

Json::Json(std::nullptr_t) : _jsonValue(std::make_unique<JsonValue>(nullptr)) { }
Json::Json(bool val) : _jsonValue(std::make_unique<JsonValue>(val)) { }
Json::Json(double val) : _jsonValue(std::make_unique<JsonValue>(val)) { }
Json::Json(const std::string& val) : _jsonValue(std::make_unique<JsonValue>(val)) { }
Json::Json(std::string&& val) : _jsonValue(std::make_unique<JsonValue>(std::move(val))) { }
Json::Json(const array& val) : _jsonValue(std::make_unique<JsonValue>(val)) { }
Json::Json(array&& val) : _jsonValue(std::make_unique<JsonValue>(std::move(val))) { }
Json::Json(const object& val) : _jsonValue(std::make_unique<JsonValue>(val)) { }
Json::Json(object&& val) : _jsonValue(std::make_unique<JsonValue>(std::move(val))) { }

Json::~Json() = default;

Json::Json(const Json& rhs) {
    switch (rhs.getType()) {
        case JsonType::Null: {
            _jsonValue = std::make_unique<JsonValue>(nullptr);
            break;
        } case JsonType::Bool: {
            _jsonValue = std::make_unique<JsonValue>(rhs.toBool());
            break;
        } case JsonType::Number: {
            _jsonValue = std::make_unique<JsonValue>(rhs.toNumber());
            break;
        } case JsonType::String: {
            _jsonValue = std::make_unique<JsonValue>(rhs.toString());
            break;
        } case JsonType::Array: {
            _jsonValue = std::make_unique<JsonValue>(rhs.toArray());
            break;
        } case JsonType::Object: {
            _jsonValue = std::make_unique<JsonValue>(rhs.toObject());
            break;
        } default: {
            break;
        }
    }
}

Json& Json::operator=(Json rhs) {
    Json temp(rhs);
    swap(temp);
    return *this;
}

Json::Json(Json&& rhs) noexcept : _jsonValue(std::move(rhs._jsonValue)) {
    rhs._jsonValue = nullptr;
}

const Json& Json::operator[](std::size_t pos) const {
    return _jsonValue->operator[](pos);
}

Json& Json::operator[](std::size_t pos) {
    return _jsonValue->operator[](pos);
}

const Json& Json::operator[](const std::string& key) const {
    return _jsonValue->operator[](key);
}

Json& Json::operator[](const std::string& key) {
    return _jsonValue->operator[](key);
}

Json Json::parse(const std::string& content, std::string& errMsg) noexcept {
    try {
        JsonParser p(content);
        return p.parse();
    } catch (JsonException& e) {
        errMsg = e.what();
        return Json(nullptr);
    }
}

std::string Json::serializeString() const noexcept {
    std::string res = "\"";
    for (auto c : _jsonValue->toString()) {
        switch (c) {
            case '\"':
                res += "\\\"";
                break;
            case '\\':
                res += "\\\\";
                break;
            case '\b':
                res += "\\b";
                break;
            case '\f':
                res += "\\f";
                break;
            case '\n':
                res += "\\n";
                break;
            case '\r':
                res += "\\r";
                break;
            case '\t':
                res += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[7];
                    sprintf(buf, "\\u%04X", c);
                    res += buf;
                } else {
                    res += c;
                }
        }
    }
    return res + '\"';
}

std::string Json::serializeArray() const noexcept {
    std::string res = "[ ";
    for (std::size_t i = 0; i != _jsonValue->size(); ++i) {
        if (i > 0) {
            res += ", ";
        }
        res += this->operator[](i).serialize();
    }
    return res + " ]";
}

std::string Json::serializeObject() const noexcept {
    std::string res = "{ ";
    bool first = true;
    for (auto&& p : _jsonValue->toObject()) {
        if (first) {
            first = false;
        } else {
            res += ", ";
        }
        res += "\"" + p.first + "\"";
        res += ": ";
        res += p.second.serialize();
    }
    return res + " }";
}

std::string Json::serialize() const noexcept  {
    switch (_jsonValue->getType()) {
        case JsonType::Null:
            return "null";
        case JsonType::Bool:
            return _jsonValue->toBool() ? "true" : "false";
        case JsonType::Number:
            char buf[32];
            sprintf(buf, "%.17g", _jsonValue->toNumber());
            return std::string(buf);
        case JsonType::String:
            return serializeString();
        case JsonType::Array:
            return serializeArray();
        default:
            return serializeObject();
    }
}

JsonType Json::getType() const noexcept { return _jsonValue->getType(); }
bool Json::isNull() const noexcept { return getType() == JsonType::Null; }
bool Json::isBool() const noexcept { return getType() == JsonType::Bool; }
bool Json::isNumber() const noexcept { return getType() == JsonType::Number; }
bool Json::isString() const noexcept { return getType() == JsonType::String; }
bool Json::isArray() const noexcept { return getType() == JsonType::Array; }
bool Json::isObject() const noexcept { return getType() == JsonType::Object; }

bool Json::toBool() const { return _jsonValue->toBool(); }
double Json::toNumber() const { return _jsonValue->toNumber(); }
const std::string& Json::toString() const { return _jsonValue->toString(); }
const Json::array& Json::toArray() const { return _jsonValue->toArray(); }
const Json::object& Json::toObject() const { return _jsonValue->toObject(); }

size_t Json::size() const { return _jsonValue->size(); }

void Json::swap(Json& rhs) noexcept {
    using std::swap;
    swap(_jsonValue, rhs._jsonValue);
}

bool operator==(const Json& lhs, const Json& rhs) {
    if (lhs.getType() != rhs.getType()) {
        return false;
    }
    switch (lhs.getType()) {
        case JsonType::Null: {
            return true;
        }
        case JsonType::Bool: {
            return lhs.toBool() == rhs.toBool();
        }
        case JsonType::Number: {
            return lhs.toNumber() == rhs.toNumber();
        }
        case JsonType::String: {
            return lhs.toString() == rhs.toString();
        }
        case JsonType::Array: {
            return lhs.toArray() == rhs.toArray();
        }
        case JsonType::Object: {
            return lhs.toObject() == rhs.toObject();
        }
        default: { return false; }
    }
}
