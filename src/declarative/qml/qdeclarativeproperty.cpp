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

#include "qdeclarativeproperty.h"
#include "qdeclarativeproperty_p.h"

#include "qdeclarativecompositetypedata_p.h"
#include "qdeclarative.h"
#include "qdeclarativebinding_p.h"
#include "qdeclarativecontext.h"
#include "qdeclarativecontext_p.h"
#include "qdeclarativeboundsignal_p.h"
#include "qdeclarativeengine.h"
#include "qdeclarativeengine_p.h"
#include "qdeclarativedeclarativedata_p.h"
#include "qdeclarativestringconverters_p.h"
#include "qdeclarativelist_p.h"
#include "qdeclarativecompiler_p.h"

#include <QStringList>
#include <QtCore/qdebug.h>

#include <math.h>

QT_BEGIN_NAMESPACE

/*!
\class QDeclarativeProperty
\brief The QDeclarativeProperty class abstracts accessing properties on objects created from  QML.

As QML uses Qt's meta-type system all of the existing QMetaObject classes can be used to introspect
and interact with objects created by QML.  However, some of the new features provided by QML - such 
as type safety and attached properties - are most easily used through the QDeclarativeProperty class 
that simplifies some of their natural complexity.

Unlike QMetaProperty which represents a property on a class type, QDeclarativeProperty encapsulates 
a property on a specific object instance.  To read a property's value, programmers create a 
QDeclarativeProperty instance and call the read() method.  Likewise to write a property value the
write() method is used.

\code

QObject *object = declarativeComponent.create();

QDeclarativeProperty property(object, "font.pixelSize");
qWarning() << "Current pixel size:" << property.read().toInt();
property.write(24);
qWarning() << "Pixel size should now be 24:" << property.read().toInt();

\endcode
*/

/*!
    Create an invalid QDeclarativeProperty.
*/
QDeclarativeProperty::QDeclarativeProperty()
: d(new QDeclarativePropertyPrivate)
{
    d->q = this;
}

/*!  \internal */
QDeclarativeProperty::~QDeclarativeProperty()
{
    delete d; d = 0;
}

/*!
    Creates a QDeclarativeProperty for the default property of \a obj. If there is no
    default property, an invalid QDeclarativeProperty will be created.
 */
QDeclarativeProperty::QDeclarativeProperty(QObject *obj)
: d(new QDeclarativePropertyPrivate)
{
    d->q = this;
    d->initDefault(obj);
}

/*!
    Creates a QDeclarativeProperty for the default property of \a obj. If there is no
    default property, an invalid QDeclarativeProperty will be created.
 */
QDeclarativeProperty::QDeclarativeProperty(QObject *obj, QDeclarativeContext *ctxt)
: d(new QDeclarativePropertyPrivate)
{
    d->q = this;
    d->context = ctxt;
    d->engine = ctxt?ctxt->engine():0;
    d->initDefault(obj);
}

/*!
    Creates a QDeclarativeProperty for the default property of \a obj. If there is no
    default property, an invalid QDeclarativeProperty will be created.
 */
QDeclarativeProperty::QDeclarativeProperty(QObject *obj, QDeclarativeEngine *engine)
: d(new QDeclarativePropertyPrivate)
{
    d->q = this;
    d->context = 0;
    d->engine = engine;
    d->initDefault(obj);
}

/*!
    Initialize from the default property of \a obj
*/
void QDeclarativePropertyPrivate::initDefault(QObject *obj)
{
    if (!obj)
        return;

    QMetaProperty p = QDeclarativeMetaType::defaultProperty(obj);
    core.load(p);
    if (core.isValid()) 
        object = obj;
}

/*!
    Creates a QDeclarativeProperty for the property \a name of \a obj.
 */
QDeclarativeProperty::QDeclarativeProperty(QObject *obj, const QString &name)
: d(new QDeclarativePropertyPrivate)
{
    d->q = this;
    d->initProperty(obj, name);
    if (!isValid()) d->object = 0;
}

/*!
    Creates a QDeclarativeProperty for the property \a name of \a obj.
 */
QDeclarativeProperty::QDeclarativeProperty(QObject *obj, const QString &name, QDeclarativeContext *ctxt)
: d(new QDeclarativePropertyPrivate)
{
    d->q = this;
    d->context = ctxt;
    d->engine = ctxt?ctxt->engine():0;
    d->initProperty(obj, name);
    if (!isValid()) { d->object = 0; d->context = 0; d->engine = 0; }
}

/*!
    Creates a QDeclarativeProperty for the property \a name of \a obj.
 */
QDeclarativeProperty::QDeclarativeProperty(QObject *obj, const QString &name, QDeclarativeEngine *engine)
: d(new QDeclarativePropertyPrivate)
{
    d->q = this;
    d->context = 0;
    d->engine = engine;
    d->initProperty(obj, name);
    if (!isValid()) { d->object = 0; d->context = 0; d->engine = 0; }
}

Q_GLOBAL_STATIC(QDeclarativeValueTypeFactory, qmlValueTypes);

