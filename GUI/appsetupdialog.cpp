#include <thread>
#include "appesetupdialog.h"
#include "ui_appsetupdialog.h"

TAppSetupDialog::TAppSetupDialog(int lang, int thread, bool results, bool scroll, bool protocol, QWidget *parent) :
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
    ui->cbAutoScroll->setChecked(scroll);
    ui->cbAutoSaveResults->setChecked(results);
    ui->cbAutoSaveProtocol->setChecked(protocol);
    ui->sbThread->setMinimum(1);
    ui->sbThread->setMaximum((std::thread::hardware_concurrency() - 1 <= 0) ? 1 : std::thread::hardware_concurrency() - 1);
    ui->sbThread->setValue((thread <= 0) ? 1 : thread);
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

int TAppSetupDialog::getNumThread(void)
{
    return ui->sbThread->value();
}


bool TAppSetupDialog::getIsAutoSaveResults(void)
{
    return ui->cbAutoSaveResults->isChecked();
}

bool TAppSetupDialog::getIsAutoScroll(void)
{
    return ui->cbAutoScroll->isChecked();
}

bool TAppSetupDialog::getIsAutoSaveProtocol(void)
{
    return ui->cbAutoSaveProtocol->isChecked();
}
