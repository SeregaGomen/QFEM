#include <QTextDocument>
#include <QTextEdit>
#include <QMessageBox>
#include <QCloseEvent>
#include <QComboBox>
#include <sstream>

#include "vcdialog.h"
#include "setuptaskdialog.h"
#include "ui_setuptaskdialog.h"
#include "glmesh.h"
#include "lclist.h"
#include "msg/msg.h"
#include "parser/parser.h"
#include "object/object.h"

TSetupTaskDialog::TSetupTaskDialog(TFEMObject* obj, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TSetupTaskDialog)
{
    ui->setupUi(this);

    femObject = obj;
    isBoundaryConditionChanged = false;

    // Настраиваем контекстное меню
    ui->twBC->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->twCV->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->twVV->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->twSV->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->twPV->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->twStressStrainCurve->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->twVariables->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->twYoungModulus->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->twPoissonsRatio->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->twThermalExpansion->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->twTemperature->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->twThickness->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->twDensity->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->twDamping->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->twCV, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowContextMenu1(QPoint)));
    connect(ui->twVV, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowContextMenu1(QPoint)));
    connect(ui->twSV, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowContextMenu1(QPoint)));
    connect(ui->twBC, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowContextMenu1(QPoint)));
    connect(ui->twPV, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twStressStrainCurve, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twVariables, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twYoungModulus, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twPoissonsRatio, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twThermalExpansion, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twTemperature, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twThickness, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twDensity, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twDamping, SIGNAL(customContextMenuRequested(QPoint)),this, SLOT(slotShowContextMenu2(QPoint)));

    connect(ui->twBC->selectionModel(), &QItemSelectionModel::selectionChanged, ([=](void) { setEnabledBtn(ui->tbRemoveBC, ui->twBC); }));
    connect(ui->twCV->selectionModel(), &QItemSelectionModel::selectionChanged, ([=](void) { setEnabledBtn(ui->tbRemoveCV, ui->twCV); }));
    connect(ui->twSV->selectionModel(), &QItemSelectionModel::selectionChanged, ([=](void) { setEnabledBtn(ui->tbRemoveSV, ui->twSV); }));
    connect(ui->twVV->selectionModel(), &QItemSelectionModel::selectionChanged, ([=](void) { setEnabledBtn(ui->tbRemoveVV, ui->twVV); }));
    connect(ui->twPV->selectionModel(), &QItemSelectionModel::selectionChanged, ([=](void) { setEnabledBtn(ui->tbRemovePV, ui->twPV); }));
    connect(ui->twStressStrainCurve->selectionModel(), &QItemSelectionModel::selectionChanged, ([=](void) { setEnabledBtn(ui->tbRemoveProp, ui->twStressStrainCurve); }));
    connect(ui->twVariables->selectionModel(), &QItemSelectionModel::selectionChanged, ([=](void) { setEnabledBtn(ui->tbRemoveVariables, ui->twVariables); }));
    connect(ui->twYoungModulus->selectionModel(), &QItemSelectionModel::selectionChanged, ([=](void) { setEnabledBtn(ui->tbRemoveYoungModulus, ui->twYoungModulus); }));
    connect(ui->twPoissonsRatio->selectionModel(), &QItemSelectionModel::selectionChanged, ([=](void) { setEnabledBtn(ui->tbRemovePoissonsRatio, ui->twPoissonsRatio); }));
    connect(ui->twThermalExpansion->selectionModel(), &QItemSelectionModel::selectionChanged, ([=](void) { setEnabledBtn(ui->tbRemoveThermalExpansion, ui->twThermalExpansion); }));
    connect(ui->twTemperature->selectionModel(), &QItemSelectionModel::selectionChanged, ([=](void) { setEnabledBtn(ui->tbRemoveTemperature, ui->twTemperature); }));
    connect(ui->twThickness->selectionModel(), &QItemSelectionModel::selectionChanged, ([=](void) { setEnabledBtn(ui->tbRemoveThickness, ui->twThickness); }));
    connect(ui->twDensity->selectionModel(), &QItemSelectionModel::selectionChanged, ([=](void) { setEnabledBtn(ui->tbRemoveDensity, ui->twDensity); }));
    connect(ui->twDamping->selectionModel(), &QItemSelectionModel::selectionChanged, ([=](void) { setEnabledBtn(ui->tbRemoveDamping, ui->twDamping); }));

    connect(ui->rbStatic, &QRadioButton::clicked, this, &TSetupTaskDialog::enabledParams);
    connect(ui->rbDynamic, &QRadioButton::clicked, this, &TSetupTaskDialog::enabledParams);
    connect(ui->rbLinear, &QRadioButton::clicked, this, &TSetupTaskDialog::enabledParams);
    connect(ui->rbMVS, &QRadioButton::clicked, this, &TSetupTaskDialog::enabledParams);
    connect(ui->rbMES, &QRadioButton::clicked, this, &TSetupTaskDialog::enabledParams);

    connect(ui->tbAddPV, &QToolButton::clicked, ([=](void) { addRow(ui->twPV); }));
    connect(ui->tbAddVV, &QToolButton::clicked, ([=](void) { addRow(ui->twVV); }));
    connect(ui->tbAddSV, &QToolButton::clicked, ([=](void) { addRow(ui->twSV); }));
    connect(ui->tbAddCV, &QToolButton::clicked, ([=](void) { addRow(ui->twCV); }));
    connect(ui->tbAddBC, &QToolButton::clicked, ([=](void) { addRow(ui->twBC); }));
    connect(ui->tbAddProp, &QToolButton::clicked, ([=](void) { addRow(ui->twStressStrainCurve); }));
    connect(ui->tbAddVariable, &QToolButton::clicked, ([=](void) { addRow(ui->twVariables); }));
    connect(ui->tbAddYoungModulus, &QToolButton::clicked, ([=](void) { addRow(ui->twYoungModulus); }));
    connect(ui->tbAddPoissonsRatio, &QToolButton::clicked, ([=](void) { addRow(ui->twPoissonsRatio); }));
    connect(ui->tbAddThermalExpansion, &QToolButton::clicked, ([=](void) { addRow(ui->twThermalExpansion); }));
    connect(ui->tbAddTemperature, &QToolButton::clicked, ([=](void) { addRow(ui->twTemperature); }));
    connect(ui->tbAddThickness, &QToolButton::clicked, ([=](void) { addRow(ui->twThickness); }));
    connect(ui->tbAddDensity, &QToolButton::clicked, ([=](void) { addRow(ui->twDensity); }));
    connect(ui->tbAddDamping, &QToolButton::clicked, ([=](void) { addRow(ui->twDamping); }));

    connect(ui->tbRemovePV, &QToolButton::clicked, ([=](void) { removeRow(ui->twPV); setEnabledBtn(ui->tbRemovePV, ui->twPV); }));
    connect(ui->tbRemoveVV, &QToolButton::clicked, ([=](void) { removeRow(ui->twVV); setEnabledBtn(ui->tbRemoveVV, ui->twVV); }));
    connect(ui->tbRemoveSV, &QToolButton::clicked, ([=](void) { removeRow(ui->twSV); setEnabledBtn(ui->tbRemoveSV, ui->twSV); }));
    connect(ui->tbRemoveCV, &QToolButton::clicked, ([=](void) { removeRow(ui->twCV); setEnabledBtn(ui->tbRemoveCV, ui->twCV); }));
    connect(ui->tbRemoveBC, &QToolButton::clicked, ([=](void) { removeRow(ui->twBC); setEnabledBtn(ui->tbRemoveBC, ui->twBC); }));
    connect(ui->tbRemoveProp, &QToolButton::clicked, ([=](void) { removeRow(ui->twStressStrainCurve); setEnabledBtn(ui->tbRemoveProp, ui->twStressStrainCurve); }));
    connect(ui->tbRemoveVariables, &QToolButton::clicked, ([=](void) { removeRow(ui->twVariables); setEnabledBtn(ui->tbRemoveVariables, ui->twVariables); }));
    connect(ui->tbRemoveYoungModulus, &QToolButton::clicked, ([=](void) { removeRow(ui->twYoungModulus); setEnabledBtn(ui->tbRemoveYoungModulus, ui->twYoungModulus); }));
    connect(ui->tbRemovePoissonsRatio, &QToolButton::clicked, ([=](void) { removeRow(ui->twPoissonsRatio); setEnabledBtn(ui->tbRemovePoissonsRatio, ui->twPoissonsRatio); }));
    connect(ui->tbRemoveThermalExpansion, &QToolButton::clicked, ([=](void) { removeRow(ui->twThermalExpansion); setEnabledBtn(ui->tbRemoveThermalExpansion, ui->twThermalExpansion); }));
    connect(ui->tbRemoveTemperature, &QToolButton::clicked, ([=](void) { removeRow(ui->twTemperature); setEnabledBtn(ui->tbRemoveTemperature, ui->twTemperature); }));
    connect(ui->tbRemoveThickness, &QToolButton::clicked, ([=](void) { removeRow(ui->twThickness); setEnabledBtn(ui->tbRemoveThickness, ui->twThickness); }));
    connect(ui->tbRemoveDensity, &QToolButton::clicked, ([=](void) { removeRow(ui->twDensity); setEnabledBtn(ui->tbRemoveDensity, ui->twDensity); }));
    connect(ui->tbRemoveDamping, &QToolButton::clicked, ([=](void) { removeRow(ui->twDamping); setEnabledBtn(ui->tbRemoveDamping, ui->twDamping); }));

    connect(ui->twPV, SIGNAL(cellChanged(int, int)), this, SLOT(slotCellEdited(int, int)));
    connect(ui->twBC, SIGNAL(cellChanged(int, int)), this, SLOT(slotCellEdited(int, int)));
    connect(ui->twCV, SIGNAL(cellChanged(int, int)), this, SLOT(slotCellEdited(int, int)));
    connect(ui->twSV, SIGNAL(cellChanged(int, int)), this, SLOT(slotCellEdited(int, int)));
    connect(ui->twYoungModulus, SIGNAL(cellChanged(int, int)), this, SLOT(slotCellEdited(int, int)));
    connect(ui->twPoissonsRatio, SIGNAL(cellChanged(int, int)), this, SLOT(slotCellEdited(int, int)));
    connect(ui->twThermalExpansion, SIGNAL(cellChanged(int, int)), this, SLOT(slotCellEdited(int, int)));
    connect(ui->twTemperature, SIGNAL(cellChanged(int, int)), this, SLOT(slotCellEdited(int, int)));
    connect(ui->twThickness, SIGNAL(cellChanged(int, int)), this, SLOT(slotCellEdited(int, int)));
    connect(ui->twDensity, SIGNAL(cellChanged(int, int)), this, SLOT(slotCellEdited(int, int)));
    connect(ui->twDamping, SIGNAL(cellChanged(int, int)), this, SLOT(slotCellEdited(int, int)));

    createMenu();

    ui->twFuncName->setItemDelegate(new THTMLDelegate(this));
}

