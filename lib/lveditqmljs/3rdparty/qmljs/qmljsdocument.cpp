/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, The Qt Company gives you certain additional
** rights.  These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "qmljsdocument.h"
#include "qmljsbind.h"
#include "qmljsconstants.h"
#include "qmljsimportdependencies.h"
#include <qmljs/parser/qqmljslexer_p.h>
#include <qmljs/parser/qqmljsparser_p.h>

#include <utils/qtcassert.h>

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

#include <algorithm>

using namespace QQmlJS;
using namespace QQmlJS::AST;

/*!
    \class QmlJS::Document
    \brief The Document class creates a QML or JavaScript document.
    \sa Snapshot

    Documents are usually created by the ModelManagerInterface
    and stored in a Snapshot. They allow access to data such as
    the file path, source code, abstract syntax tree and the Bind
    instance for the document.

    To make sure unused and outdated documents are removed correctly, Document
    instances are usually accessed through a shared pointer, see Document::Ptr.

    Documents in a Snapshot are immutable: They, or anything reachable through them,
    must not be changed. This allows Documents to be shared freely among threads
    without extra synchronization.
*/

/*!
    \class QmlJS::LibraryInfo
    \brief The LibraryInfo class creates a QML library.
    \sa Snapshot

    A LibraryInfo is created when the ModelManagerInterface finds
    a QML library and parses the qmldir file. The instance holds information about
    which Components the library provides and which plugins to load.

    The ModelManager will try to extract detailed information about the types
    defined in the plugins this library loads. Once it is done, the data will
    be available through the metaObjects() function.
*/

/*!
    \class QmlJS::Snapshot
    \brief The Snapshot class holds and offers access to a set of
    Document::Ptr and LibraryInfo instances.
    \sa Document LibraryInfo

    Usually Snapshots are copies of the snapshot maintained and updated by the
    ModelManagerInterface that updates its instance as parsing
    threads finish and new information becomes available.
*/

Document::Document(const QString &fileName, Dialect language)
    : _engine(0)
    , _ast(0)
    , _bind(0)
    , _fileName(QDir::cleanPath(fileName))
    , _editorRevision(0)
    , _language(language)
    , _parsedCorrectly(false)
{
    QFileInfo fileInfo(fileName);
    _path = fileName == "" ? "" : QDir::cleanPath(fileInfo.absolutePath());

    if (language.isQmlLikeLanguage()) {
        _componentName = fileInfo.baseName();

        if (! _componentName.isEmpty()) {
            // ### TODO: check the component name.

            if (! _componentName.at(0).isUpper())
                _componentName.clear();
        }
    }
}

Document::~Document()
{
    if (_bind)
        delete _bind;

    if (_engine)
        delete _engine;
}

Document::MutablePtr Document::create(const QString &fileName, Dialect language)
{
    Document::MutablePtr doc(new Document(fileName, language));
    doc->_ptr = doc;
    return doc;
}

Document::Ptr Document::ptr() const
{
    return _ptr.toStrongRef();
}

bool Document::isQmlDocument() const
{
    return _language.isQmlLikeLanguage();
}

Dialect Document::language() const
{
    return _language;
}

void Document::setLanguage(Dialect l)
{
    _language = l;
}

QString Document::importId() const
{
    return _fileName;
}

QByteArray Document::fingerprint() const
{
    return _fingerprint;
}

AST::UiProgram *Document::qmlProgram() const
{
    return cast<UiProgram *>(_ast);
}

AST::Program *Document::jsProgram() const
{
    return cast<Program *>(_ast);
}

AST::ExpressionNode *Document::expression() const
{
    if (_ast)
        return _ast->expressionCast();

    return 0;
}

AST::Node *Document::ast() const
{
    return _ast;
}

const QQmlJS::Engine *Document::engine() const
{
    return _engine;
}

QList<DiagnosticMessage> Document::diagnosticMessages() const
{
    return _diagnosticMessages;
}

QString Document::source() const
{
    return _source;
}

void Document::setSource(const QString &source)
{
    _source = source;
    QCryptographicHash sha(QCryptographicHash::Sha1);
    sha.addData(source.toUtf8());
    _fingerprint = sha.result();
}

int Document::editorRevision() const
{
    return _editorRevision;
}

void Document::setEditorRevision(int revision)
{
    _editorRevision = revision;
}

QString Document::fileName() const
{
    return _fileName;

}

QString Document::path() const
{
    return _path;
}

QString Document::componentName() const
{
    return _componentName;
}

namespace {
class CollectDirectives : public Directives
{

    void addLocation(int line, int column) {
        const SourceLocation loc = SourceLocation(
                    0,  // placeholder
                    0,  // placeholder
                    static_cast<quint32>(line),
                    static_cast<quint32>(column));
        _locations += loc;
    }

    QList<SourceLocation> _locations;



