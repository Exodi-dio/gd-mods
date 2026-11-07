#pragma once

#include <Geode/Geode.hpp>
#include <string>
#include <unordered_map>
#include <variant>

namespace rift {

class Value {
public:
    using ValueType = std::variant<int64_t, double, std::string, bool>;

    static Value integer(int64_t v) { Value val; val.m_val = v; return val; }
    static Value floating(double v) { Value val; val.m_val = v; return val; }
    static Value string(const std::string& v) { Value val; val.m_val = v; return val; }
    static Value string(const char* v) { Value val; val.m_val = std::string(v); return val; }
    static Value boolean(bool v) { Value val; val.m_val = v; return val; }

private:
    ValueType m_val;
    friend class Script;
};

class Script {
public:
    explicit Script(std::string format) : m_format(std::move(format)) {}
    void setVariable(const std::string& name, const Value& value);
    std::string run();
    ~Script() = default;

private:
    std::string m_format;
    std::unordered_map<std::string, Value> m_variables;
};

inline geode::Result<Script*> compile(const std::string& format) {
    return geode::Ok(new Script(format));
}

} // namespace rift
