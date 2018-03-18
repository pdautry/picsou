#include "picsoumodelservice.h"

#include <QFile>
#include <QJsonObject>
#include <QJsonDocument>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "picsou.h"
#include "model/picsoudb.h"

#define XML_ELEM_OP "operation"
#define XML_ATTR_YEAR "year"
#define XML_ATTR_MONTH "month"
#define XML_ATTR_DAY "day"
#define XML_ATTR_AMOUNT "amount"
#define XML_ATTR_BUDGET "budget"
#define XML_ATTR_RECIPIENT "recipient"
#define XML_ATTR_PAYMENT_METHOD "paymentMethod"
#define XML_ATTR_DESCRIPTION "description"

PicsouModelService::~PicsouModelService()
{
    close_db();
}

PicsouModelService::PicsouModelService(PicsouApplication *papp) :
    PicsouAbstractService(papp),
    _db(nullptr),
    _filename(QString()),
    _is_db_modified(false)
{

}

bool PicsouModelService::initialize()
{
    return true;
}

void PicsouModelService::terminate()
{
    close_db();
}

bool PicsouModelService::new_db(QString filename,
                                QString name,
                                QString description)
{
    bool success;

    if(is_db_opened()) {
        success=false; goto end;
    }

    _db=PicsouDBPtr(new PicsouDB(PICSOU_DB_MAJOR,
                                 PICSOU_DB_MINOR,
                                 name,
                                 description));
    _filename=filename;
    _is_db_modified=true;

    connect(_db, &PicsouDB::modified, this, &PicsouModelService::notify_ui);

    success=true;
end:
    return success;
}

bool PicsouModelService::open_db(QString filename)
{
    bool success;
    QJsonDocument doc;
    QJsonParseError err;
    QFile f(filename);

    if(is_db_opened()) {
        success=false; goto end;
    }

    if(!f.open(QIODevice::ReadOnly)) {
        success=false; goto end;
    }

    doc=QJsonDocument::fromJson(f.readAll(), &err);

    if(doc.isNull()) {
        /* LOG err.errorString() */
        success=false; goto end;
    }

    _db=PicsouDBPtr(new PicsouDB);

    if(!_db->read(doc.object())) {
        success=false; goto end;
    }

    _filename=filename;

    connect(_db, &PicsouDB::modified, this, &PicsouModelService::notify_ui);

    success=true;
end:
    return success;
}

bool PicsouModelService::save_db()
{
    return save_db_as(_filename);
}

bool PicsouModelService::save_db_as(QString filename)
{
    bool success;
    QJsonObject json;
    QByteArray json_data;
    QFile f(filename);

    if(!is_db_opened()) {
        success=false; goto end;
    }

    if(!_db->write(json)) {
        success=false; goto end;
    }

    if(!f.open(QIODevice::WriteOnly)) {
        success=false; goto end;
    }

    json_data=QJsonDocument(json).toJson();

    if(f.write(json_data)!=json_data.size()) {
        success=false; goto end;
    }

    f.close();
    _filename=filename;
    _is_db_modified=false;
    success=true;
end:
    return success;
}

bool PicsouModelService::close_db()
{
    bool success=false;

    if(is_db_opened()) {
        _filename.clear();
        _is_db_modified=false;
        delete _db;
        success=true;
    }

    return success;
}

bool PicsouModelService::is_db_opened()
{
    return (!_db.isNull());
}

QList<OperationPtr> PicsouModelService::load_ops(ImportExportFormat fmt,
                                                 QString filename)
{
    QFile f(filename);
    QList<OperationPtr> ops;

    if(!f.open(QIODevice::ReadOnly)) {
        goto end;
    }

    switch (fmt) {
        case CSV: ops=csv_load_ops(f); break;
        case XML: ops=xml_load_ops(f); break;
        case JSON: ops=json_load_ops(f); break;
    }

    f.close();
end:
    return ops;
}

bool PicsouModelService::dump_ops(ImportExportFormat fmt,
                                  QString filename,
                                  QList<OperationPtr> ops)
{
    bool success=true;
    QFile f(filename);
    QJsonObject obj;

    if(!f.open(QIODevice::WriteOnly)) {
        success=false; goto end;
    }

    switch (fmt) {
        case CSV: success=csv_dump_ops(f, ops); break;
        case XML: success=xml_dump_ops(f, ops); break;
        case JSON: success=json_dump_ops(f, ops); break;
    }

    f.close();
end:
    return success;
}

UserPtr PicsouModelService::find_user(QUuid id) const
{
    return _db->find_user(id);
}

AccountPtr PicsouModelService::find_account(QUuid id) const
{
    return _db->find_account(id);
}

void PicsouModelService::notify_ui()
{
    _is_db_modified=true;
    emit updated(_db);
}

QList<OperationPtr> PicsouModelService::xml_load_ops(QFile &f)
{
    bool ok;
    int y,m,d;
    double amount;
    OperationPtr op;
    QList<OperationPtr> ops;
    QXmlStreamReader xml(&f);
    QXmlStreamAttributes attrs;
    while (!xml.atEnd()) {

        switch (xml.readNext()) {
        case QXmlStreamReader::Invalid:
            goto error;
        case QXmlStreamReader::StartDocument:
            if(xml.qualifiedName()=="operation") {
                attrs=xml.attributes();

                amount=attrs.value(XML_ATTR_AMOUNT).toDouble(&ok);
                if(!ok) { goto error; }
                y=attrs.value(XML_ATTR_YEAR).toInt(&ok);
                if(!ok) { goto error; }
                m=attrs.value(XML_ATTR_MONTH).toInt(&ok);
                if(!ok) { goto error; }
                d=attrs.value(XML_ATTR_DAY).toInt(&ok);
                if(!ok) { goto error; }

                op=OperationPtr(new Operation(amount,
                                              QDate(y, m, d),
                                              attrs.value(XML_ATTR_BUDGET).toString(),
                                              attrs.value(XML_ATTR_RECIPIENT).toString(),
                                              attrs.value(XML_ATTR_DESCRIPTION).toString(),
                                              attrs.value(XML_ATTR_PAYMENT_METHOD).toString(),
                                              nullptr));
                ops.append(op);
            }
            break;
        default: break;
        };

    }

    if(!xml.hasError()) {
        goto end;
    }

error:
    qWarning("import parsing error:");
    foreach (OperationPtr op, ops) {
        delete op;
    }
    ops.clear();
end:
    return ops;
}

