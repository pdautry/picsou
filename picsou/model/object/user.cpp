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
#include "user.h"
#include "utils/macro.h"


const QString User::KW_NAME="name";
const QString User::KW_BUDGETS="budgets";
const QString User::KW_ACCOUNTS="accounts";

User::User(PicsouDBO *parent) :
    PicsouDBO(false, parent)
{

}

User::User(const QString &name, const QString &pswd, PicsouDBO *parent) :
    PicsouDBO(true, parent),
    m_name(name)
{
    PicsouDBO::init_wkey(pswd);
}

bool User::update(const QString &name, const QString &old_pswd, const QString &new_pswd, QString &error)
{
    LOG_IN("name="<<name<<"old_pswd,new_pswd")
    if(!old_pswd.isEmpty()&&!new_pswd.isEmpty()) {
        LOG_INFO("attemtping wkey rewraping.")
        if(!rewrap(old_pswd, new_pswd)) {
            LOG_CRITICAL("failed to rewrap.")
            error=tr("Failed to update user: MK rewraping failed.");
            LOG_BOOL_RETURN(false)
        }
        LOG_INFO("rewraping succeeded.")
    } else {
        LOG_INFO("passphrase unchanged.")
    }
    m_name=name;
    emit modified();
    LOG_BOOL_RETURN(true)
}

bool User::add_budget(const Amount &amount, const QString &name, const QString &description, QString &error)
{
    BudgetShPtr existing=find_budget(name);
    if(!existing.isNull()) {
        error=tr("A budget having the same name already exist.");
        return false;
    }
    BudgetShPtr budget=BudgetShPtr(new Budget(amount, name, description, this));
    m_budgets.insert(budget->id(), budget);
    emit modified();
    return true;
}

bool User::remove_budget(QUuid id)
{
    bool success=false;
    switch (m_budgets.remove(id)) {
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

bool User::add_account(const QString &name, const QString &notes, bool archived, const Amount &initial_amount, QString &error)
{
    AccountShPtr existing=find_account(name);
    if(!existing.isNull()) {
        error=tr("An account having the same name already exist.");
        return false;
    }
    AccountShPtr account=AccountShPtr(new Account(name, notes, archived, initial_amount, this));
    m_accounts.insert(account->id(), account);
    emit modified();
    return true;
}

bool User::remove_account(QUuid id)
{
    bool success=false;
    switch (m_accounts.remove(id)) {
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

bool budget_cmp(const BudgetShPtr &a, const BudgetShPtr &b)
{
    return a->name()<b->name();
}

BudgetShPtrList User::budgets(bool sorted) const
{
    BudgetShPtrList budgets=m_budgets.values();
    if(sorted) {
        std::sort(budgets.begin(), budgets.end(), budget_cmp);
    }
    return budgets;
}

QStringList User::budgets_str(bool sorted) const
{
    QStringList budgets_str;
    BudgetShPtrList budget_list=budgets(sorted);
    for(auto &budget : budget_list) {
        budgets_str<<budget->name();
    }
    return budgets_str;
}

bool account_cmp(const AccountShPtr &a, const AccountShPtr &b)
{
    return a->name()<b->name();
}

AccountShPtrList User::accounts(bool sorted) const
{
    AccountShPtrList accounts=m_accounts.values();
    if(sorted) {
        std::sort(accounts.begin(), accounts.end(), account_cmp);
    }
    return accounts;
}

BudgetShPtr User::find_budget(QUuid id) const
{
    BudgetShPtr budget;
    QHash<QUuid, BudgetShPtr>::const_iterator it=m_budgets.find(id);
    if(it!=m_budgets.end()) {
        budget=*it;
    }
    return budget;
}

BudgetShPtr User::find_budget(const QString &name) const
{
    for(const auto &budget : m_budgets) {
        if(budget->name()==name) {
            return budget;
        }
    }
    return BudgetShPtr();
}

AccountShPtr User::find_account(QUuid id) const
{
    AccountShPtr account;
    QHash<QUuid, AccountShPtr>::const_iterator it=m_accounts.find(id);
    if(it!=m_accounts.end()) {
        account=*it;
    }
    return account;
}

AccountShPtr User::find_account(const QString &name) const
{
    for(const auto &account : m_accounts) {
        if(account->name()==name) {
            return account;
        }
    }
    return AccountShPtr();
}

bool User::read(const QJsonObject &json)
{
    LOG_IN("<QJsonObject>")
    static const QStringList keys=(QStringList()<<KW_NAME);
    JSON_CHECK_KEYS(keys);
    m_name=json[KW_NAME].toString();
    set_valid(PicsouDBO::read_wrapped(json));
    LOG_BOOL_RETURN(valid())
}

bool User::write(QJsonObject &json) const
{
    LOG_IN("<QJsonObject>")
    json[KW_NAME]=m_name;
    LOG_BOOL_RETURN(PicsouDBO::write_wrapped(json))
}

bool User::read_unwrapped(const QJsonObject &json)
{
    LOG_IN("<QJsonObject>")
    static const QStringList keys=(QStringList()<<KW_BUDGETS
                                                <<KW_ACCOUNTS);
    JSON_CHECK_KEYS(keys);
    JSON_READ_LIST(json, KW_BUDGETS, m_budgets, Budget, this);
    JSON_READ_LIST(json, KW_ACCOUNTS, m_accounts, Account, this);
    set_valid(true);
    LOG_BOOL_RETURN(valid())
}

bool User::write_unwrapped(QJsonObject &json) const
{
    LOG_IN("<QJsonObject>")
    JSON_WRITE_LIST(json, KW_BUDGETS, m_budgets.values());
    JSON_WRITE_LIST(json, KW_ACCOUNTS, m_accounts.values());
    LOG_BOOL_RETURN(true)
}

bool User::operator <(const User &other)
{
    return (m_name<other.m_name);
}