void QDeclarativePropertyPrivate::initProperty(QObject *obj, const QString &name)
{
    if (!obj) return;

    QDeclarativeTypeNameCache *typeNameCache = context?QDeclarativeContextPrivate::get(context)->imports:0;

    QStringList path = name.split(QLatin1Char('.'));
    if (path.isEmpty()) return;

    QObject *currentObject = obj;

    // Everything up to the last property must be an "object type" property
    for (int ii = 0; ii < path.count() - 1; ++ii) {
        const QString &pathName = path.at(ii);

        if (QDeclarativeTypeNameCache::Data *data = typeNameCache?typeNameCache->data(pathName):0) {
            if (data->type) {
                QDeclarativeAttachedPropertiesFunc func = data->type->attachedPropertiesFunction();
                if (!func) return; // Not an attachable type

                currentObject = qmlAttachedPropertiesObjectById(data->type->index(), currentObject);
                if (!currentObject) return; // Something is broken with the attachable type
            } else {
                Q_ASSERT(data->typeNamespace);
                if ((ii + 1) == path.count()) return; // No type following the namespace
                
                ++ii; data = data->typeNamespace->data(path.at(ii));
                if (!data || !data->type) return; // Invalid type in namespace 

                QDeclarativeAttachedPropertiesFunc func = data->type->attachedPropertiesFunction();
                if (!func) return; // Not an attachable type

                currentObject = qmlAttachedPropertiesObjectById(data->type->index(), currentObject);
                if (!currentObject) return; // Something is broken with the attachable type
            }
        } else {

            QDeclarativePropertyCache::Data local;
            QDeclarativePropertyCache::Data *property = 
                QDeclarativePropertyCache::property(engine, obj, pathName, local);

            if (!property) return; // Not a property
            if (property->flags & QDeclarativePropertyCache::Data::IsFunction) 
                return; // Not an object property 

            if (ii == (path.count() - 2) && QDeclarativeValueTypeFactory::isValueType(property->propType)) {
                // We're now at a value type property.  We can use a global valuetypes array as we 
                // never actually use the objects, just look up their properties.
                QObject *typeObject = (*qmlValueTypes())[property->propType];
                if (!typeObject) return; // Not a value type

                int idx = typeObject->metaObject()->indexOfProperty(path.last().toUtf8().constData());
                if (idx == -1) return; // Value type property does not exist

                QMetaProperty vtProp = typeObject->metaObject()->property(idx);

                object = currentObject;
                core = *property;
                valueType.flags = QDeclarativePropertyCache::Data::flagsForProperty(vtProp);
                valueType.valueTypeCoreIdx = idx;
                valueType.valueTypePropType = vtProp.userType();

                return; 
            } else {
                if (!(property->flags & QDeclarativePropertyCache::Data::IsQObjectDerived)) 
                    return; // Not an object property

                void *args[] = { &currentObject, 0 };
                QMetaObject::metacall(currentObject, QMetaObject::ReadProperty, property->coreIndex, args);
                if (!currentObject) return; // No value

            }
        }

    }

    const QString &terminal = path.last();

    if (terminal.count() >= 3 &&
        terminal.at(0) == QLatin1Char('o') &&
        terminal.at(1) == QLatin1Char('n') &&
        terminal.at(2).isUpper()) {

        QString signalName = terminal.mid(2);
        signalName[0] = signalName.at(0).toLower();

        QMetaMethod method = QDeclarativeCompiler::findSignalByName(currentObject->metaObject(), signalName.toLatin1().constData());
        if (method.signature()) {
            object = currentObject;
            core.load(method);
            return;
        }
    }

    // Property
    QDeclarativePropertyCache::Data local;
    QDeclarativePropertyCache::Data *property = 
        QDeclarativePropertyCache::property(engine, currentObject, terminal, local);
    if (property && !(property->flags & QDeclarativePropertyCache::Data::IsFunction)) {
        object = currentObject;
        core = *property;
    }
}

/*!
    Create a copy of \a other.
*/
QDeclarativeProperty::QDeclarativeProperty(const QDeclarativeProperty &other)
: d(new QDeclarativePropertyPrivate(*other.d))
{
    d->q = this;
}

/*!
  \enum QDeclarativeProperty::PropertyTypeCategory

  This enum specifies a category of QML property.

  \value InvalidCategory The property is invalid, or is a signal property.
  \value List The property is a QDeclarativeListProperty list property
  \value Object The property is a QObject derived type pointer
  \value Normal The property is a normal value property.
 */

/*!
  \enum QDeclarativeProperty::Type

  This enum specifies a type of QML property.

  \value Invalid The property is invalid.
  \value Property The property is a regular Qt property.
  \value SignalProperty The property is a signal property.
*/

/*!
    Returns the property category.
*/
QDeclarativeProperty::PropertyTypeCategory QDeclarativeProperty::propertyTypeCategory() const
{
    return d->propertyTypeCategory();
}