TSetupTaskDialog::~TSetupTaskDialog()
{
    delete ui;
}

void TSetupTaskDialog::createMenu(void)
{
    menu1.addAction(new QAction(tr("&Select all"), this));
    menu1.addAction(new QAction(tr("Clear &all"), this));
    menu1.addSeparator();
    menu1.addAction(new QAction(tr("Select &x"), this));
    menu1.addAction(new QAction(tr("Select &y"), this));
    menu1.addAction(new QAction(tr("Select &z"), this));
    menu1.addSeparator();
    menu1.addAction(new QAction(tr("&Remove all"), this));
    //-----------------------------------------
    menu2.addAction(new QAction(tr("&Select all"), this));
    menu2.addAction(new QAction(tr("Clear &all"), this));
    menu2.addSeparator();
    menu2.addAction(new QAction(tr("Select &U"), this));
    menu2.addAction(new QAction(tr("Select &V"), this));
    menu2.addAction(new QAction(tr("Select &W"), this));
    menu2.addSeparator();
    menu2.addAction(new QAction(tr("&Remove all"), this));
    //-----------------------------------------
    menu3.addAction(new QAction(tr("&Remove all"), this));
}


void TSetupTaskDialog::slotShowContextMenu1(const QPoint &pos)
{
    QPoint globalPos;
    QAction* selectedItem;
    QTableWidget* twPtr = static_cast<QTableWidget*>(ui->tabWidget->focusWidget());
    QMenu* menu = (ui->tabWidget->currentIndex() == 5) ? &menu1 : &menu2;

    if (sender()->inherits("QAbstractScrollArea"))
        globalPos = (reinterpret_cast<QAbstractScrollArea*>(sender()))->viewport()->mapToGlobal(pos);
    else
        globalPos = (reinterpret_cast<QWidget*>(sender()))->mapToGlobal(pos);

    if ((selectedItem = menu->exec(globalPos)))
    {
        if (selectedItem == menu->actions().at(0))
            setDirect(twPtr, DIR_X | DIR_Y | DIR_Z);
        else if (selectedItem == menu->actions().at(1))
            setDirect(twPtr, 0);
        else if (selectedItem == menu->actions().at(3))
            setDirect(twPtr, DIR_X);
        else if (selectedItem == menu->actions().at(4))
            setDirect(twPtr, DIR_Y);
        else if (selectedItem == menu->actions().at(5))
            setDirect(twPtr, DIR_Z);
        else if (selectedItem == menu->actions().at(7))
            removeAllRows(twPtr);
    }
}

void TSetupTaskDialog::slotShowContextMenu2(const QPoint &pos)
{
    QPoint globalPos;
    QAction* selectedItem;
    QTableWidget* twPtr = static_cast<QTableWidget*>(ui->tabWidget->focusWidget());

    if (sender()->inherits("QAbstractScrollArea"))
        globalPos = (reinterpret_cast<QAbstractScrollArea*>(sender()))->viewport()->mapToGlobal(pos);
    else
        globalPos = (reinterpret_cast<QWidget*>(sender()))->mapToGlobal(pos);

    if ((selectedItem = menu3.exec(globalPos)))
        removeAllRows(twPtr);
}


void TSetupTaskDialog::setDirect(QTableWidget* twPtr,int dir)
{
    for (int i = 0; i < twPtr->rowCount(); i++)
    {
        if ((dir & DIR_X) == DIR_X)
            twPtr->item(i,2)->setCheckState(Qt::Checked);
        else
            twPtr->item(i,2)->setCheckState(Qt::Unchecked);
        if ((dir & DIR_Y) == DIR_Y)
            twPtr->item(i,3)->setCheckState(Qt::Checked);
        else
            twPtr->item(i,3)->setCheckState(Qt::Unchecked);
        if ((dir & DIR_Z) == DIR_Z)
            twPtr->item(i,4)->setCheckState(Qt::Checked);
        else
            twPtr->item(i,4)->setCheckState(Qt::Unchecked);
    }
}

void TSetupTaskDialog::removeAllRows(QTableWidget* twPtr)
{
    int i = 0;

    while (twPtr->rowCount())
        twPtr->removeRow(i);
}

void TSetupTaskDialog::setEnabledBtn(QToolButton* btn, QTableWidget* tw)
{
    btn->setEnabled(tw->selectionModel()->hasSelection());
}

