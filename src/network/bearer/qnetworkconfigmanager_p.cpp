/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the QtNetwork module of the Qt Toolkit.
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

#include "qnetworkconfigmanager_p.h"
#include "qbearerplugin_p.h"

#include <QtCore/private/qfactoryloader_p.h>

#include <QtCore/qdebug.h>
#include <QtCore/qtimer.h>
#include <QtCore/qstringlist.h>
#include <QtCore/qthread.h>
#include <QtCore/private/qcoreapplication_p.h>

#ifndef QT_NO_BEARERMANAGEMENT

QT_BEGIN_NAMESPACE

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
                          (QBearerEngineFactoryInterface_iid, QLatin1String("/bearer")))

QNetworkConfigurationManagerPrivate::QNetworkConfigurationManagerPrivate()
:   pollTimer(0), mutex(QMutex::Recursive), forcedPolling(0), firstUpdate(true)
{
    qRegisterMetaType<QNetworkConfiguration>("QNetworkConfiguration");

    moveToThread(QCoreApplicationPrivate::mainThread());
    updateConfigurations();
}

QNetworkConfigurationManagerPrivate::~QNetworkConfigurationManagerPrivate()
{
    QMutexLocker locker(&mutex);

    qDeleteAll(sessionEngines);
}

QNetworkConfiguration QNetworkConfigurationManagerPrivate::defaultConfiguration()
{
    QMutexLocker locker(&mutex);

    foreach (QBearerEngine *engine, sessionEngines) {
        QNetworkConfigurationPrivatePointer ptr = engine->defaultConfiguration();

        if (ptr) {
            QNetworkConfiguration config;
            config.d = ptr;
            return config;
        }
    }

    // Engines don't have a default configuration.

    // Return first active snap
    QNetworkConfigurationPrivatePointer defaultConfiguration;

    foreach (QBearerEngine *engine, sessionEngines) {
        QHash<QString, QNetworkConfigurationPrivatePointer>::Iterator it;
        QHash<QString, QNetworkConfigurationPrivatePointer>::Iterator end;

        QMutexLocker locker(&engine->mutex);

        for (it = engine->snapConfigurations.begin(), end = engine->snapConfigurations.end();
             it != end; ++it) {
            QNetworkConfigurationPrivatePointer ptr = it.value();

            QMutexLocker configLocker(&ptr->mutex);

            if ((ptr->state & QNetworkConfiguration::Active) == QNetworkConfiguration::Active) {
                QNetworkConfiguration config;
                config.d = ptr;
                return config;
            } else if (!defaultConfiguration) {
                if ((ptr->state & QNetworkConfiguration::Discovered) ==
                    QNetworkConfiguration::Discovered) {
                    defaultConfiguration = ptr;
                }
            }
        }
    }

    // No Active SNAPs return first Discovered SNAP.
    if (defaultConfiguration) {
        QNetworkConfiguration config;
        config.d = defaultConfiguration;
        return config;
    }

    /*
        No Active or Discovered SNAPs, find the perferred access point.
        The following priority order is used:

            1. Active Ethernet
            2. Active WLAN
            3. Active Other
            4. Discovered Ethernet
            5. Discovered WLAN
            6. Discovered Other
    */

    defaultConfiguration.reset();

    foreach (QBearerEngine *engine, sessionEngines) {
        QHash<QString, QNetworkConfigurationPrivatePointer>::Iterator it;
        QHash<QString, QNetworkConfigurationPrivatePointer>::Iterator end;

        QMutexLocker locker(&engine->mutex);

        for (it = engine->accessPointConfigurations.begin(),
             end = engine->accessPointConfigurations.end(); it != end; ++it) {
            QNetworkConfigurationPrivatePointer ptr = it.value();

            const QString bearerName = ptr->bearerName();
            QMutexLocker configLocker(&ptr->mutex);

            if ((ptr->state & QNetworkConfiguration::Discovered) ==
                QNetworkConfiguration::Discovered) {
                if (!defaultConfiguration) {
                    defaultConfiguration = ptr;
                } else {
                    if (defaultConfiguration->state == ptr->state) {
                        if (defaultConfiguration->bearerName() == QLatin1String("Ethernet")) {
                            // do nothing
                        } else if (defaultConfiguration->bearerName() == QLatin1String("WLAN")) {
                            // ethernet beats wlan
                            if (bearerName == QLatin1String("Ethernet"))
                                defaultConfiguration = ptr;
                        } else {
                            // ethernet and wlan beats other
                            if (bearerName == QLatin1String("Ethernet") ||
                                bearerName == QLatin1String("WLAN")) {
                                defaultConfiguration = ptr;
                            }
                        }
                    } else {
                        // active beats discovered
                        if ((defaultConfiguration->state & QNetworkConfiguration::Active) !=
                            QNetworkConfiguration::Active) {
                            defaultConfiguration = ptr;
                        }
                    }
                }
            }
        }
    }

    // No Active InternetAccessPoint return first Discovered InternetAccessPoint.
    if (defaultConfiguration) {
        QNetworkConfiguration config;
        config.d = defaultConfiguration;
        return config;
    }

    return QNetworkConfiguration();
}