QDeclarativeProperty::PropertyTypeCategory 
QDeclarativePropertyPrivate::propertyTypeCategory() const
{
    uint type = q->type();

    if (isValueType()) {
        return QDeclarativeProperty::Normal;
    } else if (type & QDeclarativeProperty::Property) {
        int type = propertyType();
        if (type == QVariant::Invalid)
            return QDeclarativeProperty::InvalidCategory;
        else if (QDeclarativeValueTypeFactory::isValueType((uint)type))
            return QDeclarativeProperty::Normal;
        else if (core.flags & QDeclarativePropertyCache::Data::IsQObjectDerived)
            return QDeclarativeProperty::Object;
        else if (core.flags & QDeclarativePropertyCache::Data::IsQList)
            return QDeclarativeProperty::List;
        else 
            return QDeclarativeProperty::Normal;
    } else {
        return QDeclarativeProperty::InvalidCategory;
    }
}

/*!
    Returns the type name of the property, or 0 if the property has no type
    name.
*/
const char *QDeclarativeProperty::propertyTypeName() const
{
    if (d->isValueType()) {

        QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(d->context);
        QDeclarativeValueType *valueType = 0;
        if (ep) valueType = ep->valueTypes[d->core.propType];
        else valueType = QDeclarativeValueTypeFactory::valueType(d->core.propType);
        Q_ASSERT(valueType);

        const char *rv = valueType->metaObject()->property(d->valueType.valueTypeCoreIdx).typeName();

        if (!ep) delete valueType;

        return rv;
    } else if (d->object && type() & Property && d->core.isValid()) {
        return d->object->metaObject()->property(d->core.coreIndex).typeName();
    } else {
        return 0;
    }
}

/*!
    Returns true if \a other and this QDeclarativeProperty represent the same 
    property.
*/
bool QDeclarativeProperty::operator==(const QDeclarativeProperty &other) const
{
    // category is intentially omitted here as it is generated 
    // from the other members
    return d->object == other.d->object &&
           d->core == other.d->core &&
           d->valueType == other.d->valueType;
}

/*!
    Returns the QVariant type of the property, or QVariant::Invalid if the 
    property has no QVariant type.
*/
int QDeclarativeProperty::propertyType() const
{
    return d->propertyType();
}

bool QDeclarativePropertyPrivate::isValueType() const
{
    return valueType.valueTypeCoreIdx != -1;
}

int QDeclarativePropertyPrivate::propertyType() const
{
    uint type = q->type();
    if (isValueType()) {
        return valueType.valueTypePropType;
    } else if (type & QDeclarativeProperty::Property) {
        if (core.propType == (int)QVariant::LastType)
            return qMetaTypeId<QVariant>();
        else
            return core.propType;
    } else {
        return QVariant::Invalid;
    }
}

/*!
    Returns the type of the property.
*/
QDeclarativeProperty::Type QDeclarativeProperty::type() const
{
    if (d->core.flags & QDeclarativePropertyCache::Data::IsFunction)
        return SignalProperty;
    else if (d->core.isValid())
        return Property;
    else
        return Invalid;
}

/*!
    Returns true if this QDeclarativeProperty represents a regular Qt property.
*/
bool QDeclarativeProperty::isProperty() const
{
    return type() & Property;
}

/*!
    Returns true if this QDeclarativeProperty represents a QML signal property.
*/
bool QDeclarativeProperty::isSignalProperty() const
{
    return type() & SignalProperty;
}

/*!
    Returns the QDeclarativeProperty's QObject.
*/
QObject *QDeclarativeProperty::object() const
{
    return d->object;
}

/*!
    Assign \a other to this QDeclarativeProperty.
*/
QDeclarativeProperty &QDeclarativeProperty::operator=(const QDeclarativeProperty &other)
{
    d->context = other.d->context;
    d->engine = other.d->engine;
    d->object = other.d->object;

    d->isNameCached = other.d->isNameCached;
    d->core = other.d->core;
    d->nameCache = other.d->nameCache;

    d->valueType = other.d->valueType;

    return *this;
}

/*!
    Returns true if the property is writable, otherwise false.
*/
bool QDeclarativeProperty::isWritable() const
{
    QDeclarativeProperty::PropertyTypeCategory category = propertyTypeCategory();

    if (!d->object)
        return false;
    if (category == List)
        return true;
    else if (type() & SignalProperty)
        return false;
    else if (d->core.isValid() && d->object)
        return d->core.flags & QDeclarativePropertyCache::Data::IsWritable;
    else
        return false;
}

/*!
    Returns true if the property is designable, otherwise false.
*/
bool QDeclarativeProperty::isDesignable() const
{
    if (type() & Property && d->core.isValid() && d->object)
        return d->object->metaObject()->property(d->core.coreIndex).isDesignable();
    else
        return false;
}

/*!
    Returns true if the property is resettable, otherwise false.
*/
bool QDeclarativeProperty::isResettable() const
{
    if (type() & Property && d->core.isValid() && d->object)
        return d->core.flags & QDeclarativePropertyCache::Data::IsResettable;
    else
        return false;
}

