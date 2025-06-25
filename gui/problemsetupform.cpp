#include <QPushButton>
#include <QMessageBox>
#include <QTableWidget>
#include <QTextEdit>
#include <QPainter>
#include <QTextDocument>
#include <QDialogButtonBox>
#include <QTableWidget>
#include "problemsetupform.h"
#include "ui_problemsetupform.h"
#include "object/object.h"
#include "parser/parser.h"

TProblemSetupForm::TProblemSetupForm(TFEMObject * fo, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TProblemSetupForm)
{
    ui->setupUi(this);

    femObject = fo;
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

    connect(ui->twCV, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu1(QPoint)));
    connect(ui->twVV, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu1(QPoint)));
    connect(ui->twSV, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu1(QPoint)));
    connect(ui->twBC, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu1(QPoint)));
    connect(ui->twPV, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twStressStrainCurve, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twVariables, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twYoungModulus, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twPoissonsRatio, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twThermalExpansion, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twTemperature, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twThickness, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twDensity, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu2(QPoint)));
    connect(ui->twDamping, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(slotShowContextMenu2(QPoint)));

    connect(ui->twBC->selectionModel(), &QItemSelectionModel::selectionChanged, this, ([=](void) { setEnabledBtn(ui->tbRemoveBC, ui->twBC); }));
    connect(ui->twCV->selectionModel(), &QItemSelectionModel::selectionChanged, this, ([=](void) { setEnabledBtn(ui->tbRemoveLoad, ui->twCV); }));
    connect(ui->twSV->selectionModel(), &QItemSelectionModel::selectionChanged, this, ([=](void) { setEnabledBtn(ui->tbRemoveLoad, ui->twSV); }));
    connect(ui->twVV->selectionModel(), &QItemSelectionModel::selectionChanged, this, ([=](void) { setEnabledBtn(ui->tbRemoveLoad, ui->twVV); }));
    connect(ui->twPV->selectionModel(), &QItemSelectionModel::selectionChanged, this, ([=](void) { setEnabledBtn(ui->tbRemoveLoad, ui->twPV); }));
    connect(ui->twStressStrainCurve->selectionModel(), &QItemSelectionModel::selectionChanged, this, ([=](void) { setEnabledBtn(ui->tbRemoveProp, ui->twStressStrainCurve); }));
    connect(ui->twVariables->selectionModel(), &QItemSelectionModel::selectionChanged, this, ([=](void) { setEnabledBtn(ui->tbRemoveVariables, ui->twVariables); }));
    connect(ui->twYoungModulus->selectionModel(), &QItemSelectionModel::selectionChanged, this, ([=](void) { setEnabledBtn(ui->tbRemoveYoungModulus, ui->twYoungModulus); }));
    connect(ui->twPoissonsRatio->selectionModel(), &QItemSelectionModel::selectionChanged, this, ([=](void) { setEnabledBtn(ui->tbRemovePoissonsRatio, ui->twPoissonsRatio); }));
    connect(ui->twThermalExpansion->selectionModel(), &QItemSelectionModel::selectionChanged, this, ([=](void) { setEnabledBtn(ui->tbRemoveThermalExpansion, ui->twThermalExpansion); }));
    connect(ui->twTemperature->selectionModel(), &QItemSelectionModel::selectionChanged, this, ([=](void) { setEnabledBtn(ui->tbRemoveTemperature, ui->twTemperature); }));
    connect(ui->twThickness->selectionModel(), &QItemSelectionModel::selectionChanged, this, ([=](void) { setEnabledBtn(ui->tbRemoveThickness, ui->twThickness); }));
    connect(ui->twDensity->selectionModel(), &QItemSelectionModel::selectionChanged, this, ([=](void) { setEnabledBtn(ui->tbRemoveDensity, ui->twDensity); }));
    connect(ui->twDamping->selectionModel(), &QItemSelectionModel::selectionChanged, this, ([=](void) { setEnabledBtn(ui->tbRemoveDamping, ui->twDamping); }));

    connect(ui->twYoungModulus->model(), &QAbstractItemModel::rowsInserted, this, ([=](void) { ui->tbShowYoungModulus->setEnabled(true); }));
    connect(ui->twYoungModulus->model(), &QAbstractItemModel::rowsRemoved, this, ([=](void) { ui->tbShowYoungModulus->setEnabled(bool(ui->twYoungModulus->rowCount())); }));
    connect(ui->twPoissonsRatio->model(), &QAbstractItemModel::rowsInserted, this, ([=](void) { ui->tbShowPoissonRatio->setEnabled(true); }));
    connect(ui->twPoissonsRatio->model(), &QAbstractItemModel::rowsRemoved, this, ([=](void) { ui->tbShowPoissonRatio->setEnabled(bool(ui->twPoissonsRatio->rowCount())); }));
    connect(ui->twThickness->model(), &QAbstractItemModel::rowsInserted, this, ([=](void) { ui->tbShowThickness->setEnabled(true); }));
    connect(ui->twThickness->model(), &QAbstractItemModel::rowsRemoved, this, ([=](void) { ui->tbShowThickness->setEnabled(bool(ui->twThickness->rowCount())); }));
    connect(ui->twThermalExpansion->model(), &QAbstractItemModel::rowsInserted, this, ([=](void) { ui->tbShowAlpha->setEnabled(true); }));
    connect(ui->twThermalExpansion->model(), &QAbstractItemModel::rowsRemoved, this, ([=](void) { ui->tbShowAlpha->setEnabled(bool(ui->twThermalExpansion->rowCount())); }));
    connect(ui->twTemperature->model(), &QAbstractItemModel::rowsInserted, this, ([=](void) { ui->tbShowTemperature->setEnabled(true); }));
    connect(ui->twTemperature->model(), &QAbstractItemModel::rowsRemoved, this, ([=](void) { ui->tbShowTemperature->setEnabled(bool(ui->twTemperature->rowCount())); }));
    connect(ui->twBC->model(), &QAbstractItemModel::rowsInserted, this, ([=](void) { ui->tbShowBoundaryConditions->setEnabled(true); }));
    connect(ui->twBC->model(), &QAbstractItemModel::rowsRemoved, this, ([=](void) { ui->tbShowBoundaryConditions->setEnabled(bool(ui->twBC->rowCount())); }));
    connect(ui->twDensity->model(), &QAbstractItemModel::rowsInserted, this, ([=](void) { ui->tbShowDensity->setEnabled(true); }));
    connect(ui->twDensity->model(), &QAbstractItemModel::rowsRemoved, this, ([=](void) { ui->tbShowDensity->setEnabled(bool(ui->twDensity->rowCount())); }));
    connect(ui->twDamping->model(), &QAbstractItemModel::rowsInserted, this, ([=](void) { ui->tbShowDamping->setEnabled(true); }));
    connect(ui->twDamping->model(), &QAbstractItemModel::rowsRemoved, this, ([=](void) { ui->tbShowDamping->setEnabled(bool(ui->twDamping->rowCount())); }));

    connect(ui->twVV->model(), &QAbstractItemModel::rowsInserted, this, ([=](void) { ui->tbShowLoads->setEnabled(true); }));
    connect(ui->twVV->model(), &QAbstractItemModel::rowsRemoved, this, ([=](void) { ui->tbShowLoads->setEnabled(bool(ui->twVV->rowCount())); }));
    connect(ui->twSV->model(), &QAbstractItemModel::rowsInserted, this, ([=](void) { ui->tbShowLoads->setEnabled(true); }));
    connect(ui->twSV->model(), &QAbstractItemModel::rowsRemoved, this, ([=](void) { ui->tbShowLoads->setEnabled(bool(ui->twSV->rowCount())); }));
    connect(ui->twPV->model(), &QAbstractItemModel::rowsInserted, this, ([=](void) { ui->tbShowLoads->setEnabled(true); }));
    connect(ui->twPV->model(), &QAbstractItemModel::rowsRemoved, this, ([=](void) { ui->tbShowLoads->setEnabled(bool(ui->twPV->rowCount())); }));
    connect(ui->twCV->model(), &QAbstractItemModel::rowsInserted, this, ([=](void) { ui->tbShowLoads->setEnabled(true); }));
    connect(ui->twCV->model(), &QAbstractItemModel::rowsRemoved, this, ([=](void) { ui->tbShowLoads->setEnabled(bool(ui->twCV->rowCount())); }));

    connect(ui->twBC->model(), &QAbstractItemModel::rowsInserted, this, ([=](void) { ui->tbShowBoundaryConditions->setEnabled(true); }));
    connect(ui->twBC->model(), &QAbstractItemModel::rowsRemoved, this, ([=](void) { ui->tbShowBoundaryConditions->setEnabled(bool(ui->twBC->rowCount())); }));


    connect(ui->rbStatic, &QRadioButton::clicked, this, &TProblemSetupForm::enabledParams);
    connect(ui->rbDynamic, &QRadioButton::clicked, this, &TProblemSetupForm::enabledParams);
    connect(ui->rbLinear, &QRadioButton::clicked, this, &TProblemSetupForm::enabledParams);
    connect(ui->rbMVS, &QRadioButton::clicked, this, &TProblemSetupForm::enabledParams);
    connect(ui->rbMES, &QRadioButton::clicked, this, &TProblemSetupForm::enabledParams);

    connect(ui->tbAddLoad, &QToolButton::clicked, this, ([=](void) { addRow(getLoadTab()); }));
    connect(ui->tbAddBC, &QToolButton::clicked, this, ([=](void) { addRow(ui->twBC); }));
    connect(ui->tbAddProp, &QToolButton::clicked, this, ([=](void) { addRow(ui->twStressStrainCurve); }));
    connect(ui->tbAddVariable, &QToolButton::clicked, this, ([=](void) { addRow(ui->twVariables); }));
    connect(ui->tbAddYoungModulus, &QToolButton::clicked, this, ([=](void) { addRow(ui->twYoungModulus); }));
    connect(ui->tbAddPoissonsRatio, &QToolButton::clicked, this, ([=](void) { addRow(ui->twPoissonsRatio); }));
    connect(ui->tbAddThermalExpansion, &QToolButton::clicked, this, ([=](void) { addRow(ui->twThermalExpansion); }));
    connect(ui->tbAddTemperature, &QToolButton::clicked, this, ([=](void) { addRow(ui->twTemperature); }));
    connect(ui->tbAddThickness, &QToolButton::clicked, this, ([=](void) { addRow(ui->twThickness); }));
    connect(ui->tbAddDensity, &QToolButton::clicked, this, ([=](void) { addRow(ui->twDensity); }));
    connect(ui->tbAddDamping, &QToolButton::clicked, this, ([=](void) { addRow(ui->twDamping); }));

    connect(ui->tbRemoveLoad, &QToolButton::clicked, this, ([=](void) { removeRow(getLoadTab()); setEnabledBtn(ui->tbRemoveLoad, getLoadTab()); }));
    connect(ui->tbRemoveBC, &QToolButton::clicked, this, ([=](void) { removeRow(ui->twBC); setEnabledBtn(ui->tbRemoveBC, ui->twBC); }));
    connect(ui->tbRemoveProp, &QToolButton::clicked, this, ([=](void) { removeRow(ui->twStressStrainCurve); setEnabledBtn(ui->tbRemoveProp, ui->twStressStrainCurve); }));
    connect(ui->tbRemoveVariables, &QToolButton::clicked, this, ([=](void) { removeRow(ui->twVariables); setEnabledBtn(ui->tbRemoveVariables, ui->twVariables); }));
    connect(ui->tbRemoveYoungModulus, &QToolButton::clicked, this, ([=](void) { removeRow(ui->twYoungModulus); setEnabledBtn(ui->tbRemoveYoungModulus, ui->twYoungModulus); }));
    connect(ui->tbRemovePoissonsRatio, &QToolButton::clicked, this, ([=](void) { removeRow(ui->twPoissonsRatio); setEnabledBtn(ui->tbRemovePoissonsRatio, ui->twPoissonsRatio); }));
    connect(ui->tbRemoveThermalExpansion, &QToolButton::clicked, this, ([=](void) { removeRow(ui->twThermalExpansion); setEnabledBtn(ui->tbRemoveThermalExpansion, ui->twThermalExpansion); }));
    connect(ui->tbRemoveTemperature, &QToolButton::clicked, this, ([=](void) { removeRow(ui->twTemperature); setEnabledBtn(ui->tbRemoveTemperature, ui->twTemperature); }));
    connect(ui->tbRemoveThickness, &QToolButton::clicked, this, ([=](void) { removeRow(ui->twThickness); setEnabledBtn(ui->tbRemoveThickness, ui->twThickness); }));
    connect(ui->tbRemoveDensity, &QToolButton::clicked, this, ([=](void) { removeRow(ui->twDensity); setEnabledBtn(ui->tbRemoveDensity, ui->twDensity); }));
    connect(ui->tbRemoveDamping, &QToolButton::clicked, this, ([=](void) { removeRow(ui->twDamping); setEnabledBtn(ui->tbRemoveDamping, ui->twDamping); }));

    connect(ui->tbShowYoungModulus, &QToolButton::clicked, this, ([=](void) { if (getParams()) emit clicked(static_cast<int>(ParamType::YoungModulus)); }));
    connect(ui->tbShowPoissonRatio, &QToolButton::clicked, this, ([=](void) { if (getParams()) emit clicked(static_cast<int>(ParamType::PoissonRatio)); }));
    connect(ui->tbShowThickness, &QToolButton::clicked, this, ([=](void) { if (getParams()) emit clicked(static_cast<int>(ParamType::Thickness)); }));
    connect(ui->tbShowBoundaryConditions, &QToolButton::clicked, this, ([=](void) { if (getParams()) emit clicked(static_cast<int>(ParamType::BoundaryCondition)); }));
    connect(ui->tbShowAlpha, &QToolButton::clicked, this, ([=](void) { if (getParams()) emit clicked(static_cast<int>(ParamType::Alpha)); }));
    connect(ui->tbShowTemperature, &QToolButton::clicked, this, ([=](void) { if (getParams()) emit clicked(static_cast<int>(ParamType::Temperature)); }));
    connect(ui->tbShowDensity, &QToolButton::clicked, this, ([=](void) { if (getParams()) emit clicked(static_cast<int>(ParamType::Density)); }));
    connect(ui->tbShowDamping, &QToolButton::clicked, this, ([=](void) { if (getParams()) emit clicked(static_cast<int>(ParamType::Damping)); }));

    connect(ui->tbShowLoads, &QToolButton::clicked, this, ([=](void)
    {
        ParamType c_param[4] = { ParamType::VolumeLoad, ParamType::SurfaceLoad, ParamType::ConcentratedLoad, ParamType::PressureLoad };

        if (getParams()) emit clicked(static_cast<int>(c_param[ui->tabWidgetLoads->currentIndex()]));
    }));
    connect(ui->tabWidgetLoads, &QTabWidget::currentChanged, this, ([=](void) { setEnabledBtn(ui->tbRemoveLoad, ui->tbShowLoads, getLoadTab()); }));

    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, ([=](void) { slotCancelButton(); }));
    connect(ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, ([=](void) { slotApplyButton(); }));

    createMenu();

    ui->twFuncName->setItemDelegate(new TDelegateHTML(this));
}

