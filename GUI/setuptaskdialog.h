#ifndef SETUPTASKDIALOG_H
#define SETUPTASKDIALOG_H

#include <QtCore>
#include <QMenu>
#include <QDialog>
#include <QItemDelegate>
#include "object/plist.h"
#include "object/params.h"



class QComboBox;
class QTableWidget;
class TFEMObject;
class QToolButton;

namespace Ui {
    class TSetupTaskDialog;
}


class QXmlStreamWriter;

class THTMLDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    THTMLDelegate(QObject *parent = nullptr) : QItemDelegate(parent) {}
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


class TSetupTaskDialog : public QDialog
{
    Q_OBJECT
private:
    bool isBoundaryConditionChanged;    // Признак того, что изменились краевые условия (для пересчета)
    Ui::TSetupTaskDialog *ui;
    TFEMObject* femObject;  // Указатель на объект расчета
    QMenu menu1;    // Контекстные меню
    QMenu menu2;
    QMenu menu3;
    void enabledParams(void);
    void enabledFuncNames(void);
    void setInitialParam(void);
    void setBoundaryConditionValue(void);
    void setPressureLoad(void);
    void setVolumeLoad(void);
    void setSurfaceLoad(void);
    void setConcentratedLoad(void);
    void setPlasticityParam(void);
    void setFuncNames(void);
    void setVariables(void);
    void setElasticParam(void);
    void setDensity(void);
    void setDamping(void);
    void setThermalExpansionParam(void);
    void setThickness(void);
    void setDirect(QTableWidget*, int, int);
    void setDirect(QTableWidget*, int);
    void setTableValue(ParameterType, QTableWidget*, bool = true);
    void createMenu(void);
    void removeAllRows(QTableWidget*);
    void addRow(QTableWidget*);
    void removeRow(QTableWidget*);
    void setEnabledBtn(QToolButton*, QTableWidget*);
    void reload(void);
    void getTableValue(ParameterType, QTableWidget*, bool = true);
    void getBoundaryConditionValue(void);
    void getFunNames(void);
    void getVariables(void);
    void getVolumeLoad(void);
    void getSurfaceLoad(void);
    void getConcentratedLoad(void);
    void getPressureLoad(void);
    void getElasticParam(void);
    void getThickness();
    void getEps(void);
    void getThermalExpansionParam(void);
    void getWidth(void);
    void getPrecission(void);
    void getDensity(void);
    void getDamping(void);
    void getTime(void);
    void getTheta(void);
    void getInitialParam(void);
    void getPlasticityParam(void);
    bool checkTable(QTableWidget*, int, int = -1);
    bool checkDynamicParams(void);
    bool checkYoungModulus(void);
    bool checkPoissonRatio(void);
    bool checkThermalExpansion(void);
    bool checkTemperature(void);
    bool checkThickness(void);
    bool checkVolumeLoad(void);
    bool checkSurfaceLoad(void);
    bool checkConcentratedLoad(void);
    bool checkPressureLoad(void);
    bool checkLimitValue(void);
    bool checkVariables(void);
    bool checkFuncNames(void);
    bool checkPlasticity(void);
    bool checkPredicate(QString);
    bool checkExpression(QString);
    bool checkExpression(QString, double&);
    bool isStatic(void);
    bool isDynamic(void);
    bool check(void);
    int checkVertex(double, double, double);
    int getDirect(QTableWidget*, int);

private slots:
    void slotShowContextMenu1(const QPoint&);
    void slotShowContextMenu2(const QPoint&);
    void slotCellEdited(int, int)
    {
        isBoundaryConditionChanged = true;
    }
    void accept(void);

public:
    explicit TSetupTaskDialog(TFEMObject*,QWidget *parent = nullptr);
    ~TSetupTaskDialog();
    void setup(void);
    void clear(void);
    void changeLanguage(void);
    void setBCChanged(bool p)
    {
        isBoundaryConditionChanged = p;
    }
    bool isBCChanged(void)
    {
        return isBoundaryConditionChanged;
    }
    bool decodeStressStarinCurve(string, matrix<double>&);
};

#endif // SETUPTASKDIALOG_H
