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
#include <qtest.h>
#include <QtDeclarative/qdeclarativeengine.h>
#include <QtDeclarative/qdeclarativecomponent.h>
#include <QtCore/qfile.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfileinfo.h>
#include <QtCore/qdir.h>

#include <private/qdeclarativeproperty_p.h>
#include <private/qdeclarativemetatype_p.h>

#include "testtypes.h"

#include "../../../shared/util.h"

/*
This test case covers QML language issues.  This covers everything that does not
involve evaluating ECMAScript expressions and bindings.

Evaluation of expressions and bindings is covered in qmlecmascript
*/
class tst_qdeclarativelanguage : public QObject
{
    Q_OBJECT
public:
    tst_qdeclarativelanguage() {
        QDeclarativeMetaType::registerCustomStringConverter(qMetaTypeId<MyCustomVariantType>(), myCustomVariantTypeConverter);
        QFileInfo fileInfo(__FILE__);
        engine.addImportPath(fileInfo.absoluteDir().filePath(QLatin1String("data/lib")));
    }

private slots:
    void initTestCase();
    void cleanupTestCase();

    void errors_data();
    void errors();

    void simpleObject();
    void simpleContainer();
    void interfaceProperty();
    void interfaceQList();
    void assignObjectToSignal();
    void assignObjectToVariant();
    void assignLiteralSignalProperty();
    void assignQmlComponent();
    void assignBasicTypes();
    void assignTypeExtremes();
    void assignCompositeToType();
    void customParserTypes();
    void rootAsQmlComponent();
    void inlineQmlComponents();
    void idProperty();
    void autoNotifyConnection();
    void assignSignal();
    void dynamicProperties();
    void dynamicPropertiesNested();
    void listProperties();
    void dynamicObjectProperties();
    void dynamicSignalsAndSlots();
    void simpleBindings();
    void autoComponentCreation();
    void propertyValueSource();
    void attachedProperties();
    void dynamicObjects();
    void customVariantTypes();
    void valueTypes();
    void cppnamespace();
    void aliasProperties();
    void componentCompositeType();
    void i18n();
    void i18n_data();
    void onCompleted();
    void scriptString();
    void defaultPropertyListOrder();
    void declaredPropertyValues();

    void importsBuiltin_data();
    void importsBuiltin();
    void importsLocal_data();
    void importsLocal();
    void importsRemote_data();
    void importsRemote();
    void importsInstalled_data();
    void importsInstalled();
    void importsOrder_data();
    void importsOrder();

    void qmlAttachedPropertiesObjectMethod();

    void customOnProperty();

    // regression tests for crashes
    void crash1();
    void crash2();

private:
    QDeclarativeEngine engine;
    void testType(const QString& qml, const QString& type);
};

#define VERIFY_ERRORS(errorfile) \
    if (!errorfile) { \
        if (qgetenv("DEBUG") != "" && !component.errors().isEmpty()) \
            qWarning() << "Unexpected Errors:" << component.errors(); \
        QVERIFY(!component.isError()); \
        QVERIFY(component.errors().isEmpty()); \
    } else { \
        QFile file(QLatin1String(SRCDIR) + QLatin1String("/data/") + QLatin1String(errorfile)); \
        QVERIFY(file.open(QIODevice::ReadOnly)); \
        QByteArray data = file.readAll(); \
        file.close(); \
        QList<QByteArray> expected = data.split('\n'); \
        expected.removeAll(QByteArray("")); \
        QList<QDeclarativeError> errors = component.errors(); \
        QList<QByteArray> actual; \
        for (int ii = 0; ii < errors.count(); ++ii) { \
            const QDeclarativeError &error = errors.at(ii); \
            QByteArray errorStr = QByteArray::number(error.line()) + ":" +  \
                                  QByteArray::number(error.column()) + ":" + \
                                  error.description().toUtf8(); \
            actual << errorStr; \
        } \
        if (qgetenv("DEBUG") != "" && expected != actual) \
            qWarning() << "Expected:" << expected << "Actual:" << actual;  \
        if (qgetenv("QDECLARATIVELANGUAGE_UPDATEERRORS") != "" && expected != actual) {\
            QFile file(QLatin1String("data/") + QLatin1String(errorfile)); \
            QVERIFY(file.open(QIODevice::WriteOnly)); \
            for (int ii = 0; ii < actual.count(); ++ii) { \
                file.write(actual.at(ii)); file.write("\n"); \
            } \
            file.close(); \
        } else { \
            QCOMPARE(expected, actual); \
        } \
    }

inline QUrl TEST_FILE(const QString &filename)
{
    QFileInfo fileInfo(__FILE__);
    return QUrl::fromLocalFile(fileInfo.absoluteDir().filePath(QLatin1String("data/") + filename));
}

inline QUrl TEST_FILE(const char *filename)
{
    return TEST_FILE(QLatin1String(filename));
}

void tst_qdeclarativelanguage::cleanupTestCase()
{
    QVERIFY(QFile::remove(TEST_FILE(QString::fromUtf8("I18nType\303\201\303\242\303\243\303\244\303\245.qml")).toLocalFile()));
}