TProblemSetupForm::~TProblemSetupForm()
{
    delete ui;
}

void TProblemSetupForm::slotApplyButton(void)
{
    getParams();
}


void TProblemSetupForm::createMenu(void)
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

void TProblemSetupForm::setEnabledBtn(QToolButton* btn, QTableWidget* tw)
{
    btn->setEnabled(tw->selectionModel()->hasSelection());
}

void TProblemSetupForm::setEnabledBtn(QToolButton* btn1, QToolButton* btn2, QTableWidget* tw)
{
    btn1->setEnabled(tw->selectionModel()->hasSelection());
    btn2->setEnabled(tw->rowCount());
}

// Открытие/закрытие доступа к полям ввода в зависимости от метода рассчета и размерности задачи
void TProblemSetupForm::enabledParams(void)
{
    ui->tbShowLoads->setEnabled(getLoadTab()->rowCount());

    ui->gbCalculationMethod->setEnabled(ui->rbStatic->isChecked());

    // Параметры динамического расчета
    ui->gbTime->setEnabled(ui->rbDynamic->isChecked());
    ui->gbInitialConditions->setEnabled(ui->rbDynamic->isChecked());
    ui->gbDensity->setEnabled(ui->rbDynamic->isChecked());
    ui->gbDamping->setEnabled(ui->rbDynamic->isChecked());
    ui->textTime->setEnabled(ui->rbDynamic->isChecked());
    ui->textThetaWilson->setEnabled(ui->rbDynamic->isChecked());
    ui->twStressStrainCurve->setEnabled(not ui->rbDynamic->isChecked());
    ui->textLoadStep->setEnabled(not ui->rbDynamic->isChecked());

    // Задание режима доступа к полям ввода в зависимости от размерности задачи
    ui->textCoordX->setEnabled(true);
    ui->textCoordY->setVisible(femObject->getMesh().getFreedom() > 1);
    ui->textCoordZ->setVisible(femObject->getMesh().getFreedom() > 2);
    ui->textCoordY->setEnabled(femObject->getMesh().getFreedom() > 1);
    ui->textCoordZ->setEnabled(femObject->getMesh().getFreedom() > 2);

    ui->textV->setEnabled(femObject->getMesh().getFreedom() > 1);
    ui->textVt->setEnabled(femObject->getMesh().getFreedom() > 1);
    ui->textVtt->setEnabled(femObject->getMesh().getFreedom() > 1);
    ui->textW->setEnabled(femObject->getMesh().getFreedom() > 2);
    ui->textWt->setEnabled(femObject->getMesh().getFreedom() > 2);
    ui->textWtt->setEnabled(femObject->getMesh().getFreedom() > 2);

    ui->textV->setVisible(femObject->getMesh().getFreedom() > 1);
    ui->textVt->setVisible(femObject->getMesh().getFreedom() > 1);
    ui->textVtt->setVisible(femObject->getMesh().getFreedom() > 1);
    ui->textW->setVisible(femObject->getMesh().getFreedom() > 2);
    ui->textWt->setVisible(femObject->getMesh().getFreedom() > 2);
    ui->textWtt->setVisible(femObject->getMesh().getFreedom() > 2);

    // Толщина элемента недоступна для 3D-задачи
    ui->gbThickness->setEnabled((femObject->getMesh().isPlate() or femObject->getMesh().isShell() or femObject->getMesh().is1D() or femObject->getMesh().is2D()) ? true : false);


    ui->gbPoissonRatio->setEnabled(femObject->getMesh().getFreedom() > 1);

    // Задание доступности упруго-ластических параметров
    ui->textLoadStep->setEnabled(ui->rbStatic->isChecked() and (ui->rbMES->isChecked() or ui->rbMVS->isChecked()));
    ui->gbStressStrainCurve->setEnabled(ui->rbStatic->isChecked() and (ui->rbMES->isChecked() or ui->rbMVS->isChecked()));

    // Задание доступности функций
    enabledFuncNames();
}

