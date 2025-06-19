#include <QMessageBox>
#include <QPushButton>
#include "vcdialog.h"
#include "ui_vcdialog.h"


TVCDialog::TVCDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TVCDialog)
{
    ui->setupUi(this);

    ui->buttonBox->button(QDialogButtonBox::Ok )->setEnabled(false);
    ui->lineEdit->setFocus();
    connect(ui->lineEdit, SIGNAL(textChanged(QString)), this, SLOT(slotChangeText(QString)));
}

TVCDialog::~TVCDialog()
{
    delete ui;
}

QString TVCDialog::getExpression(void)
{
    return ui->lineEdit->text();
}

void TVCDialog::accept(void)
{
    if (not ui->lineEdit->text().length())
    {
        QMessageBox::critical(this, tr("Error"), tr("Invalid expression!"));
        return;
    }
    QDialog::accept();
}

void TVCDialog::showEvent(QShowEvent*)
{
//    ui->lineEdit->setText(expression);
//    ui->lineEdit_2->setText(value);
}


void TVCDialog::slotChangeText(const QString&)
{
    ui->buttonBox->button(QDialogButtonBox::Ok )->setEnabled(ui->lineEdit->text().length());
}

void TVCDialog::changeLanguage(void)
{
    ui->retranslateUi(this);
}