QList<QNetworkConfiguration> QNetworkConfigurationManagerPrivate::allConfigurations(QNetworkConfiguration::StateFlags filter)
{
    QList<QNetworkConfiguration> result;

    QMutexLocker locker(&mutex);

    foreach (QBearerEngine *engine, sessionEngines) {
        QHash<QString, QNetworkConfigurationPrivatePointer>::Iterator it;
        QHash<QString, QNetworkConfigurationPrivatePointer>::Iterator end;

        QMutexLocker locker(&engine->mutex);

        //find all InternetAccessPoints
        for (it = engine->accessPointConfigurations.begin(),
             end = engine->accessPointConfigurations.end(); it != end; ++it) {
            QNetworkConfigurationPrivatePointer ptr = it.value();

            QMutexLocker configLocker(&ptr->mutex);

            if ((ptr->state & filter) == filter) {
                QNetworkConfiguration pt;
                pt.d = ptr;
                result << pt;
            }
        }

        //find all service networks
        for (it = engine->snapConfigurations.begin(),
             end = engine->snapConfigurations.end(); it != end; ++it) {
            QNetworkConfigurationPrivatePointer ptr = it.value();

            QMutexLocker configLocker(&ptr->mutex);

            if ((ptr->state & filter) == filter) {
                QNetworkConfiguration pt;
                pt.d = ptr;
                result << pt;
            }
        }
    }

    return result;
}

QNetworkConfiguration QNetworkConfigurationManagerPrivate::configurationFromIdentifier(const QString &identifier)
{
    QNetworkConfiguration item;

    QMutexLocker locker(&mutex);

    foreach (QBearerEngine *engine, sessionEngines) {
        QMutexLocker locker(&engine->mutex);

        if (engine->accessPointConfigurations.contains(identifier))
            item.d = engine->accessPointConfigurations.value(identifier);
        else if (engine->snapConfigurations.contains(identifier))
            item.d = engine->snapConfigurations.value(identifier);
        else if (engine->userChoiceConfigurations.contains(identifier))
            item.d = engine->userChoiceConfigurations.value(identifier);
        else
            continue;

        return item;
    }

    return item;
}

bool QNetworkConfigurationManagerPrivate::isOnline()
{
    QMutexLocker locker(&mutex);

    return !onlineConfigurations.isEmpty();
}

QNetworkConfigurationManager::Capabilities QNetworkConfigurationManagerPrivate::capabilities()
{
    QMutexLocker locker(&mutex);

    QNetworkConfigurationManager::Capabilities capFlags;

    foreach (QBearerEngine *engine, sessionEngines)
        capFlags |= engine->capabilities();

    return capFlags;
}

void QNetworkConfigurationManagerPrivate::configurationAdded(QNetworkConfigurationPrivatePointer ptr)
{
    QMutexLocker locker(&mutex);

    if (!firstUpdate) {
        QNetworkConfiguration item;
        item.d = ptr;
        emit configurationAdded(item);
    }

    ptr->mutex.lock();
    if (ptr->state == QNetworkConfiguration::Active) {
        ptr->mutex.unlock();
        onlineConfigurations.insert(ptr->id);
        if (!firstUpdate && onlineConfigurations.count() == 1)
            emit onlineStateChanged(true);
    } else {
        ptr->mutex.unlock();
    }
}

void QNetworkConfigurationManagerPrivate::configurationRemoved(QNetworkConfigurationPrivatePointer ptr)
{
    QMutexLocker locker(&mutex);

    ptr->mutex.lock();
    ptr->isValid = false;
    ptr->mutex.unlock();

    if (!firstUpdate) {
        QNetworkConfiguration item;
        item.d = ptr;
        emit configurationRemoved(item);
    }

    onlineConfigurations.remove(ptr->id);
    if (!firstUpdate && onlineConfigurations.isEmpty())
        emit onlineStateChanged(false);
}

void QNetworkConfigurationManagerPrivate::configurationChanged(QNetworkConfigurationPrivatePointer ptr)
{
    QMutexLocker locker(&mutex);

    if (!firstUpdate) {
        QNetworkConfiguration item;
        item.d = ptr;
        emit configurationChanged(item);
    }

    bool previous = !onlineConfigurations.isEmpty();

    ptr->mutex.lock();
    if (ptr->state == QNetworkConfiguration::Active)
        onlineConfigurations.insert(ptr->id);
    else
        onlineConfigurations.remove(ptr->id);
    ptr->mutex.unlock();

    bool online = !onlineConfigurations.isEmpty();

    if (!firstUpdate && online != previous)
        emit onlineStateChanged(online);
}