// Открытие/закрытие доступа к полям ввода в зависимости от метода рассчета и размерности задачи
void TSetupTaskDialog::enabledParams(void)
{
    ui->gbCalculationMethod->setEnabled(ui->rbStatic->isChecked());

    // Параметры динамического расчета
    ui->gbTime->setEnabled(ui->rbDynamic->isChecked());
    ui->gbInitialConditions->setEnabled(ui->rbDynamic->isChecked());
    ui->gbDensity->setEnabled(ui->rbDynamic->isChecked());
    ui->gbDamping->setEnabled(ui->rbDynamic->isChecked());
    ui->gbTimeName->setVisible(ui->rbDynamic->isChecked());
    ui->textThetaWilson->setEnabled(ui->rbDynamic->isChecked());

    // Задание режима доступа к полям ввода в зависимости от размерности задачи
    ui->textCoordX->setEnabled(true);
    ui->textCoordY->setVisible(femObject->getMesh().getFreedom() > 1);
    ui->textCoordZ->setVisible(femObject->getMesh().getFreedom() > 2);
    ui->label_Y->setVisible(femObject->getMesh().getFreedom() > 1);
    ui->label_Z->setVisible(femObject->getMesh().getFreedom() > 2);

    ui->label_V->setVisible(femObject->getMesh().getFreedom() > 1);
    ui->label_Vt->setVisible(femObject->getMesh().getFreedom() > 1);
    ui->label_Vtt->setVisible(femObject->getMesh().getFreedom() > 1);
    ui->label_W->setVisible(femObject->getMesh().getFreedom() > 2);
    ui->label_Wt->setVisible(femObject->getMesh().getFreedom() > 2);
    ui->label_Wtt->setVisible(femObject->getMesh().getFreedom() > 2);

    ui->textV->setVisible(femObject->getMesh().getFreedom() > 1);
    ui->textVt->setVisible(femObject->getMesh().getFreedom() > 1);
    ui->textVtt->setVisible(femObject->getMesh().getFreedom() > 1);
    ui->textW->setVisible(femObject->getMesh().getFreedom() > 2);
    ui->textWt->setVisible(femObject->getMesh().getFreedom() > 2);
    ui->textWtt->setVisible(femObject->getMesh().getFreedom() > 2);

    // Толщина элемента недоступна для 3D-задачи
    ui->gbThickness->setEnabled((femObject->getMesh().isPlate() || femObject->getMesh().isShell() || femObject->getMesh().is1D() || femObject->getMesh().is2D()) ? true : false);


    ui->gbPoissonRatio->setEnabled(femObject->getMesh().getFreedom() > 1);

    // Задание доступности упруго-ластических параметров
    ui->textLoadStep->setEnabled(ui->rbMES->isChecked() || ui->rbMVS->isChecked());
    ui->gbStressStrainCurve->setEnabled(ui->rbMES->isChecked() || ui->rbMVS->isChecked());

    // Задание доступности функций
    enabledFuncNames();
}

// Формирование списка граничных условий
void TSetupTaskDialog::setBoundaryConditionValue(void)
{
    QTableWidgetItem* newItem;
    QStringList Titles;
    int count = 0,
        direct;


    removeAllRows(ui->twBC);

    Titles << tr("Expression") << tr("Predicate");
    if (femObject->getMesh().isPlate())
    {
        Titles << ui->twFuncName->item(2, 1)->text();   // W
        Titles << ui->twFuncName->item(3, 1)->text();   // Tx
        Titles << ui->twFuncName->item(4, 1)->text();   // Ty
    }
    else
    {
        Titles << ui->twFuncName->item(0, 1)->text();   // U
        Titles << ui->twFuncName->item(1, 1)->text();   // V
        Titles << ui->twFuncName->item(2, 1)->text();   // W
    }
    ui->twBC->setHorizontalHeaderLabels(Titles);

    ui->twBC->setColumnWidth(0, 200);
    ui->twBC->setColumnWidth(1, 200);
    ui->twBC->setColumnWidth(2, 24);
    ui->twBC->setColumnWidth(3, 24);
    ui->twBC->setColumnWidth(4, 24);


    ui->twBC->setColumnHidden(3, false);
    ui->twBC->setColumnHidden(4, false);

    if (femObject->getMesh().getFreedom() < 2)
        ui->twBC->setColumnHidden(3, true);
    if (femObject->getMesh().getFreedom() < 3)
        ui->twBC->setColumnHidden(4, true);

    for (auto it = femObject->getParams().plist.begin(); it != femObject->getParams().plist.end(); it++)
        if (it->getType() == BOUNDARY_CONDITION_PARAMETER)
        {
            direct = it->getDirect();
            ui->twBC->insertRow(count);
            newItem = new QTableWidgetItem(QString("%1").arg(it->getExpression().c_str()));
            ui->twBC->setItem(count, 0, newItem);
            newItem = new QTableWidgetItem(QString("%1").arg(it->getPredicate().c_str()));
            ui->twBC->setItem(count, 1, newItem);
            setDirect(ui->twBC, count, direct);
            count++;
        }
}

// Извлечение параметров упругости
void TSetupTaskDialog::getElasticParam(void)
{
    getTableValue(YOUNG_MODULUS_PARAMETER, ui->twYoungModulus, false);
    getTableValue(POISSON_RATIO_PARAMETER, ui->twPoissonsRatio, false);
}

// Извлечение параметров температурного расширения
void TSetupTaskDialog::getThermalExpansionParam(void)
{
    getTableValue(ALPHA_PARAMETER, ui->twThermalExpansion, false);
    getTableValue(TEMPERATURE_PARAMETER, ui->twTemperature, false);
}

// Извлечение плотности
void TSetupTaskDialog::getDensity(void)
{
    getTableValue(DENSITY_PARAMETER, ui->twDensity, false);
}

// Извлечение параметра демпфирования
void TSetupTaskDialog::getDamping(void)
{
    getTableValue(DAMPING_PARAMETER, ui->twDamping, false);
}

// Извлечение параметра точности расчета
void TSetupTaskDialog::getEps(void)
{
    femObject->getParams().eps = ui->textEps->text().toDouble();
}

// Извлечение параметра толщины элемента
void TSetupTaskDialog::getThickness(void)
{
    getTableValue(THICKNESS_PARAMETER, ui->twThickness, false);
}

// Извлечение параметров вывода
void TSetupTaskDialog::getWidth(void)
{
    femObject->getParams().width = ui->textWidth->text().toInt();
}

void TSetupTaskDialog::getPrecission(void)
{
    femObject->getParams().precision = ui->textPrecision->text().toInt();
}

// Извлечение теты-Вильсона
void TSetupTaskDialog::getTheta(void)
{
    femObject->getParams().theta = ui->textThetaWilson->text().toDouble();
}

// Извлечение времени
void TSetupTaskDialog::getTime(void)
{
    femObject->getParams().t0 = ui->textT0->text().toDouble();
    femObject->getParams().t1 = ui->textT1->text().toDouble();
    femObject->getParams().th = ui->textTH->text().toDouble();
}

// Извлечение начальных условий
void TSetupTaskDialog::getInitialParam(void)
{
    femObject->getParams().plist.addInitialCondition(ui->textU->text().toStdString(), FUN_U);     // U(t=0) = 0
    femObject->getParams().plist.addInitialCondition(ui->textUt->text().toStdString(), FUN_UT);   // Ut(t=0) = 0
    femObject->getParams().plist.addInitialCondition(ui->textUtt->text().toStdString(), FUN_UTT); // Utt(t=0) = 0

    femObject->getParams().plist.addInitialCondition(ui->textV->text().toStdString(), FUN_V);     // V(t=0) = 0
    femObject->getParams().plist.addInitialCondition(ui->textVt->text().toStdString(), FUN_VT);   // Vt(t=0) = 0
    femObject->getParams().plist.addInitialCondition(ui->textVtt->text().toStdString(), FUN_VTT); // Vtt(t=0) = 0

    femObject->getParams().plist.addInitialCondition(ui->textW->text().toStdString(), FUN_W);     // W(t=0) = 0
    femObject->getParams().plist.addInitialCondition(ui->textWt->text().toStdString(), FUN_WT);   // Wt(t=0) = 0
    femObject->getParams().plist.addInitialCondition(ui->textWtt->text().toStdString(), FUN_WTT); // Wtt(t=0) = 0
}

void TSetupTaskDialog::getVolumeLoad(void)
{
    getTableValue(VOLUME_LOAD_PARAMETER, ui->twVV);
}

void TSetupTaskDialog::getSurfaceLoad(void)
{
    getTableValue(SURFACE_LOAD_PARAMETER, ui->twSV);
}

void TSetupTaskDialog::getConcentratedLoad(void)
{
    getTableValue(CONCENTRATED_LOAD_PARAMETER, ui->twCV);
}

void TSetupTaskDialog::getPressureLoad(void)
{
    getTableValue(PRESSURE_LOAD_PARAMETER, ui->twPV, false);
}

// Извлечение граничных условий
void TSetupTaskDialog::getBoundaryConditionValue(void)
{
    getTableValue(BOUNDARY_CONDITION_PARAMETER, ui->twBC);
}

void TSetupTaskDialog::getTableValue(ParameterType type, QTableWidget* tw, bool isDirect)
{
    int direct = 0;

    for (int i = 0; i < tw->rowCount(); i++)
    {
        if (isDirect)
            direct = getDirect(tw, i);
        femObject->getParams().plist.addParameter(type, tw->item(i, 0)->text().toStdString(), tw->item(i, 1)->text().toStdString(), direct);
    }
}

