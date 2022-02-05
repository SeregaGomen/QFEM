#include "gbdialog.h"
#include "ui_gbdialog.h"

TGBDialog::TGBDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TGBDialog)
{
    ui->setupUi(this);
}

TGBDialog::~TGBDialog()
{
    delete ui;
}

void TGBDialog::set(double mX,double mY,double mZ,double MX,double MY,double MZ,QString typeFE,unsigned numNodes,unsigned numFE)
{
    ui->minX->setText(QString("%1").arg(mX));
    ui->minY->setText(QString("%1").arg(mY));
    ui->minZ->setText(QString("%1").arg(mZ));

    ui->maxX->setText(QString("%1").arg(MX));
    ui->maxY->setText(QString("%1").arg(MY));
    ui->maxZ->setText(QString("%1").arg(MZ));

    ui->length->setText(QString("%1").arg(MX - mX));
    ui->width->setText(QString("%1").arg(MY - mY));
    ui->height->setText(QString("%1").arg(MZ - mZ));

    ui->typeFE->setText(typeFE);
    ui->numNodes->setText(QString("%1").arg(numNodes));
    ui->numFE->setText(QString("%1").arg(numFE));
}

void TGBDialog::changeLanguage(void)
{
    ui->retranslateUi(this);
}
