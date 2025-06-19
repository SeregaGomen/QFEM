#ifndef HELPDIALOG_H
#define HELPDIALOG_H

#include <QDialog>

namespace Ui {
class THelpDialog;
}

class THelpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit THelpDialog(int,QWidget *parent = nullptr);
    ~THelpDialog();

private:
    Ui::THelpDialog *ui;
};

#endif // HELPDIALOG_H
