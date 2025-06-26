#ifndef MAINSETUPFORM_H
#define MAINSETUPFORM_H

#include <QWidget>
#include <QMenu>
#include <QItemDelegate>
#include <string>
#include "util/matrix.h"

class QToolButton;
class QTableWidget;
class TFEMObject;
enum class Direction;

namespace Ui {
    class TProblemSetupForm;
}

enum class ParamType;
enum class ErrorCode;

class TDelegateHTML : public QItemDelegate
{
    Q_OBJECT
public:
    TDelegateHTML(QObject *parent = nullptr) : QItemDelegate(parent) {}
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


class TProblemSetupForm : public QWidget
{
    Q_OBJECT

signals:
    void clicked(int);

public:
    explicit TProblemSetupForm(TFEMObject*, QWidget* = nullptr);
    ~TProblemSetupForm();
    void changeLanguage(void);
    void clear(void);
    void setup(void);
    bool decodeStressStarinCurve(string, matrix<double>&);
    bool getParams(void);

private slots:
    void slotShowContextMenu1(const QPoint&);
    void slotShowContextMenu2(const QPoint&);

private:
    Ui::TProblemSetupForm *ui;
    // Указатель на объект расчета
    TFEMObject *femObject = nullptr;
    // Контекстные меню
    QMenu menu1;
    QMenu menu2;
    QMenu menu3;
    void createMenu(void);
    void setEnabledBtn(QToolButton*, QTableWidget*);
    void setEnabledBtn(QToolButton*, QToolButton*, QTableWidget*);
    void enabledParams(void);
    void addRow(QTableWidget*);
    void removeRow(QTableWidget*);
    void enabledFuncNames(void);
    void setDirect(QTableWidget*, Direction);
    void setDirect(QTableWidget*, int, Direction);
    void removeAllRows(QTableWidget*);
    void setTableValue(ParamType, QTableWidget*, bool = true);
    void setElasticParam(void);
    void setThermalExpansionParam(void);
    void setDensity(void);
    void setDamping(void);
    void setInitialParam(void);
    void setThickness(void);
    void setFuncNames(void);
    void setVariables(void);
    void setPressureLoad(void);
    void setVolumeLoad(void);
    void setSurfaceLoad(void);
    void setConcentratedLoad(void);
    void setPlasticityParam(void);
    void setBoundaryConditionValue(void);
    void getVariables(void);
    void getTableValue(ParamType, QTableWidget*, bool = true);
    void getElasticParam(void);
    void getEps(void);
    void getThickness();
    void getWidth(void);
    void getPrecission(void);
    void getTime(void);
    void getTheta(void);
    void getThermalExpansionParam(void);
    void getDensity(void);
    void getDamping(void);
    void getInitialParam(void);
    void getFunNames(void);
    void getVolumeLoad(void);
    void getSurfaceLoad(void);
    void getConcentratedLoad(void);
    void getPressureLoad(void);
    void getBoundaryConditionValue(void);
    void getPlasticityParam(void);

    bool check(void);
    bool checkTable(QTableWidget*, int = -1);
    bool checkYoungModulus(void);
    bool checkPoissonRatio(void);
    bool checkThermalExpansion(void);
    bool checkTemperature(void);
    bool checkThickness(void);
    bool checkDynamicParams(void);
    bool checkVolumeLoad(void);
    bool checkSurfaceLoad(void);
    bool checkConcentratedLoad(void);
    bool checkPressureLoad(void);
    bool checkLimitValue(void);
    bool checkFuncNames(void);
    bool checkPlasticity(void);
    bool checkVariables(void);

    ErrorCode getExpression(QString, double&, double = 0, double = 0, double = 0);
    ErrorCode checkExpression(QString);
    Direction getDirect(QTableWidget*, int);
    QTableWidget* getLoadTab(void);
};

#endif // MAINSETUPFORM_H
