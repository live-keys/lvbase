/****************************************************************************
**
** Copyright (C) 2014-2018 Dinu SV.
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

#include "live/documentqmlinfo.h"
#include "live/documentqmlvalueobjects.h"
#include "live/qmldeclaration.h"
#include "live/projectdocument.h"
#include "live/projectfile.h"
#include "bindingchannel.h"

#include "qmljs/qmljsdocument.h"
#include "qmljs/qmljsbind.h"
#include "qmlidvisitor_p.h"
#include "documentqmlranges_p.h"

#include <QQmlProperty>
#include <QQmlListReference>

namespace lv{

namespace{

    /// \private
    BindingPath::Node* findDeclarationPathImpl(
        DocumentQmlValueObjects::RangeObject *object,
        QmlDeclaration::Ptr declaration)
    {
        int position = declaration->position();

        for ( int i = 0; i < object->properties.size(); ++i ){

            // found property start position
            if ( object->properties[i]->begin == position ){

                if ( declaration->identifierChain().isEmpty() )
                    return nullptr;

                // iterate property chain (eg. border.size => (border, size))
                BindingPath::Node* currentParent = nullptr;

                for ( int j = 0; j < declaration->identifierChain().size(); ++j ){
                    BindingPath::PropertyNode* n = new BindingPath::PropertyNode;
                    n->propertyName = declaration->identifierChain()[j];
                    n->objectName = object->properties[i]->object();
                    n->parent = currentParent;
                    if ( currentParent )
                        currentParent->child = n;
                    currentParent = n;
                }

                // found property
                declaration->setValuePositionOffset(
                    object->properties[i]->valueBegin -
                    object->properties[i]->begin -
                    declaration->identifierLength()
                );

                declaration->setValueLength(
                    object->properties[i]->end - object->properties[i]->valueBegin
                );

                BindingPath::Node* n = currentParent;
                while ( n->parent != nullptr )
                    n = n->parent;
                return n;

            } else if ( object->properties[i]->child &&
                        object->properties[i]->begin < position &&
                        object->properties[i]->end > position )
            {
                BindingPath::Node* currentParent = nullptr;

                QStringList propertyNames = object->properties[i]->name();

                for ( int j = 0; j < propertyNames.size(); ++j ){
                    BindingPath::PropertyNode* n = new BindingPath::PropertyNode;
                    n->propertyName = propertyNames[j];
                    n->objectName = object->properties[i]->object();
                    n->parent = currentParent;
                    if ( currentParent )
                        currentParent->child = n;
                    currentParent = n;
                }

                BindingPath::Node* n = findDeclarationPathImpl(
                    object->properties[i]->child, declaration
                );
                if ( !n ){
                    delete currentParent;
                    return nullptr;
                }

                n->parent = currentParent;
                currentParent->child = n;

                while ( n->parent != nullptr )
                    n = n->parent;
                return n;
            }
        } // properties end

        for ( int i = 0; i < object->children.size(); ++i ){

            if ( position > object->children[i]->begin &&
                 position < object->children[i]->end )
            {
                BindingPath::IndexNode* indexNode = new BindingPath::IndexNode;
                indexNode->index = i;

                BindingPath::Node* n = findDeclarationPathImpl(
                    object->children[i], declaration
                );

                if ( !n ){
                    delete indexNode;
                    return nullptr;
                }

                indexNode->child = n;
                n->parent = indexNode;

                return indexNode;

            } else if ( position == object->children[i]->begin ){ // found object

                // return the top property, and the index of the object
                declaration->setValuePositionOffset(0);
                declaration->setValueLength(object->children[i]->end - object->children[i]->begin);

                BindingPath::IndexNode* indexNode = new BindingPath::IndexNode;
                indexNode->index = i;

                return indexNode;
            }
        }

        return nullptr;
    }

    /// \private
    void traversePath(BindingPath* path, BindingPath::Node* n, QObject* object){
        if ( n == nullptr || object == nullptr)
            return;

        if ( n->type() == BindingPath::Node::Property ){
            BindingPath::PropertyNode* pn = static_cast<BindingPath::PropertyNode*>(n);

            QQmlProperty prop(object, pn->propertyName);
            if ( !prop.isValid() )
                return;

            if ( n->child == nullptr ){
                path->updateConnection(prop);
                return;
            } else {
                traversePath(path, n->child, prop.read().value<QObject*>());
            }
        } else if ( n->type() == BindingPath::Node::Index ){
            BindingPath::IndexNode* in = static_cast<BindingPath::IndexNode*>(n);

            QQmlProperty prop(object);
            if ( !prop.isValid() )
                return;
            if ( prop.propertyTypeCategory() == QQmlProperty::Object && in->index != 0 )
                return;

            if ( n->child == nullptr ){
                path->updateConnection(prop, in->index);
                return;
            } else {
                if ( prop.propertyTypeCategory() == QQmlProperty::Object ){
                    traversePath(path, n->child, prop.read().value<QObject*>());
                } else if ( prop.propertyTypeCategory() == QQmlProperty::List ){
                    QQmlListReference ppref = qvariant_cast<QQmlListReference>(prop.read());
                    if ( ppref.canAt() && ppref.canCount() && ppref.count() > in->index ){
                        traversePath(path, n->child, ppref.at(in->index));
                    } else
                        return;
                }
            }
        }
    }

} // namespace

// class DocumentQmlInfoPrivate
// ------------------------------------------------------------------------------

/// \private
class DocumentQmlInfoPrivate{
public:
    QmlJS::Document::MutablePtr     internalDoc;
    QmlJS::Bind*                    internalDocBind;
    DocumentQmlRanges               ranges;
    QList<DocumentQmlInfo::Message> messages;
};

/**
 * \class lv::DocumentQmlInfo::ValueReference
 * \ingroup lveditqmljs
 * \brief Contains a reference to a value within a parsed qml document
 */

