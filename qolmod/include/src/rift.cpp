#include "rift.hpp"

void rift::Script::setVariable(const std::string& name, const Value& value) {
    m_variables[name] = value;
}

std::string rift::Script::run() {
    std::string result;
    result.reserve(m_format.size());

    for (size_t i = 0; i < m_format.size(); ++i) {
        if (m_format[i] == '{' && i + 1 < m_format.size()) {
            auto end = m_format.find('}', i + 1);
            if (end == std::string::npos) {
                result += m_format[i];
                continue;
            }
            auto name = m_format.substr(i + 1, end - i - 1);
            auto it = m_variables.find(name);
            if (it != m_variables.end()) {
                std::string replacement;
                std::visit([&replacement](auto&& arg) {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, int64_t>) {
                        replacement = std::to_string(arg);
                    } else if constexpr (std::is_same_v<T, double>) {
                        char buf[64];
                        std::snprintf(buf, sizeof(buf), "%.2f", arg);
                        replacement = buf;
                    } else if constexpr (std::is_same_v<T, std::string>) {
                        replacement = arg;
                    } else if constexpr (std::is_same_v<T, bool>) {
                        replacement = arg ? "true" : "false";
                    }
                }, it->second.m_val);
                result += replacement;
            }
            i = end;
        } else {
            result += m_format[i];
        }
    }
    return result;
}
