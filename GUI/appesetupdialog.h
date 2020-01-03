#ifndef APPSETUPDIALOG_H
#define APPSETUPDIALOG_H

#include <QDialog>

namespace Ui {
class TAppSetupDialog;
}

class TAppSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TAppSetupDialog(int,bool,bool,bool,QWidget *parent = nullptr);
    ~TAppSetupDialog();
    void changeLanguage(void);
    int getLangNo(void);
    bool getIsBlackBkg(void);
    bool getIsAutoScroll(void);
    bool getIsAutoSaveProtocol(void);

private:
    Ui::TAppSetupDialog *ui;
};

#endif // APPSETUPDIALOG_H