/**
 * \property lv::DocumentQmlInfo::ValueReference::value
 * \brief Internal reference to the value
 */

/**
 * \property lv::DocumentQmlInfo::ValueReference::parent
 * \brief Reference to the parent
 */

/**
 * \class lv::DocumentQmlInfo::ASTReference
 * \ingroup lveditqmljs
 * \brief Reference to an AST node within the parsed qml document
 */

/**
 * \property lv::DocumentQmlInfo::ASTReference::node
 * \brief Internal reference to the AST node
 */


/**
 * \class lv::DocumentQmlInfo::Message
 * \ingroup lveditqmljs
 * \brief Message information containing parsing status.
 */


/**
 * \fn lv::DocumentQmlInfo::Message::Message()
 * \brief DocumentQmlInfo::Message constructor
 */

/**
 * \property lv::DocumentQmlInfo::Message::kind
 * \brief Message severity level
 */

/**
 * \property lv::DocumentQmlInfo::Message::position
 * \brief Position the message was generated at
 */

/**
 * \property lv::DocumentQmlInfo::Message::line
 * \brief Line the message was generated at
 */

/**
 * \property lv::DocumentQmlInfo::Message::text
 * \brief Message text
 */

// class DocumentQmlInfo
// ------------------------------------------------------------------------------

/**
 * \class lv::DocumentQmlInfo
 * \ingroup lveditqmljs
 * \brief Contains parsed information about a Project qml document
 */


/**
 * \brief Returns the lv::DocumentQmlInfo::Dialect according to the file \p extension
 */
DocumentQmlInfo::Dialect DocumentQmlInfo::extensionToDialect(const QString &extension){
    static QHash<QString, DocumentQmlInfo::Dialect> map;
    map["js"]         = DocumentQmlInfo::Javascript;
    map["qml"]        = DocumentQmlInfo::Qml;
    map["qmltypes"]   = DocumentQmlInfo::QmlTypeInfo;
    map["qmlproject"] = DocumentQmlInfo::QmlProject;
    map["json"]       = DocumentQmlInfo::Json;

    return map.value(extension, DocumentQmlInfo::Unknown);
}

/**
 * \brief DocumentQmlInfo constructor.
 *
 * This constructor is private. Use lv::DocumentQmlInfo::create() function.
 */
DocumentQmlInfo::DocumentQmlInfo(const QString &fileName)
    : d_ptr(new DocumentQmlInfoPrivate)
{
    Q_D(DocumentQmlInfo);
    DocumentQmlInfo::Dialect dialect = extensionToDialect(QFileInfo(fileName).suffix());
    if ( dialect == DocumentQmlInfo::Javascript )
        d->internalDoc = QmlJS::Document::create(fileName, QmlJS::Dialect::JavaScript);
    else if ( dialect == DocumentQmlInfo::Qml || fileName == "" ){
        d->internalDoc = QmlJS::Document::create(fileName, QmlJS::Dialect::Qml);
    } else
        d->internalDoc = QmlJS::Document::create(fileName, QmlJS::Dialect::NoLanguage);
}