void QNetworkConfigurationManagerPrivate::updateConfigurations()
{
    QMutexLocker locker(&mutex);

    if (firstUpdate) {
        if (sender())
            return;

        updating = false;

        QFactoryLoader *l = loader();

        QBearerEngine *generic = 0;

        foreach (const QString &key, l->keys()) {
            QBearerEnginePlugin *plugin = qobject_cast<QBearerEnginePlugin *>(l->instance(key));
            if (plugin) {
                QBearerEngine *engine = plugin->create(key);
                if (!engine)
                    continue;

                if (key == QLatin1String("generic"))
                    generic = engine;
                else
                    sessionEngines.append(engine);

                engine->moveToThread(QCoreApplicationPrivate::mainThread());

                connect(engine, SIGNAL(updateCompleted()),
                        this, SLOT(updateConfigurations()));
                connect(engine, SIGNAL(configurationAdded(QNetworkConfigurationPrivatePointer)),
                        this, SLOT(configurationAdded(QNetworkConfigurationPrivatePointer)));
                connect(engine, SIGNAL(configurationRemoved(QNetworkConfigurationPrivatePointer)),
                        this, SLOT(configurationRemoved(QNetworkConfigurationPrivatePointer)));
                connect(engine, SIGNAL(configurationChanged(QNetworkConfigurationPrivatePointer)),
                        this, SLOT(configurationChanged(QNetworkConfigurationPrivatePointer)));

                QMetaObject::invokeMethod(engine, "initialize");
            }
        }

        if (generic)
            sessionEngines.append(generic);
    }

    QBearerEngine *engine = qobject_cast<QBearerEngine *>(sender());
    if (!updatingEngines.isEmpty() && engine) {
        int index = sessionEngines.indexOf(engine);
        if (index >= 0)
            updatingEngines.remove(index);
    }

    if (updating && updatingEngines.isEmpty()) {
        updating = false;
        emit configurationUpdateComplete();
    }

    if (engine && !pollingEngines.isEmpty()) {
        int index = sessionEngines.indexOf(engine);
        if (index >= 0)
            pollingEngines.remove(index);

        if (pollingEngines.isEmpty())
            startPolling();
    }

    if (firstUpdate)
        firstUpdate = false;
}

void QNetworkConfigurationManagerPrivate::performAsyncConfigurationUpdate()
{
    QMutexLocker locker(&mutex);

    if (sessionEngines.isEmpty()) {
        emit configurationUpdateComplete();
        return;
    }

    updating = true;

    for (int i = 0; i < sessionEngines.count(); ++i) {
        updatingEngines.insert(i);
        QMetaObject::invokeMethod(sessionEngines.at(i), "requestUpdate");
    }
}

QList<QBearerEngine *> QNetworkConfigurationManagerPrivate::engines()
{
    QMutexLocker locker(&mutex);

    return sessionEngines;
}

void QNetworkConfigurationManagerPrivate::startPolling()
{
    QMutexLocker locker(&mutex);

    bool pollingRequired = false;

    if (forcedPolling > 0) {
        foreach (QBearerEngine *engine, sessionEngines) {
            if (engine->requiresPolling()) {
                pollingRequired = true;
                break;
            }
        }
    }

    if (!pollingRequired) {
        foreach (QBearerEngine *engine, sessionEngines) {
            if (engine->configurationsInUse()) {
                pollingRequired = true;
                break;
            }
        }
    }

    if (pollingRequired) {
        if (!pollTimer) {
            pollTimer = new QTimer(this);
            pollTimer->setInterval(10000);
            pollTimer->setSingleShot(true);
            connect(pollTimer, SIGNAL(timeout()), this, SLOT(pollEngines()));
        }

        pollTimer->start();
    }
}

void QNetworkConfigurationManagerPrivate::pollEngines()
{
    QMutexLocker locker(&mutex);

    for (int i = 0; i < sessionEngines.count(); ++i) {
        if ((forcedPolling && sessionEngines.at(i)->requiresPolling()) ||
            sessionEngines.at(i)->configurationsInUse()) {
            pollingEngines.insert(i);
            QMetaObject::invokeMethod(sessionEngines.at(i), "requestUpdate");
        }
    }
}

void QNetworkConfigurationManagerPrivate::enablePolling()
{
    QMutexLocker locker(&mutex);

    ++forcedPolling;

    if (forcedPolling == 1)
        QMetaObject::invokeMethod(this, "startPolling");
}

void QNetworkConfigurationManagerPrivate::disablePolling()
{
    QMutexLocker locker(&mutex);

    --forcedPolling;
}

QT_END_NAMESPACE

#endif // QT_NO_BEARERMANAGEMENT