/*!
    Returns true if the QDeclarativeProperty refers to a valid property, otherwise
    false.
*/
bool QDeclarativeProperty::isValid() const
{
    return type() != Invalid;
}

/*!
    Return the name of this QML property.
*/
QString QDeclarativeProperty::name() const
{
    if (!d->isNameCached) {
        // ###
        if (!d->object) {
        } else if (d->isValueType()) {
            QString rv = d->core.name(d->object) + QLatin1Char('.');

            QDeclarativeEnginePrivate *ep = d->engine?QDeclarativeEnginePrivate::get(d->engine):0;
            QDeclarativeValueType *valueType = 0;
            if (ep) valueType = ep->valueTypes[d->core.propType];
            else valueType = QDeclarativeValueTypeFactory::valueType(d->core.propType);
            Q_ASSERT(valueType);

            rv += QString::fromUtf8(valueType->metaObject()->property(d->valueType.valueTypeCoreIdx).name());

            if (!ep) delete valueType;

            d->nameCache = rv;
        } else if (type() & SignalProperty) {
            QString name = QLatin1String("on") + d->core.name(d->object);
            name[2] = name.at(2).toUpper();
            d->nameCache = name;
        } else {
            d->nameCache = d->core.name(d->object);
        }
        d->isNameCached = true;
    }

    return d->nameCache;
}

/*!
  Returns the \l{QMetaProperty} {Qt property} associated with
  this QML property.
 */
QMetaProperty QDeclarativeProperty::property() const
{
    if (type() & Property && d->core.isValid() && d->object)
        return d->object->metaObject()->property(d->core.coreIndex);
    else
        return QMetaProperty();
}

/*!
    Return the QMetaMethod for this property if it is a SignalProperty, 
    otherwise returns an invalid QMetaMethod.
*/
QMetaMethod QDeclarativeProperty::method() const
{
    if (type() & SignalProperty && d->object)
        return d->object->metaObject()->method(d->core.coreIndex);
    else
        return QMetaMethod();
}


/*!
    Returns the binding associated with this property, or 0 if no binding 
    exists.
*/
QDeclarativeAbstractBinding *
QDeclarativePropertyPrivate::binding(const QDeclarativeProperty &that) 
{
    if (!that.isProperty() || !that.d->object)
        return 0;

    QDeclarativeDeclarativeData *data = QDeclarativeDeclarativeData::get(that.d->object);
    if (!data) 
        return 0;

    if (!data->hasBindingBit(that.d->core.coreIndex))
        return 0;

    QDeclarativeAbstractBinding *binding = data->bindings;
    while (binding) {
        // ### This wont work for value types
        if (binding->propertyIndex() == that.d->core.coreIndex)
            return binding; 
        binding = binding->m_nextBinding;
    }
    return 0;
}

/*!
    Set the binding associated with this property to \a newBinding.  Returns
    the existing binding (if any), otherwise 0.

    \a newBinding will be enabled, and the returned binding (if any) will be
    disabled.

    Ownership of \a newBinding transfers to QML.  Ownership of the return value
    is assumed by the caller.

    \a flags is passed through to the binding and is used for the initial update (when
    the binding sets the intial value, it will use these flags for the write).
*/
QDeclarativeAbstractBinding *
QDeclarativePropertyPrivate::setBinding(const QDeclarativeProperty &that,
                                            QDeclarativeAbstractBinding *newBinding, 
                                            WriteFlags flags) 
{
    if (!that.isProperty() || !that.d->object) {
        if (newBinding)
            newBinding->destroy();
        return 0;
    }

    return that.d->setBinding(that.d->object, that.d->core, newBinding, flags);
}

QDeclarativeAbstractBinding *
QDeclarativePropertyPrivate::setBinding(QObject *object, const QDeclarativePropertyCache::Data &core, 
                                   QDeclarativeAbstractBinding *newBinding, WriteFlags flags)
{
    QDeclarativeDeclarativeData *data = QDeclarativeDeclarativeData::get(object, 0 != newBinding);

    if (data && data->hasBindingBit(core.coreIndex)) {
        QDeclarativeAbstractBinding *binding = data->bindings;
        while (binding) {
            // ### This wont work for value types
            if (binding->propertyIndex() == core.coreIndex) {
                binding->setEnabled(false);

                if (newBinding) 
                    newBinding->setEnabled(true, flags);

                return binding; // ### QDeclarativeAbstractBinding;
            }

            binding = binding->m_nextBinding;
        }
    } 

    if (newBinding)
        newBinding->setEnabled(true, flags);

    return 0;
}

/*!
    Returns the expression associated with this signal property, or 0 if no 
    signal expression exists.
*/
QDeclarativeExpression *
QDeclarativePropertyPrivate::signalExpression(const QDeclarativeProperty &that)
{
    if (!(that.type() & QDeclarativeProperty::SignalProperty))
        return 0;

    const QObjectList &children = that.d->object->children();
    
    for (int ii = 0; ii < children.count(); ++ii) {
        QObject *child = children.at(ii);

        QDeclarativeBoundSignal *signal = QDeclarativeBoundSignal::cast(child);
        if (signal && signal->index() == that.index()) 
            return signal->expression();
    }

    return 0;
}