void TProblemSetupForm::addRow(QTableWidget* ptw)
{
    QTableWidgetItem* newItem;
    int count = ptw->rowCount();

    ptw->insertRow(count);
    ptw->setItem(count, 0, new QTableWidgetItem(""));
    ptw->setItem(count, 1, new QTableWidgetItem(""));

    if (ptw == ui->twVV or ptw == ui->twSV or ptw == ui->twCV or ptw == ui->twBC)
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
    }
}
QTableWidget* TProblemSetupForm::getLoadTab(void)
{
    QTableWidget* ret = nullptr;

    switch (ui->tabWidgetLoads->currentIndex())
    {
        case 0:
            ret = ui->twVV;
            break;
        case 1:
            ret = ui->twSV;
            break;
        case 2:
            ret = ui->twCV;
            break;
        case 3:
            ret = ui->twPV;
    }
    return ret;
}

void TProblemSetupForm::removeRow(QTableWidget* ptw)
{
    ptw->removeRow(ptw->currentRow());
}

void TProblemSetupForm::enabledFuncNames(void)
{
    bool isStatic = ui->rbStatic->isChecked();
    vector<int> index;
    FEType type = femObject->getMesh().getTypeFE();

    if (type == FEType::fe1d2)
    {
        if (isStatic)
            index = { 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
        else
            index = { 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0 };
    }
    else if (type == FEType::fe2d3 or type == FEType::fe2d4 or type == FEType::fe2d6)
    {
        if (isStatic)
            index = { 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 };
        else
            index = { 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0 };
    }
    else if (type == FEType::fe2d3p or type == FEType::fe2d4p or type == FEType::fe2d6p)
    {
        if (isStatic)
            index = { 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 };
        else
            index = { 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1 };
    }
    else if (type == FEType::fe3d4 or type == FEType::fe3d8 or type == FEType::fe3d10)
    {
        if (isStatic)
            index = { 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 };
        else
            index = { 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
    }
    else if (type == FEType::fe3d3s or type == FEType::fe3d4s or type == FEType::fe3d6s)
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

QWidget* TDelegateHTML::createEditor(QWidget *parent,const QStyleOptionViewItem &/* option */,const QModelIndex &/* index */) const
{
    QTextEdit *editor = new QTextEdit(parent);

    return editor;
}

void TDelegateHTML::setEditorData(QWidget *editor,const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();
    QTextEdit* ed = static_cast<QTextEdit*>(editor);

    ed->setText(value);
}

void TDelegateHTML::setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const
{
    QTextEdit* ed = static_cast<QTextEdit*>(editor);
    QString value;

    value = ed->toHtml();
    model->setData(index, value, Qt::EditRole);
}

void TDelegateHTML::updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void TDelegateHTML::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
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

     if (index.column() == 1 and index.flags() not_eq Qt::ItemIsEditable)
     {
         painter->fillRect(option.rect, QColor(213,213,213));
     }
     if (value.isValid() and not value.isNull())
     {
         if (index.column() == 0)
             text = val[index.row()];
         document.setHtml(text);
         painter->translate(option.rect.topLeft());
         document.drawContents(painter);
         painter->translate(-option.rect.topLeft());

     }
}

void TProblemSetupForm::slotShowContextMenu1(const QPoint &pos)
{
    QPoint globalPos;
    QAction* selectedItem;
    QTableWidget* twPtr = static_cast<QTableWidget*>(focusWidget());
    QMenu* menu = (twPtr == ui->twVV or twPtr == ui->twSV or twPtr == ui->twCV) ? &menu1 : &menu2;

    if (sender()->inherits("QAbstractScrollArea"))
        globalPos = (reinterpret_cast<QAbstractScrollArea*>(sender()))->viewport()->mapToGlobal(pos);
    else
        globalPos = (reinterpret_cast<QWidget*>(sender()))->mapToGlobal(pos);

    if ((selectedItem = menu->exec(globalPos)))
    {
        if (selectedItem == menu->actions().at(0))
            setDirect(twPtr, Direction::X | Direction::Y | Direction::Z);
        else if (selectedItem == menu->actions().at(1))
            setDirect(twPtr, Direction::Undefined);
        else if (selectedItem == menu->actions().at(3))
            setDirect(twPtr, Direction::X);
        else if (selectedItem == menu->actions().at(4))
            setDirect(twPtr, Direction::Y);
        else if (selectedItem == menu->actions().at(5))
            setDirect(twPtr, Direction::Z);
        else if (selectedItem == menu->actions().at(7))
            removeAllRows(twPtr);
    }
}

void TProblemSetupForm::slotShowContextMenu2(const QPoint &pos)
{
    QPoint globalPos;
    //QAction* selectedItem;
    QTableWidget* twPtr = static_cast<QTableWidget*>(focusWidget());

    if (sender()->inherits("QAbstractScrollArea"))
        globalPos = (reinterpret_cast<QAbstractScrollArea*>(sender()))->viewport()->mapToGlobal(pos);
    else
        globalPos = (reinterpret_cast<QWidget*>(sender()))->mapToGlobal(pos);

    if (menu3.exec(globalPos))
        removeAllRows(twPtr);
}


void TProblemSetupForm::setDirect(QTableWidget* twPtr, Direction dir)
{
    for (int i = 0; i < twPtr->rowCount(); i++)
    {
        if (contains(dir, Direction::X))
            twPtr->item(i, 2)->setCheckState(Qt::Checked);
        else
            twPtr->item(i, 2)->setCheckState(Qt::Unchecked);
        if (contains(dir, Direction::Y))
            twPtr->item(i, 3)->setCheckState(Qt::Checked);
        else
            twPtr->item(i, 3)->setCheckState(Qt::Unchecked);
        if (contains(dir, Direction::Z))
            twPtr->item(i, 4)->setCheckState(Qt::Checked);
        else
            twPtr->item(i, 4)->setCheckState(Qt::Unchecked);
    }
}

void TProblemSetupForm::removeAllRows(QTableWidget* twPtr)
{
    int i = 0;

    while (twPtr->rowCount())
        twPtr->removeRow(i);
}

void TProblemSetupForm::changeLanguage(void)
{
    ui->retranslateUi(this);
}

void TProblemSetupForm::clear(void)
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

    ui->textLoadStep->setText("1");
    ui->rbLinear->setChecked(true);
    ui->rbMVS->setChecked(false);
    ui->rbMES->setChecked(false);

    ui->rbStatic->setChecked(true);
    ui->rbDynamic->setChecked(false);

    //enabledParams();
}

// Начальная инициализация параметров расчета в диалоге
void TProblemSetupForm::setup(void)
{
    clear();

    // Задаем "доступность" полей ввода
    ui->textCoordX->setEnabled(true);
    ui->textCoordY->setEnabled(femObject->getMesh().getFreedom() > 1);
    ui->textCoordZ->setEnabled(femObject->getMesh().getFreedom() > 2);
    ui->textTime->setEnabled(ui->rbDynamic->isChecked());

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
    if (femObject->getParams().eps not_eq 0.0)
        ui->textEps->setText(QString("%1").arg(femObject->getParams().eps));

    // Инициализация толщины элемента
    setThickness();

    // Инициализация метода расчета
    if (femObject->getParams().fType == FEMType::DynamicProblem)
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
}

// Инициализация упругих характеристик
void TProblemSetupForm::setElasticParam(void)
{
    QStringList Titles;

    Titles << tr("Expression") << tr("Predicate");
    ui->twYoungModulus->setHorizontalHeaderLabels(Titles);
//    ui->twYoungModulus->setColumnWidth(0, 100);
//    ui->twYoungModulus->setColumnWidth(1, 200);
    setTableValue(ParamType::YoungModulus, ui->twYoungModulus, false);

    ui->twPoissonsRatio->setHorizontalHeaderLabels(Titles);
//    ui->twPoissonsRatio->setColumnWidth(0, 100);
//    ui->twPoissonsRatio->setColumnWidth(1, 200);
    setTableValue(ParamType::PoissonRatio, ui->twPoissonsRatio, false);
}

void TProblemSetupForm::setTableValue(ParamType type, QTableWidget* tw, bool isDirect)
{
    int count = 0;
    QString ssc;
    QTableWidgetItem* newItem;

    removeAllRows(tw);
    for (auto it = femObject->getParams().plist.begin(); it not_eq femObject->getParams().plist.end(); it++)
        if (it->getType() == type)
        {
            tw->insertRow(count);
            if (type == ParamType::StressStrainCurve)
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

// Задание номера комбинации выбранных функций в списке граничных условий
void TProblemSetupForm::setDirect(QTableWidget* p, int i, Direction direct)
{
    p->setItem(i, 2, new QTableWidgetItem(0));
    p->setItem(i, 3, new QTableWidgetItem(0));
    p->setItem(i, 4, new QTableWidgetItem(0));

    if (direct == Direction::Undefined)
    {
        p->item(i, 2)->setCheckState(Qt::Unchecked);
        p->item(i, 3)->setCheckState(Qt::Unchecked);
        p->item(i, 4)->setCheckState(Qt::Unchecked);
        return;
    }

    if (contains(direct, Direction::X))
        p->item(i, 2)->setCheckState(Qt::Checked);
    else
        p->item(i, 2)->setCheckState(Qt::Unchecked);
    if (contains(direct, Direction::Y))
        p->item(i, 3)->setCheckState(Qt::Checked);
    else
        p->item(i, 3)->setCheckState(Qt::Unchecked);
    if (contains(direct, Direction::Z))
        p->item(i, 4)->setCheckState(Qt::Checked);
    else
        p->item(i, 4)->setCheckState(Qt::Unchecked);

}

void TProblemSetupForm::setThermalExpansionParam(void)
{
    QStringList Titles;

    Titles << tr("Expression") << tr("Predicate");
    ui->twThermalExpansion->setHorizontalHeaderLabels(Titles);
    ui->twThermalExpansion->setColumnWidth(0, 100);
    ui->twThermalExpansion->setColumnWidth(1, 200);
    setTableValue(ParamType::Alpha, ui->twThermalExpansion, false);

    ui->twTemperature->setHorizontalHeaderLabels(Titles);
    ui->twTemperature->setColumnWidth(0, 100);
    ui->twTemperature->setColumnWidth(1, 200);
    setTableValue(ParamType::Temperature, ui->twTemperature, false);
}

void TProblemSetupForm::setDensity(void)
{
    QStringList Titles;

    Titles << tr("Expression") << tr("Predicate");
    ui->twDensity->setHorizontalHeaderLabels(Titles);
    ui->twDensity->setColumnWidth(0, 100);
    ui->twDensity->setColumnWidth(1, 200);
    setTableValue(ParamType::Density, ui->twDensity, false);
}

void TProblemSetupForm::setDamping(void)
{
    QStringList Titles;

    Titles << tr("Expression") << tr("Predicate");
    ui->twDamping->setHorizontalHeaderLabels(Titles);
    ui->twDamping->setColumnWidth(0, 100);
    ui->twDamping->setColumnWidth(1, 200);
    setTableValue(ParamType::Damping, ui->twDamping, false);
}

void TProblemSetupForm::setInitialParam(void)
{
    InitialCondition init;

    for (auto it = femObject->getParams().plist.begin(); it not_eq femObject->getParams().plist.end(); it++)
    {
        if (it->getType() not_eq ParamType::InitialCondition)
            continue;

        init = it->getInitialCondition();
        if (contains(init, InitialCondition::U))
            ui->textU->setText(it->getExpression().c_str());
        if (contains(init, InitialCondition::Ut))
            ui->textUt->setText(it->getExpression().c_str());
        if (contains(init, InitialCondition::Utt))
            ui->textUtt->setText(it->getExpression().c_str());
        if (contains(init, InitialCondition::V))
            ui->textV->setText(it->getExpression().c_str());
        if (contains(init, InitialCondition::Vt))
            ui->textVt->setText(it->getExpression().c_str());
        if (contains(init, InitialCondition::Vtt))
            ui->textVtt->setText(it->getExpression().c_str());
        if (contains(init, InitialCondition::W))
            ui->textW->setText(it->getExpression().c_str());
        if (contains(init, InitialCondition::Wt))
            ui->textWt->setText(it->getExpression().c_str());
        if (contains(init, InitialCondition::Wtt))
            ui->textWtt->setText(it->getExpression().c_str());
    }
}

void TProblemSetupForm::setThickness(void)
{
    QStringList Titles;

    Titles << tr("Expression");
    ui->twThickness->setHorizontalHeaderLabels(Titles);
    ui->twThickness->setColumnWidth(0, 100);
    ui->twThickness->setColumnWidth(1, 200);
    setTableValue(ParamType::Thickness, ui->twThickness, false);
}

void TProblemSetupForm::setFuncNames(void)
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

void TProblemSetupForm::setVariables(void)
{
    int i = 0;

    for (auto it = femObject->getParams().variables.begin(); it not_eq femObject->getParams().variables.end(); ++it)
    {
        ui->twVariables->insertRow(i);
        ui->twVariables->setItem(i, 0, new QTableWidgetItem(QString("%1").arg(it->first.c_str())));
        ui->twVariables->setItem(i++, 1, new QTableWidgetItem(QString("%1").arg(it->second)));
    }
}

// Инициализация объемных нагрузок
void TProblemSetupForm::setVolumeLoad(void)
{
    QStringList Titles;

    Titles << tr("Expression");
    Titles << tr("Predicate");
    Titles << ui->textCoordX->text(); // x
    Titles << ui->textCoordY->text(); // y
    Titles << ui->textCoordZ->text(); // z
    ui->twVV->setHorizontalHeaderLabels(Titles);
    ui->twVV->setColumnWidth(0, 300);
    ui->twVV->setColumnWidth(1, 300);
    ui->twVV->setColumnWidth(2, 24);
    ui->twVV->setColumnWidth(3, 24);
    ui->twVV->setColumnWidth(4, 24);

    ui->twVV->setColumnHidden(2, false);
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
    setTableValue(ParamType::VolumeLoad, ui->twVV);
}

// Инициализация нагрузки-давления
void TProblemSetupForm::setPressureLoad(void)
{
    QStringList Titles;

    Titles << tr("Expression");
    Titles << tr("Predicate");
    ui->twPV->setHorizontalHeaderLabels(Titles);
    ui->twPV->setColumnWidth(0, 340);
    ui->twPV->setColumnWidth(1, 340);

    setTableValue(ParamType::PressureLoad, ui->twPV, false);
}

// Инициализация поверхностных нагрузок
void TProblemSetupForm::setSurfaceLoad(void)
{
    QStringList Titles;

    Titles << tr("Expression");
    Titles << tr("Predicate");
    Titles << ui->textCoordX->text(); // x
    Titles << ui->textCoordY->text(); // y
    Titles << ui->textCoordZ->text(); // z
    ui->twSV->setHorizontalHeaderLabels(Titles);

    ui->twSV->setColumnWidth(0, 300);
    ui->twSV->setColumnWidth(1, 300);
    ui->twSV->setColumnWidth(2, 24);
    ui->twSV->setColumnWidth(3, 24);
    ui->twSV->setColumnWidth(4, 24);


    ui->twSV->setColumnHidden(2, false);
    ui->twSV->setColumnHidden(3, false);
    ui->twSV->setColumnHidden(4, false);

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

    setTableValue(ParamType::SurfaceLoad, ui->twSV);
}


// Инициализация сосредоточенных нагрузок
void TProblemSetupForm::setConcentratedLoad(void)
{
    QStringList Titles;

    Titles << tr("Expression");
    Titles << tr("Predicate");
    Titles << ui->textCoordX->text(); // x
    Titles << ui->textCoordY->text(); // y
    Titles << ui->textCoordZ->text(); // z
    ui->twCV->setHorizontalHeaderLabels(Titles);

    ui->twCV->setColumnWidth(0, 300);
    ui->twCV->setColumnWidth(1, 300);
    ui->twCV->setColumnWidth(2, 24);
    ui->twCV->setColumnWidth(3, 24);
    ui->twCV->setColumnWidth(4, 24);


    ui->twCV->setColumnHidden(2, false);
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
    setTableValue(ParamType::ConcentratedLoad, ui->twCV);
}

// Формирование списка граничных условий
void TProblemSetupForm::setBoundaryConditionValue(void)
{
    QTableWidgetItem* newItem;
    QStringList Titles;
    int count = 0;
    Direction direct;


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

    ui->twBC->setColumnWidth(0, 320);
    ui->twBC->setColumnWidth(1, 320);
    ui->twBC->setColumnWidth(2, 24);
    ui->twBC->setColumnWidth(3, 24);
    ui->twBC->setColumnWidth(4, 24);


    ui->twBC->setColumnHidden(3, false);
    ui->twBC->setColumnHidden(4, false);

    if (femObject->getMesh().getFreedom() < 2)
        ui->twBC->setColumnHidden(3, true);
    if (femObject->getMesh().getFreedom() < 3)
        ui->twBC->setColumnHidden(4, true);

    for (auto it = femObject->getParams().plist.begin(); it not_eq femObject->getParams().plist.end(); it++)
        if (it->getType() == ParamType::BoundaryCondition)
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

// Инициализация параметров пластичности
void TProblemSetupForm::setPlasticityParam(void)
{
    QStringList Titles;

    ui->rbLinear->setChecked((femObject->getParams().pMethod == PlasticityMethod::Linear) ? true : false);
    ui->rbMVS->setChecked((femObject->getParams().pMethod == PlasticityMethod::MVS) ? true : false);
    ui->rbMES->setChecked((femObject->getParams().pMethod == PlasticityMethod::MES) ? true : false);
    ui->textLoadStep->setText(QString("%1").arg(femObject->getParams().loadStep));

    Titles << tr("Stress-Strain curve") << tr("Predicate");
    ui->twStressStrainCurve->setHorizontalHeaderLabels(Titles);
    ui->twStressStrainCurve->setColumnWidth(0, 370);
    ui->twStressStrainCurve->setColumnWidth(1, 370);
    setTableValue(ParamType::StressStrainCurve, ui->twStressStrainCurve, false);
}

bool TProblemSetupForm::decodeStressStarinCurve(string str, matrix<double>& ssc)
{
    char ch[4];
    double val[2];
    stringstream ss(str);
    vector<double> v;

    str.erase(remove(str.begin(), str.end(), ' '), str.end());
    ss >> ch[0];
    if (ch[0] not_eq '{')
        return false;
    while (ss.good())
    {
        ss >> ch[0] >> val[0] >> ch[1] >> val[1] >> ch[2] >> ch[3];
        if (ss.bad())
            return false;
        if (ch[0] not_eq '{' or ch[1] not_eq ',' or ch[2] not_eq '}')
            return false;
        if (ch[3] not_eq ',' and ch[3] not_eq '}')
            return false;
        v.push_back(val[0]);
        v.push_back(val[1]);
        if (ch[3] == '}')
            break;
    }
    if (ss.bad())
        return false;
    ssc.resize(unsigned(v.size() / 2), 2);
    for (unsigned i = 0; i < v.size(); i += 2)
    {
        ssc[i / 2][0] = v[i];
        ssc[i / 2][1] = v[i + 1];
    }
    return true;
}

// Проверка параметров расчета в диалоге при нажатии ОК
bool TProblemSetupForm::check(void)
{
    double val;
    unsigned w;

    if (not checkYoungModulus())
        return false;
    if (not checkPoissonRatio())
        return false;
    if (not checkThermalExpansion())
        return false;
    if (not checkTemperature())
        return false;
    if (not checkThickness())
        return false;

    if (ui->textEps->text().length())
    {
        if (getExpression(ui->textEps->text(), val) not_eq ErrorCode::Undefined or val <= 0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified calculation errors!"));
            return false;
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified calculation errors!"));
        return false;
    }

    if (ui->textWidth->text().length())
    {
        if (getExpression(ui->textWidth->text(), val) not_eq ErrorCode::Undefined or val <= 0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set output parameters!"));
            return false;
        }
        w = unsigned(val);
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly set output parameters!"));
        return false;
    }

    if (ui->textPrecision->text().length())
    {
        if (getExpression(ui->textPrecision->text(), val) not_eq ErrorCode::Undefined or val <= 0 or unsigned(val) >= w)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set output parameters!"));
            return false;
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly set output parameters!"));
        return false;
    }

    if (ui->rbDynamic->isChecked())
        if (not checkDynamicParams())
            return false;

    if (not checkVolumeLoad())
        return false;
    if (not checkSurfaceLoad())
        return false;
    if (not checkConcentratedLoad())
        return false;
    if (not checkPressureLoad())
        return false;
    if (not checkLimitValue())
        return false;
    if (not checkFuncNames())
        return false;
    if (not checkPlasticity())
        return false;
    if (not checkVariables())
        return false;

    return true;
}

bool TProblemSetupForm::checkFuncNames(void)
{
    QString msg[] = {
                     tr("volumetric load X"),
                     tr("volumetric load Y"),
                     tr("volumetric load Z"),
                     tr("coordinate x"),
                     tr("coordinate y"),
                     tr("coordinate z")
                  };
    QLineEdit* edit[] = { ui->textCoordX, ui->textCoordY, ui->textCoordZ, ui->textTime };

    for (int i = 0; i < 24; i++)
    {
        if (not ui->twFuncName->item(i, 1)->text().length())
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrect function name %1!").arg(ui->twFuncName->item(i, 0)->text()));
            return false;
        }
    }
    for (unsigned i = 24; i < femObject->getParams().names.size(); i++)
    {
        if (not edit[i - 24]->text().length())
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrect function name %1!").arg(msg[i - 21]));
            return false;
        }
    }
    return true;
}

bool TProblemSetupForm::checkVariables(void)
{
    bool  isOk;

    for (int i = 0; i < ui->twVariables->rowCount(); i++)
    {
        ui->twVariables->item(i,1)->text().toDouble(&isOk);
        if (not isOk)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set variable value in a row: %1!").arg(i + 1));
            return false;
        }
    }
    return true;
}

bool TProblemSetupForm::checkPlasticity(void)
{
    double val;
    bool  isOk;
    matrix<double> ssc;

    if (ui->rbLinear->isChecked())
        return true;
    val = ui->textLoadStep->text().toDouble(&isOk);
    if (not isOk or val <=0)
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly set load step!"));
        return false;
    }
    for (int i = 0; i < ui->twStressStrainCurve->rowCount(); i++)
    {
        if (not decodeStressStarinCurve(ui->twStressStrainCurve->item(i, 0)->text().toStdString(), ssc))
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set stress-strain curve in a row: %1!").arg(i + 1));
            return false;
        }
        if (ui->twStressStrainCurve->item(i, 1)->text().length() and checkExpression(ui->twStressStrainCurve->item(i, 1)->text()) not_eq ErrorCode::Undefined)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set predicate in a row: %1!").arg(i + 1));
            return false;
        }
    }
    return true;
}