void TSetupTaskDialog::setTableValue(ParameterType type, QTableWidget* tw, bool isDirect)
{
    int count = 0;
    QString ssc;
    QTableWidgetItem* newItem;

    removeAllRows(tw);
    for (auto it = femObject->getParams().plist.begin(); it != femObject->getParams().plist.end(); it++)
        if (it->getType() == type)
        {
            tw->insertRow(count);
            if (type == STRESS_STRAIN_CURVE_PARAMETER)
            {
                ssc = "{";
                for (unsigned i = 0; i < it->getStressStrainCurve().size1(); i++)
                {
                    ssc += "{" + QString("%1").arg(it->getStressStrainCurve(i, 0)) + "," + QString("%1").arg(it->getStressStrainCurve(i, 1)) + "}";
                    if (i < it->getStressStrainCurve().size1() - 1)
                        ssc += ",";
                }
                ssc += "}";
                newItem = new QTableWidgetItem(QString("%1").arg(ssc.toStdString().c_str()));
            }
            else
                newItem = new QTableWidgetItem(QString("%1").arg(it->getExpression().c_str()));
            tw->setItem(count, 0, newItem);
            newItem = new QTableWidgetItem(QString("%1").arg(it->getPredicate().c_str()));
            tw->setItem(count, 1, newItem);
            if (isDirect)
                setDirect(tw, count, it->getDirect());
            count++;
        }
}

bool TSetupTaskDialog::isStatic(void)
{
    return ui->rbStatic->isChecked();
}

bool TSetupTaskDialog::isDynamic(void)
{
    return ui->rbDynamic->isChecked();
}

void TSetupTaskDialog::clear(void)
{
    ui->textEps->setText("1.0e-6");
    ui->textWidth->setText("12");
    ui->textPrecision->setText("5");

    ui->textT0->clear();
    ui->textT1->clear();
    ui->textTH->clear();

    ui->textU->setText("0");
    ui->textV->setText("0");
    ui->textW->setText("0");
    ui->textUt->setText("0");
    ui->textVt->setText("0");
    ui->textWt->setText("0");
    ui->textUtt->setText("0");
    ui->textVtt->setText("0");
    ui->textWtt->setText("0");

    ui->textCoordX->setText("x");
    ui->textCoordY->setText("y");
    ui->textCoordZ->setText("z");

    ui->textTime->setText("t");

    removeAllRows(ui->twVV);
    removeAllRows(ui->twSV);
    removeAllRows(ui->twCV);
    removeAllRows(ui->twPV);
    removeAllRows(ui->twBC);
    removeAllRows(ui->twStressStrainCurve);
    removeAllRows(ui->twFuncName);
    removeAllRows(ui->twVariables);
    removeAllRows(ui->twYoungModulus);
    removeAllRows(ui->twPoissonsRatio);
    removeAllRows(ui->twThermalExpansion);
    removeAllRows(ui->twTemperature);
    removeAllRows(ui->twThickness);
    removeAllRows(ui->twDensity);
    removeAllRows(ui->twDamping);

    ui->textLoadStep->setText("0.01");
    ui->rbLinear->setChecked(true);
    ui->rbMVS->setChecked(false);
    ui->rbMES->setChecked(false);

    ui->rbStatic->setChecked(true);
    ui->rbDynamic->setChecked(false);

    enabledParams();
}


void TSetupTaskDialog::addRow(QTableWidget* ptw)
{
    QTableWidgetItem* newItem;
    int count = ptw->rowCount();

    ptw->insertRow(count);
    ptw->setItem(count, 0, new QTableWidgetItem(""));
    ptw->setItem(count, 1, new QTableWidgetItem(""));

    if (ptw == ui->twVV || ptw == ui->twSV || ptw == ui->twCV || ptw == ui->twBC)
    {
        newItem = new QTableWidgetItem();
        newItem->setCheckState(Qt::Unchecked);
        newItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        ptw->setItem(count, 2, newItem);
        newItem = new QTableWidgetItem();
        newItem->setCheckState(Qt::Unchecked);
        newItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        ptw->setItem(count, 3, newItem);
        newItem = new QTableWidgetItem();
        newItem->setCheckState(Qt::Unchecked);
        newItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        ptw->setItem(count, 4, newItem);
        isBoundaryConditionChanged = true;
    }
}

void TSetupTaskDialog::removeRow(QTableWidget* ptw)
{
    ptw->removeRow(ptw->currentRow());
//    setEnabledBtnLV();
    if (ptw == ui->twVV || ptw == ui->twSV || ptw == ui->twCV || ptw == ui->twPV || ptw == ui->twBC)
        isBoundaryConditionChanged = true;
}

void TSetupTaskDialog::setInitialParam(void)
{
    int direct;

    for (auto it = femObject->getParams().plist.begin(); it != femObject->getParams().plist.end(); it++)
    {
        if (it->getType() != INITIAL_CONDITION_PARAMETER)
            continue;

        direct = it->getDirect();
        if ((direct & FUN_U) == FUN_U)
            ui->textU->setText(it->getExpression().c_str());
        if ((direct & FUN_UT) == FUN_UT)
            ui->textUt->setText(it->getExpression().c_str());
        if ((direct & FUN_UTT) == FUN_UTT)
            ui->textUtt->setText(it->getExpression().c_str());
        if ((direct & FUN_V) == FUN_V)
            ui->textV->setText(it->getExpression().c_str());
        if ((direct & FUN_VT) == FUN_VT)
            ui->textVt->setText(it->getExpression().c_str());
        if ((direct & FUN_VTT) == FUN_VTT)
            ui->textVtt->setText(it->getExpression().c_str());
        if ((direct & FUN_W) == FUN_W)
            ui->textW->setText(it->getExpression().c_str());
        if ((direct & FUN_WT) == FUN_WT)
            ui->textWt->setText(it->getExpression().c_str());
        if ((direct & FUN_WTT) == FUN_WTT)
            ui->textWtt->setText(it->getExpression().c_str());
    }
}


// Начальная инициализация параметров расчета в диалоге
void TSetupTaskDialog::setup(void)
{
    clear();

    // Задаем "доступность" полей ввода
    ui->textCoordX->setEnabled(true);
//    ui->textCoordY->setEnabled(femObject->getMesh().getDimension() > 1);
//    ui->textCoordZ->setEnabled(femObject->getMesh().getDimension() > 2);
    //ui->textTime->setEnabled(ui->rbDynamic->isChecked());
    ui->textCoordY->setVisible(femObject->getMesh().getFreedom() > 1);
    ui->textCoordZ->setVisible(femObject->getMesh().getFreedom() > 2);
    ui->label_Y->setVisible(femObject->getMesh().getFreedom() > 1);
    ui->label_Z->setVisible(femObject->getMesh().getFreedom() > 2);
    ui->gbTimeName->setVisible(ui->rbDynamic->isChecked());


    // Инициализация упругих параметров
    setElasticParam();

    // Инициализация параметров теплового расширения
    setThermalExpansionParam();

    // Инициализация параметров вывода
    ui->textWidth->setText(QString("%1").arg(femObject->getParams().width));
    ui->textPrecision->setText(QString("%1").arg(femObject->getParams().precision));

    // Инициализация динамических параметров
    setDensity();
    setDamping();
    ui->textThetaWilson->setText(QString("%1").arg(femObject->getParams().theta));
    ui->textT0->setText(QString("%1").arg(femObject->getParams().t0));
    ui->textT1->setText(QString("%1").arg(femObject->getParams().t1));
    ui->textTH->setText(QString("%1").arg(femObject->getParams().th));
    setInitialParam();

    // Инициализация погрешности
    if (femObject->getParams().eps != 0.0)
        ui->textEps->setText(QString("%1").arg(femObject->getParams().eps));

    // Инициализация толщины элемента
    setThickness();

    // Инициализация метода расчета
    if (femObject->getParams().fType == DynamicProblem)
        ui->rbDynamic->setChecked(true);
    else
        ui->rbStatic->setChecked(true);

    // Инициализация названий функций и аргументов функций
    setFuncNames();
    // Инициализация вспомагательных переменных
    setVariables();


    // Инициализация нагрузок
    setVolumeLoad();
    setSurfaceLoad();
    setConcentratedLoad();
    setPressureLoad();

    // Инициализация краевых условий
    setBoundaryConditionValue();

    // Инициализация параметров пластичности
    setPlasticityParam();


    enabledParams();
    isBoundaryConditionChanged = false;
}

