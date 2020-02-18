#ifndef MAINSETUPFORM_H
#define MAINSETUPFORM_H

#include <QWidget>
#include <QMenu>
#include <QItemDelegate>
#include "util/matrix.h"

class QToolButton;
class QTableWidget;
class TFEMObject;

namespace Ui {
    class TProblemSetupForm;
}

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

public:
    explicit TProblemSetupForm(TFEMObject *, QWidget * = nullptr);
    ~TProblemSetupForm();
    void changeLanguage(void);
    void clear(void);
    void setup(void);
    bool decodeStressStarinCurve(string, matrix<double>&);


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
    void enabledParams(void);
    void addRow(QTableWidget*);
    void removeRow(QTableWidget*);
    void enabledFuncNames(void);
    void setDirect(QTableWidget*, int);
    void setDirect(QTableWidget*, int, int);
    void removeAllRows(QTableWidget*);
    void setTableValue(int, QTableWidget*, bool = true);
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


    QTableWidget* getLoadTab(void);
};

#endif // MAINSETUPFORM_H
