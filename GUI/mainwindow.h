#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "qstdredirector.h"

namespace Ui {
    class TMainWindow;
}

class TTerminal;
class QProgressBar;
class QDockWidget;
class QTabWidget;
class TSetupImageDialog;
class QTranslator;
class QThread;
class TFEMProcessor;
class TBCProcessor;
class TProblemSetupForm;


const int maxRecentFiles = 5;


class TMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit TMainWindow(QWidget *parent = nullptr);
    ~TMainWindow();

public slots:
    void slotOpenDocument(void);
    void slotCloseDocument(void);
    void slotSaveAsDocument(void);
    void slotAnalyseFunction(void);
    void slotAddFunction(void);
    void slotSetTerminal(void);
    void slotSetRotate(void);
    void slotSetScale(void);
    void slotSetTranslate(void);
    void slotSetRotate(int);
    void slotSetScale(int);
    void slotSetTranslate(int);
    void slotRestoreImage(void);
    void slotSetupTaskParams(void);
    void slotSetupImageParams(void);
    void slotStartProcess(void);
    void slotStopProcess(void);
    void slotSaveResults(void);
    void slotSetScroll(void);
    void slotCloseTab(int);
    void slotChangeTab(int);
    void slotCloseTerminal(void);
    void slotShowGabarites(void);
    void slotAppSettings(void);
    void slotAboutProgramm(void);
    void slotHelpProgramm(void);
    void slotDataCopy(void);


protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void slotOpenRecentFile(void);
    void slotErorrMsg(QString);
    void slotMsg(QString);
    void slotShowParam(int);

private:
    QThread* thread;
    Ui::TMainWindow* ui;
    QTranslator* translator;
    TTerminal* terminal;
    QDockWidget* dock;
    QTabWidget* tabWidget;
    TFEMProcessor* femProcessor;
    TBCProcessor* bcProcessor;
    QString curFile;
    QMenu* recentFilesMenu;
    QAction* recentFileActs[maxRecentFiles];
    QAction* separatorAct;
    QStringList files;
    QStdRedirector<> *myCout;
    QStdRedirector<> *myCerr;
    TSetupImageDialog *iDlg;
    TProblemSetupForm *pForm;
    QProgressBar *pb;
    bool isUntitled;
    bool isAutoScroll;
    bool isBlackBkg;
    bool isAutoSaveProtocol;
    int curTabIndex;
    unsigned getTimeDeltaIndex(QString);
    void init(void);
    void startSolvingProblem(void);
    void setCurrentFile(QString);
    void readSettings(void);
    void createRecentMenu(void);
    void writeSettings(void);
    void setupRecentActions(void);
    void updateRecentFileActions(QString);
    void loadFile(QString);
    void checkMenuState(void);
    void showProtocol(QString);
    void sayParams(QString&);
    void sayParam(QString&, const QString&, int, bool = true);
    void saveDocument(QString);
    void addFuncToAnalyse(QString, QString);
    void setupLanguage(void);
    bool calcExpression(QString, QString&);
    bool loadMesh(QString);
    bool saveQFPF(QString);
    bool loadQFPF(QString);
    bool saveQRES(QString);
    bool loadQRES(QString);
    bool loadRES(QString);
    bool checkParams(void);
    void lcProcess(void);
    void femProcess(void);
    void repaintResults(void);
    void saveMesh(QJsonObject&);
    void saveParam(QJsonObject&);
    void saveResults(QJsonObject&);
    void loadMesh(const QJsonObject&);
    void loadParam(const QJsonObject&);
    void loadResults(const QJsonArray&);
};



#endif // MAINWINDOW_H
