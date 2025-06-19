#ifndef FLDIALOG_H
#define FLDIALOG_H

#include <QDialog>
#include <QString>

class TFEMObject;

namespace Ui {
    class TFunListDialog;
}

class TFunListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TFunListDialog(QWidget *parent = nullptr);
    ~TFunListDialog();
    void setup(TFEMObject*);
    void changeLanguage(void);
    QString getName(void);
    int getIndex(void);
private:
    Ui::TFunListDialog *ui;
};

#endif // FLDIALOG_H
