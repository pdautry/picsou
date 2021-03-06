/*
 *  Picsou | Keep track of your expenses !
 *  Copyright (C) 2018  koromodako
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "picsoutreeitem.h"

PicsouTreeItem::~PicsouTreeItem()
{

}

PicsouTreeItem::PicsouTreeItem(QTreeWidget *view,
                               Type type,
                               const QIcon &icon,
                               const QString &text,
                               QUuid id,
                               int year,
                               int month,
                               bool wrapped) :
    QTreeWidgetItem(view, type),
    PicsouItem(id),
    m_year(year),
    m_month(month),
    m_wrapped(wrapped)
{
    setIcon(0, icon);
    setText(0, text);
}

PicsouTreeItem::PicsouTreeItem(QTreeWidgetItem *parent,
                               Type type,
                               const QIcon &icon,
                               const QString &text,
                               QUuid id,
                               int year,
                               int month,
                               bool wrapped) :
    QTreeWidgetItem(parent, type),
    PicsouItem(id),
    m_year(year),
    m_month(month),
    m_wrapped(wrapped)
{
    setIcon(0, icon);
    setText(0, text);
}

PicsouTreeItem *PicsouTreeItem::parent() const
{
    return static_cast<PicsouTreeItem*>(QTreeWidgetItem::parent());
}
