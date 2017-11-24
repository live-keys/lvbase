/****************************************************************************
**
** Copyright (C) 2014-2017 Dinu SV.
** (contact: mail@dinusv.com)
** This file is part of Live CV Application.
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
****************************************************************************/

#ifndef LVVISUALLOG_H
#define LVVISUALLOG_H

#include <iostream>
#include <sstream>
#include <ostream>
#include <functional>

#include <QDate>
#include <QTextStream>

#include "live/mlnode.h"

namespace lv{

class VisualLogModel;

class LV_BASE_EXPORT VisualLog{

public:
    enum Output{
        Console = 1,
        Error = 2,
        File = 4,
        View = 8,
        Extensions = 16
    };

    class Configuration;
    class ConfigurationContainer;

    class LV_BASE_EXPORT SourceLocation{
    public:
        SourceLocation(const QString& file, int line, const QString& fileName);

        QString file;
        int     line;
        QString functionName;
    };

    class LV_BASE_EXPORT MessageInfo{

    public:
        enum Level{
            Fatal = 0,
            Error,
            Warning,
            Info,
            Debug,
            Verbose
        };

        friend class VisualLog;

    public:
        ~MessageInfo();

        static QString levelToString(VisualLog::MessageInfo::Level level);
        static VisualLog::MessageInfo::Level levelFromString(const QString& str);

        QString sourceFileName() const;
        int     sourceLineNumber() const;
        QString sourceFunctionName() const;
        const QDateTime& stamp() const;
        QString prefix(const VisualLog::Configuration* configuration) const;

    private:
        MessageInfo();
        MessageInfo(Level level);
        MessageInfo(const MessageInfo&);
        MessageInfo& operator = (const MessageInfo&);

        QString expand(const QString& pattern) const;

        Level              m_level;
        SourceLocation*    m_location;
        mutable QDateTime* m_stamp;
    };

    class LV_BASE_EXPORT Transport{
    public:
        virtual ~Transport(){}
        virtual void onMessage(const VisualLog::Configuration* configuration,
                               const VisualLog::MessageInfo& messageInfo,
                               const QString& message) = 0;
        virtual void onObject(const VisualLog::Configuration* configuration,
                              const VisualLog::MessageInfo& messageInfo,
                              const QString& type,
                              const MLNode& node) = 0;
    };

public:
    VisualLog();
    VisualLog(MessageInfo::Level level);
    VisualLog(const QString& configuration);
    VisualLog(const QString& configuration, MessageInfo::Level level);
    ~VisualLog();

    VisualLog& at(const QString& file, int line = 0, const QString& functionName = "");

    template<typename T> VisualLog& operator <<( const T& x );
    template<typename T> VisualLog& operator <<( std::ostream& (*f)(std::ostream&) );
    template<typename T> VisualLog& operator <<( std::ostream& (*f)(std::ios&) );
    template<typename T> VisualLog& operator <<( std::ostream (*f)(std::ios_base& ) );

    VisualLog& f();
    VisualLog& e();
    VisualLog& w();
    VisualLog& i();
    VisualLog& d();
    VisualLog& v();

    template<typename T> void object(MessageInfo::Level level, const T& value);

    template<typename T> void f(const T& value);
    template<typename T> void e(const T& value);
    template<typename T> void w(const T& value);
    template<typename T> void i(const T& value);
    template<typename T> void d(const T& value);
    template<typename T> void v(const T& value);


    bool canLog();
    void configure(const QString& configuration, const MLNode &options);
    void configure(VisualLog::Configuration* configuration, const MLNode& options);
    void addTransport(const QString& configuration, Transport* transport);
    void addTransport(VisualLog::Configuration* configuration, Transport* transport);
    void removeTransports(const QString& configuration);
    void removeTransports(VisualLog::Configuration* configuration);

    int totalConfigurations();

    void flushLine();
    void closeFile();

    void asView(const QString& viewPath, const QVariant& viewData);
    template<typename T> void asObject(const QString& type, const T& value);
    void asObject(const QString& type, const MLNode& value);


    static VisualLogModel* model();
    static void setViewTransport(VisualLogModel* model);

    static void flushConsole(const QString& data);

private:
    // disable copy
    VisualLog(const VisualLog&);
    VisualLog& operator = (const VisualLog&);

    void init();
    void flushFile(const QString &data);
    void flushHandler(const QString& data);
    QString prefix();
    bool canLogObjects(VisualLog::Configuration* configuration);