/*!
    Set the signal expression associated with this signal property to \a expr.
    Returns the existing signal expression (if any), otherwise 0.

    Ownership of \a expr transfers to QML.  Ownership of the return value is
    assumed by the caller.
*/
QDeclarativeExpression *
QDeclarativePropertyPrivate::setSignalExpression(const QDeclarativeProperty &that,
                                                     QDeclarativeExpression *expr) 
{
    if (!(that.type() & QDeclarativeProperty::SignalProperty)) {
        delete expr;
        return 0;
    }

    const QObjectList &children = that.d->object->children();
    
    for (int ii = 0; ii < children.count(); ++ii) {
        QObject *child = children.at(ii);

        QDeclarativeBoundSignal *signal = QDeclarativeBoundSignal::cast(child);
        if (signal && signal->index() == that.index()) 
            return signal->setExpression(expr);
    }

    if (expr) {
        QDeclarativeBoundSignal *signal = new QDeclarativeBoundSignal(that.d->object, that.method(), that.d->object);
        return signal->setExpression(expr);
    } else {
        return 0;
    }
}

/*!
    Returns the property value.
*/
QVariant QDeclarativeProperty::read() const
{
    if (!d->object)
        return QVariant();

    if (type() & SignalProperty) {

        return QVariant();

    } else if (type() & Property) {

        return d->readValueProperty();

    }
    return QVariant();
}

/*!
Return the \a name property value of \a object.  This method is equivalent to:
\code
    QDeclarativeProperty p(object, name);
    p.read();
\endcode
*/
QVariant QDeclarativeProperty::read(QObject *object, const QString &name)
{
    QDeclarativeProperty p(object, name);
    return p.read();
}

/*!
Return the \a name property value of \a object.  This method is equivalent to:
\code
    QDeclarativeProperty p(object, name, context);
    p.read();
\endcode
*/
QVariant QDeclarativeProperty::read(QObject *object, const QString &name, QDeclarativeContext *ctxt)
{
    QDeclarativeProperty p(object, name, ctxt);
    return p.read();
}

/*!
Return the \a name property value of \a object.  This method is equivalent to:
\code
    QDeclarativeProperty p(object, name, engine);
    p.read();
\endcode
*/
QVariant QDeclarativeProperty::read(QObject *object, const QString &name, QDeclarativeEngine *engine)
{
    QDeclarativeProperty p(object, name, engine);
    return p.read();
}

QVariant QDeclarativePropertyPrivate::readValueProperty()
{
    if (isValueType()) {

        QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(context);
        QDeclarativeValueType *valueType = 0;
        if (ep) valueType = ep->valueTypes[core.propType];
        else valueType = QDeclarativeValueTypeFactory::valueType(core.propType);
        Q_ASSERT(valueType);

        valueType->read(object, core.coreIndex);

        QVariant rv =
            valueType->metaObject()->property(this->valueType.valueTypeCoreIdx).read(valueType);

        if (!ep) delete valueType;
        return rv;

    } else if (core.flags & QDeclarativePropertyCache::Data::IsQList) {

        QDeclarativeListProperty<QObject> prop;
        void *args[] = { &prop, 0 };
        QMetaObject::metacall(object, QMetaObject::ReadProperty, core.coreIndex, args);
        return QVariant::fromValue(QDeclarativeListReferencePrivate::init(prop, core.propType, engine)); 

    } else if (core.flags & QDeclarativePropertyCache::Data::IsQObjectDerived) {

        QObject *rv = 0;
        void *args[] = { &rv, 0 };
        QMetaObject::metacall(object, QMetaObject::ReadProperty, core.coreIndex, args);
        return QVariant::fromValue(rv);

    } else {

        return object->metaObject()->property(core.coreIndex).read(object.data());

    }
}

//writeEnumProperty MIRRORS the relelvant bit of QMetaProperty::write AND MUST BE KEPT IN SYNC!
bool QDeclarativePropertyPrivate::writeEnumProperty(const QMetaProperty &prop, int idx, QObject *object, const QVariant &value, int flags)
{
    if (!object || !prop.isWritable())
        return false;

    QVariant v = value;
    if (prop.isEnumType()) {
        QMetaEnum menum = prop.enumerator();
        if (v.userType() == QVariant::String
#ifdef QT3_SUPPORT
            || v.userType() == QVariant::CString
#endif
            ) {
            if (prop.isFlagType())
                v = QVariant(menum.keysToValue(value.toByteArray()));
            else
                v = QVariant(menum.keyToValue(value.toByteArray()));
        } else if (v.userType() != QVariant::Int && v.userType() != QVariant::UInt) {
            int enumMetaTypeId = QMetaType::type(QByteArray(menum.scope()) + "::" + menum.name());
            if ((enumMetaTypeId == 0) || (v.userType() != enumMetaTypeId) || !v.constData())
                return false;
            v = QVariant(*reinterpret_cast<const int *>(v.constData()));
        }
        v.convert(QVariant::Int);
    }

    // the status variable is changed by qt_metacall to indicate what it did
    // this feature is currently only used by QtDBus and should not be depended
    // upon. Don't change it without looking into QDBusAbstractInterface first
    // -1 (unchanged): normal qt_metacall, result stored in argv[0]
    // changed: result stored directly in value, return the value of status
    int status = -1;
    void *argv[] = { v.data(), &v, &status, &flags };
    QMetaObject::metacall(object, QMetaObject::WriteProperty, idx, argv);
    return status;
}