    QString documentPath;
public:
    CollectDirectives(const QString &documentPath)
        : documentPath(documentPath)
        , isLibrary(false)

    {}

    virtual void pragmaLibrary() { isLibrary = true; }
    virtual void importFile(const QString &jsfile, const QString &module, int, int)
    {
        imports += ImportInfo::pathImport(
                    documentPath, jsfile, LanguageUtils::ComponentVersion(), module);
    }

    virtual void importModule(const QString &uri, const QString &version, const QString &module, int, int)
    {
        imports += ImportInfo::moduleImport(uri, LanguageUtils::ComponentVersion(version), module);
    }

    virtual QList<SourceLocation> locations() { return _locations; }

    bool isLibrary;
    QList<ImportInfo> imports;
};

} // anonymous namespace

bool Document::parse_helper(int startToken)
{
    Q_ASSERT(! _engine);
    Q_ASSERT(! _ast);
    Q_ASSERT(! _bind);

    _engine = new Engine();

    Lexer lexer(_engine);
    Parser parser(_engine);

    QString source = _source;
    lexer.setCode(source, /*line = */ 1, /*qmlMode = */_language.isQmlLikeLanguage());

    CollectDirectives collectDirectives(path());
    _engine->setDirectives(&collectDirectives);

    switch (startToken) {
    case QQmlJSGrammar::T_FEED_UI_PROGRAM:
        _parsedCorrectly = parser.parse();
        break;
    case QQmlJSGrammar::T_FEED_JS_SCRIPT:
    case QQmlJSGrammar::T_FEED_JS_MODULE: {
        _parsedCorrectly = parser.parseProgram();
        const QList<SourceLocation> locations = collectDirectives.locations();
        for (const auto &d : locations) {
           _jsdirectives << d;
        }
    }
    case QQmlJSGrammar::T_FEED_JS_EXPRESSION:
        _parsedCorrectly = parser.parseExpression();
        break;
    default:
        Q_ASSERT(0);
    }

    _ast = parser.rootNode();
    _diagnosticMessages = parser.diagnosticMessages();

    _bind = new Bind(this, &_diagnosticMessages, collectDirectives.isLibrary, collectDirectives.imports);

    return _parsedCorrectly;
}

bool Document::parse()
{
    if (isQmlDocument())
        return parseQml();

    return parseJavaScript();
}

bool Document::parseQml()
{
    return parse_helper(QQmlJSGrammar::T_FEED_UI_PROGRAM);
}

bool Document::parseJavaScript()
{
    return parse_helper(QQmlJSGrammar::T_FEED_JS_SCRIPT);
}

bool Document::parseExpression()
{
    return parse_helper(QQmlJSGrammar::T_FEED_JS_EXPRESSION);
}

Bind *Document::bind() const
{
    return _bind;
}

LibraryInfo::LibraryInfo(Status status)
    : _status(status)
    , _dumpStatus(NoTypeInfo)
{
    updateFingerprint();
}

LibraryInfo::LibraryInfo(const QmlDirParser &parser, const QByteArray &fingerprint)
    : _status(Found)
    , _components(parser.components().values())
    , _plugins(parser.plugins())
    , _typeinfos(parser.typeInfos())
    , _fingerprint(fingerprint)
    , _dumpStatus(NoTypeInfo)
{
    if (_fingerprint.isEmpty())
        updateFingerprint();
}

LibraryInfo::~LibraryInfo()
{
}

