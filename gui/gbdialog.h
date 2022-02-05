#ifndef GBDIALOG_H
#define GBDIALOG_H

#include <QDialog>

namespace Ui {
class TGBDialog;
}

class TGBDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit TGBDialog(QWidget *parent = nullptr);
    ~TGBDialog();
    void set(double,double,double,double,double,double,QString,unsigned,unsigned);
    void changeLanguage(void);

private:
    Ui::TGBDialog *ui;
};

#endif // GBDIALOG_H