// Инициализация объемных нагрузок
void TSetupTaskDialog::setPlasticityParam(void)
{
    QStringList Titles;

    ui->rbLinear->setChecked((femObject->getParams().pMethod == Linear) ? true : false);
    ui->rbMVS->setChecked((femObject->getParams().pMethod == MVS) ? true : false);
    ui->rbMES->setChecked((femObject->getParams().pMethod == MES) ? true : false);
    ui->textLoadStep->setText(QString("%1").arg(femObject->getParams().forceStep));

    Titles << tr("Stress-Strain curve") << tr("Predicate");
    ui->twStressStrainCurve->setHorizontalHeaderLabels(Titles);
    ui->twStressStrainCurve->setColumnWidth(0, 300);
    ui->twStressStrainCurve->setColumnWidth(1, 200);
    setTableValue(STRESS_STRAIN_CURVE_PARAMETER, ui->twStressStrainCurve, false);
}

// Инициализация упругих характеристик
void TSetupTaskDialog::setElasticParam(void)
{
    QStringList Titles;

    Titles << tr("Expression") << tr("Predicate");
    ui->twYoungModulus->setHorizontalHeaderLabels(Titles);
    ui->twYoungModulus->setColumnWidth(0, 100);
    ui->twYoungModulus->setColumnWidth(1, 200);
    setTableValue(YOUNG_MODULUS_PARAMETER, ui->twYoungModulus, false);

    ui->twPoissonsRatio->setHorizontalHeaderLabels(Titles);
    ui->twPoissonsRatio->setColumnWidth(0, 100);
    ui->twPoissonsRatio->setColumnWidth(1, 200);
    setTableValue(POISSON_RATIO_PARAMETER, ui->twPoissonsRatio, false);
}

void TSetupTaskDialog::setDensity(void)
{
    QStringList Titles;

    Titles << tr("Expression") << tr("Predicate");
    ui->twDensity->setHorizontalHeaderLabels(Titles);
    ui->twDensity->setColumnWidth(0, 100);
    ui->twDensity->setColumnWidth(1, 200);
    setTableValue(DENSITY_PARAMETER, ui->twDensity, false);
}

void TSetupTaskDialog::setDamping(void)
{
    QStringList Titles;

    Titles << tr("Expression") << tr("Predicate");
    ui->twDamping->setHorizontalHeaderLabels(Titles);
    ui->twDamping->setColumnWidth(0, 100);
    ui->twDamping->setColumnWidth(1, 200);
    setTableValue(DAMPING_PARAMETER, ui->twDamping, false);
}

void TSetupTaskDialog::setThermalExpansionParam(void)
{
    QStringList Titles;

    Titles << tr("Expression") << tr("Predicate");
    ui->twThermalExpansion->setHorizontalHeaderLabels(Titles);
    ui->twThermalExpansion->setColumnWidth(0, 100);
    ui->twThermalExpansion->setColumnWidth(1, 200);
    setTableValue(ALPHA_PARAMETER, ui->twThermalExpansion, false);

    ui->twTemperature->setHorizontalHeaderLabels(Titles);
    ui->twTemperature->setColumnWidth(0, 100);
    ui->twTemperature->setColumnWidth(1, 200);
    setTableValue(TEMPERATURE_PARAMETER, ui->twTemperature, false);
}

void TSetupTaskDialog::setThickness(void)
{
    QStringList Titles;

    Titles << tr("Expression");
    ui->twThickness->setHorizontalHeaderLabels(Titles);
    ui->twThickness->setColumnWidth(0, 100);
    ui->twThickness->setColumnWidth(1, 200);
    setTableValue(THICKNESS_PARAMETER, ui->twThickness, false);
}


// Инициализация объемных нагрузок
void TSetupTaskDialog::setVolumeLoad(void)
{
    QStringList Titles;

    Titles << tr("Expression");
    Titles << tr("Predicate");
    Titles << ui->textCoordX->text(); // x
    Titles << ui->textCoordY->text(); // y
    Titles << ui->textCoordZ->text(); // z
    ui->twVV->setHorizontalHeaderLabels(Titles);
    ui->twVV->setColumnWidth(0, 200);
    ui->twVV->setColumnWidth(1, 200);
    ui->twVV->setColumnWidth(2, 24);
    ui->twVV->setColumnWidth(3, 24);
    ui->twVV->setColumnWidth(4, 24);

    ui->twVV->setColumnHidden(3, false);
    ui->twVV->setColumnHidden(4, false);

    if (femObject->getMesh().isPlate())
    {
        ui->twVV->setColumnHidden(2, true);
        ui->twVV->setColumnHidden(3, true);
    }
    else
    {
        if (femObject->getMesh().getFreedom() < 2)
            ui->twVV->setColumnHidden(3, true);
        if (femObject->getMesh().getFreedom() < 3)
            ui->twVV->setColumnHidden(4, true);
    }
    setTableValue(VOLUME_LOAD_PARAMETER, ui->twVV);
}

// Инициализация нагрузки-давления
void TSetupTaskDialog::setPressureLoad(void)
{
    QStringList Titles;

    Titles << tr("Expression");
    Titles << tr("Predicate");
    ui->twPV->setHorizontalHeaderLabels(Titles);
    ui->twPV->setColumnWidth(0, 200);
    ui->twPV->setColumnWidth(1, 200);

    setTableValue(PRESSURE_LOAD_PARAMETER, ui->twPV, false);
}

// Инициализация поверхностных нагрузок
void TSetupTaskDialog::setSurfaceLoad(void)
{
    QStringList Titles;

    Titles << tr("Expression");
    Titles << tr("Predicate");
    Titles << ui->textCoordX->text(); // x
    Titles << ui->textCoordY->text(); // y
    Titles << ui->textCoordZ->text(); // z
    ui->twSV->setHorizontalHeaderLabels(Titles);

    ui->twSV->setColumnWidth(0, 200);
    ui->twSV->setColumnWidth(1, 200);
    ui->twSV->setColumnWidth(2, 24);
    ui->twSV->setColumnWidth(3, 24);
    ui->twSV->setColumnWidth(4, 24);


    ui->twSV->setColumnHidden(2, false);
    ui->twSV->setColumnHidden(3, false);

    if (femObject->getMesh().isPlate())
    {
        ui->twSV->setColumnHidden(2, true);
        ui->twSV->setColumnHidden(3, true);
    }
    else
    {
        if (femObject->getMesh().getFreedom() < 2)
            ui->twSV->setColumnHidden(3, true);
        if (femObject->getMesh().getFreedom() < 3)
            ui->twSV->setColumnHidden(4, true);
    }

    setTableValue(SURFACE_LOAD_PARAMETER, ui->twSV);
}


// Инициализация сосредоточенных нагрузок
void TSetupTaskDialog::setConcentratedLoad(void)
{
    QStringList Titles;

    Titles << tr("Expression");
    Titles << tr("Predicate");
    Titles << ui->textCoordX->text(); // x
    Titles << ui->textCoordY->text(); // y
    Titles << ui->textCoordZ->text(); // z
    ui->twCV->setHorizontalHeaderLabels(Titles);

    ui->twCV->setColumnWidth(0, 200);
    ui->twCV->setColumnWidth(1, 200);
    ui->twCV->setColumnWidth(2, 24);
    ui->twCV->setColumnWidth(3, 24);
    ui->twCV->setColumnWidth(4, 24);


    ui->twCV->setColumnHidden(3, false);
    ui->twCV->setColumnHidden(4, false);

    if (femObject->getMesh().isPlate())
    {
        ui->twCV->setColumnHidden(2, true);
        ui->twCV->setColumnHidden(3, true);
    }
    else
    {
        if (femObject->getMesh().getFreedom() < 2)
            ui->twCV->setColumnHidden(3, true);
        if (femObject->getMesh().getFreedom() < 3)
            ui->twCV->setColumnHidden(4, true);
    }
    setTableValue(CONCENTRATED_LOAD_PARAMETER, ui->twCV);
}

