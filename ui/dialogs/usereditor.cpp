#include "usereditor.h"
#include "ui_usereditor.h"

UserEditor::~UserEditor()
{
    delete ui;
    /* do not delete private members here */
}

UserEditor::UserEditor(QString *username,
                       QSecureMemory *old_pwd,
                       QSecureMemory *new_pwd,
                       QWidget *parent) :
    QDialog(parent),
    _username(username),
    _old_pwd(old_pwd),
    _new_pwd(new_pwd),
    ui(new Ui::UserEditor)
{
    bool user_edit=false;
    ui->setupUi(this);

    setWindowTitle(tr("User Editor"));

    ui->error->setVisible(false);

    ui->old_pwd->setEchoMode(QLineEdit::Password);
    ui->new_pwd->setEchoMode(QLineEdit::Password);
    ui->repeat_new_pwd->setEchoMode(QLineEdit::Password);

    if((user_edit=!_username->isNull())) {
        ui->username->setText(*_username);
    }

    ui->old_pwd->setVisible(user_edit);
    ui->old_pwd_label->setVisible(user_edit);

    connect(ui->save, &QPushButton::clicked,
            this, &UserEditor::accept);
    connect(ui->cancel, &QPushButton::clicked,
            this, &UserEditor::reject);
}


#define ERASE_LINE_EDIT(le) \
    (le)->selectAll(); (le)->del()
#define SECMEMCPY(dst, src) \
    do{ \
        QByteArray ba; \
        ba=(src)->text().toUtf8(); \
        if(ba.length()>0) { \
            ERASE_LINE_EDIT(src); \
            if(!(dst)->resize(ba.length())) { \
                goto exception; \
            } \
            memcpy((dst)->data(), ba.constData(), ba.length()); \
            for(int i=0; i<ba.length(); i++) { \
                ba[i]=0; \
            } \
            ba.clear(); \
        } \
    } while(0)

void UserEditor::accept()
{
    ui->error->setVisible(false);

    if(ui->new_pwd->text()!=ui->repeat_new_pwd->text()) {
        ui->error->setVisible(true);
        return;
    }

    ERASE_LINE_EDIT(ui->repeat_new_pwd);
    SECMEMCPY(_old_pwd, ui->old_pwd);
    SECMEMCPY(_new_pwd, ui->new_pwd);
    (*_username)=ui->username->text();
    QDialog::accept();
    goto end;

exception:
    QDialog::reject();
end:
    return;
}

#undef SECMEMCPY
#undef ERASE_LINE_EDIT