/**
 * \brief Constructs a new lv::DocumentQmlInfo object.
 */
DocumentQmlInfo::Ptr DocumentQmlInfo::create(const QString &fileName){
    return DocumentQmlInfo::Ptr(new DocumentQmlInfo(fileName));
}

/**
 * \brief Extract the declared id's within a qml document.
 */
QStringList DocumentQmlInfo::extractIds() const{
    Q_D(const DocumentQmlInfo);
    if ( d->internalDocBind->idEnvironment() == 0 )
        return QStringList();

    IdExtractor extractor;
    d->internalDocBind->idEnvironment()->processMembers(&extractor);
    return extractor.ids();
}

/**
 * \brief Return a reference to the root object of this qml document
 */
const DocumentQmlInfo::ValueReference DocumentQmlInfo::rootObject(){
    Q_D(DocumentQmlInfo);
    return DocumentQmlInfo::ValueReference(d->internalDocBind->rootObjectValue(), this);
}

/**
 * \brief Returns a reference to the value represented by the given id.
 */
const DocumentQmlInfo::ValueReference DocumentQmlInfo::valueForId(const QString &id) const{
    Q_D(const DocumentQmlInfo);
    if ( d->internalDocBind->idEnvironment() == 0 )
        return DocumentQmlInfo::ValueReference();

    IdValueExtractor valueExtractor(id);
    d->internalDocBind->idEnvironment()->processMembers(&valueExtractor);
    return DocumentQmlInfo::ValueReference(valueExtractor.value(), this);
}

/**
 * \brief Extracts a usable object given a value reference.
 *
 * For example, you can use this class together with lv::DocumentQmlInfo::valueForId() to inspect
 * a specific object defined with an id within the qml document.
 */
DocumentQmlObject DocumentQmlInfo::extractValueObject(
        const ValueReference &valueref,
        ValueReference *parent) const
{
    DocumentQmlObject vodata;
    if ( isValueNull(valueref) || valueref.parent != this )
        return vodata;

    if ( const QmlJS::ASTObjectValue* vob = valueref.value->asAstObjectValue() ){
        if ( vob->typeName() )
            vodata.setTypeName(vob->typeName()->name.toString());

        ValueMemberExtractor extractor(&vodata);
        vob->processMembers(&extractor);
        if ( parent ){
            parent->value = extractor.parent();
            parent->parent = this;
        }
    }

    return vodata;
 }

/**
 * \brief Extract the name of the type given by this value reference.
 */
QString DocumentQmlInfo::extractTypeName(const DocumentQmlInfo::ValueReference &valueref) const{
    if ( isValueNull(valueref) || valueref.parent != this )
        return "";

    if ( const QmlJS::ASTObjectValue* vob = valueref.value->asAstObjectValue() )
        if ( vob->typeName() )
            return vob->typeName()->name.toString();

    return "";
}

/**
 * \brief Extract the range of the type given by a value reference
 *
 * \p begin and \p end will be populated with the given range, or -1 if the range cannot be extracted
 */
void DocumentQmlInfo::extractTypeNameRange(const DocumentQmlInfo::ValueReference &valueref, int &begin, int &end){
    if ( isValueNull(valueref) || valueref.parent != this ){
        begin = -1;
        end = -1;
        return;
    }

    if ( const QmlJS::ASTObjectValue* vob = valueref.value->asAstObjectValue() ){
        if ( vob->typeName() ){
            begin = vob->typeName()->firstSourceLocation().begin();
            end = vob->typeName()->lastSourceLocation().end();
        }
    }
}

/**
 * \brief Extract the full range given by a value reference
 *
 * \p begin and \p end will be populated with the given range, or -1 if the range cannot be extracted
 */
