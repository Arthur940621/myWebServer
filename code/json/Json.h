#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

enum class JsonType {
    Null,
    Bool,
    Number,
    String,
    Array,
    Object
};

class JsonValue;

class Json final {

public:
    using array = std::vector<Json>;
    using object = std::unordered_map<std::string, Json>;

    Json() : Json(nullptr) { };
    Json(std::nullptr_t);
    Json(bool val);
    Json(int val) : Json(1.0 * val) { }
    Json(double val);
    Json(const char* cstr) : Json(std::string(cstr)) { }
    Json(const std::string& val);
    Json(const array& val);
    Json(const object& val);
    Json(std::string&& val);
    Json(array&& val);
    Json(object&& val);

    ~Json();

    Json(const Json&);
    Json& operator=(Json);
    Json(Json&&) noexcept;

    static Json parse(const std::string& content, std::string& errMsg) noexcept;
    std::string serialize() const noexcept;

    JsonType getType() const noexcept;
    bool isNull() const noexcept;
    bool isBool() const noexcept;
    bool isNumber() const noexcept;
    bool isString() const noexcept;
    bool isArray() const noexcept;
    bool isObject() const noexcept;

    bool toBool() const;
    double toNumber() const;
    const std::string& toString() const;
    const array& toArray() const;
    const object& toObject() const;

    std::size_t size() const;
    Json& operator[](std::size_t);
    const Json& operator[](std::size_t) const;
    Json& operator[](const std::string&);
    const Json& operator[](const std::string&) const;

private:
    void swap(Json& rhs) noexcept;
    std::string serializeString() const noexcept;
    std::string serializeArray() const noexcept;
    std::string serializeObject() const noexcept;
    std::unique_ptr<JsonValue> _jsonValue;
};

inline std::ostream& operator<<(std::ostream& os, const Json& json) {
    return os << json.serialize();
}
bool operator==(const Json&, const Json&);
inline bool operator!=(const Json& lhs, const Json& rhs) {
    return !(lhs == rhs);
}