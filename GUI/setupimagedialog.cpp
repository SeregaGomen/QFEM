#include <cmath>
#include <QColorDialog>

#include "setupimagedialog.h"
#include "ui_setupimagedialog.h"

TSetupImageDialog::TSetupImageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TSetupImageDialog)
{
    ui->setupUi(this);
    initParams();

    connect(ui->hsTranslateX, SIGNAL(valueChanged  (int)),this, SLOT(slotTranslateX(int)));
    connect(ui->hsTranslateY, SIGNAL(valueChanged  (int)),this, SLOT(slotTranslateY(int)));
    connect(ui->hsTranslateZ, SIGNAL(valueChanged  (int)),this, SLOT(slotTranslateZ(int)));
    connect(ui->hsAngleX, SIGNAL(valueChanged  (int)),this, SLOT(slotRotateX(int)));
    connect(ui->hsAngleY, SIGNAL(valueChanged  (int)),this, SLOT(slotRotateY(int)));
    connect(ui->hsAngleZ, SIGNAL(valueChanged  (int)),this, SLOT(slotRotateZ(int)));
    connect(ui->hsScale, SIGNAL(valueChanged  (int)),this, SLOT(slotScale(int)));
    connect(ui->hsKoff, SIGNAL(valueChanged  (int)),this, SLOT(slotTransformation(int)));
    connect(ui->pbColor, SIGNAL(clicked()), this, SLOT(slotChangeBkgColor()));

}

TSetupImageDialog::~TSetupImageDialog()
{
    delete ui;
}

void TSetupImageDialog::slotChangeBkgColor(void)
{
    QColorDialog* cDlg = new QColorDialog(bkgColor,this);

    if (cDlg->exec() == QDialog::Accepted)
    {
        bkgColor = cDlg->currentColor();
//        ui->pbColor->setStyleSheet(QString("background-color: rgb(%1, %2, %3); color: rgb(99, 99, 0)").arg(bkgColor.red()).arg(bkgColor.green()).arg(bkgColor.blue()));
        ui->pbColor->setStyleSheet(QString("background-color: rgb(%1, %2, %3)").arg(bkgColor.red()).arg(bkgColor.green()).arg(bkgColor.blue()));
    }
    delete cDlg;
}

void TSetupImageDialog::setup(ImageType type)
{
    // Инициализация параметров визуализации
    if (dimension == 1)
    {
        ui->chbMesh->setChecked(true);
        ui->chbMesh->setEnabled(false);
    }
    else
    {
        ui->chbMesh->setChecked(params.isMesh);
        ui->chbMesh->setEnabled(true);
    }
    ui->chbVertex->setChecked(params.isVertex);
//    if (dimension == 1)
//    {
//        ui->chbFace->setChecked(false);
//        ui->chbFace->setEnabled(false);
//    }
//    else
        ui->chbFace->setChecked(params.isFace);
    if (dimension < 3)
    {
        ui->chbLight->setChecked(false);
        ui->chbLight->setEnabled(false);
        ui->chbNormal->setEnabled(false);
    }
    else
        ui->chbLight->setChecked(params.isLight);
    ui->chbDoubleSided->setChecked(params.isDoubleSided);
    ui->chbCoord->setChecked(params.isCoord);
    ui->chbNegative->setChecked(params.isNegative);
    ui->chbValueScale->setChecked(params.isShowLegend);
    ui->chbNormal->setChecked(params.isNormal);

    ui->rbBW->setChecked(params.isBW);
    ui->rbColor->setChecked(params.isColor);

    if (type == ImageType::func)
        setTransformValue();
    setScaleValue();
    setAngleX();
    setAngleY();
    setAngleZ();
    setTranslateX();
    setTranslateY();
    setTranslateZ();

    ui->hsAlpha->setSliderPosition(int(params.alpha*10.0f));

    switch (params.numColor)
    {
        case 16:
            ui->hsNumColor->setSliderPosition(1);
            break;
        case 32:
            ui->hsNumColor->setSliderPosition(2);
            break;
        case 64:
            ui->hsNumColor->setSliderPosition(3);
            break;
        case 128:
            ui->hsNumColor->setSliderPosition(4);
            break;
        case 256:
            ui->hsNumColor->setSliderPosition(5);
            break;
    }


    ui->gbFunc->setEnabled(type != ImageType::mesh);
    ui->gbKoff->setEnabled(type == ImageType::func);
    ui->chbValueScale->setEnabled(type != ImageType::mesh);
    ui->chbNormal->setEnabled(type == ImageType::mesh);

    // Цвет фона
    bkgColor = params.bkgColor;
    ui->pbColor->setAutoFillBackground(true);
    ui->pbColor->setStyleSheet(QString("background-color: rgb(%1, %2, %3)").arg(params.bkgColor.red()).arg(params.bkgColor.green()).arg(params.bkgColor.blue()));


}

void TSetupImageDialog::initParams(void)
{
    params.init();
}

void TSetupImageDialog::clear(void)
{
    initParams();
//    setup();
}


