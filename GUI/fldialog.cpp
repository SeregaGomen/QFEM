#include "fldialog.h"
#include "ui_funlistdialog.h"
#include "object/object.h"

TFunListDialog::TFunListDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TFunListDialog)
{
    ui->setupUi(this);
}

TFunListDialog::~TFunListDialog()
{
    delete ui;
}

// Заполнение списка названиями функций
void TFunListDialog::setup(TFEMObject* obj)
{
    ui->listWidget->clear();
    for (unsigned i = 0; i < obj->getResult().size(); i++)
        ui->listWidget->addItem(QString(obj->getResult(i).getName().c_str()));
    ui->listWidget->setCurrentRow(0);
}

QString TFunListDialog::getName(void)
{
    return ui->listWidget->item(ui->listWidget->currentRow())->text();
}

int TFunListDialog::getIndex(void)
{
    return ui->listWidget->currentRow();
}

void TFunListDialog::changeLanguage(void)
{
    ui->retranslateUi(this);
}