// Проверка граничных условий
bool TProblemSetupForm::checkLimitValue(void)
{
    return checkTable(ui->twBC);
}

// Проверка упругих параметров
bool TProblemSetupForm::checkYoungModulus(void)
{
    if (not ui->twYoungModulus->rowCount())
    {
        QMessageBox::critical(this, tr("Error"), tr("Young's modulus not defined!"));
        return false;
    }
    return checkTable(ui->twYoungModulus);
}

bool TProblemSetupForm::checkPoissonRatio(void)
{
    if (not ui->twPoissonsRatio->rowCount() and not femObject->getMesh().is1D())
    {
        QMessageBox::critical(this, tr("Error"), tr("Poisson's ratio not defined!"));
        return false;
    }
    return checkTable(ui->twPoissonsRatio);
}

// Проверка значений в таблице
bool TProblemSetupForm::checkTable(QTableWidget* tw, int tab_no)
{
    for (int i = 0; i < tw->rowCount(); i++)
    {
        if (not tw->item(i, 0)->text().length() or checkExpression(tw->item(i, 0)->text()) not_eq ErrorCode::Undefined)
        {
            if (tab_no not_eq -1)
                ui->tabWidgetLoads->setCurrentIndex(tab_no);
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified expression in a row: %1!").arg(i + 1));
            return false;
        }
        if (tw->item(i, 1)->text().length() and checkExpression(tw->item(i, 1)->text()) not_eq ErrorCode::Undefined)
        {
            if (tab_no not_eq -1)
                ui->tabWidgetLoads->setCurrentIndex(tab_no);
            QMessageBox::critical(this, tr("Error"), tr("Predicate is not set correctly in the string: %1!").arg(i + 1));
            return false;
        }
    }
    return true;
}