bool QDeclarativePropertyPrivate::writeValueProperty(const QVariant &value, WriteFlags flags)
{
    // Remove any existing bindings on this property
    if (!(flags & DontRemoveBinding)) {
        QDeclarativeAbstractBinding *binding = setBinding(*q, 0);
        if (binding) binding->destroy();
    }

    bool rv = false;
    if (isValueType()) {
        QDeclarativeEnginePrivate *ep = QDeclarativeEnginePrivate::get(context);

        QDeclarativeValueType *writeBack = 0;
        if (ep) {
            writeBack = ep->valueTypes[core.propType];
        } else {
            writeBack = QDeclarativeValueTypeFactory::valueType(core.propType);
        }

        writeBack->read(object, core.coreIndex);

        QDeclarativePropertyCache::Data data = core;
        data.flags = valueType.flags;
        data.coreIndex = valueType.valueTypeCoreIdx;
        data.propType = valueType.valueTypePropType;
        rv = write(writeBack, data, value, context, flags);

        writeBack->write(object, core.coreIndex, flags);
        if (!ep) delete writeBack;

    } else {

        rv = write(object, core, value, context, flags);

    }

    return rv;
}

bool QDeclarativePropertyPrivate::write(QObject *object, const QDeclarativePropertyCache::Data &property, 
                                            const QVariant &value, QDeclarativeContext *context, 
                                            WriteFlags flags)
{
    int coreIdx = property.coreIndex;
    int status = -1;    //for dbus

    if (property.flags & QDeclarativePropertyCache::Data::IsEnumType) {
        QMetaProperty prop = object->metaObject()->property(property.coreIndex);
        QVariant v = value;
        // Enum values come through the script engine as doubles
        if (value.userType() == QVariant::Double) { 
            double integral;
            double fractional = modf(value.toDouble(), &integral);
            if (qFuzzyIsNull(fractional))
                v.convert(QVariant::Int);
        }
        return writeEnumProperty(prop, coreIdx, object, v, flags);
    }

    int propertyType = property.propType;
    int variantType = value.userType();

    QDeclarativeEnginePrivate *enginePriv = QDeclarativeEnginePrivate::get(context);

    if (propertyType == QVariant::Url) {

        QUrl u;
        bool found = false;
        if (variantType == QVariant::Url) {
            u = value.toUrl();
            found = true;
        } else if (variantType == QVariant::ByteArray) {
            u = QUrl(QString::fromUtf8(value.toByteArray()));
            found = true;
        } else if (variantType == QVariant::String) {
            u = QUrl(value.toString());
            found = true;
        }

        if (!found)
            return false;

        if (context && u.isRelative() && !u.isEmpty())
            u = context->resolvedUrl(u);
        int status = -1;
        void *argv[] = { &u, 0, &status, &flags };
        QMetaObject::metacall(object, QMetaObject::WriteProperty, coreIdx, argv);

    } else if (variantType == propertyType) {

        void *a[] = { (void *)value.constData(), 0, &status, &flags };
        QMetaObject::metacall(object, QMetaObject::WriteProperty, coreIdx, a);

    } else if (qMetaTypeId<QVariant>() == propertyType) {

        void *a[] = { (void *)&value, 0, &status, &flags };
        QMetaObject::metacall(object, QMetaObject::WriteProperty, coreIdx, a);

    } else if (property.flags & QDeclarativePropertyCache::Data::IsQObjectDerived) {

        const QMetaObject *valMo = rawMetaObjectForType(enginePriv, value.userType());
        
        if (!valMo)
            return false;

        QObject *o = *(QObject **)value.constData();
        const QMetaObject *propMo = rawMetaObjectForType(enginePriv, propertyType);

        if (o) valMo = o->metaObject();

        if (canConvert(valMo, propMo)) {
            void *args[] = { &o, 0, &status, &flags };
            QMetaObject::metacall(object, QMetaObject::WriteProperty, coreIdx, 
                                  args);
        } else if (!o && canConvert(propMo, valMo)) {
            // In the case of a null QObject, we assign the null if there is 
            // any change that the null variant type could be up or down cast to 
            // the property type.
            void *args[] = { &o, 0, &status, &flags };
            QMetaObject::metacall(object, QMetaObject::WriteProperty, coreIdx, 
                                  args);
        } else {
            return false;
        }

    } else if (property.flags & QDeclarativePropertyCache::Data::IsQList) {

        const QMetaObject *listType = 0;
        if (enginePriv) {
            listType = enginePriv->rawMetaObjectForType(enginePriv->listType(property.propType));
        } else {
            QDeclarativeType *type = QDeclarativeMetaType::qmlType(QDeclarativeMetaType::listType(property.propType));
            if (!type) return false;
            listType = type->baseMetaObject();
        }
        if (!listType) return false;

        QDeclarativeListProperty<void> prop;
        void *args[] = { &prop, 0 };
        QMetaObject::metacall(object, QMetaObject::ReadProperty, coreIdx, args);

        if (!prop.clear) return false;

        prop.clear(&prop);

        if (value.userType() == qMetaTypeId<QList<QObject *> >()) {
            const QList<QObject *> &list = qvariant_cast<QList<QObject *> >(value);

            for (int ii = 0; ii < list.count(); ++ii) {
                QObject *o = list.at(ii);
                if (o && !canConvert(o->metaObject(), listType))
                    o = 0;
                prop.append(&prop, (void *)o);
            }
        } else {
            QObject *o = enginePriv?enginePriv->toQObject(value):QDeclarativeMetaType::toQObject(value);
            if (o && !canConvert(o->metaObject(), listType))
                o = 0;
            prop.append(&prop, (void *)o);
        }

    } else {
        Q_ASSERT(variantType != propertyType);

        QVariant v = value;
        if (v.convert((QVariant::Type)propertyType)) {
            void *a[] = { (void *)v.constData(), 0, &status, &flags};
            QMetaObject::metacall(object, QMetaObject::WriteProperty, coreIdx, a);
        } else if ((uint)propertyType >= QVariant::UserType && variantType == QVariant::String) {
            QDeclarativeMetaType::StringConverter con = QDeclarativeMetaType::customStringConverter(propertyType);
            if (!con)
                return false;

            QVariant v = con(value.toString());
            if (v.userType() == propertyType) {
                void *a[] = { (void *)v.constData(), 0, &status, &flags};
                QMetaObject::metacall(object, QMetaObject::WriteProperty, coreIdx, a);
            }
        } else if (variantType == QVariant::String) {
            bool ok = false;
            QVariant v = QDeclarativeStringConverters::variantFromString(value.toString(), propertyType, &ok);
            if (!ok)
                return false;

            void *a[] = { (void *)v.constData(), 0, &status, &flags};
            QMetaObject::metacall(object, QMetaObject::WriteProperty, coreIdx, a);
        } else {
            return false;
        }
    }

    return true;
}

