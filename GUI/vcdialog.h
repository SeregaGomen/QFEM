#ifndef VCDIALOG_H
#define VCDIALOG_H

#include <QDialog>
#include <QTableWidget>

namespace Ui {
class TVCDialog;
}

class TVCDialog : public QDialog
{
    Q_OBJECT
    
public slots:
    void accept(void);

public:
    explicit TVCDialog(QWidget *parent = nullptr);
    ~TVCDialog();
    void changeLanguage(void);
    QString getExpression(void);

protected:
    void	showEvent(QShowEvent*);

private slots:
    void slotChangeText(const QString&);

private:
    Ui::TVCDialog *ui;
};

#endif // VCDIALOG_H
