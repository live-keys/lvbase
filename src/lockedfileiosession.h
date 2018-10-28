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

#ifndef LVLOCKEDFILEIOSESSION_H
#define LVLOCKEDFILEIOSESSION_H

#include "live/lvbaseglobal.h"
#include <memory>

namespace lv{

class FileLock;

class LockedFileIOSessionPrivate;
class LV_BASE_EXPORT LockedFileIOSession{

    DISABLE_COPY(LockedFileIOSession);

public:
    typedef std::shared_ptr<LockedFileIOSession> Ptr;

public:
    ~LockedFileIOSession();

    static LockedFileIOSession::Ptr createInstance();

    std::string readFromFile(const std::string& path);
    bool writeToFile(const std::string& path, const std::string& data);

private:
    LockedFileIOSession();

    LockedFileIOSessionPrivate* m_d;
};

}// namespace

#endif // LVLOCKEDFILEIOSESSION_H