// Проверка параметров расчета в диалоге при нажатии ОК
bool TSetupTaskDialog::check(void)
{
    double val;
    bool ret;
    unsigned w;

    if (!checkYoungModulus())
        return false;
    if (!checkPoissonRatio())
        return false;
    if (!checkThermalExpansion())
        return false;
    if (!checkTemperature())
        return false;
    if (!checkThickness())
        return false;

    if (ui->textEps->text().length())
    {
        ret = checkExpression(ui->textEps->text(),val);
        if (!ret || val <= 0)
        {
            ui->tabWidget->setCurrentIndex(0);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified calculation errors!"));
            return false;
        }
    }
    else
    {
        ui->tabWidget->setCurrentIndex(0);
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified calculation errors!"));
        return false;
    }

    if (ui->textWidth->text().length())
    {
        ret = checkExpression(ui->textWidth->text(),val);
        if (!ret || val <= 0)
        {
            ui->tabWidget->setCurrentIndex(0);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set output parameters!"));
            return false;
        }
        w = unsigned(val);
    }
    else
    {
        ui->tabWidget->setCurrentIndex(0);
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly set output parameters!"));
        return false;
    }

    if (ui->textPrecision->text().length())
    {
        ret = checkExpression(ui->textPrecision->text(),val);
        if (!ret || val <= 0 || unsigned(val) >= w)
        {
            ui->tabWidget->setCurrentIndex(0);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set output parameters!"));
            return false;
        }
    }
    else
    {
        ui->tabWidget->setCurrentIndex(0);
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly set output parameters!"));
        return false;
    }



    if (ui->rbDynamic->isChecked())
        if (!checkDynamicParams())
            return false;


    if (!checkVolumeLoad())
        return false;
    if (!checkSurfaceLoad())
        return false;
    if (!checkConcentratedLoad())
        return false;
    if (!checkPressureLoad())
        return false;
    if (!checkLimitValue())
        return false;
    if (!checkFuncNames())
        return false;
    if (!checkPlasticity())
        return false;
    if (!checkVariables())
        return false;

    return true;
}

// Проверка значений в таблице
bool TSetupTaskDialog::checkTable(QTableWidget* tw, int tab_no1, int tab_no2)
{
    for (int i = 0; i < tw->rowCount(); i++)
    {
        if (!tw->item(i, 0)->text().length() || !checkExpression(tw->item(i, 0)->text()))
        {
            ui->tabWidget->setCurrentIndex(tab_no1);
            if (tab_no2 != -1)
                ui->tabWidgetLoads->setCurrentIndex(tab_no2);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified expression in a row: %1!").arg(i + 1));
            return false;
        }
        if (tw->item(i, 1)->text().length() && !checkPredicate(tw->item(i, 1)->text()))
        {
            ui->tabWidget->setCurrentIndex(tab_no1);
            if (tab_no2 != -1)
                ui->tabWidgetLoads->setCurrentIndex(tab_no2);
            QMessageBox::critical(this, tr("Error"), tr("Predicate is not set correctly in the string: %1!").arg(i + 1));
            return false;
        }
    }
    return true;
}

// Проверка упругих параметров
bool TSetupTaskDialog::checkYoungModulus(void)
{
    return checkTable(ui->twYoungModulus, 1);
}

bool TSetupTaskDialog::checkPoissonRatio(void)
{
    return checkTable(ui->twPoissonsRatio, 1);
}

// Проверка параметров теплового расширения
bool TSetupTaskDialog::checkThermalExpansion(void)
{
    return checkTable(ui->twThermalExpansion, 2);
}

bool TSetupTaskDialog::checkTemperature(void)
{
    return checkTable(ui->twTemperature, 2);
}

// Проверка толщины КЭ
bool TSetupTaskDialog::checkThickness(void)
{
    return checkTable(ui->twThickness, 3);
}

// Проверка объемной нагрузки
bool TSetupTaskDialog::checkVolumeLoad(void)
{
    return checkTable(ui->twVV, 5, 0);
}

// Проверка поверхностной нагрузки
bool TSetupTaskDialog::checkSurfaceLoad(void)
{
    return checkTable(ui->twSV, 5, 1);
}

// Проверка сосредоточенной нагрузки
bool TSetupTaskDialog::checkConcentratedLoad(void)
{
    return checkTable(ui->twCV, 5, 2);
}

// Проверка нагрузки давлением
bool TSetupTaskDialog::checkPressureLoad(void)
{
    return checkTable(ui->twPV, 5, 3);
}

// Проверка граничных условий
bool TSetupTaskDialog::checkLimitValue(void)
{
    return checkTable(ui->twBC, 6);
}

/*
void SetupTaskDialog::closeEvent(QCloseEvent* event)
 {
    if (!reload())
        event->ignore();
    else
        QDialog::closeEvent(event);
 }
*/

void TSetupTaskDialog::accept(void)
{
    if (check())
    {
        reload();
        QDialog::accept();
    }
}

int TSetupTaskDialog::checkVertex(double x, double y, double z)
{
    int ret = -1;
    double eps = 1.0E-3;

    for (unsigned i = 0; i < femObject->getMesh().getNumVertex(); i++)
    {
        if (fabs(x - femObject->getMesh().getX(i, 0)) > eps) continue;
        if (femObject->getMesh().getFreedom() > 1)
            if (fabs(y - femObject->getMesh().getX(i,1)) > eps) continue;
        if (femObject->getMesh().getFreedom() > 2)
            if (fabs(z - femObject->getMesh().getX(i,2)) > eps) continue;
        ret = int(i);
        break;
    }
    return ret;
}

// Проверка правильности ввода выражения
bool TSetupTaskDialog::checkExpression(QString e)
{
    TParser parser;

    getVariables();
    parser.set_variables(femObject->getParams().variables);
    parser.set_variable(ui->textCoordX->text().toStdString());
    if (femObject->getMesh().getDimension() > 1)
        parser.set_variable(ui->textCoordY->text().toStdString());
    if (femObject->getMesh().getDimension() > 2)
        parser.set_variable(ui->textCoordZ->text().toStdString());
    if (ui->rbDynamic->isChecked())
        parser.set_variable(ui->textTime->text().toStdString());

    parser.set_expression(e.toStdString());
    return (parser.get_error() == NO_ERR) ? true : false;
}

bool TSetupTaskDialog::checkExpression(QString e, double& val)
{
    TParser parser;

    getVariables();
    parser.set_variables(femObject->getParams().variables);
    parser.set_variable(ui->textCoordX->text().toStdString());
    if (femObject->getMesh().getDimension() > 1)
        parser.set_variable(ui->textCoordY->text().toStdString());
    if (femObject->getMesh().getDimension() > 2)
        parser.set_variable(ui->textCoordZ->text().toStdString());
    if (ui->rbDynamic->isChecked())
        parser.set_variable(ui->textTime->text().toStdString());

    try
    {
        parser.set_expression(e.toStdString());
    }
    catch (ErrorCode& err)
    {
        cerr << endl << sayError(err) << endl;
        return false;
    }
    val = parser.run();
    return true;
}

bool TSetupTaskDialog::checkPredicate(QString e)
{
    TParser parser;

    getVariables();
    parser.set_variables(femObject->getParams().variables);
    parser.set_variable(ui->textCoordX->text().toStdString());
    if (femObject->getMesh().getDimension() > 1)
        parser.set_variable(ui->textCoordY->text().toStdString());
    if (femObject->getMesh().getDimension() > 2)
        parser.set_variable(ui->textCoordZ->text().toStdString());

    try
    {
        parser.set_expression(e.toStdString());
    }
    catch (ErrorCode& err)
    {
        cerr << endl << sayError(err) << endl;
        return false;
    }
    return true;
}