void DocumentQmlInfo::extractRange(const DocumentQmlInfo::ValueReference &valueref, int &begin, int &end){
    if ( isValueNull(valueref) || valueref.parent != this ){
        begin = -1;
        end = -1;
        return;
    }

    if ( const QmlJS::ASTObjectValue* vob = valueref.value->asAstObjectValue() ){
        if ( vob->initializer() ){
            begin = vob->initializer()->firstSourceLocation().begin();
            end = vob->initializer()->lastSourceLocation().end();
        }
    }
}

/**
 * \brief Finds the ranges within this document
 */
void DocumentQmlInfo::createRanges(){
    Q_D(DocumentQmlInfo);
    d->ranges(d->internalDoc);
}

/**
 * \brief Find the value reference at a given \p position
 */
const DocumentQmlInfo::ValueReference DocumentQmlInfo::valueAtPosition(int position) const{
    Q_D(const DocumentQmlInfo);

    DocumentQmlRanges::Range range = d->ranges.findClosestRange(position);
    if ( range.ast == 0 )
        return DocumentQmlInfo::ValueReference();

    QmlJS::ObjectValue* value = d->internalDocBind->findQmlObject(range.ast);
    return DocumentQmlInfo::ValueReference(value, this);
}

/**
 * \brief Find the value reference and range at a given \p position
 *
 * The \p begin and \p end arguments will be populated with the range positino
 */
const DocumentQmlInfo::ValueReference DocumentQmlInfo::valueAtPosition(
        int position,
        int &begin,
        int &end) const
{
    Q_D(const DocumentQmlInfo);

    QList<DocumentQmlRanges::Range> rangePath = d->ranges.findRangePath(position);
    foreach( const DocumentQmlRanges::Range& range, rangePath ){
        begin = range.begin;
        end   = range.end;

//        QmlJS::ObjectValue* value = d->internalDocBind->findQmlObject(range.ast);
//        return QDocumentQmlInfo::ValueReference(value, this);
    }

    DocumentQmlRanges::Range range = d->ranges.findClosestRange(position);
    if ( range.ast == 0 )
        return DocumentQmlInfo::ValueReference();

    begin = range.begin;
    end   = range.end;

    QmlJS::ObjectValue* value = d->internalDocBind->findQmlObject(range.ast);
    return DocumentQmlInfo::ValueReference(value, this);
}

/**
 * \brief Returns the reference to an AST node at a specified \p position
 */
const DocumentQmlInfo::ASTReference DocumentQmlInfo::astObjectAtPosition(int position){
    Q_D(const DocumentQmlInfo);
    DocumentQmlRanges::Range range = d->ranges.findClosestRange(position);
    return DocumentQmlInfo::ASTReference(range.ast);
}

/**
 * \brief Check wether the value reference is null.
 *
 * \returns true if \p vr is null, false otherwise
 */
bool DocumentQmlInfo::isValueNull(const DocumentQmlInfo::ValueReference& vr) const{
    return vr.value == 0;
}

/**
 * \brief Check wether the document was parsed correctly after calling the lv::DocumentQmlInfo::parse() method
 * \returns true if it was, false otherwise
 */
bool DocumentQmlInfo::isParsedCorrectly() const{
    Q_D(const DocumentQmlInfo);
    return d->internalDoc->isParsedCorrectly();
}

/**
 * \brief Parses the source code to an AST form
 * \returns True if the parse went correctly, false otherwise
 */
bool DocumentQmlInfo::parse(const QString &source){
    Q_D(DocumentQmlInfo);
    d->messages.clear();
    d->internalDoc->setSource(source);
    bool parseResult = d->internalDoc->parse();
    d->internalDocBind = d->internalDoc->bind();

    foreach( const QmlJS::DiagnosticMessage& message, d->internalDoc->diagnosticMessages() ){
        DocumentQmlInfo::Message::Severity severity = DocumentQmlInfo::Message::Hint;
        switch( message.kind ){
        case QmlJS::Severity::Hint: severity = DocumentQmlInfo::Message::Hint; break;
        case QmlJS::Severity::MaybeWarning: severity = DocumentQmlInfo::Message::MaybeWarning; break;
        case QmlJS::Severity::Warning: severity = DocumentQmlInfo::Message::Warning; break;
        case QmlJS::Severity::MaybeError: severity = DocumentQmlInfo::Message::MaybeError; break;
        case QmlJS::Severity::Error: severity = DocumentQmlInfo::Message::Error; break;
        }

        d->messages.append(
            DocumentQmlInfo::Message(
                severity, message.loc.offset, message.loc.startLine, message.message
            )
        );
    }

    return parseResult;
}

