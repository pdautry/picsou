#include "accounteditor.h"
#include "ui_accounteditor.h"

AccountEditor::~AccountEditor()
{
    delete ui;
}

AccountEditor::AccountEditor(QString *name,
                             QString *description,
                             QWidget *parent) :
    QDialog(parent),
    _name(name),
    _description(description),
    ui(new Ui::AccountEditor)
{
    ui->setupUi(this);

    setWindowTitle(tr("Account Editor"));

    if(!_name->isNull()) {
        ui->name->setText(*_name);
    }
    if(!_description->isNull()) {
        ui->description->setPlainText(*_description);
    }

    connect(ui->save, &QPushButton::clicked,
            this, &AccountEditor::accept);
    connect(ui->cancel, &QPushButton::clicked,
            this, &AccountEditor::reject);
}

void AccountEditor::accept()
{
    (*_name)=ui->name->text();
    (*_description)=ui->description->toPlainText();
    QDialog::accept();
}


