#ifndef PICSOUTREEWIDGETITEM_H
#define PICSOUTREEWIDGETITEM_H

#include <QTreeWidgetItem>

#include "ui/picsouitem.h"

class PicsouTreeItem : public QTreeWidgetItem, public PicsouItem
{
public:
    enum Type {
        T_ROOT,
        T_USER,
        T_ACCOUNT,
        T_YEAR,
        T_MONTH
    };

    virtual ~PicsouTreeItem();
    PicsouTreeItem(QTreeWidget *view,
                   Type type,
                   const QString &text,
                   QUuid id);
    PicsouTreeItem(QTreeWidgetItem *parent,
                   Type type,
                   const QString &text,
                   QUuid id);

};

#endif // PICSOUTREEWIDGETITEM_H