bool TSetupImageDialog::reload(void)
{
    params.isVertex = ui->chbVertex->isChecked();
    params.isMesh = ui->chbMesh->isChecked();
    params.isFace = ui->chbFace->isChecked();
    params.isCoord = ui->chbCoord->isChecked();
    params.isLight = ui->chbLight->isChecked();
    params.isDoubleSided = ui->chbDoubleSided->isChecked();
    params.isColor = ui->rbColor->isChecked();
    params.isSpectral = false;
    params.isBW = ui->rbBW->isChecked();
    params.isNegative = ui->chbNegative->isChecked();
    params.isShowLegend = ui->chbValueScale->isChecked();
    params.isNormal = ui->chbNormal->isChecked();
    params.koff = ui->leKoff->text().toFloat();
    params.alpha = float(ui->hsAlpha->sliderPosition())*0.1f;
    params.bkgColor = bkgColor;
    params.scale = ui->leScale->text().toFloat();
    params.angleX = ui->leAngleX->text().toInt();
    params.angleY = ui->leAngleY->text().toInt();
    params.angleZ = ui->leAngleZ->text().toInt();
    params.translateX = ui->leTranslateX->text().toFloat();
    params.translateY = ui->leTranslateY->text().toFloat();
    params.translateZ = ui->leTranslateZ->text().toFloat();

    switch (ui->hsNumColor->sliderPosition())
    {
        case 1:
            params.numColor = 16;
            break;
        case 2:
            params.numColor = 32;
            break;
        case 3:
            params.numColor = 64;
            break;
        case 4:
            params.numColor = 128;
            break;
        case 5:
            params.numColor = 256;
            break;
    }
    return true;
}

void TSetupImageDialog::accept(void)
{
    if (reload())
        QDialog::accept();
}

void TSetupImageDialog::slotTransformation(int pos)
{
    double koff = (pos == 0) ? 0 : pow(10,0.1*pos);

    ui->leKoff->setText(tr("%1").arg(koff,4,'e',2));
}

void TSetupImageDialog::slotScale(int pos)
{
    double koff = (pos == 0) ? 1 : pow(10,0.02*pos);

    ui->leScale->setText((pos == 0) ? QString("1") : QString("%1").arg(koff,6,'f',3));
}

void TSetupImageDialog::slotTranslateX(int pos)
{
    ui->leTranslateX->setText(tr("%1").arg(double(pos)/10));
}

void TSetupImageDialog::slotTranslateY(int pos)
{
    ui->leTranslateY->setText(tr("%1").arg(double(pos)/10));
}

void TSetupImageDialog::slotTranslateZ(int pos)
{
    ui->leTranslateZ->setText(tr("%1").arg(double(pos)/10));
}

void TSetupImageDialog::slotRotateX(int pos)
{
    ui->leAngleX->setText(tr("%1").arg(pos));
}

void TSetupImageDialog::slotRotateY(int pos)
{
    ui->leAngleY->setText(tr("%1").arg(pos));
}

void TSetupImageDialog::slotRotateZ(int pos)
{
    ui->leAngleZ->setText(tr("%1").arg(pos));
}

void TSetupImageDialog::setTransformValue(void)
{
    int pos = (params.koff == 0.0f) ? 0 : int(ceil(10.0f*log10(params.koff)));

    ui->hsKoff->setValue(pos);
    ui->leKoff->setText((pos == 0) ? QString("0") : QString("%1").arg(double(params.koff), 4, 'e', 2));
}

void TSetupImageDialog::changeLanguage(void)
{
    ui->retranslateUi(this);
}

void TSetupImageDialog::setScaleValue(void)
{
    int pos = int(ceil(50.0f*log10(params.scale)));

    ui->hsScale->setValue(pos);
    ui->leScale->setText((pos == 0) ? QString("1") : QString("%1").arg(double(params.scale), 6, 'f', 3));
}

void TSetupImageDialog::setAngleX(void)
{
    int angle = (params.angleX%360 < 0) ? 360 + params.angleX%360 : params.angleX%360;

    ui->leAngleX->setText(QString("%1").arg(angle));
    ui->hsAngleX->setValue(angle);
}

void TSetupImageDialog::setAngleY(void)
{
    int angle = (params.angleY%360 < 0) ? 360 + params.angleY%360 : params.angleY%360;

    ui->leAngleY->setText(QString("%1").arg(angle));
    ui->hsAngleY->setValue(angle);
}

void TSetupImageDialog::setAngleZ(void)
{
    int angle = (params.angleZ%360 < 0) ? 360 + params.angleZ%360 : params.angleZ%360;

    ui->leAngleZ->setText(QString("%1").arg(angle));
    ui->hsAngleZ->setValue(angle);
}

void TSetupImageDialog::setTranslateX(void)
{
    ui->leTranslateX->setText(QString("%1").arg(double(params.translateX), 4, 'f',2));
    ui->hsTranslateX->setValue(int(params.translateX*10));
}

void TSetupImageDialog::setTranslateY(void)
{
    ui->leTranslateY->setText(QString("%1").arg(double(params.translateY), 4, 'f',2));
    ui->hsTranslateY->setValue(int(params.translateY*10));
}

void TSetupImageDialog::setTranslateZ(void)
{
    ui->leTranslateZ->setText(QString("%1").arg(double(params.translateZ), 4, 'f', 2));
    ui->hsTranslateZ->setValue(int(params.translateZ*10));
}
