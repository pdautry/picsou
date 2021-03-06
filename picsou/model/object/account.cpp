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
#include "account.h"
#include "utils/macro.h"

#include <QSet>
#include <QJsonArray>

const QString Account::KW_OPS="ops";
const QString Account::KW_NAME="name";
const QString Account::KW_NOTES="notes";
const QString Account::KW_ARCHIVED="archived";
const QString Account::KW_INITIAL_AMOUNT="init_amount";
const QString Account::KW_PAYMENT_METHODS="payment_methods";
const QString Account::KW_SCHEDULED_OPS="scheduled_ops";

Account::Account(PicsouDBO *parent) :
    PicsouDBO(false, parent),
    m_name(QString()),
    m_notes(QString()),
    m_archived(false),
    m_initial_amount(0.)
{

}

Account::Account(const QString &name,
                 const QString &notes,
                 bool archived,
                 const Amount &intial_amount,
                 PicsouDBO *parent) :
    PicsouDBO(true, parent),
    m_name(name),
    m_notes(notes),
    m_archived(archived),
    m_initial_amount(intial_amount)
{

}

void Account::update(const QString &name,
                     const QString &notes,
                     bool archived,
                     const Amount &initial_amount)
{
    m_name=name;
    m_notes=notes;
    m_archived=archived;
    m_initial_amount=initial_amount;
    emit modified();
}

bool Account::add_payment_method(const QString &name, QString &error)
{
    if(m_archived) {
        error=tr("Cannot modify an archived account.");
        return false;
    }
    PaymentMethodShPtr existing=find_payment_method(name);
    if(!existing.isNull()) {
        error=tr("A payment method having the same name already exist.");
        return false;
    }
    PaymentMethodShPtr pm=PaymentMethodShPtr(new PaymentMethod(name, this));
    m_payment_methods.insert(pm->id(), pm);
    emit modified();
    return true;
}

bool Account::remove_payment_method(QUuid id)
{
    if(m_archived) {
        return false;
    }
    bool success=false;
    switch (m_payment_methods.remove(id)) {
    case 0:
        /* TRACE */
        break;
    case 1:
        success=true;
        emit modified();
        break;
    default:
        /* TRACE */
        break;
    }
    return success;
}

bool Account::add_scheduled_operation(const Amount &amount,
                                      const QString &budget,
                                      const QString &srcdst,
                                      const QString &description,
                                      const QString &payment_method,
                                      const QString &name,
                                      const Schedule &schedule,
                                      QString &error)
{
    if(m_archived) {
        error=tr("Cannot modify an archived account.");
        return false;
    }
    ScheduledOperationShPtr sop=ScheduledOperationShPtr(new ScheduledOperation(amount,
                                                                               budget,
                                                                               srcdst,
                                                                               description,
                                                                               payment_method,
                                                                               name,
                                                                               schedule,
                                                                               this));
    m_scheduled_ops.insert(sop->id(), sop);
    emit modified();
    return true;
}

bool Account::remove_scheduled_operation(QUuid id, QString &error)
{
    if(m_archived) {
        error=tr("Cannot modify an archived account.");
        return false;
    }
    bool success=false;
    switch (m_scheduled_ops.remove(id)) {
    case 0:
        error=tr("Failed to remove scheduled operation: not found.");
        break;
    case 1:
        success=true;
        emit modified();
        break;
    default:
        error=tr("Failed to remove scheduled operation: doublons deleted.");
        break;
    }
    return success;
}

bool Account::add_operation(bool verified,
                            const Amount &amount,
                            const QDate &date,
                            const QString &budget,
                            const QString &recipient,
                            const QString &description,
                            const QString &payment_method,
                            QString &error)
{
    if(m_archived) {
        error=tr("Cannot modify an archived account.");
        return false;
    }
    OperationShPtr op=OperationShPtr(new Operation(verified,
                                                   amount,
                                                   date,
                                                   budget,
                                                   recipient,
                                                   description,
                                                   payment_method,
                                                   this));
     m_ops.insert(op->id(), op);
     emit modified();
     return true;
}

bool Account::add_operations(const OperationShPtrList &ops, QString &error)
{
    if(m_archived) {
        error=tr("Cannot modify an archived account.");
        return false;
    }
    bool success=false;
    for(const auto &op : ops) {
        op->set_parent(this);
        m_ops.insert(op->id(), op);
    }
    if(ops.length()>0) {
        emit modified();
        success=true;
    }
    return success;
}

bool Account::remove_operation(QUuid id, QString &error)
{
    if(m_archived) {
        error=tr("Cannot modify an archived account.");
        return false;
    }
    bool success=false;
    switch (m_ops.remove(id)) {
    case 0:
        error=tr("Failed to remove operation: not found.");
        break;
    case 1:
        success=true;
        emit modified();
        break;
    default:
        error=tr("Failed to remove operation: doublons deleted.");
        break;
    }
    return success;
}

