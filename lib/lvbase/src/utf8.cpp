#include "utf8.h"
#include "utf8proc.h"
#include "live/exception.h"

#include <locale>
#include <functional>
#include <algorithm>
#include <cctype>

namespace lv{

/**
 * \class lv::Utf8
 * \brief Encapsulates an Utf8 string
 * \private
 */

Utf8::Utf8()
    : m_data(new std::string)
{
}

Utf8::Utf8(const std::string &str)
    : m_data(new std::string(str))
{
}

Utf8::Utf8(const char *str)
    : m_data(new std::string(str))
{
}

Utf8::Utf8(const char *str, size_t size)
    : m_data(new std::string(str, size))
{
}

Utf8::~Utf8(){
}

char Utf8::byteAt(size_t pos) const{
    return (*m_data)[pos];
}

const std::string &Utf8::data() const{
    return *m_data;
}

size_t Utf8::find(char ch, size_t offset) const{
    return m_data->find(ch, offset);
}

size_t Utf8::find(const char *ch, size_t offset) const{
    return m_data->find(ch, offset);
}

size_t Utf8::find(const std::string &str, size_t offset) const{
    return m_data->find(str, offset);
}

size_t Utf8::find(const Utf8 &str, size_t offset) const{
    return m_data->find(str.data(), offset);
}

size_t Utf8::findLast(char ch, size_t offset) const{
    return m_data->rfind(ch, offset);
}

size_t Utf8::findLast(const char *str, size_t offset) const{
    return m_data->rfind(str, offset);
}

size_t Utf8::findLast(const std::string &str, size_t offset) const{
    return m_data->rfind(str, offset);
}

size_t Utf8::findLast(const Utf8 &str, size_t offset) const{
    return m_data->rfind(str.data(), offset);
}

Utf8 Utf8::substr(size_t start, size_t length) const{
    return Utf8(m_data->substr(start, length));
}

char Utf8::operator[](size_t index) const{
    return m_data->at(index);
}

int Utf8::compare(const Utf8 &other) const{
    return m_data->compare(other.data());
}

int Utf8::compare(const std::string &other) const{
    return m_data->compare(other);
}

int Utf8::compare(const char *other) const{
    return m_data->compare(other);
}

bool Utf8::startsWith(const Utf8 &other) const{
    return m_data->rfind(other.data(), 0) == 0;
}

bool Utf8::startsWith(const std::string &str) const{
    return m_data->rfind(str, 0) == 0;
}

bool Utf8::startsWith(const char *str) const{
    return m_data->rfind(str, 0) == 0;
}

bool Utf8::startsWith(char ch) const{
    return m_data->rfind(ch, 0) == 0;
}

bool Utf8::endsWith(const Utf8 &other) const{
    if ( length() > other.length() ){
        return (0 == m_data->compare(m_data->length() - other.length(), other.length(), other.data()));
    }
    return false;
}

bool Utf8::endsWith(const std::string &str) const{
    if ( length() > str.length() ){
        return (0 == m_data->compare(m_data->length() - str.length(), str.length(), str));
    }
    return false;
}

bool Utf8::endsWith(const char *str) const{
    return endsWith(std::string(str));
}

bool Utf8::endsWith(char ch) const{
    if ( length() > 0 )
        return m_data->at(m_data->length() - 1) == ch;
    return false;
}

Utf8 Utf8::toLower() const{
    utf8proc_int32_t nextcodepoint;

    const char* strdata = m_data->c_str();
    size_t strlength = length();

    utf8proc_uint8_t* lowerCharDst = new utf8proc_uint8_t[4];

    std::string* lowerStr = new std::string;
    lowerStr->reserve(strlength);

    while ( strlength > 0 ){
        utf8proc_ssize_t charsize = utf8proc_iterate(reinterpret_cast<const utf8proc_uint8_t*>(strdata), -1, &nextcodepoint);
        if ( charsize <= 0 ){
            delete[] lowerCharDst;
            return "";
        }

        strlength -= charsize;
        strdata   += charsize;

        utf8proc_int32_t lowercodepoint = utf8proc_tolower(nextcodepoint);
        utf8proc_ssize_t encodecharsize = utf8proc_encode_char(lowercodepoint, lowerCharDst);
        if ( encodecharsize == 0 ){
            encodecharsize = utf8proc_encode_char(nextcodepoint, lowerCharDst);
            if ( encodecharsize == 0 ){
                delete[] lowerCharDst;
                return "";
            }
        }
        for ( int i = 0; i < encodecharsize; ++i )
            lowerStr += static_cast<char>(lowerCharDst[i]);
    }

    delete[] lowerCharDst;
    return Utf8(lowerStr);
}

Utf8 Utf8::toUpper() const{
    utf8proc_int32_t nextcodepoint;

    const char* strdata = m_data->c_str();
    size_t strlength = length();

    utf8proc_uint8_t* upperCharDst = new utf8proc_uint8_t[4];

    std::string* upperStr = new std::string;
    upperStr->reserve(strlength);

    while ( strlength > 0 ){
        utf8proc_ssize_t charsize = utf8proc_iterate(reinterpret_cast<const utf8proc_uint8_t*>(strdata), -1, &nextcodepoint);
        if ( charsize <= 0 ){
            delete[] upperCharDst;
            return "";
        }

        strlength -= charsize;
        strdata   += charsize;

        utf8proc_int32_t uppercodepoint = utf8proc_toupper(nextcodepoint);
        utf8proc_ssize_t encodecharsize = utf8proc_encode_char(uppercodepoint, upperCharDst);
        if ( encodecharsize == 0 ){
            encodecharsize = utf8proc_encode_char(nextcodepoint, upperCharDst);
            if ( encodecharsize == 0 ){
                delete[] upperCharDst;
                return "";
            }
        }
        for ( int i = 0; i < encodecharsize; ++i )
            upperStr += static_cast<char>(upperCharDst[i]);

    }

    delete[] upperCharDst;
    return Utf8(upperStr);
}

size_t Utf8::size() const{
    return m_data->size();
}

size_t Utf8::length() const{
    return size();
}

size_t Utf8::utfLength() const{
    utf8proc_int32_t nextcodepoint;

    const char* strdata = m_data->c_str();
    size_t strlength = m_data->length();
    size_t utflength = 0;

    while ( strlength > 0 ){
        utf8proc_ssize_t size =
            utf8proc_iterate(reinterpret_cast<const utf8proc_uint8_t*>(strdata), -1, &nextcodepoint);
        if ( size <= 0 )
            return 0;

        utflength += size;
        strlength -= size;
        strdata   += size;

    }
    return utflength;
}

bool Utf8::isSpace(uint32_t c){
    return ( c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f' );
}

void Utf8::trimLeft(std::string &s){
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
            std::not1(std::ptr_fun<int, int>(std::isspace))));
}

void Utf8::trimRight(std::string &s){
    s.erase(std::find_if(s.rbegin(), s.rend(),
            std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

void Utf8::trim(std::string &s){
    trimRight(s);
    trimLeft(s);
}

Utf8 Utf8::trimLeft() const{
    Utf8 result(*m_data);
    trimLeft(*result.m_data);
    return result;
}

Utf8 Utf8::trimRight() const{
    Utf8 result(*m_data);
    trimRight(*result.m_data);
    return result;
}

Utf8 Utf8::trim() const{
    Utf8 result(*m_data);
    trim(*result.m_data);
    return result;
}

Utf8::Utf8(std::string *strPtr)
    : m_data(strPtr)
{
}

void Utf8::throwFormatError(const std::string &message) const{
    THROW_EXCEPTION(lv::Exception, message, lv::Exception::toCode("Format"));
}

}// namespace