// Проверка правильности ввода выражения
ErrorCode TProblemSetupForm::checkExpression(QString e)
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
    }
    return parser.get_error();
}

ErrorCode TProblemSetupForm::getExpression(QString e, double& val, double x, double y, double z)
{
    TParser parser;

    getVariables();
    parser.set_variables(femObject->getParams().variables);
    parser.set_variable(ui->textCoordX->text().toStdString(), x);
    if (femObject->getMesh().getDimension() > 1)
        parser.set_variable(ui->textCoordY->text().toStdString(), y);
    if (femObject->getMesh().getDimension() > 2)
        parser.set_variable(ui->textCoordZ->text().toStdString(), z);
    if (ui->rbDynamic->isChecked())
        parser.set_variable(ui->textTime->text().toStdString());

    try
    {
        parser.set_expression(e.toStdString());
        val = parser.run();
    }
    catch (ErrorCode& err)
    {
        cerr << endl << sayError(err) << endl;
    }
    return parser.get_error();
}

void TProblemSetupForm::getVariables(void)
{
    for (int i = 0; i < ui->twVariables->rowCount(); i++)
        femObject->getParams().variables[ui->twVariables->item(i, 0)->text().toStdString()] = double(ui->twVariables->item(i, 1)->text().toFloat());
}