QList<OperationPtr> PicsouModelService::csv_load_ops(QFile &f)
{
    double amount;
    bool ok, instr;
    QByteArray line;
    OperationPtr op;
    int y, m, d, idx;
    QList<OperationPtr> ops;
    QByteArray::const_iterator c;
    QString buffer, budget, recipient, payment_method;

    while(!f.atEnd()) {
        line=f.readLine().trimmed();
        if(line.length()>0) {
            idx=0;
            instr=false;

            for(c=line.constBegin(); c!=line.constEnd(); c++) {
                switch (*c) {
                case '"':
                    instr=!instr;
                    break;
                case ',':
                    if(!instr){
                        switch (idx) {
                        case 0: /* year */
                            y=buffer.toInt(&ok);
                            if(!ok) { goto error; }
                            break;
                        case 1: /* month */
                            m=buffer.toInt(&ok);
                            if(!ok) { goto error; }
                            break;
                        case 2: /* day */
                            d=buffer.toInt(&ok);
                            if(!ok) { goto error; }
                            break;
                        case 3: /* amount */
                            amount=buffer.toDouble(&ok);
                            if(!ok) { goto error; }
                            break;
                        case 4: /* budget */
                            budget=buffer;
                            break;
                        case 5: /* recipient */
                            recipient=buffer;
                            break;
                        case 6: /* payment method */
                            payment_method=buffer;
                            break;
                        case 7: /* description */
                            op=OperationPtr(new Operation(amount,
                                                          QDate(y, m, d),
                                                          budget,
                                                          recipient,
                                                          buffer,
                                                          payment_method,
                                                          nullptr));
                            ops.append(op);
                            break;
                        }
                        buffer.clear();
                        idx++;
                    }
                    break;
                default:
                    buffer.append(c);
                }
            }

        }
    }
    goto end;

error:
    qWarning("import parsing error:");
    foreach (OperationPtr op, ops) {
        delete op;
    }
    ops.clear();
end:
    return ops;
}

QList<OperationPtr> PicsouModelService::json_load_ops(QFile &f)
{
    QByteArray line;
    OperationPtr op;
    QJsonDocument doc;
    QList<OperationPtr> ops;

    while(!f.atEnd()) {
        line=f.readLine().trimmed();
        if(line.length()>0) {
            doc=QJsonDocument::fromJson(line);
            if(!doc.isNull()) {
                op=OperationPtr(new Operation(nullptr));
                ops.append(op);
                if(!op->read(doc.object())) {
                    goto error;
                }
            }
        }
    }
    goto end;

error:
    qWarning("import parsing error:");
    foreach (OperationPtr op, ops) {
        delete op;
    }
    ops.clear();
end:
    return ops;
}

bool PicsouModelService::xml_dump_ops(QFile &f, QList<OperationPtr> ops)
{
    bool success;
    QXmlStreamWriter xml(&f);
    xml.writeStartDocument("1.0", true);
    xml.writeStartElement("operations");
    foreach (OperationPtr op, ops) {
        xml.writeEmptyElement(XML_ELEM_OP);
        xml.writeAttribute(XML_ATTR_YEAR, QString::number(op->date().year()));
        xml.writeAttribute(XML_ATTR_MONTH, QString::number(op->date().month()));
        xml.writeAttribute(XML_ATTR_DAY, QString::number(op->date().day()));
        xml.writeAttribute(XML_ATTR_AMOUNT, op->amount_str());
        xml.writeAttribute(XML_ATTR_BUDGET, op->budget());
        xml.writeAttribute(XML_ATTR_RECIPIENT, op->recipient());
        xml.writeAttribute(XML_ATTR_PAYMENT_METHOD, op->payment_method());
        xml.writeAttribute(XML_ATTR_DESCRIPTION, op->description());
    }
    xml.writeEndElement();
    xml.writeEndDocument();
    success=true;
//end:
    return success;
}

bool PicsouModelService::csv_dump_ops(QFile &f, QList<OperationPtr> ops)
{
    bool success;
    foreach (OperationPtr op, ops) {
        f.write(QString("%0,%1,%2,%3,\"%4\",\"%5\",\"%6\",\"%7\"\n").arg(
                    QString::number(op->date().year()),
                    QString::number(op->date().month()),
                    QString::number(op->date().day()),
                    op->amount_str(),
                    op->budget().replace('"', '\''),
                    op->recipient().replace('"', '\''),
                    op->payment_method().replace('"', '\''),
                    op->description().replace('"', '\''))
                .toUtf8());
    }
    success=true;
//end:
    return success;
}

bool PicsouModelService::json_dump_ops(QFile &f, QList<OperationPtr> ops)
{
    bool success;
    QJsonObject obj;
    foreach (OperationPtr op, ops) {
        if(!op->write(obj)) {
            f.write(tr("-- [export error] --").toUtf8());
            success=false; goto end;
        }
        f.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    }
    success=true;
end:
    return success;
}