QByteArray LibraryInfo::calculateFingerprint() const
{
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(reinterpret_cast<const char *>(&_status), sizeof(_status));
    int len = _components.size();
    hash.addData(reinterpret_cast<const char *>(&len), sizeof(len));
    foreach (const QmlDirParser::Component &component, _components) {
        len = component.fileName.size();
        hash.addData(reinterpret_cast<const char *>(&len), sizeof(len));
        hash.addData(reinterpret_cast<const char *>(component.fileName.constData()), len * sizeof(QChar));
        hash.addData(reinterpret_cast<const char *>(&component.majorVersion), sizeof(component.majorVersion));
        hash.addData(reinterpret_cast<const char *>(&component.minorVersion), sizeof(component.minorVersion));
        len = component.typeName.size();
        hash.addData(reinterpret_cast<const char *>(&len), sizeof(len));
        hash.addData(reinterpret_cast<const char *>(component.typeName.constData()), component.typeName.size() * sizeof(QChar));
        int flags = (component.singleton ?  (1 << 0) : 0) + (component.internal ? (1 << 1) : 0);
        hash.addData(reinterpret_cast<const char *>(&flags), sizeof(flags));
    }
    len = _plugins.size();
    hash.addData(reinterpret_cast<const char *>(&len), sizeof(len));
    foreach (const QmlDirParser::Plugin &plugin, _plugins) {
        len = plugin.path.size();
        hash.addData(reinterpret_cast<const char *>(&len), sizeof(len));
        hash.addData(reinterpret_cast<const char *>(plugin.path.constData()), len * sizeof(QChar));
        len = plugin.name.size();
        hash.addData(reinterpret_cast<const char *>(&len), sizeof(len));
        hash.addData(reinterpret_cast<const char *>(plugin.name.constData()), len * sizeof(QChar));
    }
    len = _typeinfos.size();
    hash.addData(reinterpret_cast<const char *>(&len), sizeof(len));
    foreach (const QmlDirParser::TypeInfo &typeinfo, _typeinfos) {
        len = typeinfo.fileName.size();
        hash.addData(reinterpret_cast<const char *>(&len), sizeof(len));
        hash.addData(reinterpret_cast<const char *>(typeinfo.fileName.constData()), len * sizeof(QChar));
    }
    len = _metaObjects.size();
    hash.addData(reinterpret_cast<const char *>(&len), sizeof(len));
    QList<QByteArray> metaFingerprints;
    foreach (const LanguageUtils::FakeMetaObject::ConstPtr &metaObject, _metaObjects)
        metaFingerprints.append(metaObject->fingerprint());
    std::sort(metaFingerprints.begin(), metaFingerprints.end());
    foreach (const QByteArray &fp, metaFingerprints)
        hash.addData(fp);
    hash.addData(reinterpret_cast<const char *>(&_dumpStatus), sizeof(_dumpStatus));
    len = _dumpError.size(); // localization dependent (avoid?)
    hash.addData(reinterpret_cast<const char *>(&len), sizeof(len));
    hash.addData(reinterpret_cast<const char *>(_dumpError.constData()), len * sizeof(QChar));

    len = _moduleApis.size();
    hash.addData(reinterpret_cast<const char *>(&len), sizeof(len));
    foreach (const ModuleApiInfo &moduleInfo, _moduleApis)
        moduleInfo.addToHash(hash); // make it order independent?

    QByteArray res(hash.result());
    res.append('L');
    return res;
}

void LibraryInfo::updateFingerprint()
{
    _fingerprint = calculateFingerprint();
}

Snapshot::Snapshot()
{
}

Snapshot::~Snapshot()
{
}

Snapshot::Snapshot(const Snapshot &o)
    : _documents(o._documents),
      _documentsByPath(o._documentsByPath),
      _libraries(o._libraries),
      _dependencies(o._dependencies)
{
}

void Snapshot::insert(const Document::Ptr &document, bool allowInvalid)
{
    if (document && (allowInvalid || document->qmlProgram() || document->jsProgram())) {
        const QString fileName = document->fileName();
        const QString path = document->path();
        remove(fileName);
        _documentsByPath[path].append(document);
        _documents.insert(fileName, document);
        CoreImport cImport;
        cImport.importId = document->importId();
        cImport.language = document->language();
        cImport.possibleExports << Export(ImportKey(ImportType::File, fileName),
                                          QString(), true, QFileInfo(fileName).baseName());
        cImport.fingerprint = document->fingerprint();
        _dependencies.addCoreImport(cImport);
    }
}

void Snapshot::remove(const QString &fileName)
{
    Document::Ptr doc = _documents.value(fileName);
    if (!doc.isNull()) {
        const QString &path = doc->path();

        QList<Document::Ptr> docs = _documentsByPath.value(path);
        docs.removeAll(doc);
        _documentsByPath[path] = docs;

        _documents.remove(fileName);
    }
}

const QQmlJS::ImportDependencies *Snapshot::importDependencies() const
{
    return &_dependencies;
}

QQmlJS::ImportDependencies *Snapshot::importDependencies()
{
    return &_dependencies;
}

Document::MutablePtr Snapshot::documentFromSource(
        const QString &code, const QString &fileName,
        Dialect language) const
{
    Document::MutablePtr newDoc = Document::create(fileName, language);

    if (Document::Ptr thisDocument = document(fileName))
        newDoc->_editorRevision = thisDocument->_editorRevision;

    newDoc->setSource(code);
    return newDoc;
}

Document::Ptr Snapshot::document(const QString &fileName) const
{
    return _documents.value(QDir::cleanPath(fileName));
}

QList<Document::Ptr> Snapshot::documentsInDirectory(const QString &path) const
{
    return _documentsByPath.value(QDir::cleanPath(path));
}

LibraryInfo Snapshot::libraryInfo(const QString &path) const
{
    return _libraries.value(QDir::cleanPath(path));
}


void ModuleApiInfo::addToHash(QCryptographicHash &hash) const
{
    int len = uri.length();
    hash.addData(reinterpret_cast<const char *>(&len), sizeof(len));
    hash.addData(reinterpret_cast<const char *>(uri.constData()), len * sizeof(QChar));
    version.addToHash(hash);
    len = cppName.length();
    hash.addData(reinterpret_cast<const char *>(&len), sizeof(len));
    hash.addData(reinterpret_cast<const char *>(cppName.constData()), len * sizeof(QChar));
}
