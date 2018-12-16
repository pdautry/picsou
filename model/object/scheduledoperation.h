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
#ifndef SCHEDULEDOPERATION_H
#define SCHEDULEDOPERATION_H

#include "schedule.h"
#include "operation.h"


class ScheduledOperation : public Operation
{
    Q_OBJECT
public:
    static const QString KW_NAME;

    ScheduledOperation(PicsouModelObj *parent);
    ScheduledOperation(const Amount &amount,
                       const QString &budget,
                       const QString &recipient,
                       const QString &description,
                       const QString &payment_method,
                       const QString &name,
                       const Schedule &schedule,
                       PicsouModelObj *parent);

    inline QString name() const { return _name; }
    inline Schedule schedule() const { return _schedule; }

    void update(const Amount &amount,
                const QString &budget,
                const QString &recipient,
                const QString &description,
                const QString &payment_method,
                const QString &name,
                const Schedule &schedule);

    bool read(const QJsonObject &json);
    bool write(QJsonObject &json) const;

private:
    QString _name;
    Schedule _schedule;
};

DECL_PICSOU_MOD_OBJ_PTR(ScheduledOperation,
                        ScheduledOperationPtr,
                        ScheduledOperationShPtr,
                        ScheduledOperationPtrList,
                        ScheduledOperationShPtrList);

#endif // SCHEDULEDOPERATION_H
