/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtDeclarative module of the Qt Toolkit.
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

#ifndef QDECLARATIVESTATE_H
#define QDECLARATIVESTATE_H

#include <qdeclarative.h>
#include <qdeclarativeproperty.h>
#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Declarative)

class QDeclarativeActionEvent;
class QDeclarativeAbstractBinding;
class QDeclarativeBinding;
class QDeclarativeExpression;
class Q_DECLARATIVE_EXPORT QDeclarativeAction
{
public:
    QDeclarativeAction();
    QDeclarativeAction(QObject *, const QString &, const QVariant &);
    QDeclarativeAction(QObject *, const QString &,
                       QDeclarativeContext *, const QVariant &);

    bool restore:1;
    bool actionDone:1;
    bool reverseEvent:1;
    bool deletableToBinding:1;

    QDeclarativeProperty property;
    QVariant fromValue;
    QVariant toValue;

    QDeclarativeAbstractBinding *fromBinding;
    QDeclarativeAbstractBinding *toBinding;
    QDeclarativeActionEvent *event;

    //strictly for matching
    QObject *specifiedObject;
    QString specifiedProperty;

    void deleteFromBinding();
};

class QDeclarativeActionEvent
{
public:
    virtual ~QDeclarativeActionEvent();
    virtual QString typeName() const;

    enum Reason { ActualChange, FastForward };

    virtual void execute(Reason reason = ActualChange);
    virtual bool isReversable();
    virtual void reverse(Reason reason = ActualChange);
    virtual void saveOriginals() {}
    virtual void copyOriginals(QDeclarativeActionEvent *) {}

    virtual bool isRewindable() { return isReversable(); }
    virtual void rewind() {}
    virtual void saveCurrentValues() {}
    virtual void saveTargetValues() {}

    virtual bool changesBindings();
    virtual void clearBindings();
    virtual bool override(QDeclarativeActionEvent*other);
};

//### rename to QDeclarativeStateChange?
class QDeclarativeStateGroup;
class Q_DECLARATIVE_EXPORT QDeclarativeStateOperation : public QObject
{
    Q_OBJECT
public:
    QDeclarativeStateOperation(QObject *parent = 0)
        : QObject(parent) {}
    typedef QList<QDeclarativeAction> ActionList;

    virtual ActionList actions();

protected:
    QDeclarativeStateOperation(QObjectPrivate &dd, QObject *parent = 0);
};

typedef QDeclarativeStateOperation::ActionList QDeclarativeStateActions;

class QDeclarativeTransition;
class QDeclarativeStatePrivate;
class Q_DECLARATIVE_EXPORT QDeclarativeState : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QDeclarativeBinding *when READ when WRITE setWhen)
    Q_PROPERTY(QString extend READ extends WRITE setExtends)
    Q_PROPERTY(QDeclarativeListProperty<QDeclarativeStateOperation> changes READ changes)
    Q_CLASSINFO("DefaultProperty", "changes")
    Q_CLASSINFO("DeferredPropertyNames", "changes")

public:
    QDeclarativeState(QObject *parent=0);
    virtual ~QDeclarativeState();

    QString name() const;
    void setName(const QString &);

    /*'when' is a QDeclarativeBinding to limit state changes oscillation
     due to the unpredictable order of evaluation of bound expressions*/
    bool isWhenKnown() const;
    QDeclarativeBinding *when() const;
    void setWhen(QDeclarativeBinding *);

    QString extends() const;
    void setExtends(const QString &);

    QDeclarativeListProperty<QDeclarativeStateOperation> changes();
    int operationCount() const;
    QDeclarativeStateOperation *operationAt(int) const;

    QDeclarativeState &operator<<(QDeclarativeStateOperation *);

    void apply(QDeclarativeStateGroup *, QDeclarativeTransition *, QDeclarativeState *revert);
    void cancel();

    QDeclarativeStateGroup *stateGroup() const;
    void setStateGroup(QDeclarativeStateGroup *);

Q_SIGNALS:
    void completed();

private:
    Q_DECLARE_PRIVATE(QDeclarativeState)
    Q_DISABLE_COPY(QDeclarativeState)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeStateOperation)
QML_DECLARE_TYPE(QDeclarativeState)

QT_END_HEADER

#endif // QDECLARATIVESTATE_H