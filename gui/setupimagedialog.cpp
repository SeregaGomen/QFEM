#include <cmath>
#include <QColorDialog>

#include "setupimagedialog.h"
#include "imageparams.h"
#include "meshview.h"
#include "ui_setupimagedialog.h"

TSetupImageDialog::TSetupImageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TSetupImageDialog)
{
    ui->setupUi(this);
    //initParams();

    connect(ui->hsTranslateX, SIGNAL(valueChanged(int)), this, SLOT(slotTranslateX(int)));
    connect(ui->hsTranslateY, SIGNAL(valueChanged(int)), this, SLOT(slotTranslateY(int)));
    connect(ui->hsAngleX, SIGNAL(valueChanged(int)), this, SLOT(slotRotateX(int)));
    connect(ui->hsAngleY, SIGNAL(valueChanged(int)), this, SLOT(slotRotateY(int)));
    connect(ui->hsAngleZ, SIGNAL(valueChanged(int)), this, SLOT(slotRotateZ(int)));
    connect(ui->hsScale, SIGNAL(valueChanged(int)), this, SLOT(slotScale(int)));
    connect(ui->hsRatio, SIGNAL(valueChanged(int)), this, SLOT(slotTransformation(int)));
    connect(ui->pbColor, SIGNAL(clicked()), this, SLOT(slotChangeBkgColor()));

    connect(ui->rbMesh, SIGNAL(clicked()), this, SLOT(slotIsMesh()));
    connect(ui->rbSurface, SIGNAL(clicked()), this, SLOT(slotIsSurface()));
    connect(ui->rbSurfaceMesh, SIGNAL(clicked()), this, SLOT(slotIsSurfaceMesh()));
    connect(ui->chbAxis, SIGNAL(clicked()), this, SLOT(slotIsAxis()));
    connect(ui->chbLight, SIGNAL(clicked()), this, SLOT(slotIsLight()));
    connect(ui->hsAlpha, SIGNAL(valueChanged(int)), this, SLOT(slotChangeAlpha(int)));
    connect(ui->hsNumColor, SIGNAL(valueChanged(int)), this, SLOT(slotChangeNumColor(int)));
    connect(ui->chbAutorotate, SIGNAL(clicked()), this, SLOT(slotIsAutoRotate()));
    connect(ui->chbLegend, SIGNAL(clicked()), this, SLOT(slotIsShowLegend()));

    connect(ui->rbColor, SIGNAL(clicked()), this, SLOT(slotIsColor()));
    connect(ui->rbGrayscale, SIGNAL(clicked()), this, SLOT(slotIsGrayscale()));
}

TSetupImageDialog::~TSetupImageDialog()
{
    delete ui;
}

void TSetupImageDialog::slotIsGrayscale(void)
{
    params->isGrayscale = true;
    glWidget->redraw();
}

void TSetupImageDialog::slotIsColor(void)
{
    params->isGrayscale = false;
    glWidget->redraw();
}

void TSetupImageDialog::slotIsShowLegend(void)
{
    params->isShowLegend = !params->isShowLegend;
    glWidget->update();
}

void TSetupImageDialog::slotChangeNumColor(int value)
{
    ui->labelNumColor->setText(QString::number((value + 1)*32));
    params->numColor = (value + 1)*32;
    glWidget->redraw();
}

void TSetupImageDialog::slotIsAutoRotate(void)
{
    params->isAutoRotate = ui->chbAutorotate->isChecked();
    if (!params->isAutoRotate)
    {
        ui->gbRotate->setVisible(true);
        ui->hsAngleX->setValue(params->angle[0]);
        ui->hsAngleY->setValue(params->angle[1]);
        ui->hsAngleZ->setValue(params->angle[2]);
    }
    else
        ui->gbRotate->setVisible(false);
    // ui->hsAngleX->setEnabled(!params->isAutoRotate);
    // ui->hsAngleY->setEnabled(!params->isAutoRotate);
    // ui->hsAngleZ->setEnabled(!params->isAutoRotate);
    glWidget->update();

    emit sendAutoRotateState(params->isAutoRotate);
}

void TSetupImageDialog::slotIsLight(void)
{
    params->isLight = ui->chbLight->isChecked();
    glWidget->update();
}

void TSetupImageDialog::slotIsMesh(void)
{
    params->isSurface = false;
    params->isMesh = true;
    glWidget->update();
}

void TSetupImageDialog::slotIsSurface(void)
{
    params->isSurface = true;
    params->isMesh = false;
    glWidget->update();
}

void TSetupImageDialog::slotIsSurfaceMesh(void)
{
    params->isSurface = params->isMesh = true;
    glWidget->update();
}

void TSetupImageDialog::slotIsAxis(void)
{
    params->isAxis = ui->chbAxis->isChecked();
    glWidget->update();
}

void TSetupImageDialog::slotChangeAlpha(int value)
{
    ui->labelTransparency->setText(QString::number(float(value)/10.0f, 'f', 1));
    params->alpha = float(value)/10.0f;
    glWidget->update();
}

void TSetupImageDialog::write(QDataStream& file)
{
    params->write(file);
}
void TSetupImageDialog::read(QDataStream& file)
{
    params->read(file);
}

void TSetupImageDialog::slotChangeBkgColor(void)
{
    QColorDialog* cDlg = new QColorDialog(bkgColor, this);

    if (cDlg->exec() == QDialog::Accepted)
    {
        bkgColor = cDlg->currentColor();
//        ui->pbColor->setStyleSheet(QString("background-color: rgb(%1, %2, %3); color: rgb(99, 99, 0)").arg(bkgColor.red()).arg(bkgColor.green()).arg(bkgColor.blue()));
        ui->pbColor->setStyleSheet(QString("background-color: rgb(%1, %2, %3)").arg(bkgColor.red()).arg(bkgColor.green()).arg(bkgColor.blue()));
        params->bkgColor = bkgColor;
        glWidget->update();
    }
    delete cDlg;
}