// Проверка корректности ввода параметров расчета в динамике
bool TSetupTaskDialog::checkDynamicParams(void)
{
    bool ret;
    double theta,
           t0,
           t1,
           th;

    if (!checkTable(ui->twDensity, 4))
        return false;
    if (!checkTable(ui->twDamping, 4))
        return false;

    if (ui->textThetaWilson->text().length())
    {
        ret = checkExpression(ui->textThetaWilson->text(),theta);
        if (!ret || theta <= 0)
        {
            ui->tabWidget->setCurrentIndex(2);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the Wilson-theta!"));
            return false;
        }
    }
    else
    {
        ui->tabWidget->setCurrentIndex(2);
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the Wilson-theta!"));
        return false;
    }
    if (ui->textT0->text().length())
    {
        ret = checkExpression(ui->textT0->text(),t0);
        if (!ret || t0 < 0)
        {
            ui->tabWidget->setCurrentIndex(2);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the start time!"));
            return false;
        }
    }
    else
    {
        ui->tabWidget->setCurrentIndex(2);
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the start time!"));
        return false;
    }
    if (ui->textT1->text().length())
    {
        ret = checkExpression(ui->textT1->text(),t1);
        if (!ret || t1 <= t0)
        {
            ui->tabWidget->setCurrentIndex(2);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the stop time!"));
            return false;
        }
    }
    else
    {
        ui->tabWidget->setCurrentIndex(2);
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the stop time!"));
        return false;
    }
    if (ui->textTH->text().length())
    {
        ret = checkExpression(ui->textTH->text(),th);
        if (!ret || th > t1 - t0)
        {
            ui->tabWidget->setCurrentIndex(2);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the time step!"));
            return false;
        }
    }
    else
    {
        ui->tabWidget->setCurrentIndex(2);
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the time step!"));
        return false;
    }

    if (ui->textU->text().length())
    {
        ret = checkExpression(ui->textU->text());
        if (!ret)
        {
            ui->tabWidget->setCurrentIndex(2);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textV->text().length())
    {
        ret = checkExpression(ui->textV->text());
        if (!ret)
        {
            ui->tabWidget->setCurrentIndex(2);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textW->text().length())
    {
        ret = checkExpression(ui->textW->text());
        if (!ret)
        {
            ui->tabWidget->setCurrentIndex(2);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textUt->text().length())
    {
        ret = checkExpression(ui->textUt->text());
        if (!ret)
        {
            ui->tabWidget->setCurrentIndex(2);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textVt->text().length())
    {
        ret = checkExpression(ui->textVt->text());
        if (!ret)
        {
            ui->tabWidget->setCurrentIndex(2);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textWt->text().length())
    {
        ret = checkExpression(ui->textWt->text());
        if (!ret)
        {
            ui->tabWidget->setCurrentIndex(2);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textUtt->text().length())
    {
        ret = checkExpression(ui->textUtt->text());
        if (!ret)
        {
            ui->tabWidget->setCurrentIndex(2);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textVtt->text().length())
    {
        ret = checkExpression(ui->textVtt->text());
        if (!ret)
        {
            ui->tabWidget->setCurrentIndex(2);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textWtt->text().length())
    {
        ret = checkExpression(ui->textWtt->text());
        if (!ret)
        {
            ui->tabWidget->setCurrentIndex(2);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    return true;
}

bool TSetupTaskDialog::checkFuncNames(void)
{
    QString msg[] = {
                     tr("volumetric force X"),
                     tr("volumetric force Y"),
                     tr("volumetric force Z"),
                     tr("coordinate x"),
                     tr("coordinate y"),
                     tr("coordinate z")
                  };
    QLineEdit* edit[] = { ui->textCoordX, ui->textCoordY, ui->textCoordZ, ui->textTime };

    for (int i = 0; i < 24; i++)
    {
        if (!ui->twFuncName->item(i, 1)->text().length())
        {
            ui->tabWidget->setCurrentIndex(4);
            QMessageBox::critical(this, tr("Error"), tr("Incorrect function name %1!").arg(ui->twFuncName->item(i, 0)->text()));
            return false;
        }
    }
    for (unsigned i = 24; i < femObject->getParams().names.size(); i++)
    {
        if (!edit[i - 24]->text().length())
        {
            ui->tabWidget->setCurrentIndex(4);
            QMessageBox::critical(this, tr("Error"), tr("Incorrect function name %1!").arg(msg[i - 21]));
            return false;
        }
    }
    return true;
}

void TSetupTaskDialog::getFunNames(void)
{
    QTextDocument txt;

    femObject->getParams().names.clear();
    femObject->getParams().names.push_back(ui->textCoordX->text().toStdString());
    femObject->getParams().names.push_back(ui->textCoordY->text().toStdString());
    femObject->getParams().names.push_back(ui->textCoordZ->text().toStdString());
    femObject->getParams().names.push_back(ui->textTime->text().toStdString());
    for (int i = 0; i < ui->twFuncName->rowCount(); i++)
    {
        txt.setHtml(ui->twFuncName->item(i,1)->text());
        femObject->getParams().names.push_back(txt.toPlainText().toStdString());
    }
}

void TSetupTaskDialog::getVariables(void)
{
    for (int i = 0; i < ui->twVariables->rowCount(); i++)
        femObject->getParams().variables[ui->twVariables->item(i, 0)->text().toStdString()] = double(ui->twVariables->item(i, 1)->text().toFloat());
}

void TSetupTaskDialog::setFuncNames(void)
{
    ui->textCoordX->setText(QString(femObject->getParams().names[0].c_str()));
    ui->textCoordY->setText(QString(femObject->getParams().names[1].c_str()));
    ui->textCoordZ->setText(QString(femObject->getParams().names[2].c_str()));
    ui->textTime->setText(QString(femObject->getParams().names[3].c_str()));
    for (int i = 0; i < int(femObject->getParams().names.size()) - 4; i++)
    {
        ui->twFuncName->insertRow(i);
        ui->twFuncName->setItem(i, 0, new QTableWidgetItem(QString("%1").arg(femObject->getParams().stdNames()[unsigned(4 + i)].c_str())));
        ui->twFuncName->setItem(i, 1, new QTableWidgetItem(QString("%1").arg(femObject->getParams().names[unsigned(4 + i)].c_str())));
        ui->twFuncName->item(i,0)->setFlags(Qt::ItemFlags(~Qt::ItemIsEditable));
        if (i == 6)
        {
//            ui->twFuncName->item(i,1)->setBackgroundColor(QColor(125, 125, 125));
            Qt::ItemFlags eFlags = ui->twFuncName->item(i,1)->flags();

            eFlags &= ~Qt::ItemIsEditable;
            ui->twFuncName->item(i,1)->setFlags(eFlags);
        }
    }
}

void TSetupTaskDialog::setVariables(void)
{
    int i = 0;

    for (auto it = femObject->getParams().variables.begin(); it != femObject->getParams().variables.end(); ++it)
    {
        ui->twVariables->insertRow(i);
        ui->twVariables->setItem(i, 0, new QTableWidgetItem(QString("%1").arg(it->first.c_str())));
        ui->twVariables->setItem(i++, 1, new QTableWidgetItem(QString("%1").arg(it->second)));
    }
}

// Определение номера комбинации выбранных функций в списке граничных условий
int TSetupTaskDialog::getDirect(QTableWidget* p, int i)
{
    int direct = 0;

    if (p->item(i,2)->checkState() == Qt::Checked)
        direct |= DIR_X;
    if (p->item(i,3)->checkState() == Qt::Checked)
        direct |= DIR_Y;
    if (p->item(i,4)->checkState() == Qt::Checked)
        direct |= DIR_Z;
    return direct;
}

// Задание номера комбинации выбранных функций в списке граничных условий
void TSetupTaskDialog::setDirect(QTableWidget* p, int i, int direct)
{
    p->setItem(i, 2, new QTableWidgetItem(0));
    p->setItem(i, 3, new QTableWidgetItem(0));
    p->setItem(i, 4, new QTableWidgetItem(0));

    if (direct == -1)
    {
        p->item(i, 2)->setCheckState(Qt::Unchecked);
        p->item(i, 3)->setCheckState(Qt::Unchecked);
        p->item(i, 4)->setCheckState(Qt::Unchecked);
        return;
    }

    if ((direct & DIR_X) == DIR_X)
        p->item(i, 2)->setCheckState(Qt::Checked);
    else
        p->item(i, 2)->setCheckState(Qt::Unchecked);
    if ((direct & DIR_Y) == DIR_Y)
        p->item(i, 3)->setCheckState(Qt::Checked);
    else
        p->item(i, 3)->setCheckState(Qt::Unchecked);
    if ((direct & DIR_Z) == DIR_Z)
        p->item(i, 4)->setCheckState(Qt::Checked);
    else
        p->item(i, 4)->setCheckState(Qt::Unchecked);

}


bool TSetupTaskDialog::checkPlasticity(void)
{
    double val;
    bool  isOk;
    matrix<double> ssc;

    if (ui->rbLinear->isChecked())
        return true;
    val = ui->textLoadStep->text().toDouble(&isOk);
    if (!isOk || val <=0)
    {
        ui->tabWidget->setCurrentIndex(3);
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly set force step!"));
        return false;
    }
    for (int i = 0; i < ui->twStressStrainCurve->rowCount(); i++)
    {
        if (!decodeStressStarinCurve(ui->twStressStrainCurve->item(i, 0)->text().toStdString(), ssc))
        {
            ui->tabWidget->setCurrentIndex(7);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set stress-strain curve in a row: %1!").arg(i + 1));
            return false;
        }
        if (ui->twStressStrainCurve->item(i, 1)->text().length() && !checkExpression(ui->twStressStrainCurve->item(i, 1)->text()))
        {
            ui->tabWidget->setCurrentIndex(7);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set predicate in a row: %1!").arg(i + 1));
            return false;
        }
    }
    return true;
}

bool TSetupTaskDialog::checkVariables(void)
{
    bool  isOk;

    for (int i = 0; i < ui->twVariables->rowCount(); i++)
    {
        ui->twVariables->item(i,1)->text().toDouble(&isOk);
        if (!isOk)
        {
            ui->tabWidget->setCurrentIndex(5);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set variable value in a row: %1!").arg(i + 1));
            return false;
        }
    }
    return true;
}


void TSetupTaskDialog::getPlasticityParam(void)
{
    matrix<double> ssc(unsigned(ui->twStressStrainCurve->rowCount()), 2);

    femObject->getParams().pMethod = (ui->rbLinear->isChecked()) ? Linear : (ui->rbMVS->isChecked()) ? MVS : MES;
    femObject->getParams().forceStep = ui->textLoadStep->text().toDouble();

    for (unsigned i = 0; i < unsigned(ui->twStressStrainCurve->rowCount()); i++)
    {
        decodeStressStarinCurve(ui->twStressStrainCurve->item(int(i), 0)->text().toStdString(), ssc);
        femObject->getParams().plist.addStressStrainCurve(ssc, ui->twStressStrainCurve->item(int(i), 1)->text().toStdString());
    }
}

// Загрузка параметров расчета в Object
void TSetupTaskDialog::reload(void)
{
    femObject->getParams().clear();
    femObject->setTaskParam((ui->rbDynamic->isChecked()) ? DynamicProblem : StaticProblem);
    femObject->getParams().tMethod = Wilson;

    getElasticParam();
    getEps();
    getThickness();
    getWidth();
    getPrecission();
    getThermalExpansionParam();
    getDensity();
    getDamping();
    getTheta();
    getTime();
    getInitialParam();

    getFunNames();
    getVariables();
    getVolumeLoad();
    getSurfaceLoad();
    getConcentratedLoad();
    getPressureLoad();
    getBoundaryConditionValue();
    getPlasticityParam();
}

void TSetupTaskDialog::changeLanguage(void)
{
    ui->retranslateUi(this);
}


QWidget* THTMLDelegate::createEditor(QWidget *parent,const QStyleOptionViewItem &/* option */,const QModelIndex &/* index */) const
{
    QTextEdit *editor = new QTextEdit(parent);

    return editor;
}

void THTMLDelegate::setEditorData(QWidget *editor,const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QTextEdit* ed = static_cast<QTextEdit*>(editor);

    ed->setText(value);
}

void THTMLDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const
{
    QTextEdit* ed = static_cast<QTextEdit*>(editor);
    QString value;

    value = ed->toHtml();
    model->setData(index, value, Qt::EditRole);
}

void THTMLDelegate::updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void THTMLDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
     QTextDocument document;
     QVariant value = index.data(Qt::DisplayRole);
     QString text = value.toString(),
             val[] = {
                        "<font size=5>u</font>",
                        "<font size=5>v</font>",
                        "<font size=5>w</font>",
                        "<font size=5>&theta;<sub>x</sub></font>",
                        "<font size=5>&theta;<sub>y</sub></font>",
                        "<font size=5>&theta;<sub>z</sub></font>",
                        "<font size=5>&epsilon;<sub>xx</sub></font>",
                        "<font size=5>&epsilon;<sub>yy</sub></font>",
                        "<font size=5>&epsilon;<sub>zz</sub></font>",
                        "<font size=5>&epsilon;<sub>xy</sub></font>",
                        "<font size=5>&epsilon;<sub>xz</sub></font>",
                        "<font size=5>&epsilon;<sub>yz</sub></font>",
                        "<font size=5>&sigma;<sub>xx</sub></font>",
                        "<font size=5>&sigma;<sub>yy</sub></font>",
                        "<font size=5>&sigma;<sub>zz</sub></font>",
                        "<font size=5>&sigma;<sub>xy</sub></font>",
                        "<font size=5>&sigma;<sub>xz</sub></font>",
                        "<font size=5>&sigma;<sub>yz</sub></font>",
                        "<font size=5>u<sub>t</sub></font>",
                        "<font size=5>v<sub>t</sub></font>",
                        "<font size=5>w<sub>t</sub></font>",
                        "<font size=5>u<sub>tt</sub></font>",
                        "<font size=5>v<sub>tt</sub></font>",
                        "<font size=5>w<sub>tt</sub></font>"
                     };

     if (index.column() == 1 && index.flags() != Qt::ItemIsEditable)
     {
         painter->fillRect(option.rect, QColor(213,213,213));
     }
     if (value.isValid() && !value.isNull())
     {
         if (index.column() == 0)
             text = val[index.row()];
         document.setHtml(text);
         painter->translate(option.rect.topLeft());
         document.drawContents(painter);
         painter->translate(-option.rect.topLeft());

     }
}

void TSetupTaskDialog::enabledFuncNames(void)
{
    bool isStatic = ui->rbStatic->isChecked();
    vector<int> index;
    FEType type = femObject->getMesh().getTypeFE();

    if (type == FE1D2)
    {
        if (isStatic)
            index = { 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        else
            index = { 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0 };
    }
    else if (type == FE2D3 || type == FE2D4 || type == FE2D6)
    {
        if (isStatic)
            index = { 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
        else
            index = { 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0 };
    }
    else if (type == FE2D3P || type == FE2D4P || type == FE2D6P)
    {
        if (isStatic)
            index = { 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 };
        else
            index = { 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1 };
    }
    else if (type == FE3D4 || type == FE3D8 || type == FE3D10)
    {
        if (isStatic)
            index = { 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 };
        else
            index = { 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    }
    else if (type == FE3D3S || type == FE3D4S || type == FE3D6S)
    {
        if (isStatic)
            index = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 };
        else
            index = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    }
    for (int i = 0; i < ui->twFuncName->rowCount(); i++)
    {
        ui->twFuncName->setRowHidden(i, false);
        ui->twFuncName->item(i, 0)->setFlags(Qt::ItemFlags(~Qt::ItemIsEditable));
        ui->twFuncName->item(i, 1)->setText(femObject->getParams().names[unsigned(4 + i)].c_str());
        if (index[unsigned(i)])
            ui->twFuncName->item(i, 1)->setFlags(ui->twFuncName->item(i,1)->flags() | Qt::ItemIsEditable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        else
            ui->twFuncName->setRowHidden(i,true);
    }
}

bool TSetupTaskDialog::decodeStressStarinCurve(string str, matrix<double>& ssc)
{
    char ch[4];
    double val[2];
    stringstream ss(str);
    vector<double> v;

    ss >> ch[0];
    if (ch[0] != '{')
        return false;
    while (ss.good())
    {
        ss >> ch[0] >> val[0] >> ch[1] >> val[1] >> ch[2] >> ch[3];
        if (ss.bad())
            return false;
        if (ch[0] != '{' || ch[1] != ',' || ch[2] != '}')
            return false;
        if (ch[3] != ',' && ch[3] != '}')
            return false;
        v.push_back(val[0]);
        v.push_back(val[1]);
    }
    if (ss.bad() || ch[3] != '}')
        return false;
    ssc.resize(unsigned(v.size() / 2), 2);
    for (unsigned i = 0; i < v.size(); i += 2)
    {
        ssc[i / 2][0] = v[i];
        ssc[i / 2][1] = v[i + 1];
    }
    return true;
}
