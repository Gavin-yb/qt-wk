/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef BYTEARRAYPROTOTYPE_H
#define BYTEARRAYPROTOTYPE_H

#include <QtCore/QByteArray>
#include <QtCore/QObject>
#include <QtScript/QScriptable>
#include <QtScript/QScriptValue>

//! [0]
class ByteArrayPrototype : public QObject, public QScriptable
{
Q_OBJECT
public:
    ByteArrayPrototype(QObject *parent = 0);
    ~ByteArrayPrototype();

public slots:
    void chop(int n);
    bool equals(const QByteArray &other);
    QByteArray left(int len) const;
    QByteArray mid(int pos, int len = -1) const;
    QScriptValue remove(int pos, int len);
    QByteArray right(int len) const;
    QByteArray simplified() const;
    QByteArray toBase64() const;
    QByteArray toLower() const;
    QByteArray toUpper() const;
    QByteArray trimmed() const;
    void truncate(int pos);
    QString toLatin1String() const;
    QScriptValue valueOf() const;

private:
    QByteArray *thisByteArray() const;
};
//! [0]


#endif
