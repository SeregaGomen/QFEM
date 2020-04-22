#include "helpdialog.h"
#include "ui_helpdialog.h"

THelpDialog::THelpDialog(int curTab,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::THelpDialog)
{
    ui->setupUi(this);
    ui->tabWidget->setCurrentIndex(curTab);
}

THelpDialog::~THelpDialog()
{
    delete ui;
}
