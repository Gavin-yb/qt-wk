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

#include <QDebug>
#include <QTime>

#include "itemrecyclinglist.h"
#include "listitemcontainer.h"
#include "abstractviewitem.h"
#include "recycledlistitem.h"
#include "theme.h"
#include "scrollbar.h"

ItemRecyclingList::ItemRecyclingList(const int itemBuffer, QGraphicsWidget * parent)
    : ItemRecyclingListView(parent),
      m_listModel(new ListModel(this))
{    
    ListItemContainer *container = new ListItemContainer(itemBuffer, this, this);
    container->setParentItem(this);
    ItemRecyclingListView::setContainer(container);
    ItemRecyclingListView::setModel(m_listModel, new RecycledListItem(this));
    setObjectName("ItemRecyclingList");
    connect(Theme::p(), SIGNAL(themeChanged()), this, SLOT(themeChange()));

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

/* virtual */
ItemRecyclingList::~ItemRecyclingList()
{
}

/* virtual */
void ItemRecyclingList::insertItem(int index, RecycledListItem *item)
{
    if (index<0)
        index = 0;
    if (index > m_listModel->rowCount())
        index = m_listModel->rowCount();
    if (m_listModel && item)
        m_listModel->insert(index,item);

    updateListItemBackgrounds(index);
}

/* virtual */
void ItemRecyclingList::addItem(RecycledListItem *item)
{
    if (item)
        m_listModel->appendRow(item);

    const int index = m_listModel->rowCount()-1;
    updateListItemBackgrounds(index);
}

/* virtual */
void ItemRecyclingList::clear()
{
    m_listModel->clear();
}

/* virtual */
AbstractViewItem *ItemRecyclingList::takeItem(const int row)
{
    if (row < 0 || row >= m_listModel->rowCount() || !m_listModel)
        return 0;
    return m_listModel->takeItem(row);
}

/*virtual*/
void ItemRecyclingList::setItemPrototype(AbstractViewItem* prototype)
{
    ItemRecyclingListView::setItemPrototype(prototype);
}

void ItemRecyclingList::themeChange()
{
    const bool caching = listItemCaching();
    setListItemCaching(false);

    const QString iconName = Theme::p()->pixmapPath()+"contact_default_icon.svg";
    const int count = m_listModel->rowCount();

    for (int i=0; i<count; ++i)
    {
        RecycledListItem *ritem = m_listModel->item(i);
        if (ritem) {
            ListItem *item = ritem->item();

            // Update default icons
            const QString filename = item->icon(ListItem::LeftIcon)->fileName();
            if (filename.contains("contact_default_icon")) {
                item->icon(ListItem::LeftIcon)->setFileName(iconName);
            }

            // Update status icons
            QString statusIcon = item->icon(ListItem::RightIcon)->fileName();
            const int index = statusIcon.indexOf("contact_status");
            if (index != -1) {
                statusIcon.remove(0, index);
                item->icon(ListItem::RightIcon)->setFileName(Theme::p()->pixmapPath()+statusIcon);
            }

            // Update fonts
            item->setFont(Theme::p()->font(Theme::ContactName), ListItem::FirstPos);
            item->setFont(Theme::p()->font(Theme::ContactNumber), ListItem::SecondPos);
            item->setFont(Theme::p()->font(Theme::ContactEmail), ListItem::ThirdPos);

            // Update list dividers
            if (i%2) {
                item->setBackgroundBrush(Theme::p()->listItemBackgroundBrushOdd());
                item->setBackgroundOpacity(Theme::p()->listItemBackgroundOpacityOdd());
            }
            else {
                item->setBackgroundBrush(Theme::p()->listItemBackgroundBrushEven());
                item->setBackgroundOpacity(Theme::p()->listItemBackgroundOpacityEven());
            }
    
            // Update borders
            item->setBorderPen(Theme::p()->listItemBorderPen());
            item->setRounding(Theme::p()->listItemRounding());

            // Update icons
            item->icon(ListItem::LeftIcon)->setRotation(Theme::p()->iconRotation(ListItem::LeftIcon));
            item->icon(ListItem::RightIcon)->setRotation(Theme::p()->iconRotation(ListItem::RightIcon));
#if (QT_VERSION >= 0x040600)
            item->icon(ListItem::LeftIcon)->setOpacityEffectEnabled(Theme::p()->isIconOpacityEffectEnabled(ListItem::LeftIcon));
            item->icon(ListItem::RightIcon)->setOpacityEffectEnabled(Theme::p()->isIconOpacityEffectEnabled(ListItem::RightIcon));
#endif
            item->icon(ListItem::LeftIcon)->setSmoothTransformationEnabled(Theme::p()->isIconSmoothTransformationEnabled(ListItem::LeftIcon));
            item->icon(ListItem::RightIcon)->setSmoothTransformationEnabled(Theme::p()->isIconSmoothTransformationEnabled(ListItem::RightIcon));
        }
    }
    updateViewContent();
    setListItemCaching(caching);
}

void ItemRecyclingList::keyPressEvent(QKeyEvent *event)
{
    static QTime keyPressInterval = QTime::currentTime();
    static qreal step = 0.0;
    static bool repeat = false;
    int interval = keyPressInterval.elapsed();
    
    ScrollBar* sb = verticalScrollBar();
    qreal currentValue = sb->sliderPosition();

    if(interval < 250 ) {
        if(!repeat) step = 0.0;
        step = step + 2.0;
        if(step > 100) step = 100;
        repeat = true;
    }
    else {
        step = 1.0;
        if(m_listModel->item(0)) m_listModel->item(0)->size().height();
            step = m_listModel->item(0)->size().height();
        repeat = false;
    }
    
    if(event->key() == Qt::Key_Up ) { //Up Arrow
        sb->setSliderPosition(currentValue - step);
    }
    
    if(event->key() == Qt::Key_Down ) { //Down Arrow
        sb->setSliderPosition(currentValue + step);
    }
    keyPressInterval.start();
}

bool ItemRecyclingList::listItemCaching() const
{
#if (QT_VERSION >= 0x040600)
    ListItemContainer *container =
        static_cast<ListItemContainer *>(m_container);

    return container->listItemCaching();
#else
    return false;
#endif
}

void ItemRecyclingList::setListItemCaching(bool enabled)
{
#if (QT_VERSION >= 0x040600)
    ListItemContainer *container =
        static_cast<ListItemContainer *>(m_container);
    container->setListItemCaching(enabled);
#else
    Q_UNUSED(enabled)
#endif
}

void ItemRecyclingList::updateListItemBackgrounds(int index)
{
    const int itemCount = m_listModel->rowCount();

    for (int i=index; i<itemCount; ++i)
    {
        RecycledListItem *ritem = m_listModel->item(i);
        if (ritem) {
            ListItem *item = ritem->item();
            if (i%2) {
                item->setBackgroundBrush(Theme::p()->listItemBackgroundBrushOdd());
                item->setBackgroundOpacity(Theme::p()->listItemBackgroundOpacityOdd());
            }
            else {
                item->setBackgroundBrush(Theme::p()->listItemBackgroundBrushEven());
                item->setBackgroundOpacity(Theme::p()->listItemBackgroundOpacityEven());
            }
        }
    }
}

void ItemRecyclingList::setTwoColumns(const bool enabled)
{
    if (twoColumns() == enabled)
        return;

#if (QT_VERSION >= 0x040600)
    const bool caching = listItemCaching();
    setListItemCaching(false);
#endif

    m_container->setTwoColumns(enabled);
    refreshContainerGeometry();

#if (QT_VERSION >= 0x040600)
    setListItemCaching(caching);
#endif
}

bool ItemRecyclingList::twoColumns()
{    
    return m_container->twoColumns();
}