void tst_qdeclarativelanguage::errors_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("errorFile");
    QTest::addColumn<bool>("create");

    QTest::newRow("nonexistantProperty.1") << "nonexistantProperty.1.qml" << "nonexistantProperty.1.errors.txt" << false;
    QTest::newRow("nonexistantProperty.2") << "nonexistantProperty.2.qml" << "nonexistantProperty.2.errors.txt" << false;
    QTest::newRow("nonexistantProperty.3") << "nonexistantProperty.3.qml" << "nonexistantProperty.3.errors.txt" << false;
    QTest::newRow("nonexistantProperty.4") << "nonexistantProperty.4.qml" << "nonexistantProperty.4.errors.txt" << false;
    QTest::newRow("nonexistantProperty.5") << "nonexistantProperty.5.qml" << "nonexistantProperty.5.errors.txt" << false;
    QTest::newRow("nonexistantProperty.6") << "nonexistantProperty.6.qml" << "nonexistantProperty.6.errors.txt" << false;

    QTest::newRow("wrongType (string for int)") << "wrongType.1.qml" << "wrongType.1.errors.txt" << false;
    QTest::newRow("wrongType (int for bool)") << "wrongType.2.qml" << "wrongType.2.errors.txt" << false;
    QTest::newRow("wrongType (bad rect)") << "wrongType.3.qml" << "wrongType.3.errors.txt" << false;

    QTest::newRow("wrongType (invalid enum)") << "wrongType.4.qml" << "wrongType.4.errors.txt" << false;
    QTest::newRow("wrongType (int for uint)") << "wrongType.5.qml" << "wrongType.5.errors.txt" << false;
    QTest::newRow("wrongType (string for real)") << "wrongType.6.qml" << "wrongType.6.errors.txt" << false;
    QTest::newRow("wrongType (int for color)") << "wrongType.7.qml" << "wrongType.7.errors.txt" << false;
    QTest::newRow("wrongType (int for date)") << "wrongType.8.qml" << "wrongType.8.errors.txt" << false;
    QTest::newRow("wrongType (int for time)") << "wrongType.9.qml" << "wrongType.9.errors.txt" << false;
    QTest::newRow("wrongType (int for datetime)") << "wrongType.10.qml" << "wrongType.10.errors.txt" << false;
    QTest::newRow("wrongType (string for point)") << "wrongType.11.qml" << "wrongType.11.errors.txt" << false;
    QTest::newRow("wrongType (color for size)") << "wrongType.12.qml" << "wrongType.12.errors.txt" << false;
    QTest::newRow("wrongType (number string for int)") << "wrongType.13.qml" << "wrongType.13.errors.txt" << false;
    QTest::newRow("wrongType (int for string)") << "wrongType.14.qml" << "wrongType.14.errors.txt" << false;
    QTest::newRow("wrongType (int for url)") << "wrongType.15.qml" << "wrongType.15.errors.txt" << false;

    QTest::newRow("readOnly.1") << "readOnly.1.qml" << "readOnly.1.errors.txt" << false;
    QTest::newRow("readOnly.2") << "readOnly.2.qml" << "readOnly.2.errors.txt" << false;
    QTest::newRow("readOnly.3") << "readOnly.3.qml" << "readOnly.3.errors.txt" << false;

    QTest::newRow("listAssignment.2") << "listAssignment.2.qml" << "listAssignment.2.errors.txt" << false;
    QTest::newRow("listAssignment.3") << "listAssignment.3.qml" << "listAssignment.3.errors.txt" << false;

    QTest::newRow("invalidID.1") << "invalidID.qml" << "invalidID.errors.txt" << false;
    QTest::newRow("invalidID.2") << "invalidID.2.qml" << "invalidID.2.errors.txt" << false;
    QTest::newRow("invalidID.3") << "invalidID.3.qml" << "invalidID.3.errors.txt" << false;
    QTest::newRow("invalidID.4") << "invalidID.4.qml" << "invalidID.4.errors.txt" << false;
    QTest::newRow("invalidID.5") << "invalidID.5.qml" << "invalidID.5.errors.txt" << false;
    QTest::newRow("invalidID.6") << "invalidID.6.qml" << "invalidID.6.errors.txt" << false;
    QTest::newRow("invalidID.7") << "invalidID.7.qml" << "invalidID.7.errors.txt" << false;
    QTest::newRow("invalidID.8") << "invalidID.8.qml" << "invalidID.8.errors.txt" << false;
    QTest::newRow("invalidID.9") << "invalidID.9.qml" << "invalidID.9.errors.txt" << false;

    QTest::newRow("unsupportedProperty") << "unsupportedProperty.qml" << "unsupportedProperty.errors.txt" << false;
    QTest::newRow("nullDotProperty") << "nullDotProperty.qml" << "nullDotProperty.errors.txt" << true;
    QTest::newRow("fakeDotProperty") << "fakeDotProperty.qml" << "fakeDotProperty.errors.txt" << false;
    QTest::newRow("duplicateIDs") << "duplicateIDs.qml" << "duplicateIDs.errors.txt" << false;
    QTest::newRow("unregisteredObject") << "unregisteredObject.qml" << "unregisteredObject.errors.txt" << false;
    QTest::newRow("empty") << "empty.qml" << "empty.errors.txt" << false;
    QTest::newRow("missingObject") << "missingObject.qml" << "missingObject.errors.txt" << false;
    QTest::newRow("failingComponent") << "failingComponentTest.qml" << "failingComponent.errors.txt" << false;
    QTest::newRow("missingSignal") << "missingSignal.qml" << "missingSignal.errors.txt" << false;
    QTest::newRow("finalOverride") << "finalOverride.qml" << "finalOverride.errors.txt" << false;
    QTest::newRow("customParserIdNotAllowed") << "customParserIdNotAllowed.qml" << "customParserIdNotAllowed.errors.txt" << false;

    QTest::newRow("invalidGroupedProperty.1") << "invalidGroupedProperty.1.qml" << "invalidGroupedProperty.1.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.2") << "invalidGroupedProperty.2.qml" << "invalidGroupedProperty.2.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.3") << "invalidGroupedProperty.3.qml" << "invalidGroupedProperty.3.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.4") << "invalidGroupedProperty.4.qml" << "invalidGroupedProperty.4.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.5") << "invalidGroupedProperty.5.qml" << "invalidGroupedProperty.5.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.6") << "invalidGroupedProperty.6.qml" << "invalidGroupedProperty.6.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.7") << "invalidGroupedProperty.7.qml" << "invalidGroupedProperty.7.errors.txt" << true;
    QTest::newRow("invalidGroupedProperty.8") << "invalidGroupedProperty.8.qml" << "invalidGroupedProperty.8.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.9") << "invalidGroupedProperty.9.qml" << "invalidGroupedProperty.9.errors.txt" << false;
    QTest::newRow("invalidGroupedProperty.10") << "invalidGroupedProperty.10.qml" << "invalidGroupedProperty.10.errors.txt" << false;

    QTest::newRow("importNamespaceConflict") << "importNamespaceConflict.qml" << "importNamespaceConflict.errors.txt" << false;
    QTest::newRow("importVersionMissing (builtin)") << "importVersionMissingBuiltIn.qml" << "importVersionMissingBuiltIn.errors.txt" << false;
    QTest::newRow("importVersionMissing (installed)") << "importVersionMissingInstalled.qml" << "importVersionMissingInstalled.errors.txt" << false;
    QTest::newRow("importNonExist (installed)") << "importNonExist.qml" << "importNonExist.errors.txt" << false;
    QTest::newRow("importNewerVersion (installed)") << "importNewerVersion.qml" << "importNewerVersion.errors.txt" << false;
    QTest::newRow("invalidImportID") << "invalidImportID.qml" << "invalidImportID.errors.txt" << false;

    QTest::newRow("signal.1") << "signal.1.qml" << "signal.1.errors.txt" << false;
    QTest::newRow("signal.2") << "signal.2.qml" << "signal.2.errors.txt" << false;
    QTest::newRow("signal.3") << "signal.3.qml" << "signal.3.errors.txt" << false;
    QTest::newRow("signal.4") << "signal.4.qml" << "signal.4.errors.txt" << false;

    QTest::newRow("method.1") << "method.1.qml" << "method.1.errors.txt" << false;

    QTest::newRow("property.1") << "property.1.qml" << "property.1.errors.txt" << false;
    QTest::newRow("property.2") << "property.2.qml" << "property.2.errors.txt" << false;
    QTest::newRow("property.3") << "property.3.qml" << "property.3.errors.txt" << false;
    QTest::newRow("property.4") << "property.4.qml" << "property.4.errors.txt" << false;
    QTest::newRow("property.5") << "property.5.qml" << "property.5.errors.txt" << false;
    QTest::newRow("property.6") << "property.6.qml" << "property.6.errors.txt" << false;
    QTest::newRow("property.7") << "property.7.qml" << "property.7.errors.txt" << false;

    QTest::newRow("Script.1") << "script.1.qml" << "script.1.errors.txt" << false;
    QTest::newRow("Script.2") << "script.2.qml" << "script.2.errors.txt" << false;
    QTest::newRow("Script.3") << "script.3.qml" << "script.3.errors.txt" << false;
    QTest::newRow("Script.4") << "script.4.qml" << "script.4.errors.txt" << false;
    QTest::newRow("Script.5") << "script.5.qml" << "script.5.errors.txt" << false;
    QTest::newRow("Script.6") << "script.6.qml" << "script.6.errors.txt" << false;
    QTest::newRow("Script.7") << "script.7.qml" << "script.7.errors.txt" << false;
    QTest::newRow("Script.8") << "script.8.qml" << "script.8.errors.txt" << false;
    QTest::newRow("Script.9") << "script.9.qml" << "script.9.errors.txt" << false;
    QTest::newRow("Script.10") << "script.10.qml" << "script.10.errors.txt" << false;
    QTest::newRow("Script.11") << "script.11.qml" << "script.11.errors.txt" << false;
    QTest::newRow("Script.12") << "script.12.qml" << "script.12.errors.txt" << false;

    QTest::newRow("Component.1") << "component.1.qml" << "component.1.errors.txt" << false;
    QTest::newRow("Component.2") << "component.2.qml" << "component.2.errors.txt" << false;
    QTest::newRow("Component.3") << "component.3.qml" << "component.3.errors.txt" << false;
    QTest::newRow("Component.4") << "component.4.qml" << "component.4.errors.txt" << false;
    QTest::newRow("Component.5") << "component.5.qml" << "component.5.errors.txt" << false;
    QTest::newRow("Component.6") << "component.6.qml" << "component.6.errors.txt" << false;

    QTest::newRow("MultiSet.1") << "multiSet.1.qml" << "multiSet.1.errors.txt" << false;
    QTest::newRow("MultiSet.2") << "multiSet.2.qml" << "multiSet.2.errors.txt" << false;
    QTest::newRow("MultiSet.3") << "multiSet.3.qml" << "multiSet.3.errors.txt" << false;
    QTest::newRow("MultiSet.4") << "multiSet.4.qml" << "multiSet.4.errors.txt" << false;
    QTest::newRow("MultiSet.5") << "multiSet.5.qml" << "multiSet.5.errors.txt" << false;
    QTest::newRow("MultiSet.6") << "multiSet.6.qml" << "multiSet.6.errors.txt" << false;
    QTest::newRow("MultiSet.7") << "multiSet.7.qml" << "multiSet.7.errors.txt" << false;
    QTest::newRow("MultiSet.8") << "multiSet.8.qml" << "multiSet.8.errors.txt" << false;
    QTest::newRow("MultiSet.9") << "multiSet.9.qml" << "multiSet.9.errors.txt" << false;
    QTest::newRow("MultiSet.10") << "multiSet.10.qml" << "multiSet.10.errors.txt" << false;

    QTest::newRow("invalidAttachedProperty.1") << "invalidAttachedProperty.1.qml" << "invalidAttachedProperty.1.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.2") << "invalidAttachedProperty.2.qml" << "invalidAttachedProperty.2.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.3") << "invalidAttachedProperty.3.qml" << "invalidAttachedProperty.3.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.4") << "invalidAttachedProperty.4.qml" << "invalidAttachedProperty.4.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.5") << "invalidAttachedProperty.5.qml" << "invalidAttachedProperty.5.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.6") << "invalidAttachedProperty.6.qml" << "invalidAttachedProperty.6.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.7") << "invalidAttachedProperty.7.qml" << "invalidAttachedProperty.7.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.8") << "invalidAttachedProperty.8.qml" << "invalidAttachedProperty.8.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.9") << "invalidAttachedProperty.9.qml" << "invalidAttachedProperty.9.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.10") << "invalidAttachedProperty.10.qml" << "invalidAttachedProperty.10.errors.txt" << false;
    QTest::newRow("invalidAttachedProperty.11") << "invalidAttachedProperty.11.qml" << "invalidAttachedProperty.11.errors.txt" << false;

    QTest::newRow("emptySignal") << "emptySignal.qml" << "emptySignal.errors.txt" << false;
    QTest::newRow("emptySignal.2") << "emptySignal.2.qml" << "emptySignal.2.errors.txt" << false;

    QTest::newRow("nestedErrors") << "nestedErrors.qml" << "nestedErrors.errors.txt" << false;
    QTest::newRow("defaultGrouped") << "defaultGrouped.qml" << "defaultGrouped.errors.txt" << false;
    QTest::newRow("doubleSignal") << "doubleSignal.qml" << "doubleSignal.errors.txt" << false;
    QTest::newRow("invalidRoot") << "invalidRoot.qml" << "invalidRoot.errors.txt" << false;
    QTest::newRow("missingValueTypeProperty") << "missingValueTypeProperty.qml" << "missingValueTypeProperty.errors.txt" << false;
    QTest::newRow("objectValueTypeProperty") << "objectValueTypeProperty.qml" << "objectValueTypeProperty.errors.txt" << false;
    QTest::newRow("enumTypes") << "enumTypes.qml" << "enumTypes.errors.txt" << false;
}


