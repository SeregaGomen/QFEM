#include "appesetupdialog.h"
#include "ui_appsetupdialog.h"

TAppSetupDialog::TAppSetupDialog(int lang,bool black, bool scroll,bool protocol,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TAppSetupDialog)
{
    ui->setupUi(this);
    switch (lang)
    {
        case 0:
            ui->rbEN->setChecked(true);
            break;
        case 1:
            ui->rbRU->setChecked(true);
    }
    ui->cbBlackBkg->setChecked(black);
    ui->cbAutoScroll->setChecked(scroll);
    ui->cbAutoSaveProtocol->setChecked(protocol);
}

TAppSetupDialog::~TAppSetupDialog()
{
    delete ui;
}

void TAppSetupDialog::changeLanguage(void)
{
    ui->retranslateUi(this);
}

int TAppSetupDialog::getLangNo(void)
{
    if (ui->rbEN->isChecked())
        return 0;
    else if (ui->rbRU->isChecked())
        return 1;
    return 2;
}

bool TAppSetupDialog::getIsBlackBkg(void)
{
    return ui->cbBlackBkg->isChecked();
}

bool TAppSetupDialog::getIsAutoScroll(void)
{
    return ui->cbAutoScroll->isChecked();
}

bool TAppSetupDialog::getIsAutoSaveProtocol(void)
{
    return ui->cbAutoSaveProtocol->isChecked();
}