    static int removeOutputFlag(int flags, VisualLog::Output output);

    static ConfigurationContainer createDefaultConfigurations();
    static ConfigurationContainer m_registeredConfigurations;

    static VisualLogModel* m_model;

    static bool m_globalConfigured;

    int            m_output;
    MessageInfo    m_messageInfo;
    QString        m_buffer;
    Configuration* m_configuration;
    QTextStream    m_stream;
    bool           m_objectOutput;

};

template<typename T> void VisualLog::object(VisualLog::MessageInfo::Level level, const T &value){
    m_messageInfo.m_level = level;
    m_objectOutput = true;
    if ( canLog() )
        *this << value;
}

template<typename T> void VisualLog::asObject(const QString &type, const T &value){
    if ( canLog() && m_objectOutput && canLogObjects(m_configuration) ){
        MLNode mlvalue;
        ml::serialize(mlvalue, value);
        asObject(type, mlvalue);
    }
}

template<typename T> void VisualLog::f(const T &value){
    object(VisualLog::MessageInfo::Fatal, value);
}
template<typename T> void VisualLog::e(const T &value){
    object(VisualLog::MessageInfo::Error, value);
}
template<typename T> void VisualLog::w(const T &value){
    object(VisualLog::MessageInfo::Warning, value);
}
template<typename T> void VisualLog::i(const T &value){
    object(VisualLog::MessageInfo::Info, value);
}
template<typename T> void VisualLog::d(const T &value){
    object(VisualLog::MessageInfo::Debug, value);
}
template<typename T> void VisualLog::v(const T &value){
    object(VisualLog::MessageInfo::Verbose, value);
}

inline VisualLog &VisualLog::f(){
    m_messageInfo.m_level = VisualLog::MessageInfo::Fatal;
    return *this;
}

inline VisualLog &VisualLog::e(){
    m_messageInfo.m_level = VisualLog::MessageInfo::Error;
    return *this;
}

inline VisualLog &VisualLog::w(){
    m_messageInfo.m_level = VisualLog::MessageInfo::Warning;
    return *this;
}

inline VisualLog &VisualLog::i(){
    m_messageInfo.m_level = VisualLog::MessageInfo::Info;
    return *this;
}

inline VisualLog &VisualLog::d(){
    m_messageInfo.m_level = VisualLog::MessageInfo::Debug;
    return *this;
}

inline VisualLog &VisualLog::v(){
    m_messageInfo.m_level = VisualLog::MessageInfo::Verbose;
    return *this;
}

inline VisualLog &VisualLog::at(const QString &file, int line, const QString &functionName){
    m_messageInfo.m_location = new VisualLog::SourceLocation(file, line, functionName);
    return *this;
}

inline QString VisualLog::MessageInfo::sourceFileName() const{
    return m_location ? m_location->file : "";
}

inline int VisualLog::MessageInfo::sourceLineNumber() const{
    return m_location ? m_location->line : 0;
}

inline QString VisualLog::MessageInfo::sourceFunctionName() const{
    return m_location ? m_location->functionName : "";
}

inline const QDateTime &VisualLog::MessageInfo::stamp() const{
    if ( !m_stamp )
        m_stamp = new QDateTime(QDateTime::currentDateTime());
    return *m_stamp;
}

template<typename T> VisualLog& VisualLog::operator<< (const T& x){
    if ( !canLog() )
        return *this;

    m_stream << x;

    return *this;
}

template<typename T> VisualLog& VisualLog::operator<< (std::ostream &(*f)(std::ostream &)){
    if ( !canLog() )
        return *this;

    std::stringstream ss;
    f(ss);
    m_stream << ss.str().c_str();

    return *this;
}

template<typename T> VisualLog& VisualLog::operator<< ( std::ostream& (*f)(std::ios&) ){
    if ( !canLog() )
        return *this;

    std::stringstream ss;
    f(ss);
    m_stream << ss.str().c_str();

    return *this;
}

template<typename T> VisualLog& VisualLog::operator<< ( std::ostream (*f)(std::ios_base& ) ){
    if ( !canLog() )
        return *this;

    std::stringstream ss;
    f(ss);
    m_stream << ss.str().c_str();

    return *this;
}

}// namespace

#ifndef VLOG_NO_MACROS

#ifndef vlog
#define vlog(...) lv::VisualLog(__VA_ARGS__).at(__FILE__, __LINE__, __FUNCTION__)
#endif // vlog

#endif // VLOG_NO_MACROS

#endif // LVVISUALLOG_H