const QMetaObject *QDeclarativePropertyPrivate::rawMetaObjectForType(QDeclarativeEnginePrivate *engine, int userType)
{
    if (engine) {
        return engine->rawMetaObjectForType(userType);
    } else {
        QDeclarativeType *type = QDeclarativeMetaType::qmlType(userType);
        return type?type->baseMetaObject():0;
    }
}

/*!
    Set the property value to \a value.
*/
bool QDeclarativeProperty::write(const QVariant &value) const
{
    return QDeclarativePropertyPrivate::write(*this, value, 0);
}

/*!
Writes \a value to the \a name property of \a object.  This method is equivalent to:
\code
    QDeclarativeProperty p(object, name);
    p.write(value);
\endcode
*/
bool QDeclarativeProperty::write(QObject *object, const QString &name, const QVariant &value)
{
    QDeclarativeProperty p(object, name);
    return p.write(value);
}

/*!
Writes \a value to the \a name property of \a object.  This method is equivalent to:
\code
    QDeclarativeProperty p(object, name, ctxt);
    p.write(value);
\endcode
*/
bool QDeclarativeProperty::write(QObject *object, const QString &name, const QVariant &value, 
                                 QDeclarativeContext *ctxt)
{
    QDeclarativeProperty p(object, name, ctxt);
    return p.write(value);
}

/*!
Writes \a value to the \a name property of \a object.  This method is equivalent to:
\code
    QDeclarativeProperty p(object, name, engine);
    p.write(value);
\endcode
*/
bool QDeclarativeProperty::write(QObject *object, const QString &name, const QVariant &value, 
                                 QDeclarativeEngine *engine)
{
    QDeclarativeProperty p(object, name, engine);
    return p.write(value);
}

/*!
    Resets the property value.
*/
bool QDeclarativeProperty::reset() const
{
    if (isResettable()) {
        void *args[] = { 0 };
        QMetaObject::metacall(d->object, QMetaObject::ResetProperty, d->core.coreIndex, args);
        return true;
    } else {
        return false;
    }
}

bool QDeclarativePropertyPrivate::write(const QDeclarativeProperty &that,
                                            const QVariant &value, WriteFlags flags) 
{
    if (that.d->object && that.type() & QDeclarativeProperty::Property && 
        that.d->core.isValid() && that.isWritable()) 
        return that.d->writeValueProperty(value, flags);
    else 
        return false;
}

