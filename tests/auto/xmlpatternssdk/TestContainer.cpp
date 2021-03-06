/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the test suite of the Qt Toolkit.
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

#include <QPair>
#include <QtDebug>

#include "TestContainer.h"

using namespace QPatternistSDK;

TestContainer::TestContainer() : m_deleteChildren(true)
{
}

TestContainer::~TestContainer()
{
    if(m_deleteChildren)
        qDeleteAll(m_children);
}

TestResult::List TestContainer::execute(const ExecutionStage stage,
                                        TestSuite *ts)
{
    Q_ASSERT(ts);
    const unsigned int c = m_children.count();
    TestResult::List result;

    for(unsigned int i = 0; i != c; ++i)
        result += static_cast<TestItem *>(child(i))->execute(stage, ts);

    return result;
}

TestItem::ResultSummary TestContainer::resultSummary() const
{
    const int c = childCount();
    int total = 0;
    int pass = 0;

    for(int i = 0; i != c; ++i)
    {
        TestItem *t = static_cast<TestItem *>(child(i));
        const ResultSummary sum(t->resultSummary());
        pass += sum.first;
        total += sum.second;
    }

    return ResultSummary(pass, total);
}

TreeItem::List TestContainer::children() const
{
    return m_children;
}

void TestContainer::appendChild(TreeItem *item)
{
    /* When one of our children changes, we changes. */
    connect(item, SIGNAL(changed(TreeItem *)), SIGNAL(changed(TreeItem *)));
    m_children.append(item);
}

TreeItem *TestContainer::child(const unsigned int rowP) const
{
    return m_children.value(rowP);
}

unsigned int TestContainer::childCount() const
{
    return m_children.count();
}

void TestContainer::setTitle(const QString &titleP)
{
    m_title = titleP;
}

QString TestContainer::title() const
{
    return m_title;
}

bool TestContainer::isFinalNode() const
{
    return false;
}

int TestContainer::columnCount() const
{
    return 4;
}

QString TestContainer::description() const
{
    return m_description;
}

void TestContainer::setDescription(const QString &desc)
{
    m_description = desc;
}

void TestContainer::setDeleteChildren(const bool val)
{
    m_deleteChildren = val;
}

void TestContainer::removeLast()
{
    m_children.removeLast();
}

// vim: et:ts=4:sw=4:sts=4