/**
 * \brief Return a pointer to the internal QmlJS::Bind object
 */
QmlJS::Bind *DocumentQmlInfo::internalBind(){
    Q_D(DocumentQmlInfo);
    return d->internalDocBind;
}

/**
 * \brief Check wether a given type is an object or not
 * \returns true if \p typeString is an object, false otherwise
 */
bool DocumentQmlInfo::isObject(const QString &typeString){
    if ( typeString == "bool" || typeString == "double" || typeString == "enumeration" ||
         typeString == "int" || typeString == "list" || typeString == "real" ||
         typeString == "string" || typeString == "url" || typeString == "var" )
        return false;
    return true;
}

/**
 * \brief Returns the default value to be assigned for a given qml type
 */
QString DocumentQmlInfo::typeDefaultValue(const QString &typeString){
    if ( typeString == "bool" )
        return "false";
    else if ( typeString == "double" || typeString == "int" || typeString == "enumeration" || typeString == "real" )
        return "0";
    else if ( typeString == "list" )
        return "[]";
    else if ( typeString == "string" || typeString == "url " )
        return "\"\"";
    else if ( typeString == "var" )
        return "undefined";
    else
        return "null";
}

/**
 * \brief Return the path of the Document
 */
QString DocumentQmlInfo::path() const{
    Q_D(const DocumentQmlInfo);
    return d->internalDoc->path();
}

/**
 * \brief Return the component name represented by this Document
 */
QString DocumentQmlInfo::componentName() const{
    Q_D(const DocumentQmlInfo);
    return d->internalDoc->componentName();
}

/**
 * \brief Visit the AST and create the objects defined in this document
 * \returns A pointer to the lv::DocumentQmlValueObjects
 */
DocumentQmlValueObjects::Ptr DocumentQmlInfo::createObjects() const{
    Q_D(const DocumentQmlInfo);
    DocumentQmlValueObjects::Ptr objects = DocumentQmlValueObjects::create();
    objects->visit(d->internalDoc->ast());
    return objects;
}

/**
 * \brief Visit the AST from the given \p ast node and create the objects defined within
 * \returns A pointer to the lv::DocumentQmlValueObjects
 */
DocumentQmlValueObjects::Ptr DocumentQmlInfo::createObjects(const DocumentQmlInfo::ASTReference &ast) const{
    DocumentQmlValueObjects::Ptr objects = DocumentQmlValueObjects::create();
    if ( ast.node ){
        objects->visit(ast.node);
    }
    return objects;
}

/**
 * \brief Match the binding \p path with the application value and update it's connection
 *
 * This function traverses the binding path recursively starting from the \p root of
 * the application and tries to match the \p path with an application value.
 *
 * If the match is set, then the \p path value will be udpated.
 */
void DocumentQmlInfo::traverseBindingPath(BindingPath *path, QObject *root){
    traversePath(path, path->root(), root);
}

/**
 * \brief Finds the binding path associated with a declaration within a range object
 * \returns The found binding path on success, nullptr otherwise
 */
BindingPath *DocumentQmlInfo::findDeclarationPath(
        DocumentQmlValueObjects::RangeObject *root,
        QmlDeclaration::Ptr declaration)
{
    if ( !root )
        return nullptr;

    BindingPath::Node* n = findDeclarationPathImpl(root, declaration);
    if ( !n )
        return nullptr;

    BindingPath* path = new BindingPath;
    path->updatePath(n);

    return path;
}

/**
 * \brief Finds the binding path associated with a delcaration within the project \p document
 * \returns The found binding path if it exists, nullptr otherwise
 */
BindingPath* DocumentQmlInfo::findDeclarationPath(
        const QString &source,
        ProjectDocument *document,
        QmlDeclaration::Ptr declaration)
{
    DocumentQmlInfo::Ptr docinfo = DocumentQmlInfo::create(document->file()->path());
    docinfo->parse(source);

    DocumentQmlValueObjects::Ptr objects = docinfo->createObjects();
    return findDeclarationPath(objects->root(), declaration);
}

/**
 * \brief DocumentQmlInfo destructor
 */
DocumentQmlInfo::~DocumentQmlInfo(){
}

}// namespace