void TSetupImageDialog::setup(ImageType type)
{
    // Инициализация параметров визуализации
    if (dimension == 1)
    {
        ui->rbMesh->setChecked(false);
        ui->rbMesh->setEnabled(false);
        ui->rbSurfaceMesh->setChecked(false);
        ui->rbSurfaceMesh->setEnabled(false);
    }
    else
    {
        ui->rbSurface->setChecked(params->isSurface);
        ui->rbSurface->setEnabled(true);
        ui->rbMesh->setChecked(params->isMesh);
        ui->rbMesh->setEnabled(true);
        ui->rbSurfaceMesh->setChecked(params->isSurface && params->isMesh);
        ui->rbSurfaceMesh->setEnabled(true);
    }
    if (dimension < 3)
    {
        ui->chbLight->setChecked(false);
        ui->chbLight->setEnabled(false);
    }
    else
        ui->chbLight->setChecked(params->isLight);
    ui->chbAxis->setChecked(params->isAxis);
    ui->chbLegend->setChecked(params->isShowLegend);
    ui->chbAutorotate->setChecked(params->isAutoRotate);

    if (params->isAutoRotate)
    {
        ui->gbRotate->setVisible(false);
    }
    else
    {
        ui->gbRotate->setVisible(true);
        // ui->labelAngleX->setText(QString::number(ui->hsAngleX->value()));
        // ui->labelAngleY->setText(QString::number(ui->hsAngleY->value()));
        // ui->labelAngleZ->setText(QString::number(ui->hsAngleZ->value()));
        ui->labelAngleX->setText(QString::number(params->angle[0]));
        ui->labelAngleY->setText(QString::number(params->angle[1]));
        ui->labelAngleZ->setText(QString::number(params->angle[2]));

        ui->hsAngleX->setValue(params->angle[0]);
        ui->hsAngleY->setValue(params->angle[1]);
        ui->hsAngleZ->setValue(params->angle[2]);
    }


    ui->rbGrayscale->setChecked(params->isGrayscale);
    ui->rbColor->setChecked(!params->isGrayscale);

    ui->hsAlpha->setValue(int(params->alpha*10.0f));
    ui->labelTransparency->setText(QString::number(params->alpha, 'f', 1));
    ui->hsScale->setValue(int(params->scale*10.0f));
    ui->labelScale->setText(QString::number(params->scale, 'f', 1));
    ui->hsRatio->setValue(int(params->ratio*10.0f));
    ui->labelTransformation->setText(QString::number(params->ratio, 'f', 1));
    ui->hsNumColor->setValue(params->numColor/32 - 1);
    ui->labelNumColor->setText(QString::number(params->numColor));

    ui->hsTranslateX->setValue(int(params->translate[0]*2.0f));
    ui->labelTranslateX->setText(QString::number(params->translate[0], 'f', 1));
    ui->hsTranslateY->setValue(int(params->translate[1]*2.0f));
    ui->labelTranslateY->setText(QString::number(params->translate[1], 'f', 1));


    ui->gbFunc->setEnabled(type != ImageType::mesh);
    ui->gbTransformation->setEnabled(type == ImageType::func);
    //ui->chbLegend->setEnabled(type not_eq ImageType::mesh);

    // Цвет фона
    bkgColor = params->bkgColor;
    ui->pbColor->setAutoFillBackground(true);
    ui->pbColor->setStyleSheet(QString("background-color: rgb(%1, %2, %3)").arg(params->bkgColor.red()).arg(params->bkgColor.green()).arg(params->bkgColor.blue()));
}

void TSetupImageDialog::initParams(void)
{
    params->init();
}

void TSetupImageDialog::clear(void)
{
    //initParams();
//    setup();
}


void TSetupImageDialog::accept(void)
{
    QDialog::accept();
}

void TSetupImageDialog::slotTransformation(int pos)
{
    ui->labelTransformation->setText(QString::number(float(pos)/10.0f, 'f', 1));
    params->ratio = float(pos)/10.0f;
    glWidget->redraw();
}

void TSetupImageDialog::slotScale(int pos)
{
    ui->labelScale->setText(QString::number(float(pos)/10.0f, 'f', 1));
    params->scale = float(pos)/10.0f;
    glWidget->update();
}

void TSetupImageDialog::slotTranslateX(int pos)
{
    ui->labelTranslateX->setText(QString::number(float(pos)/2.0f, 'f', 1));
    params->translate[0] = float(pos)/2.0f;
    glWidget->update();
}

void TSetupImageDialog::slotTranslateY(int pos)
{
    ui->labelTranslateY->setText(QString::number(float(pos)/2.0f, 'f', 1));
    params->translate[1] = float(pos)/2.0f;
    glWidget->update();
}

void TSetupImageDialog::slotRotateX(int pos)
{
    ui->labelAngleX->setText(tr("%1").arg(pos));
    params->angle[0] = pos;
    glWidget->update();
}

void TSetupImageDialog::slotRotateY(int pos)
{
    ui->labelAngleY->setText(tr("%1").arg(pos));
    params->angle[1] = pos;
    glWidget->update();
}

void TSetupImageDialog::slotRotateZ(int pos)
{
    ui->labelAngleZ->setText(tr("%1").arg(pos));
    params->angle[2] = pos;
    glWidget->update();
}

void TSetupImageDialog::changeLanguage(void)
{
    ui->retranslateUi(this);
}

//Стандартная формула (Rec. 601 / ITU-R BT.601)

//    gray = 0.299 * R + 0.587 * G + 0.114 * B