/*!
    Returns true if the property has a change notifier signal, otherwise false.
*/
bool QDeclarativeProperty::hasNotifySignal() const
{
    if (type() & Property && d->object) {
        return d->object->metaObject()->property(d->core.coreIndex).hasNotifySignal();
    }
    return false;
}

/*!
    Returns true if the property needs a change notifier signal for bindings
    to remain upto date, false otherwise.

    Some properties, such as attached properties or those whose value never 
    changes, do not require a change notifier.
*/
bool QDeclarativeProperty::needsNotifySignal() const
{
    return type() & Property && !property().isConstant();
}

/*!
    Connects the property's change notifier signal to the
    specified \a method of the \a dest object and returns
    true. Returns false if this metaproperty does not
    represent a regular Qt property or if it has no
    change notifier signal, or if the \a dest object does
    not have the specified \a method.
*/
bool QDeclarativeProperty::connectNotifySignal(QObject *dest, int method) const
{
    if (!(type() & Property) || !d->object)
        return false;

    QMetaProperty prop = d->object->metaObject()->property(d->core.coreIndex);
    if (prop.hasNotifySignal()) {
        return QMetaObject::connect(d->object, prop.notifySignalIndex(), dest, method, Qt::DirectConnection);
    } else {
        return false;
    }
}

/*!
    Connects the property's change notifier signal to the
    specified \a slot of the \a dest object and returns
    true. Returns false if this metaproperty does not
    represent a regular Qt property or if it has no
    change notifier signal, or if the \a dest object does
    not have the specified \a slot.
*/
bool QDeclarativeProperty::connectNotifySignal(QObject *dest, const char *slot) const
{
    if (!(type() & Property) || !d->object)
        return false;

    QMetaProperty prop = d->object->metaObject()->property(d->core.coreIndex);
    if (prop.hasNotifySignal()) {
        QByteArray signal(QByteArray("2") + prop.notifySignal().signature());
        return QObject::connect(d->object, signal.constData(), dest, slot);
    } else  {
        return false;
    }
}

/*!
    Return the Qt metaobject index of the property.
*/
int QDeclarativeProperty::index() const
{
    return d->core.coreIndex;
}

int QDeclarativePropertyPrivate::valueTypeCoreIndex(const QDeclarativeProperty &that)
{
    return that.d->valueType.valueTypeCoreIdx;
}

struct SerializedData {
    bool isValueType;
    QDeclarativePropertyCache::Data core;
};

struct ValueTypeSerializedData : public SerializedData {
    QDeclarativePropertyCache::ValueTypeData valueType;
};

QByteArray QDeclarativePropertyPrivate::saveValueType(const QMetaObject *metaObject, int index, 
                                                 const QMetaObject *subObject, int subIndex)
{
    QMetaProperty prop = metaObject->property(index);
    QMetaProperty subProp = subObject->property(subIndex);

    ValueTypeSerializedData sd;
    sd.isValueType = true;
    sd.core.load(metaObject->property(index));
    sd.valueType.flags = QDeclarativePropertyCache::Data::flagsForProperty(subProp);
    sd.valueType.valueTypeCoreIdx = subIndex;
    sd.valueType.valueTypePropType = subProp.userType();

    QByteArray rv((const char *)&sd, sizeof(sd));

    return rv;
}

QByteArray QDeclarativePropertyPrivate::saveProperty(const QMetaObject *metaObject, int index)
{
    SerializedData sd;
    sd.isValueType = false;
    sd.core.load(metaObject->property(index));

    QByteArray rv((const char *)&sd, sizeof(sd));
    return rv;
}

QDeclarativeProperty 
QDeclarativePropertyPrivate::restore(const QByteArray &data, QObject *object, QDeclarativeContext *ctxt)
{
    QDeclarativeProperty prop;

    if (data.isEmpty())
        return prop;

    prop.d->object = object;
    prop.d->context = ctxt;
    prop.d->engine = ctxt?ctxt->engine():0;

    const SerializedData *sd = (const SerializedData *)data.constData();
    if (sd->isValueType) {
        const ValueTypeSerializedData *vt = (const ValueTypeSerializedData *)sd;
        prop.d->core = vt->core;
        prop.d->valueType = vt->valueType;
    } else {
        prop.d->core = sd->core;
    }

    return prop;
}

/*!
    Returns true if lhs and rhs refer to the same metaobject data
*/
bool QDeclarativePropertyPrivate::equal(const QMetaObject *lhs, const QMetaObject *rhs)
{
    return lhs == rhs || (1 && lhs && rhs && lhs->d.stringdata == rhs->d.stringdata);
}

/*!
    Returns true if from inherits to.
*/
bool QDeclarativePropertyPrivate::canConvert(const QMetaObject *from, const QMetaObject *to)
{
    if (from && to == &QObject::staticMetaObject)
        return true;

    while (from) {
        if (equal(from, to))
            return true;
        from = from->superClass();
    }
    
    return false;
}

QT_END_NAMESPACE