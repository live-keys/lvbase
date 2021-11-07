#ifndef LVUTF8_H
#define LVUTF8_H

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <string>
#include <sstream>
#include <memory>
#include <vector>

#include "lvbaseglobal.h"

namespace lv{

class LV_BASE_EXPORT Utf8{

public:
    Utf8();
    Utf8(const std::string& str);
    Utf8(const char* str);
    Utf8(const char* str, size_t size);
    ~Utf8();

    char byteAt(size_t pos) const;
    const std::string& data() const;

    size_t find(char ch, size_t offset = 0) const;
    size_t find(const char* str, size_t offset = 0) const;
    size_t find(const std::string& str, size_t offset = 0) const;
    size_t find(const Utf8& str, size_t offset = 0) const;

    size_t findLast(char ch, size_t offset = std::string::npos) const;
    size_t findLast(const char* str, size_t offset = std::string::npos) const;
    size_t findLast(const std::string& str, size_t offset = std::string::npos) const;
    size_t findLast(const Utf8& str, size_t offset = std::string::npos) const;

    Utf8 substr(size_t start, size_t length) const;

    char operator[](size_t index) const;

    Utf8 operator+(const Utf8& other) const;
    Utf8 operator+(const std::string& str) const;
    Utf8 operator+(const char* str) const;
    Utf8 operator+(char c) const;

    int compare(const Utf8& other) const;
    int compare(const std::string& other) const;
    int compare(const char* other) const;

    bool operator==(const Utf8& other) const;
    bool operator==(const char* str) const;
    bool operator!=(const Utf8& other) const;
    bool operator!=(const char* str) const;

    bool operator<(const Utf8& other) const;
    bool operator<(const char* str) const;
    bool operator<=(const Utf8& other) const;
    bool operator<=(const char* str) const;
    bool operator>(const Utf8& other) const;
    bool operator>(const char* str) const;
    bool operator>=(const Utf8& other) const;
    bool operator>=(const char* str) const;

    bool startsWith(const Utf8& other) const;
    bool startsWith(const std::string& str) const;
    bool startsWith(const char* str) const;
    bool startsWith(char ch) const;

    bool endsWith(const Utf8& other) const;
    bool endsWith(const std::string& str) const;
    bool endsWith(const char* str) const;
    bool endsWith(char ch) const;

    Utf8 toLower() const;
    Utf8 toUpper() const;

    std::vector<Utf8> split(const char* sep);

    template<typename ...Args>
    Utf8 format(Args... args) const;

    size_t size() const;
    size_t length() const;

    size_t utfLength() const;

    static bool isSpace(uint32_t c);

    static void trimLeft(std::string &str);
    static void trimRight(std::string &str);
    static void trim(std::string& str);

    Utf8 trimLeft() const;
    Utf8 trimRight() const;
    Utf8 trim() const;

private:
    Utf8(std::string* strPtr);

    void formatImpl(std::string::const_iterator sit, std::stringstream&) const;

    template<typename T, typename... Args>
    void formatImpl(std::string::const_iterator sit, std::stringstream&, const T& value, Args... args) const;
    void throwFormatError(const std::string& message) const;

    std::shared_ptr<std::string> m_data;
};

template<typename ...Args>
Utf8 Utf8::format(Args... args) const{
    std::stringstream result;
    formatImpl(m_data->cbegin(), result, args...);
    return Utf8(result.str());
}

inline void Utf8::formatImpl(std::string::const_iterator sit, std::stringstream &stream) const{
    while (sit != m_data->cend() ) {
        char ch = *sit;
        if (ch == '%') {
            if (*(sit + 1) == '%') {
                ++sit;
            } else {
                throwFormatError("Missing arguments in Utf8.format");
            }
        }
        stream << ch;
        ++sit;
    }
}

template<typename T, typename... Args>
void Utf8::formatImpl(std::string::const_iterator sit, std::stringstream &stream, const T &value, Args... args) const{
    while (sit != m_data->cend() ) {
        char ch = *sit;
        if (*sit == '%') {
            if (*(sit + 1) == '%') {
                ++sit;
            } else {
                stream << value;
                formatImpl(sit + 1, stream, args...);
                return;
            }
        }
        stream << ch;
        ++sit;
    }
    throwFormatError("Extra arguments provided to Utf8.format.");
}

inline Utf8 Utf8::operator+(const Utf8 &other) const{
    return Utf8(*m_data + *other.m_data);
}

inline Utf8 Utf8::operator+(const std::string &other) const{
    return Utf8(*m_data + other);
}

inline Utf8 Utf8::operator+(const char *str) const{
    return Utf8(*m_data + str);
}

inline Utf8 Utf8::operator+(char c) const{
    return Utf8(*m_data + c);
}

inline bool Utf8::operator ==(const Utf8 &other) const{
    return compare(other) == 0;
}

inline bool Utf8::operator ==(const char *str) const{
    return compare(str) == 0;
}

inline bool Utf8::operator !=(const Utf8 &other) const{
    return compare(other) != 0;
}

inline bool Utf8::operator !=(const char *str) const{
    return compare(str) != 0;
}

inline bool Utf8::operator<(const Utf8 &other) const{
    return compare(other) < 0;
}

inline bool Utf8::operator<(const char *str) const{
    return compare(str) < 0;
}

inline bool Utf8::operator<=(const Utf8 &other) const{
    return compare(other) <= 0;
}

inline bool Utf8::operator<=(const char *str) const{
    return compare(str) <= 0;
}

inline bool Utf8::operator>(const Utf8 &other) const{
    return compare(other) > 0;
}

inline bool Utf8::operator>(const char *str) const{
    return compare(str) > 0;
}

inline bool Utf8::operator>=(const Utf8 &other) const{
    return compare(other) >= 0;
}

inline bool Utf8::operator>=(const char *str) const{
    return compare(str) >= 0;
}

}// namespace


#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#pragma warning(pop)
#endif

#endif // UTF8_H