// Проверка параметров теплового расширения
bool TProblemSetupForm::checkThermalExpansion(void)
{
    return checkTable(ui->twThermalExpansion);
}

bool TProblemSetupForm::checkTemperature(void)
{
    return checkTable(ui->twTemperature);
}

// Проверка толщины КЭ
bool TProblemSetupForm::checkThickness(void)
{
    return checkTable(ui->twThickness);
}

// Проверка корректности ввода параметров расчета в динамике
bool TProblemSetupForm::checkDynamicParams(void)
{
    double theta,
           t0,
           t1,
           th;

    if (not checkTable(ui->twDensity, 4))
        return false;
    if (not checkTable(ui->twDamping, 4))
        return false;

    if (ui->textThetaWilson->text().length())
    {
        if (getExpression(ui->textThetaWilson->text(), theta) not_eq ErrorCode::Undefined or theta <= 0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the Wilson-theta!"));
            return false;
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the Wilson-theta!"));
        return false;
    }
    if (ui->textT0->text().length())
    {
        if (getExpression(ui->textT0->text(), t0) not_eq ErrorCode::Undefined or t0 < 0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the start time!"));
            return false;
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the start time!"));
        return false;
    }
    if (ui->textT1->text().length())
    {
        if (getExpression(ui->textT1->text(), t1) not_eq ErrorCode::Undefined or t1 <= t0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the stop time!"));
            return false;
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the stop time!"));
        return false;
    }
    if (ui->textTH->text().length())
    {
        if (getExpression(ui->textTH->text(), th) not_eq ErrorCode::Undefined or th > t1 - t0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the time step!"));
            return false;
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the time step!"));
        return false;
    }

    if (ui->textU->text().length())
    {
        if (checkExpression(ui->textU->text()) not_eq ErrorCode::Undefined)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textV->text().length())
    {
        if (checkExpression(ui->textV->text()) not_eq ErrorCode::Undefined)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textW->text().length())
    {
        if (checkExpression(ui->textW->text()) not_eq ErrorCode::Undefined)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textUt->text().length())
    {
        if (checkExpression(ui->textUt->text()) not_eq ErrorCode::Undefined)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textVt->text().length())
    {
        if (checkExpression(ui->textVt->text()) not_eq ErrorCode::Undefined)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textWt->text().length())
    {
        if (checkExpression(ui->textWt->text()) not_eq ErrorCode::Undefined)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textUtt->text().length())
    {
        if (checkExpression(ui->textUtt->text()) not_eq ErrorCode::Undefined)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textVtt->text().length())
    {
        if (checkExpression(ui->textVtt->text()) not_eq ErrorCode::Undefined)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    if (ui->textWtt->text().length())
    {
        if (checkExpression(ui->textWtt->text()) not_eq ErrorCode::Undefined)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly set the initial conditions!"));
            return false;
        }
    }
    return true;
}

// Проверка объемной нагрузки
bool TProblemSetupForm::checkVolumeLoad(void)
{
    return checkTable(ui->twVV, 0);
}

// Проверка поверхностной нагрузки
bool TProblemSetupForm::checkSurfaceLoad(void)
{
    return checkTable(ui->twSV, 1);
}

// Проверка сосредоточенной нагрузки
bool TProblemSetupForm::checkConcentratedLoad(void)
{
    return checkTable(ui->twCV, 2);
}

// Проверка нагрузки давлением
bool TProblemSetupForm::checkPressureLoad(void)
{
    return checkTable(ui->twPV, 3);
}

// Загрузка параметров расчета в Object
bool TProblemSetupForm::getParams(void)
{
    if (not check())
        return false;

    femObject->getParams().clear();
    femObject->setTaskParam((ui->rbDynamic->isChecked()) ? FEMType::DynamicProblem : FEMType::StaticProblem);
    femObject->getParams().tMethod = TimeMethod::Wilson;

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
    return true;
}

void TProblemSetupForm::getFunNames(void)
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


void TProblemSetupForm::getPlasticityParam(void)
{
    matrix<double> ssc(unsigned(ui->twStressStrainCurve->rowCount()), 2);

    femObject->getParams().pMethod = (ui->rbLinear->isChecked()) ? PlasticityMethod::Linear : (ui->rbMVS->isChecked()) ? PlasticityMethod::MVS : PlasticityMethod::MES;
    femObject->getParams().loadStep = ui->textLoadStep->text().toDouble();

    for (unsigned i = 0; i < unsigned(ui->twStressStrainCurve->rowCount()); i++)
    {
        decodeStressStarinCurve(ui->twStressStrainCurve->item(int(i), 0)->text().toStdString(), ssc);
        femObject->getParams().plist.addStressStrainCurve(ssc, ui->twStressStrainCurve->item(int(i), 1)->text().toStdString());
    }
}

void TProblemSetupForm::getVolumeLoad(void)
{
    getTableValue(ParamType::VolumeLoad, ui->twVV);
}

void TProblemSetupForm::getSurfaceLoad(void)
{
    getTableValue(ParamType::SurfaceLoad, ui->twSV);
}

void TProblemSetupForm::getConcentratedLoad(void)
{
    getTableValue(ParamType::ConcentratedLoad, ui->twCV);
}

void TProblemSetupForm::getPressureLoad(void)
{
    getTableValue(ParamType::PressureLoad, ui->twPV, false);
}

// Извлечение граничных условий
void TProblemSetupForm::getBoundaryConditionValue(void)
{
    getTableValue(ParamType::BoundaryCondition, ui->twBC);
}


// Извлечение начальных условий
void TProblemSetupForm::getInitialParam(void)
{
    femObject->getParams().plist.addInitialCondition(ui->textU->text().toStdString(), InitialCondition::U);     // U(t=0) = 0
    femObject->getParams().plist.addInitialCondition(ui->textUt->text().toStdString(), InitialCondition::Ut);   // Ut(t=0) = 0
    femObject->getParams().plist.addInitialCondition(ui->textUtt->text().toStdString(), InitialCondition::Utt); // Utt(t=0) = 0

    femObject->getParams().plist.addInitialCondition(ui->textV->text().toStdString(), InitialCondition::V);     // V(t=0) = 0
    femObject->getParams().plist.addInitialCondition(ui->textVt->text().toStdString(), InitialCondition::Vt);   // Vt(t=0) = 0
    femObject->getParams().plist.addInitialCondition(ui->textVtt->text().toStdString(), InitialCondition::Vtt); // Vtt(t=0) = 0

    femObject->getParams().plist.addInitialCondition(ui->textW->text().toStdString(), InitialCondition::W);     // W(t=0) = 0
    femObject->getParams().plist.addInitialCondition(ui->textWt->text().toStdString(), InitialCondition::Wt);   // Wt(t=0) = 0
    femObject->getParams().plist.addInitialCondition(ui->textWtt->text().toStdString(), InitialCondition::Wtt); // Wtt(t=0) = 0
}

// Извлечение плотности
void TProblemSetupForm::getDensity(void)
{
    getTableValue(ParamType::Density, ui->twDensity, false);
}

// Извлечение параметра демпфирования
void TProblemSetupForm::getDamping(void)
{
    getTableValue(ParamType::Damping, ui->twDamping, false);
}

// Извлечение параметров температурного расширения
void TProblemSetupForm::getThermalExpansionParam(void)
{
    getTableValue(ParamType::Alpha, ui->twThermalExpansion, false);
    getTableValue(ParamType::Temperature, ui->twTemperature, false);
}

// Извлечение параметров вывода
void TProblemSetupForm::getWidth(void)
{
    femObject->getParams().width = ui->textWidth->text().toInt();
}

void TProblemSetupForm::getPrecission(void)
{
    femObject->getParams().precision = ui->textPrecision->text().toInt();
}

// Извлечение теты-Вильсона
void TProblemSetupForm::getTheta(void)
{
    femObject->getParams().theta = ui->textThetaWilson->text().toDouble();
}

// Извлечение времени
void TProblemSetupForm::getTime(void)
{
    femObject->getParams().t0 = ui->textT0->text().toDouble();
    femObject->getParams().t1 = ui->textT1->text().toDouble();
    femObject->getParams().th = ui->textTH->text().toDouble();
}

// Извлечение параметра толщины элемента
void TProblemSetupForm::getThickness(void)
{
    getTableValue(ParamType::Thickness, ui->twThickness, false);
}

// Извлечение параметра точности расчета
void TProblemSetupForm::getEps(void)
{
    femObject->getParams().eps = ui->textEps->text().toDouble();
}

// Определение номера комбинации выбранных функций в списке граничных условий
Direction TProblemSetupForm::getDirect(QTableWidget* p, int i)
{
    Direction direct = Direction::Undefined;

    if (p->item(i, 2)->checkState() == Qt::Checked)
        direct |= Direction::X;
    if (p->item(i, 3)->checkState() == Qt::Checked)
        direct |= Direction::Y;
    if (p->item(i, 4)->checkState() == Qt::Checked)
        direct |= Direction::Z;
    return direct;
}

void TProblemSetupForm::getTableValue(ParamType type, QTableWidget* tw, bool isDirect)
{
    Direction direct = Direction::Undefined;

    for (int i = 0; i < tw->rowCount(); i++)
    {
        if (isDirect)
            direct = getDirect(tw, i);
        femObject->getParams().plist.addParameter(type, tw->item(i, 0)->text().toStdString(), tw->item(i, 1)->text().toStdString(), static_cast<int>(direct));
    }
}

// Извлечение параметров упругости
void TProblemSetupForm::getElasticParam(void)
{
    getTableValue(ParamType::YoungModulus, ui->twYoungModulus, false);
    getTableValue(ParamType::PoissonRatio, ui->twPoissonsRatio, false);
}

void TProblemSetupForm::slotCancelButton(void)
{
    setup();
}


//void TProblemSetupForm::showParams(QTableWidget *tw, QString name)
//{
//    emit clicked(tw, name);


//    unsigned numThread = 8, //std::thread::hardware_concurrency(),
//             step = femObject->getMesh().getNumBE() / numThread;
//    int error = NO_ERR;
//    bool isStoped = false;
//    vector<std::thread> thr(numThread);
//    vector<double> vertex(int(femObject->getMesh().getNumVertex()));

//    msg->setProcess(BC_CREATE_PROCESS, 1, int(femObject->getMesh().getNumBE()), 5);
//    // Обработка граничных условий
////    for (unsigned i = 0; i < numThread; i++)
////        thr[i] = std::thread(&TProblemSetupForm::calcParams, this, tw, ref(vertex), i * step, (i == numThread - 1) ? femObject->getMesh().getNumBE() : (i + 1) * step, ref(error), ref(isStoped));
////    for_each(thr.begin(), thr.end(), [](auto& t) { t.join(); });
//    calcParams(tw, ref(vertex), 0, femObject->getMesh().getNumBE(), ref(error), ref(isStoped));
//    msg->stopProcess();
//    if (error)
//        cerr << endl << sayError(ErrorCode(error)) << endl;

//    femObject->getResult().addResult(vertex, name.toUtf8().toStdString());
//}

//void TProblemSetupForm::calcParams(QTableWidget *tw, vector<double> &vertex, unsigned begin, unsigned end, int &error, bool &stop)
//{
//    double value;
//    vector<double> coord;
//    QString predicate,
//            expression;

////    cerr << begin << ' ' << end << endl;
////    return;
//    for (unsigned i = begin; i < end; i++)
//    {
//        msg->addProgress();
//        if (stop)
//        {
//            error = ABORT_ERR;
//            return;
//        }
//        femObject->getMesh().getCenterBE(i, coord);
//        for (int j = 0; j < tw->rowCount(); j++)
//        {
//            expression = tw->item(j, 0)->text();
//            predicate = tw->item(j, 1)->text();
//            if (predicate.length() and (error = getExpression(predicate, value, coord[0], coord[1], coord[2])) not_eq 0)
//                continue;
//            if (value == 0)
//                continue;
//            if ((error = getExpression(expression, value, coord[0], coord[1], coord[2])) not_eq 0)
//                continue;
//            if (value == 0)
//                continue;
//            for (int k = 0; k < femObject->getMesh().getSizeBE(); k++)
//                vertex[femObject->getMesh().getBE(i, k)] = value;
//            break;
//        }
//        if (error)
//            return;
//    }
//}