void tst_qdeclarativelanguage::errors()
{
    QFETCH(QString, file);
    QFETCH(QString, errorFile);
    QFETCH(bool, create);

    QDeclarativeComponent component(&engine, TEST_FILE(file));

    if(create) {
        QObject *object = component.create();
        QVERIFY(object == 0);
    }

    VERIFY_ERRORS(errorFile.toLatin1().constData());
}

void tst_qdeclarativelanguage::simpleObject()
{
    QDeclarativeComponent component(&engine, TEST_FILE("simpleObject.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
}

void tst_qdeclarativelanguage::simpleContainer()
{
    QDeclarativeComponent component(&engine, TEST_FILE("simpleContainer.qml"));
    VERIFY_ERRORS(0);
    MyContainer *container= qobject_cast<MyContainer*>(component.create());
    QVERIFY(container != 0);
    QCOMPARE(container->getChildren()->count(),2);
}

void tst_qdeclarativelanguage::interfaceProperty()
{
    QDeclarativeComponent component(&engine, TEST_FILE("interfaceProperty.qml"));
    VERIFY_ERRORS(0);
    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(object != 0);
    QVERIFY(object->interface());
    QVERIFY(object->interface()->id == 913);
}

void tst_qdeclarativelanguage::interfaceQList()
{
    QDeclarativeComponent component(&engine, TEST_FILE("interfaceQList.qml"));
    VERIFY_ERRORS(0);
    MyContainer *container= qobject_cast<MyContainer*>(component.create());
    QVERIFY(container != 0);
    QVERIFY(container->getQListInterfaces()->count() == 2);
    for(int ii = 0; ii < 2; ++ii)
        QVERIFY(container->getQListInterfaces()->at(ii)->id == 913);
}

void tst_qdeclarativelanguage::assignObjectToSignal()
{
    QDeclarativeComponent component(&engine, TEST_FILE("assignObjectToSignal.qml"));
    VERIFY_ERRORS(0);
    MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(object != 0);
    QTest::ignoreMessage(QtWarningMsg, "MyQmlObject::basicSlot");
    emit object->basicSignal();
}

void tst_qdeclarativelanguage::assignObjectToVariant()
{
    QDeclarativeComponent component(&engine, TEST_FILE("assignObjectToVariant.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
    QVariant v = object->property("a");
    QVERIFY(v.userType() == qMetaTypeId<QObject *>());
}

void tst_qdeclarativelanguage::assignLiteralSignalProperty()
{
    QDeclarativeComponent component(&engine, TEST_FILE("assignLiteralSignalProperty.qml"));
    VERIFY_ERRORS(0);
    MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->onLiteralSignal(), 10);
}

// Test is an external component can be loaded and assigned (to a qlist)
void tst_qdeclarativelanguage::assignQmlComponent()
{
    QDeclarativeComponent component(&engine, TEST_FILE("assignQmlComponent.qml"));
    VERIFY_ERRORS(0);
    MyContainer *object = qobject_cast<MyContainer *>(component.create());
    QVERIFY(object != 0);
    QVERIFY(object->getChildren()->count() == 1);
    QObject *child = object->getChildren()->at(0);
    QCOMPARE(child->property("x"), QVariant(10));
    QCOMPARE(child->property("y"), QVariant(11));
}

// Test literal assignment to all the basic types 
void tst_qdeclarativelanguage::assignBasicTypes()
{
    QDeclarativeComponent component(&engine, TEST_FILE("assignBasicTypes.qml"));
    VERIFY_ERRORS(0);
    MyTypeObject *object = qobject_cast<MyTypeObject *>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->flagProperty(), MyTypeObject::FlagVal1 | MyTypeObject::FlagVal3);
    QCOMPARE(object->enumProperty(), MyTypeObject::EnumVal2);
    QCOMPARE(object->stringProperty(), QString("Hello World!"));
    QCOMPARE(object->uintProperty(), uint(10));
    QCOMPARE(object->intProperty(), -19);
    QCOMPARE((float)object->realProperty(), float(23.2));
    QCOMPARE((float)object->doubleProperty(), float(-19.7));
    QCOMPARE((float)object->floatProperty(), float(8.5));
    QCOMPARE(object->colorProperty(), QColor("red"));
    QCOMPARE(object->dateProperty(), QDate(1982, 11, 25));
    QCOMPARE(object->timeProperty(), QTime(11, 11, 32));
    QCOMPARE(object->dateTimeProperty(), QDateTime(QDate(2009, 5, 12), QTime(13, 22, 1)));
    QCOMPARE(object->pointProperty(), QPoint(99,13));
    QCOMPARE(object->pointFProperty(), QPointF((float)-10.1, (float)12.3));
    QCOMPARE(object->sizeProperty(), QSize(99, 13));
    QCOMPARE(object->sizeFProperty(), QSizeF((float)0.1, (float)0.2));
    QCOMPARE(object->rectProperty(), QRect(9, 7, 100, 200));
    QCOMPARE(object->rectFProperty(), QRectF((float)1000.1, (float)-10.9, (float)400, (float)90.99));
    QCOMPARE(object->boolProperty(), true);
    QCOMPARE(object->variantProperty(), QVariant("Hello World!"));
    QCOMPARE(object->vectorProperty(), QVector3D(10, 1, 2.2));
    QCOMPARE(object->urlProperty(), component.url().resolved(QUrl("main.qml")));
    QVERIFY(object->objectProperty() != 0);
    MyTypeObject *child = qobject_cast<MyTypeObject *>(object->objectProperty());
    QVERIFY(child != 0);
    QCOMPARE(child->intProperty(), 8);
}

// Test edge case type assignments
void tst_qdeclarativelanguage::assignTypeExtremes()
{
    QDeclarativeComponent component(&engine, TEST_FILE("assignTypeExtremes.qml"));
    VERIFY_ERRORS(0);
    MyTypeObject *object = qobject_cast<MyTypeObject *>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->uintProperty(), 0xEE6B2800);
    QCOMPARE(object->intProperty(), -0x77359400);
}

// Test that a composite type can assign to a property of its base type
void tst_qdeclarativelanguage::assignCompositeToType()
{
    QDeclarativeComponent component(&engine, TEST_FILE("assignCompositeToType.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
}

// Tests that custom parser types can be instantiated
void tst_qdeclarativelanguage::customParserTypes()
{
    QDeclarativeComponent component(&engine, TEST_FILE("customParserTypes.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
    QVERIFY(object->property("count") == QVariant(2));
}

// Tests that the root item can be a custom component
void tst_qdeclarativelanguage::rootAsQmlComponent()
{
    QDeclarativeComponent component(&engine, TEST_FILE("rootAsQmlComponent.qml"));
    VERIFY_ERRORS(0);
    MyContainer *object = qobject_cast<MyContainer *>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->property("x"), QVariant(11));
    QCOMPARE(object->getChildren()->count(), 2);
}

// Tests that components can be specified inline
void tst_qdeclarativelanguage::inlineQmlComponents()
{
    QDeclarativeComponent component(&engine, TEST_FILE("inlineQmlComponents.qml"));
    VERIFY_ERRORS(0);
    MyContainer *object = qobject_cast<MyContainer *>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->getChildren()->count(), 1);
    QDeclarativeComponent *comp = qobject_cast<QDeclarativeComponent *>(object->getChildren()->at(0));
    QVERIFY(comp != 0);
    MyQmlObject *compObject = qobject_cast<MyQmlObject *>(comp->create());
    QVERIFY(compObject != 0);
    QCOMPARE(compObject->value(), 11);
}

// Tests that types that have an id property have it set
void tst_qdeclarativelanguage::idProperty()
{
    QDeclarativeComponent component(&engine, TEST_FILE("idProperty.qml"));
    VERIFY_ERRORS(0);
    MyContainer *object = qobject_cast<MyContainer *>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->getChildren()->count(), 1);
    MyTypeObject *child = 
        qobject_cast<MyTypeObject *>(object->getChildren()->at(0));
    QVERIFY(child != 0);
    QCOMPARE(child->id(), QString("myObjectId"));
    QCOMPARE(object->property("object"), QVariant::fromValue((QObject *)child));
}

// Tests automatic connection to notify signals if "onBlahChanged" syntax is used
// even if the notify signal for "blah" is not called "blahChanged"
void tst_qdeclarativelanguage::autoNotifyConnection()
{
    QDeclarativeComponent component(&engine, TEST_FILE("autoNotifyConnection.qml"));
    VERIFY_ERRORS(0);
    MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(object != 0);
    QMetaProperty prop = object->metaObject()->property(object->metaObject()->indexOfProperty("receivedNotify"));
    QVERIFY(prop.isValid());

    QCOMPARE(prop.read(object), QVariant::fromValue(false));
    object->setPropertyWithNotify(1);
    QCOMPARE(prop.read(object), QVariant::fromValue(true));
}

// Tests that signals can be assigned to
void tst_qdeclarativelanguage::assignSignal()
{
    QDeclarativeComponent component(&engine, TEST_FILE("assignSignal.qml"));
    VERIFY_ERRORS(0);
    MyQmlObject *object = qobject_cast<MyQmlObject *>(component.create());
    QVERIFY(object != 0);
    QTest::ignoreMessage(QtWarningMsg, "MyQmlObject::basicSlot");
    emit object->basicSignal();
    QTest::ignoreMessage(QtWarningMsg, "MyQmlObject::basicSlotWithArgs(9)");
    emit object->basicParameterizedSignal(9);
}

// Tests the creation and assignment of dynamic properties
void tst_qdeclarativelanguage::dynamicProperties()
{
    QDeclarativeComponent component(&engine, TEST_FILE("dynamicProperties.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
    QCOMPARE(object->property("intProperty"), QVariant(10));
    QCOMPARE(object->property("boolProperty"), QVariant(false));
    QCOMPARE(object->property("doubleProperty"), QVariant(-10.1));
    QCOMPARE(object->property("realProperty"), QVariant((qreal)-19.9));
    QCOMPARE(object->property("stringProperty"), QVariant("Hello World!"));
    QCOMPARE(object->property("urlProperty"), QVariant(TEST_FILE("main.qml")));
    QCOMPARE(object->property("colorProperty"), QVariant(QColor("red")));
    QCOMPARE(object->property("dateProperty"), QVariant(QDate(1945, 9, 2)));
    QCOMPARE(object->property("varProperty"), QVariant("Hello World!"));
}

// Test that nested types can use dynamic properties
void tst_qdeclarativelanguage::dynamicPropertiesNested()
{
    QDeclarativeComponent component(&engine, TEST_FILE("dynamicPropertiesNested.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);

    QCOMPARE(object->property("super_a").toInt(), 11); // Overridden
    QCOMPARE(object->property("super_c").toInt(), 14); // Inherited
    QCOMPARE(object->property("a").toInt(), 13); // New
    QCOMPARE(object->property("b").toInt(), 12); // New

    delete object;
}

// Tests the creation and assignment to dynamic list properties
void tst_qdeclarativelanguage::listProperties()
{
    QDeclarativeComponent component(&engine, TEST_FILE("listProperties.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);

    QCOMPARE(object->property("test").toInt(), 2);
}

// Tests the creation and assignment of dynamic object properties
// ### Not complete
void tst_qdeclarativelanguage::dynamicObjectProperties()
{
    QDeclarativeComponent component(&engine, TEST_FILE("dynamicObjectProperties.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);

    QVERIFY(object->property("objectProperty") == qVariantFromValue((QObject*)0));
    QVERIFY(object->property("objectProperty2") != qVariantFromValue((QObject*)0));
}

// Tests the declaration of dynamic signals and slots
void tst_qdeclarativelanguage::dynamicSignalsAndSlots()
{
    QTest::ignoreMessage(QtDebugMsg, "1921");

    QDeclarativeComponent component(&engine, TEST_FILE("dynamicSignalsAndSlots.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
    QVERIFY(object->metaObject()->indexOfMethod("signal1()") != -1);
    QVERIFY(object->metaObject()->indexOfMethod("signal2()") != -1);
    QVERIFY(object->metaObject()->indexOfMethod("slot1()") != -1);
    QVERIFY(object->metaObject()->indexOfMethod("slot2()") != -1);

    QCOMPARE(object->property("test").toInt(), 0);
    QMetaObject::invokeMethod(object, "slot3", Qt::DirectConnection, Q_ARG(QVariant, QVariant(10)));
    QCOMPARE(object->property("test").toInt(), 10);
}

void tst_qdeclarativelanguage::simpleBindings()
{
    QDeclarativeComponent component(&engine, TEST_FILE("simpleBindings.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
    QCOMPARE(object->property("value1"), QVariant(10));
    QCOMPARE(object->property("value2"), QVariant(10));
    QCOMPARE(object->property("value3"), QVariant(21));
    QCOMPARE(object->property("value4"), QVariant(10));
    QCOMPARE(object->property("objectProperty"), QVariant::fromValue(object));
}

void tst_qdeclarativelanguage::autoComponentCreation()
{
    QDeclarativeComponent component(&engine, TEST_FILE("autoComponentCreation.qml"));
    VERIFY_ERRORS(0);
    MyTypeObject *object = qobject_cast<MyTypeObject *>(component.create());
    QVERIFY(object != 0);
    QVERIFY(object->componentProperty() != 0);
    MyTypeObject *child = qobject_cast<MyTypeObject *>(object->componentProperty()->create());
    QVERIFY(child != 0);
    QCOMPARE(child->realProperty(), qreal(9));
}

void tst_qdeclarativelanguage::propertyValueSource()
{
    {
    QDeclarativeComponent component(&engine, TEST_FILE("propertyValueSource.qml"));
    VERIFY_ERRORS(0);
    MyTypeObject *object = qobject_cast<MyTypeObject *>(component.create());
    QVERIFY(object != 0);

    QList<QObject *> valueSources;
    QObjectList allChildren = object->findChildren<QObject*>();
    foreach (QObject *child, allChildren) {
        if (qobject_cast<QDeclarativePropertyValueSource *>(child)) 
            valueSources.append(child);
    }

    QCOMPARE(valueSources.count(), 1);
    MyPropertyValueSource *valueSource = 
        qobject_cast<MyPropertyValueSource *>(valueSources.at(0));
    QVERIFY(valueSource != 0);
    QCOMPARE(valueSource->prop.object(), qobject_cast<QObject*>(object));
    QCOMPARE(valueSource->prop.name(), QString(QLatin1String("intProperty")));
    }

    {
    QDeclarativeComponent component(&engine, TEST_FILE("propertyValueSource.2.qml"));
    VERIFY_ERRORS(0);
    MyTypeObject *object = qobject_cast<MyTypeObject *>(component.create());
    QVERIFY(object != 0);

    QList<QObject *> valueSources;
    QObjectList allChildren = object->findChildren<QObject*>();
    foreach (QObject *child, allChildren) {
        if (qobject_cast<QDeclarativePropertyValueSource *>(child)) 
            valueSources.append(child);
    }

    QCOMPARE(valueSources.count(), 1);
    MyPropertyValueSource *valueSource = 
        qobject_cast<MyPropertyValueSource *>(valueSources.at(0));
    QVERIFY(valueSource != 0);
    QCOMPARE(valueSource->prop.object(), qobject_cast<QObject*>(object));
    QCOMPARE(valueSource->prop.name(), QString(QLatin1String("intProperty")));
    }
}

void tst_qdeclarativelanguage::attachedProperties()
{
    QDeclarativeComponent component(&engine, TEST_FILE("attachedProperties.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
    QObject *attached = qmlAttachedPropertiesObject<MyQmlObject>(object);
    QVERIFY(attached != 0);
    QCOMPARE(attached->property("value"), QVariant(10));
    QCOMPARE(attached->property("value2"), QVariant(13));
}

// Tests non-static object properties
void tst_qdeclarativelanguage::dynamicObjects()
{
    QDeclarativeComponent component(&engine, TEST_FILE("dynamicObject.1.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
}

// Tests the registration of custom variant string converters
void tst_qdeclarativelanguage::customVariantTypes()
{
    QDeclarativeComponent component(&engine, TEST_FILE("customVariantTypes.qml"));
    VERIFY_ERRORS(0);
    MyQmlObject *object = qobject_cast<MyQmlObject*>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->customType().a, 10);
}

void tst_qdeclarativelanguage::valueTypes()
{
    QDeclarativeComponent component(&engine, TEST_FILE("valueTypes.qml"));
    VERIFY_ERRORS(0);

    QString message = QLatin1String("QML MyTypeObject (") + component.url().toString() + 
                      QLatin1String(":2:1) Binding loop detected for property \"rectProperty.width\"");
    QTest::ignoreMessage(QtWarningMsg, qPrintable(message));
    QTest::ignoreMessage(QtWarningMsg, qPrintable(message));

    MyTypeObject *object = qobject_cast<MyTypeObject*>(component.create());
    QVERIFY(object != 0);


    QCOMPARE(object->rectProperty(), QRect(10, 11, 12, 13));
    QCOMPARE(object->rectProperty2(), QRect(10, 11, 12, 13));
    QCOMPARE(object->intProperty(), 10);
    object->doAction();
    QCOMPARE(object->rectProperty(), QRect(12, 11, 14, 13));
    QCOMPARE(object->rectProperty2(), QRect(12, 11, 14, 13));
    QCOMPARE(object->intProperty(), 12);

    // ###
#if 0
    QDeclarativeProperty p(object, "rectProperty.x");
    QCOMPARE(p.read(), QVariant(12));
    p.write(13);
    QCOMPARE(p.read(), QVariant(13));

    quint32 r = QDeclarativePropertyPrivate::saveValueType(p.coreIndex(), p.valueTypeCoreIndex());
    QDeclarativeProperty p2;
    QDeclarativePropertyPrivate::restore(p2, r, object);
    QCOMPARE(p2.read(), QVariant(13));
#endif
}

void tst_qdeclarativelanguage::cppnamespace()
{
    {
        QDeclarativeComponent component(&engine, TEST_FILE("cppnamespace.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);
        delete object;
    }

    {
        QDeclarativeComponent component(&engine, TEST_FILE("cppnamespace.2.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);
        delete object;
    }
}

void tst_qdeclarativelanguage::aliasProperties()
{
    // Simple "int" alias
    {
        QDeclarativeComponent component(&engine, TEST_FILE("alias.1.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        // Read through alias
        QCOMPARE(object->property("valueAlias").toInt(), 10);
        object->setProperty("value", QVariant(13));
        QCOMPARE(object->property("valueAlias").toInt(), 13);

        // Write throught alias
        object->setProperty("valueAlias", QVariant(19));
        QCOMPARE(object->property("valueAlias").toInt(), 19);
        QCOMPARE(object->property("value").toInt(), 19);

        delete object;
    }

    // Complex object alias
    {
        QDeclarativeComponent component(&engine, TEST_FILE("alias.2.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        // Read through alias
        MyQmlObject *v = 
            qvariant_cast<MyQmlObject *>(object->property("aliasObject"));
        QVERIFY(v != 0);
        QCOMPARE(v->value(), 10);

        // Write through alias
        MyQmlObject *v2 = new MyQmlObject();
        v2->setParent(object);
        object->setProperty("aliasObject", qVariantFromValue(v2));
        MyQmlObject *v3 = 
            qvariant_cast<MyQmlObject *>(object->property("aliasObject"));
        QVERIFY(v3 != 0);
        QCOMPARE(v3, v2);

        delete object;
    }

    // Nested aliases
    {
        QDeclarativeComponent component(&engine, TEST_FILE("alias.3.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QCOMPARE(object->property("value").toInt(), 1892);
        QCOMPARE(object->property("value2").toInt(), 1892);

        object->setProperty("value", QVariant(1313));
        QCOMPARE(object->property("value").toInt(), 1313);
        QCOMPARE(object->property("value2").toInt(), 1313);

        object->setProperty("value2", QVariant(8080));
        QCOMPARE(object->property("value").toInt(), 8080);
        QCOMPARE(object->property("value2").toInt(), 8080);

        delete object;
    }

    // Enum aliases
    {
        QDeclarativeComponent component(&engine, TEST_FILE("alias.4.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QCOMPARE(object->property("enumAlias").toInt(), 1);

        delete object;
    }

    // Id aliases
    {
        QDeclarativeComponent component(&engine, TEST_FILE("alias.5.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QVariant v = object->property("otherAlias");
        QCOMPARE(v.userType(), qMetaTypeId<MyQmlObject*>());
        MyQmlObject *o = qvariant_cast<MyQmlObject*>(v);
        QCOMPARE(o->value(), 10);

        delete o;

        v = object->property("otherAlias");
        QCOMPARE(v.userType(), qMetaTypeId<MyQmlObject*>());
        o = qvariant_cast<MyQmlObject*>(v);
        QVERIFY(o == 0);

        delete object;
    }

    // Nested aliases - this used to cause a crash
    {
        QDeclarativeComponent component(&engine, TEST_FILE("alias.6.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QCOMPARE(object->property("a").toInt(), 1923);
    }

    // Ptr Alias Cleanup - check that aliases to ptr types return 0 
    // if the object aliased to is removed
    {
        QDeclarativeComponent component(&engine, TEST_FILE("alias.7.qml"));
        VERIFY_ERRORS(0);

        QObject *object = component.create();
        QVERIFY(object != 0);

        QObject *object1 = qvariant_cast<QObject *>(object->property("object"));
        QVERIFY(object1 != 0);
        QObject *object2 = qvariant_cast<QObject *>(object1->property("object"));
        QVERIFY(object2 != 0);

        QObject *alias = qvariant_cast<QObject *>(object->property("aliasedObject"));
        QVERIFY(alias == object2);

        delete object1;

        QObject *alias2 = object; // "Random" start value
        int status = -1;
        void *a[] = { &alias2, 0, &status };
        QMetaObject::metacall(object, QMetaObject::ReadProperty,
                              object->metaObject()->indexOfProperty("aliasedObject"), a);
        QVERIFY(alias2 == 0);
    }

    // Simple composite type
    {
        QDeclarativeComponent component(&engine, TEST_FILE("alias.8.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QCOMPARE(object->property("value").toInt(), 10);

        delete object;
    }

    // Complex composite type
    {
        QDeclarativeComponent component(&engine, TEST_FILE("alias.9.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QCOMPARE(object->property("value").toInt(), 10);

        delete object;
    }
}

// Test that the root element in a composite type can be a Component
void tst_qdeclarativelanguage::componentCompositeType()
{
    QDeclarativeComponent component(&engine, TEST_FILE("componentCompositeType.qml"));
    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);
}

class TestType : public QObject {
    Q_OBJECT
public:
    TestType(QObject *p=0) : QObject(p) {}
};

class TestType2 : public QObject {
    Q_OBJECT
public:
    TestType2(QObject *p=0) : QObject(p) {}
};

void tst_qdeclarativelanguage::i18n_data()
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("stringProperty");
    QTest::newRow("i18nStrings") << "i18nStrings.qml" << QString::fromUtf8("Test \303\241\303\242\303\243\303\244\303\245 (5 accented 'a' letters)");
    QTest::newRow("i18nDeclaredPropertyNames") << "i18nDeclaredPropertyNames.qml" << QString::fromUtf8("Test \303\241\303\242\303\243\303\244\303\245: 10");
    QTest::newRow("i18nDeclaredPropertyUse") << "i18nDeclaredPropertyUse.qml" << QString::fromUtf8("Test \303\241\303\242\303\243\303\244\303\245: 15");
    QTest::newRow("i18nScript") << "i18nScript.qml" << QString::fromUtf8("Test \303\241\303\242\303\243\303\244\303\245: 20");
    QTest::newRow("i18nType") << "i18nType.qml" << QString::fromUtf8("Test \303\241\303\242\303\243\303\244\303\245: 30");
    QTest::newRow("i18nNameSpace") << "i18nNameSpace.qml" << QString::fromUtf8("Test \303\241\303\242\303\243\303\244\303\245: 40");
}

void tst_qdeclarativelanguage::i18n()
{
    QFETCH(QString, file);
    QFETCH(QString, stringProperty);
    QDeclarativeComponent component(&engine, TEST_FILE(file));
    VERIFY_ERRORS(0);
    MyTypeObject *object = qobject_cast<MyTypeObject *>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->stringProperty(), stringProperty);

    delete object;
}

// Check that the Component::onCompleted attached property works
void tst_qdeclarativelanguage::onCompleted()
{
    QDeclarativeComponent component(&engine, TEST_FILE("onCompleted.qml"));
    VERIFY_ERRORS(0);
    QTest::ignoreMessage(QtDebugMsg, "Completed 6 10");
    QTest::ignoreMessage(QtDebugMsg, "Completed 6 10");
    QTest::ignoreMessage(QtDebugMsg, "Completed 10 11");
    QObject *object = component.create();
    QVERIFY(object != 0);
}

// Check that assignments to QDeclarativeScriptString properties work
void tst_qdeclarativelanguage::scriptString()
{
    QDeclarativeComponent component(&engine, TEST_FILE("scriptString.qml"));
    VERIFY_ERRORS(0);

    MyTypeObject *object = qobject_cast<MyTypeObject*>(component.create());
    QVERIFY(object != 0);
    QCOMPARE(object->scriptProperty().script(), QString("foo + bar"));
    QCOMPARE(object->scriptProperty().scopeObject(), qobject_cast<QObject*>(object));
    QCOMPARE(object->scriptProperty().context(), qmlContext(object));

    QVERIFY(object->grouped() != 0);
    QCOMPARE(object->grouped()->script().script(), QString("console.log(1921)"));
    QCOMPARE(object->grouped()->script().scopeObject(), qobject_cast<QObject*>(object));
    QCOMPARE(object->grouped()->script().context(), qmlContext(object));
}

// Check that default property assignments are correctly spliced into explicit 
// property assignments
void tst_qdeclarativelanguage::defaultPropertyListOrder()
{
    QDeclarativeComponent component(&engine, TEST_FILE("defaultPropertyListOrder.qml"));
    VERIFY_ERRORS(0);

    MyContainer *container = qobject_cast<MyContainer *>(component.create());
    QVERIFY(container  != 0);

    QCOMPARE(container->getChildren()->count(), 6);
    QCOMPARE(container->getChildren()->at(0)->property("index"), QVariant(0));
    QCOMPARE(container->getChildren()->at(1)->property("index"), QVariant(1));
    QCOMPARE(container->getChildren()->at(2)->property("index"), QVariant(2));
    QCOMPARE(container->getChildren()->at(3)->property("index"), QVariant(3));
    QCOMPARE(container->getChildren()->at(4)->property("index"), QVariant(4));
    QCOMPARE(container->getChildren()->at(5)->property("index"), QVariant(5));
}

void tst_qdeclarativelanguage::declaredPropertyValues()
{
    QDeclarativeComponent component(&engine, TEST_FILE("declaredPropertyValues.qml"));
    QEXPECT_FAIL("", "QTBUG-7860", Abort);
    VERIFY_ERRORS(0);
}

// Check that first child of qml is of given type. Empty type insists on error.
void tst_qdeclarativelanguage::testType(const QString& qml, const QString& type)
{
    QDeclarativeComponent component(&engine);
    component.setData(qml.toUtf8(), TEST_FILE("empty.qml")); // just a file for relative local imports

    QTRY_VERIFY(!component.isLoading());

    if (type.isEmpty()) {
        QVERIFY(component.isError());
    } else {
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);
        QCOMPARE(QString(object->metaObject()->className()), type);
    }
}

QML_DECLARE_TYPE(TestType)
QML_DECLARE_TYPE(TestType2)

// Import tests (QT-558)

void tst_qdeclarativelanguage::importsBuiltin_data()
{
    // QT-610

    QTest::addColumn<QString>("qml");
    QTest::addColumn<QString>("type");

    // import built-ins
    QTest::newRow("missing import")
        << "Test {}"
        << "";
    QTest::newRow("not in version 0.0")
        << "import com.nokia.Test 0.0\n"
           "Test {}"
        << "";
    QTest::newRow("in version 0.0")
        << "import com.nokia.Test 0.0\n"
           "TestTP {}"
        << "TestType";
    QTest::newRow("qualified in version 0.0")
        << "import com.nokia.Test 0.0 as T\n"
           "T.TestTP {}"
        << "TestType";
    QTest::newRow("in version 1.0")
        << "import com.nokia.Test 1.0\n"
           "Test {}"
        << "TestType";
    QTest::newRow("qualified wrong")
        << "import com.nokia.Test 1.0 as T\n" // QT-610
           "Test {}"
        << "";
    QTest::newRow("qualified right")
        << "import com.nokia.Test 1.0 as T\n"
           "T.Test {}"
        << "TestType";
    QTest::newRow("qualified right but not in version 0.0")
        << "import com.nokia.Test 0.0 as T\n"
           "T.Test {}"
        << "";
    QTest::newRow("in version 1.1")
        << "import com.nokia.Test 1.1\n"
           "Test {}"
        << "TestType";
    QTest::newRow("in version 1.3")
        << "import com.nokia.Test 1.3\n"
           "Test {}"
        << "TestType";
    QTest::newRow("in version 1.5")
        << "import com.nokia.Test 1.5\n"
           "Test {}"
        << "TestType";
    QTest::newRow("changed in version 1.8")
        << "import com.nokia.Test 1.8\n"
           "Test {}"
        << "TestType2";
    QTest::newRow("in version 1.12")
        << "import com.nokia.Test 1.12\n"
           "Test {}"
        << "TestType2";
    QTest::newRow("old in version 1.9")
        << "import com.nokia.Test 1.9\n"
           "OldTest {}"
        << "TestType";
    QTest::newRow("old in version 1.11")
        << "import com.nokia.Test 1.11\n"
           "OldTest {}"
        << "TestType";
    QTest::newRow("multiversion 1")
        << "import com.nokia.Test 1.11\n"
           "import com.nokia.Test 1.12\n"
           "Test {}"
        << "TestType2";
    QTest::newRow("multiversion 2")
        << "import com.nokia.Test 1.11\n"
           "import com.nokia.Test 1.12\n"
           "OldTest {}"
        << "TestType";
    QTest::newRow("qualified multiversion 3")
        << "import com.nokia.Test 1.0 as T0\n"
           "import com.nokia.Test 1.8 as T8\n"
           "T0.Test {}"
        << "TestType";
    QTest::newRow("qualified multiversion 4")
        << "import com.nokia.Test 1.0 as T0\n"
           "import com.nokia.Test 1.8 as T8\n"
           "T8.Test {}"
        << "TestType2";
}

void tst_qdeclarativelanguage::importsBuiltin()
{
    QFETCH(QString, qml);
    QFETCH(QString, type);
    testType(qml,type);
}

void tst_qdeclarativelanguage::importsLocal_data()
{
    QTest::addColumn<QString>("qml");
    QTest::addColumn<QString>("type");

    // import locals
    QTest::newRow("local import")
        << "import \"subdir\"\n" // QT-613
           "Test {}"
        << "QDeclarativeRectangle";
    QTest::newRow("local import second")
        << "import Qt 4.6\nimport \"subdir\"\n"
           "Test {}"
        << "QDeclarativeRectangle";
    QTest::newRow("local import subsubdir")
        << "import Qt 4.6\nimport \"subdir/subsubdir\"\n"
           "SubTest {}"
        << "QDeclarativeRectangle";
    QTest::newRow("local import QTBUG-7721 A")
        << "subdir.Test {}" // no longer allowed (QTBUG-7721)
        << "";
    QTest::newRow("local import QTBUG-7721 B")
        << "import \"subdir\" as X\n"
           "X.subsubdir.SubTest {}" // no longer allowed (QTBUG-7721)
        << "";
    QTest::newRow("local import as")
        << "import \"subdir\" as T\n"
           "T.Test {}"
        << "QDeclarativeRectangle";
    QTest::newRow("wrong local import as")
        << "import \"subdir\" as T\n"
           "Test {}"
        << "";
    QTest::newRow("library precedence over local import")
        << "import \"subdir\"\n"
           "import com.nokia.Test 1.0\n"
           "Test {}"
        << "TestType";
}

void tst_qdeclarativelanguage::importsLocal()
{
    QFETCH(QString, qml);
    QFETCH(QString, type);
    testType(qml,type);
}

void tst_qdeclarativelanguage::importsRemote_data()
{
    QTest::addColumn<QString>("qml");
    QTest::addColumn<QString>("type");

    QString serverdir = "http://127.0.0.1:14445/qtest/declarative/qmllanguage";

    QTest::newRow("remote import") << "import \""+serverdir+"\"\nTest {}" << "QDeclarativeRectangle";
    QTest::newRow("remote import with subdir") << "import \""+serverdir+"\"\nTestSubDir {}" << "QDeclarativeText";
    QTest::newRow("remote import with local") << "import \""+serverdir+"\"\nTestLocal {}" << "QDeclarativeImage";
    QTest::newRow("wrong remote import with undeclared local") << "import \""+serverdir+"\"\nWrongTestLocal {}" << "";
    QTest::newRow("wrong remote import of internal local") << "import \""+serverdir+"\"\nLocalInternal {}" << "";
    QTest::newRow("wrong remote import of undeclared local") << "import \""+serverdir+"\"\nUndeclaredLocal {}" << "";
}

#include "testhttpserver.h"

void tst_qdeclarativelanguage::importsRemote()
{
    QFETCH(QString, qml);
    QFETCH(QString, type);

    TestHTTPServer server(14445);
    server.serveDirectory(SRCDIR);

    testType(qml,type);
}

void tst_qdeclarativelanguage::importsInstalled_data()
{
    // QT-610

    QTest::addColumn<QString>("qml");
    QTest::addColumn<QString>("type");

    // import installed
    QTest::newRow("installed import 0")
        << "import com.nokia.installedtest 0.0\n"
           "InstalledTestTP {}"
        << "QDeclarativeRectangle";
    QTest::newRow("installed import 0 as TP")
        << "import com.nokia.installedtest 0.0 as TP\n"
           "TP.InstalledTestTP {}"
        << "QDeclarativeRectangle";
    QTest::newRow("installed import 1")
        << "import com.nokia.installedtest 1.0\n"
           "InstalledTest {}"
        << "QDeclarativeRectangle";
    QTest::newRow("installed import 2")
        << "import com.nokia.installedtest 1.3\n"
           "InstalledTest {}"
        << "QDeclarativeRectangle";
    QTest::newRow("installed import 3")
        << "import com.nokia.installedtest 1.4\n"
           "InstalledTest {}"
        << "QDeclarativeText";
    QTest::newRow("installed import 4")
        << "import com.nokia.installedtest 1.10\n"
           "InstalledTest {}"
        << "QDeclarativeText";
    QTest::newRow("installed import visibility") // QT-614
        << "import com.nokia.installedtest 1.4\n"
           "PrivateType {}"
        << "";
}

void tst_qdeclarativelanguage::importsInstalled()
{
    QFETCH(QString, qml);
    QFETCH(QString, type);
    testType(qml,type);
}


void tst_qdeclarativelanguage::importsOrder_data()
{
    QTest::addColumn<QString>("qml");
    QTest::addColumn<QString>("type");

    QTest::newRow("installed import overrides 1") <<
           "import com.nokia.installedtest 1.0\n"
           "import com.nokia.installedtest 1.4\n"
           "InstalledTest {}"
        << "QDeclarativeText";
    QTest::newRow("installed import overrides 2") <<
           "import com.nokia.installedtest 1.4\n"
           "import com.nokia.installedtest 1.0\n"
           "InstalledTest {}"
        << "QDeclarativeRectangle";
    QTest::newRow("installed import re-overrides 1") <<
           "import com.nokia.installedtest 1.4\n"
           "import com.nokia.installedtest 1.0\n"
           "import com.nokia.installedtest 1.4\n"
           "InstalledTest {}"
        << "QDeclarativeText";
    QTest::newRow("installed import re-overrides 2") <<
           "import com.nokia.installedtest 1.4\n"
           "import com.nokia.installedtest 1.0\n"
           "import com.nokia.installedtest 1.4\n"
           "import com.nokia.installedtest 1.0\n"
           "InstalledTest {}"
        << "QDeclarativeRectangle";

    QTest::newRow("installed import versus builtin 1") <<
           "import com.nokia.installedtest 1.5\n"
           "import Qt 4.6\n"
           "Rectangle {}"
        << "QDeclarativeRectangle";
    QTest::newRow("installed import versus builtin 2") <<
           "import Qt 4.6\n"
           "import com.nokia.installedtest 1.5\n"
           "Rectangle {}"
        << "QDeclarativeText";
    QTest::newRow("namespaces cannot be overridden by types 1") <<
           "import Qt 4.6 as Rectangle\n"
           "import com.nokia.installedtest 1.5\n"
           "Rectangle {}"
        << "";
    QTest::newRow("namespaces cannot be overridden by types 2") <<
           "import Qt 4.6 as Rectangle\n"
           "import com.nokia.installedtest 1.5\n"
           "Rectangle.Image {}"
        << "QDeclarativeImage";
}

void tst_qdeclarativelanguage::importsOrder()
{
    QFETCH(QString, qml);
    QFETCH(QString, type);
    testType(qml,type);
}

void tst_qdeclarativelanguage::qmlAttachedPropertiesObjectMethod()
{
    QObject object;

    QCOMPARE(qmlAttachedPropertiesObject<MyQmlObject>(&object, false), (QObject *)0);
    QCOMPARE(qmlAttachedPropertiesObject<MyQmlObject>(&object, true), (QObject *)0);

    {
        QDeclarativeComponent component(&engine, TEST_FILE("qmlAttachedPropertiesObjectMethod.1.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QCOMPARE(qmlAttachedPropertiesObject<MyQmlObject>(object, false), (QObject *)0);
        QVERIFY(qmlAttachedPropertiesObject<MyQmlObject>(object, true) != 0);
    }

    {
        QDeclarativeComponent component(&engine, TEST_FILE("qmlAttachedPropertiesObjectMethod.2.qml"));
        VERIFY_ERRORS(0);
        QObject *object = component.create();
        QVERIFY(object != 0);

        QVERIFY(qmlAttachedPropertiesObject<MyQmlObject>(object, false) != 0);
        QVERIFY(qmlAttachedPropertiesObject<MyQmlObject>(object, true) != 0);
    }
}

void tst_qdeclarativelanguage::crash1()
{
    QDeclarativeComponent component(&engine);
    component.setData("import Qt 4.6\nComponent {}", QUrl());
}

void tst_qdeclarativelanguage::crash2()
{
    QDeclarativeComponent component(&engine, TEST_FILE("crash2.qml"));
}

// QTBUG-8676
void tst_qdeclarativelanguage::customOnProperty()
{
    QDeclarativeComponent component(&engine, TEST_FILE("customOnProperty.qml"));

    VERIFY_ERRORS(0);
    QObject *object = component.create();
    QVERIFY(object != 0);

    QCOMPARE(object->property("on").toInt(), 10);

    delete object;
}

void tst_qdeclarativelanguage::initTestCase()
{
    registerTypes();

    qmlRegisterType<TestType>("com.nokia.Test", 0, 0, "TestTP");
    qmlRegisterType<TestType>("com.nokia.Test", 1, 0, "Test");
    qmlRegisterType<TestType>("com.nokia.Test", 1, 5, "Test");
    qmlRegisterType<TestType2>("com.nokia.Test", 1, 8, "Test");
    qmlRegisterType<TestType>("com.nokia.Test", 1, 9, "OldTest");
    qmlRegisterType<TestType2>("com.nokia.Test", 1, 12, "Test");

    // Create locale-specific file
    // For POSIX, this will just be data/I18nType.qml, since POSIX is 7-bit
    // For iso8859-1 locale, this will just be data/I18nType?????.qml where ????? is 5 8-bit characters
    // For utf-8 locale, this will be data/I18nType??????????.qml where ?????????? is 5 8-bit characters, UTF-8 encoded
    QFile in(TEST_FILE(QLatin1String("I18nType30.qml")).toLocalFile());
    QVERIFY(in.open(QIODevice::ReadOnly));
    QFile out(TEST_FILE(QString::fromUtf8("I18nType\303\201\303\242\303\243\303\244\303\245.qml")).toLocalFile());
    QVERIFY(out.open(QIODevice::WriteOnly));
    out.write(in.readAll());
}

QTEST_MAIN(tst_qdeclarativelanguage)

#include "tst_qdeclarativelanguage.moc"