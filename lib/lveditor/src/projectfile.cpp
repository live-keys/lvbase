/****************************************************************************
**
** Copyright (C) 2014-2019 Dinu SV.
** (contact: mail@dinusv.com)
** This file is part of Livekeys Application.
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

#include "live/projectfile.h"
#include "live/document.h"
#include <QFileInfo>

/**
 * \class lv::ProjectFile
 * \brief Specialization of ProjectEntry for files
 * \ingroup lveditor
 */
namespace lv{

/** Default constructor */
ProjectFile::ProjectFile(const QString &path, ProjectEntry *parent)
    : ProjectEntry(QFileInfo(path).path(), QFileInfo(path).fileName(), true, parent)
    , m_document(nullptr)
{
}

/** Constructor with a name parameter given */
ProjectFile::ProjectFile(const QString &path, const QString &name, ProjectEntry *parent)
    : ProjectEntry(path, name, true, parent)
    , m_document(nullptr)
{
}

/** Blank destructor */
ProjectFile::~ProjectFile(){
}

/**
 * Document setter
 */
void ProjectFile::setDocument(Document *document){
    if ( document == m_document )
        return;

    if ( m_document ){
        disconnect(m_document, &Document::pathChanged, this, &ProjectFile::__documentPathChanged);
        disconnect(m_document, &Document::aboutToClose, this, &ProjectFile::__documentAboutToClose);
    }

    m_document = document;

    if ( m_document ){
        connect(m_document, &Document::pathChanged, this, &ProjectFile::__documentPathChanged);
        connect(m_document, &Document::aboutToClose, this, &ProjectFile::__documentAboutToClose);
    }

    emit documentChanged();
}

void ProjectFile::__documentAboutToClose(){
    if ( parent() == nullptr )
        deleteLater();
    else {
        setDocument(nullptr);
    }
}

void ProjectFile::__documentPathChanged(){
    if ( parent() == nullptr )
        deleteLater();
    else {
        setDocument(nullptr);
    }
}

}// namespace