PaymentMethodShPtr Account::find_payment_method(QUuid id)
{
    PaymentMethodShPtr pm;
    QHash<QUuid, PaymentMethodShPtr>::const_iterator it=m_payment_methods.find(id);
    if(it!=m_payment_methods.end()) {
        pm=*it;
    }
    return pm;
}

PaymentMethodShPtr Account::find_payment_method(const QString &name)
{
    for(const auto &pm : m_payment_methods) {
        if(pm->name()==name) {
            return pm;
        }
    }
    return PaymentMethodShPtr();
}

ScheduledOperationShPtr Account::find_scheduled_operation(QUuid id)
{
    ScheduledOperationShPtr sop;
    QHash<QUuid, ScheduledOperationShPtr>::const_iterator it=m_scheduled_ops.find(id);
    if(it!=m_scheduled_ops.end()) {
        sop=*it;
    }
    return sop;
}

OperationShPtr Account::find_operation(QUuid id)
{
    OperationShPtr op;
    QHash<QUuid, OperationShPtr>::const_iterator it=m_ops.find(id);
    if(it!=m_ops.end()) {
        op=*it;
    }
    return op;
}

#define min(a, b) (((a)<(b))?(a):(b))

int Account::min_year() const
{
    int min_y=INT_MAX;
    for(const auto &op : m_ops) {
        min_y=min(min_y, op->date().year());
    }
    for(const auto &sop : m_scheduled_ops) {
        min_y=min(min_y, sop->schedule().from().year());
    }
    return min_y;

}

QStringList Account::srcdst() const
{
    QSet<QString> srcdst;
    for(const auto &op : m_ops) {
        srcdst.insert(op->srcdst());
    }
    for(const auto &sop : m_scheduled_ops) {
        srcdst.insert(sop->srcdst());
    }
    return QStringList(srcdst.values());
}

#undef min

bool pm_cmp(const PaymentMethodShPtr &a, const PaymentMethodShPtr &b)
{
    return a->name()<b->name();
}

PaymentMethodShPtrList Account::payment_methods(bool sorted) const
{
    PaymentMethodShPtrList p_methods=m_payment_methods.values();
    if(sorted) {
        std::sort(p_methods.begin(), p_methods.end(), pm_cmp);
    }
    return p_methods;
}

QStringList Account::payment_methods_str(bool sorted) const
{
    QStringList p_methods_str;
    PaymentMethodShPtrList p_methods=payment_methods(sorted);
    for(auto &pm : p_methods) {
        p_methods_str<<pm->name();
    }
    return p_methods_str;
}

bool Account::read(const QJsonObject &json)
{
    LOG_IN("<QJsonObject>")
    static const QStringList keys=(QStringList()<<KW_NAME
                                                <<KW_NOTES
                                                <<KW_PAYMENT_METHODS
                                                <<KW_SCHEDULED_OPS
                                                <<KW_OPS);
    JSON_CHECK_KEYS(keys);
    /**/
    m_name=json[KW_NAME].toString();
    m_notes=json[KW_NOTES].toString();
    if(json.contains(KW_ARCHIVED)) {
        m_archived=json[KW_ARCHIVED].toBool();
    }
    if(json.contains(KW_INITIAL_AMOUNT)) {
        m_initial_amount=json[KW_INITIAL_AMOUNT].toDouble();
    }
    JSON_READ_LIST(json, KW_PAYMENT_METHODS,
                   m_payment_methods, PaymentMethod, this);
    JSON_READ_LIST(json, KW_SCHEDULED_OPS,
                   m_scheduled_ops, ScheduledOperation, this);
    JSON_READ_LIST(json, KW_OPS,
                   m_ops, Operation, this);
    /**/
    set_valid();
    LOG_BOOL_RETURN(valid())
}

bool Account::write(QJsonObject &json) const
{
    LOG_IN("<QJsonObject>")
    json[KW_NAME]=m_name;
    json[KW_NOTES]=m_notes;
    json[KW_ARCHIVED]=m_archived;
    json[KW_INITIAL_AMOUNT]=m_initial_amount.value();
    JSON_WRITE_LIST(json, KW_PAYMENT_METHODS, m_payment_methods.values());
    JSON_WRITE_LIST(json, KW_SCHEDULED_OPS, m_scheduled_ops.values());
    JSON_WRITE_LIST(json, KW_OPS, m_ops.values());
    /**/
    LOG_BOOL_RETURN(true)
}

bool Account::operator <(const Account &other)
{
    return (m_name<other.m_name);
}
