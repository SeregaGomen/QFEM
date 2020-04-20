#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QClipboard>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QTextCodec>
#include <QtCore>
#include <QtGui>
#include <QDockWidget>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <fstream>
#include <ctime>

#include "problemsetupform.h"
#include "appesetupdialog.h"
#include "vcdialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "setupimagedialog.h"
#include "fldialog.h"
#include "glparam.h"
#include "gbdialog.h"
#include "qmsg.h"
#include "helpdialog.h"
#include "femprocessor.h"
#include "bcprocessor.h"
#include "terminal.h"
#include "parser/parser.h"
#include "object/object.h"


TMessenger* msg;
static int langNo;


TMainWindow::TMainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::TMainWindow)
{
    curTabIndex = -1;
    ui->setupUi(this);
    init();
}

TMainWindow::~TMainWindow()
{
    delete msg;
    delete thread;
    delete femProcessor;
    delete bcProcessor;
    delete myCout;
    delete myCerr;
    delete terminal;
    delete dock;
    delete tabWidget;
    delete iDlg;
//    delete pForm;
    delete ui;
}

void TMainWindow::init(void)
{
    QTextCodec* codec =  QTextCodec::codecForName("UTF-8");
#if QT_VERSION < 0x050000
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForCStrings(codec);
#endif
    QTextCodec::setCodecForLocale(codec);

    #ifndef Q_OS_LINUX
        setWindowIcon(QIcon(":/images/app.ico"));
//    #else
//        setWindowIcon(QIcon(":/images/main.png"));
    #endif

    numThread = (std::thread::hardware_concurrency() - 1 <= 0) ? 1 : std::thread::hardware_concurrency() - 1;
    isAutoSaveProtocol = isAutoScroll = isAutoSaveResults = isUntitled = true;

    femProcessor = new TFEMProcessor();
    bcProcessor = new TBCProcessor(femProcessor->getFEMObject());

    setupRecentActions();
    readSettings();
    setupLanguage();
    createRecentMenu();

    iDlg = new TSetupImageDialog(this);

    pForm = new TProblemSetupForm(femProcessor->getFEMObject());

    tabWidget = new QTabWidget();
    tabWidget->setTabsClosable(false);

    setCentralWidget(tabWidget);

    thread = new QThread(this);

    terminal = new TTerminal(thread, this);
//    terminal->setFontWeight( QFont::DemiBold );
//    terminal->setTextColor( QColor( "white" ) );
    terminal->setTextColor( QColor( "black" ) );
//    terminal->setStyleSheet("QTextEdit { background-color: rgb(0, 0, 0) }");
    terminal->setStyleSheet("QTextEdit { background-color: rgb(255, 255, 255) }");
    terminal->setReadOnly(true);
    terminal->setWordWrapMode(QTextOption::NoWrap);
    terminal->setFont(QFont("Courier"));

    //////////////////////
    dock = new QDockWidget(tr("Terminal"), this);
    dock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
    dock->setWidget(terminal);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
    dock->setVisible(ui->actionTerminal->isChecked());
    dock->setWindowTitle(tr("Terminal"));
    connect(dock, SIGNAL(visibilityChanged(bool)), this, SLOT(slotCloseTerminal(void)));

    //////////////////////
    // Перехват cout и cerr
    myCout = new QStdRedirector<>(std::cout, this);
//    connect(myCout, SIGNAL(messageChanged(QString)), terminal, SLOT(insertPlainText(QString)));
    connect(myCout, SIGNAL(messageChanged(QString)), this, SLOT(slotMsg(QString)));
    cout << endl;

    myCerr = new QStdRedirector<>(std::cerr, this);
//    connect(myCerr, SIGNAL(messageChanged(QString)), this, SLOT(setErrColor()));
//    connect(myCerr, SIGNAL(messageChanged(QString)), terminal, SLOT(insertPlainText(QString)));
    connect(myCerr, SIGNAL(messageChanged(QString)), this, SLOT(slotErorrMsg(QString)));


    //////////////////////
    // Скроллирование терминала вниз при добавлении данных
    connect(terminal, SIGNAL(textChanged()), this, SLOT(slotSetScroll()));
    //////////////////////

    connect(tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(slotCloseTab(int)));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(slotChangeTab(int)));


    checkMenuState();

    pb = new SProgressBar(statusBar());
    pb->setTextVisible(false);
    pb->hide();
    statusBar()->addPermanentWidget(pb);

    msg = new SProcessMessenger();
    connect(dynamic_cast<QObject*>(msg), SIGNAL(signalSetValue(int)), pb, SLOT(setValue(int)));
    connect(dynamic_cast<QObject*>(msg), SIGNAL(signalSetMinimum(int)), pb, SLOT(setMinimum(int)));
    connect(dynamic_cast<QObject*>(msg), SIGNAL(signalSetMaximum(int)), pb, SLOT(setMaximum(int)));

    connect(dynamic_cast<QObject*>(msg), SIGNAL(signalSetTextVisible(bool)), pb, SLOT(slotSetTextVisible(bool)));
    connect(dynamic_cast<QObject*>(msg), SIGNAL(signalSetFormat(const QString&)), pb, SLOT(slotSetFormat(const QString&)));



//    connect(pForm, &TProblemSetupForm::clicked, ([=](void) { testSignal(); }));
    connect(pForm, SIGNAL(clicked(int)), this, SLOT(slotShowParam(int)));
}

void TMainWindow::slotErorrMsg(QString msg)
{
    QColor tc = terminal->textColor();

    if (msg != "\n")
        terminal->setTextColor(QColor("red"));
    terminal->insertPlainText(msg);
    terminal->setTextColor(tc);
//    QApplication::processEvents();
}

void TMainWindow::slotMsg(QString msg)
{
    QTextCursor cursor = terminal->textCursor();

    if (msg.contains('\r'))
    {
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.insertText(msg);
    }
    else
        terminal->insertPlainText(msg);
}

void TMainWindow::slotChangeTab(int nTab)
{
    bool isEnabled = (qobject_cast<TGLMesh*>(tabWidget->widget(nTab)) == nullptr) ? false : true;

    ui->actionInfo->setEnabled(isEnabled);
    ui->actionRotate->setEnabled(isEnabled);
    ui->actionScale->setEnabled(isEnabled);
    ui->actionTranslate->setEnabled(isEnabled);
    ui->actionRestore->setEnabled(isEnabled);
//    ui->actionAnalyse->setEnabled(isEnabled);
    ui->actionAnalyse->setEnabled(femProcessor->isCalculated());
    ui->actionSetupImage->setEnabled(isEnabled);
    if (/*tabWidget->currentIndex() == 0 &&*/ !isUntitled)
        ui->actionObjectParameters->setEnabled(true);
    else
        ui->actionObjectParameters->setEnabled(false);
    if (isEnabled && !isUntitled)
    {
        if (qobject_cast<TGLMesh*>(tabWidget->widget(nTab))->getRotate())
            slotSetRotate();
        else if (qobject_cast<TGLMesh*>(tabWidget->widget(nTab))->getScale())
            slotSetScale();
        else
            slotSetTranslate();
    }
}

void TMainWindow::slotCloseTab(int nTab)
{
    int i;

    if (nTab != 0)
    {
        tabWidget->removeTab(nTab);
        if (nTab == 1 && ui->actionObjectParameters->isChecked())
            ui->actionObjectParameters->setChecked(false);
    }
    else
    {
        if (thread->isRunning())
        {
            QApplication::setOverrideCursor(Qt::BusyCursor);
            femProcessor->stop();
            bcProcessor->stop();
            while (thread->isRunning())
                QApplication::processEvents();
            QApplication::restoreOverrideCursor();
        }
        femProcessor->clear();
        bcProcessor->clear();
        pb->setValue(0);
        i = tabWidget->count() - 1;
        while (tabWidget->count() > 0)
        {
            if (qobject_cast<TGLMesh*>(tabWidget->widget(i)))
                delete tabWidget->widget(i);
            tabWidget->removeTab(i--);
        }
        iDlg->clear();
        pForm->clear();
        setWindowTitle("QFEM");
        isUntitled = true;
        checkMenuState();
        ui->actionObjectParameters->setChecked(false);
    }

}

void TMainWindow::slotShowGabarites(void)
{
    TFEMObject* femObject = femProcessor->getFEMObject();
    QString typeFE = QString("%1").arg(femObject->getMesh().feName().c_str());
    unsigned numVertex = femObject->getMesh().getNumVertex(),
             numFE = femObject->getMesh().getNumFE();
    double minX = femObject->getMesh().getMinX(0),
           minY = femObject->getMesh().getMinX(1),
           minZ = femObject->getMesh().getMinX(2),
           maxX = femObject->getMesh().getMaxX(0),
           maxY = femObject->getMesh().getMaxX(1),
           maxZ = femObject->getMesh().getMaxX(2);
    TGBDialog* dlg = new TGBDialog(this);
    double eps = femObject->getParams().eps;

    dlg->changeLanguage();
    dlg->set((fabs(minX) < eps) ? 0 : minX,(fabs(minY) < eps) ? 0 : minY,(fabs(minZ) < eps) ? 0 : minZ,(fabs(maxX) < eps) ? 0 : maxX,(fabs(maxY) < eps) ? 0 : maxY,(fabs(maxZ) < eps) ? 0 : maxZ,typeFE,numVertex,numFE);
    dlg->show();

}

void TMainWindow::slotSetScroll(void)
{
    if (isAutoScroll)
        terminal->moveCursor(QTextCursor::End);
}

void TMainWindow::checkMenuState(void)
{
    bool isEnabled = (qobject_cast<TGLMesh*>(tabWidget->currentWidget()) == nullptr) ? false : true;

    ui->actionCopy->setEnabled(!isUntitled);
    ui->actionClose->setEnabled(!isUntitled);
    ui->actionSaveAs->setEnabled(!isUntitled);
    ui->actionRotate->setEnabled(!isUntitled && isEnabled);
    ui->actionScale->setEnabled(!isUntitled && isEnabled);
    ui->actionTranslate->setEnabled(!isUntitled && isEnabled);
    ui->actionRestore->setEnabled(!isUntitled);
    ui->actionObjectParameters->setEnabled(!isUntitled);
    ui->actionStart->setEnabled(!isUntitled && !thread->isRunning());
    ui->actionStop->setEnabled(!isUntitled && thread->isRunning());
    ui->actionAnalyse->setEnabled(!isUntitled && femProcessor->isCalculated());
    ui->actionAddExpression->setEnabled(!isUntitled && femProcessor->isCalculated());
    ui->actionSetupImage->setEnabled(!isUntitled && isEnabled);
    ui->actionInfo->setEnabled(!isUntitled);
    ui->actionSaveResults->setEnabled(!isUntitled && femProcessor->isCalculated());
}

void TMainWindow::slotSetRotate(void)
{
    ui->actionRotate->setChecked(true);
    ui->actionScale->setChecked(false);
    ui->actionTranslate->setChecked(false);

    qobject_cast<TGLMesh*>(tabWidget->currentWidget())->setRotate();
}

void TMainWindow::slotSetRotate(int i)
{
    ui->actionRotate->setChecked(true);
    ui->actionScale->setChecked(false);
    ui->actionTranslate->setChecked(false);

    qobject_cast<TGLMesh*>(tabWidget->widget(i))->setRotate();
}

void TMainWindow::slotSetScale(void)
{
    ui->actionRotate->setChecked(false);
    ui->actionScale->setChecked(true);
    ui->actionTranslate->setChecked(false);
    qobject_cast<TGLMesh*>(tabWidget->currentWidget())->setScale();
}

void TMainWindow::slotSetScale(int i)
{
    ui->actionRotate->setChecked(false);
    ui->actionScale->setChecked(true);
    ui->actionTranslate->setChecked(false);
    qobject_cast<TGLMesh*>(tabWidget->widget(i))->setScale();
}

void TMainWindow::slotSetTranslate(void)
{
    ui->actionRotate->setChecked(false);
    ui->actionScale->setChecked(false);
    ui->actionTranslate->setChecked(true);
    qobject_cast<TGLMesh*>(tabWidget->currentWidget())->setTranslate();
}

void TMainWindow::slotSetTranslate(int i)
{
    ui->actionRotate->setChecked(false);
    ui->actionScale->setChecked(false);
    ui->actionTranslate->setChecked(true);
    qobject_cast<TGLMesh*>(tabWidget->widget(i))->setTranslate();
}

void TMainWindow::slotSetTerminal(void)
{
    dock->setVisible(ui->actionTerminal->isChecked());
}

void TMainWindow::slotCloseTerminal(void)
{
    ui->actionTerminal->setChecked(dock->isVisible());
}

void TMainWindow::setupRecentActions(void)
{
    for (int i = 0; i < maxRecentFiles; ++i)
    {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()), this, SLOT(slotOpenRecentFile()));
    }
}

void TMainWindow::slotOpenDocument(void)
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Opening a document"), windowFilePath(), tr("QFEM problem files (*.qfpf);; Mesh files (*.trp *.trpa *.vol *.mesh);; QFEM result files (*.qres *.res)"));

    if (!fileName.isEmpty())
        loadFile(fileName);
}

void TMainWindow::loadFile(QString fileName)
{
    bool isOk = false;

    slotCloseTab(0);
    if (QFileInfo(fileName).completeSuffix().toUpper() == "TRP" || QFileInfo(fileName).completeSuffix().toUpper() == "TRPA" || QFileInfo(fileName).completeSuffix().toUpper() == "VOL"  || QFileInfo(fileName).completeSuffix().toUpper() == "MESH")
        isOk = loadMesh(fileName);
    else if (QFileInfo(fileName).completeSuffix().toUpper() == "QRES")
        isOk = loadQRES(fileName);
    else if (QFileInfo(fileName).completeSuffix().toUpper() == "RES")
        isOk = loadRES(fileName);
    else if (QFileInfo(fileName).completeSuffix().toUpper() == "QFPF")
        isOk = loadQFPF(fileName);

    if (isOk)
    {
        tabWidget->insertTab(0, new TGLMesh(&(femProcessor->getFEMObject()->getMesh()), this), tr("Object"));
        tabWidget->setTabsClosable(true);
        tabWidget->setCurrentIndex(0);

//        qobject_cast<TGLMesh*>(tabWidget->widget(0))->setImageParams(iDlg->getImageParams());

        QApplication::setOverrideCursor(Qt::BusyCursor);
        statusBar()->showMessage(tr("File successfully downloaded"),5000);
        updateRecentFileActions(fileName);
        slotSetRotate(0);
        setCurrentFile(fileName);
        pForm->setup();
        QApplication::restoreOverrideCursor();
    }
    else
    {
//        QMessageBox::information(this, tr("Error"), tr("Error opening file %1").arg(fileName));
        statusBar()->showMessage(tr("Error opening file"),5000);
    }
}

void TMainWindow::setCurrentFile(QString fileName)
{
    isUntitled = false;
    curFile = QFileInfo(fileName).canonicalFilePath();
    setWindowTitle(QFileInfo(fileName).fileName() + " - QFEM");
    setWindowModified(false);
    setWindowFilePath(curFile);
    checkMenuState();
}

void TMainWindow::slotRestoreImage(void)
{
    qobject_cast<TGLMesh*>(tabWidget->currentWidget())->restore();
//    glWidget->init();
}

void TMainWindow::readSettings(void)
{
//    int numRecentFiles;
    QSettings settings("ZNU","QFEM");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    QString path = settings.value("path").toString();
    bool isTerminal = settings.value("terminal").toBool();
    int states = settings.value("state").toInt();

    isAutoSaveResults = settings.value("results").toBool();
    isAutoScroll = settings.value("scroll").toBool();
    isAutoSaveProtocol = settings.value("protocol").toBool();
    ui->actionTerminal->setChecked(isTerminal);
    files = settings.value("recentFileList").toStringList();
    langNo = settings.value("lang").toInt();
    numThread = settings.value("thread").toInt();
    if (Qt::WindowStates(states) == Qt::WindowMaximized)
        this->setWindowState(Qt::WindowMaximized);
    else
    {
        move(pos);
        resize(size);
    }
    setWindowFilePath(path);
}

void TMainWindow::writeSettings(void)
{
    QSettings settings("ZNU","QFEM");

    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("path", windowFilePath());
    settings.setValue("state", int(windowState()));
    settings.setValue("terminal", ui->actionTerminal->isChecked());
    settings.setValue("recentFileList", files);
    settings.setValue("lang", langNo);
    settings.setValue("results", isAutoSaveResults);
    settings.setValue("scroll", isAutoScroll);
    settings.setValue("protocol", isAutoSaveProtocol);
    settings.setValue("thread", numThread);}

void TMainWindow::closeEvent(QCloseEvent *event)
{
    isUntitled = true;
//    if (thread->isRunning())
//    {
//        QApplication::setOverrideCursor(Qt::BusyCursor);
//        femProcessor->stop();
//        lcProcessor->stop();
//        while (thread->isRunning())
//            QApplication::processEvents();
//        QApplication::restoreOverrideCursor();
//    }
    slotCloseTab(0);
    writeSettings();
    event->accept();
}


void TMainWindow::slotSaveAsDocument(void)
{
    QString fileName = QString(QFileInfo(curFile).absolutePath() + "/" +  QFileInfo(curFile).baseName() + ".qfpf"),
            fullFileName = QFileDialog::getSaveFileName(this,tr("Saving the document"),fileName,tr("QFEM Problem files (*.qfpf)"));

    if (!fullFileName.isEmpty())
        saveDocument(fullFileName);
}

void TMainWindow::saveDocument(QString fileName)
{
    if (saveQFPF(fileName))
    {
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File successfully saved"), 5000);
        updateRecentFileActions(fileName);
    }
    else
    {
//        QMessageBox::information(this, tr("Error"), tr("Error writing file %1").arg(fileName));
        statusBar()->showMessage(tr("Error writing file"),5000);
    }
}


void TMainWindow::slotCloseDocument(void)
{
    slotCloseTab(0);
    femProcessor->clear();
    bcProcessor->clear();
    isUntitled = true;
    setWindowTitle("QFEM");
    iDlg->clear();
    pForm->clear();
    checkMenuState();
}

bool TMainWindow::loadMesh(QString fileName)
{
    TFEMObject* femObject = femProcessor->getFEMObject();
    string file = fileName.toStdString();
    bool ret;

    femObject->getParams().clear();
    QApplication::setOverrideCursor(Qt::BusyCursor);
    ret = femObject->setMeshFile(file);
    QApplication::restoreOverrideCursor();
    if (!ret)
    {
        QMessageBox::critical(this, tr("Error"), tr("Error reading file %1").arg(fileName));
        return false;
    }
    return true;
}

void TMainWindow::slotOpenRecentFile(void)
{
    QAction *action = qobject_cast<QAction *>(sender());

    if (action)
        loadFile(action->data().toString());
}

void TMainWindow::updateRecentFileActions(QString fileName)
{
    // Удаляем в меню старый список Recent-файлов
    for (int i = 0; i < files.size(); ++i)
        ui->menuFile->removeAction(recentFileActs[i]);

    // Модифицируем список Recent-файлов
    setWindowFilePath(fileName);
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > maxRecentFiles)
        files.removeLast();
    createRecentMenu();
}

void TMainWindow::createRecentMenu(void)
{
    // Создаем в меню новый список Recent-файлов
    for (int i = 0; i < files.size(); ++i)
    {
        QString text = QString("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());

        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
        recentFileActs[i]->setStatusTip(files[i]);
        ui->menuFile->insertAction(ui->actionExit, recentFileActs[i]);
    }
    if (files.size() > 0)
    {
        separatorAct = ui->menuFile->insertSeparator(ui->actionExit);
        separatorAct->setVisible(files.size() > 0);
    }
}

void TMainWindow::lcProcess(void)
{
    QTextCursor textCursor = terminal->textCursor(),
                saveCursor = textCursor;

    textCursor.clearSelection();
    terminal->setTextCursor(textCursor);

    pb->reset();
    pb->show();
    ui->actionStart->setEnabled(false);
    ui->actionStop->setEnabled(true);
    bcProcessor->moveToThread(thread);
    connect(thread, SIGNAL(started()), bcProcessor, SLOT(start()));
    connect(bcProcessor, SIGNAL(finished()), thread, SLOT(terminate()));
    thread->start();
    while (thread->isRunning())
        QApplication::processEvents();
    disconnect(thread, SIGNAL(started()), bcProcessor, SLOT(start()));
    disconnect(bcProcessor, SIGNAL(finished()), thread, SLOT(terminate()));
    ui->actionStart->setEnabled(true);
    ui->actionStop->setEnabled(false);
    pb->hide();

    terminal->setTextCursor(saveCursor);
}


void TMainWindow::slotSetupTaskParams(void)
{
//    bool isFind = false;
    QScrollArea *scroll;

    if (ui->actionObjectParameters->isChecked())
    {
        // Включаем режим отображения закладки параметров расчета
        ui->actionObjectParameters->setChecked(true);
        pForm->changeLanguage();
        pForm->setup();
        scroll = new QScrollArea();
        scroll->setWidget(pForm);
        tabWidget->insertTab(1, scroll, tr("Setup"));
        tabWidget->setTabsClosable(true);
        tabWidget->setCurrentIndex(1);
    }
    else
    {
        ui->actionObjectParameters->setChecked(false);
        tabWidget->removeTab(1);
    }
}

// Запуск процедуры расчета задачи
void TMainWindow::slotStartProcess(void)
{
    QTextCursor textCursor = terminal->textCursor(),
                saveCursor = textCursor;

    textCursor.clearSelection();
    terminal->setTextCursor(textCursor);

    ui->actionStart->setEnabled(false);
    ui->actionStop->setEnabled(true);
    ui->actionAnalyse->setEnabled(false);
//    ui->actionObjectParameters->setEnabled(false);

//    QApplication::setOverrideCursor(Qt::BusyCursor);
    startSolvingProblem();
    repaintResults();
    qApp->beep();
//    QApplication::restoreOverrideCursor();

//    ui->actionObjectParameters->setEnabled(true);
    ui->actionStart->setEnabled(true);
    ui->actionStop->setEnabled(false);
    checkMenuState();

    terminal->setTextCursor(saveCursor);
}

// Остановка процедуры расчета задачи (или любого вычислительного процесса)
void TMainWindow::slotStopProcess(void)
{
    bcProcessor->stop();
    femProcessor->stop();
    while (thread->isRunning())
        QApplication::processEvents();
    checkMenuState();
    ui->actionStart->setEnabled(true);
    ui->actionStop->setEnabled(false);
}

// Запуск процесса расчета в потоке
void TMainWindow::femProcess(void)
{
    pb->reset();
    pb->show();

    femProcessor->getFEMObject()->setNumThread(numThread);
    femProcessor->moveToThread(thread);
    connect(thread, SIGNAL(started()), femProcessor, SLOT(start()));
    connect(femProcessor, SIGNAL(finished()), thread, SLOT(terminate()));
    thread->start();
    while (thread->isRunning())
        QApplication::processEvents();
    disconnect(thread, SIGNAL(started()), femProcessor, SLOT(start()));
    disconnect(femProcessor, SIGNAL(finished()), thread, SLOT(terminate()));

    pb->hide();
}

// Проверка наличия всех необходимых для успешного расчета параметров
bool TMainWindow::checkParams(void)
{
    TFEMObject* femObject = femProcessor->getFEMObject();

    if (femObject->getParams().plist.findParameter(YOUNG_MODULUS_PARAMETER) == 0)
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified Young's modulus!"));
        return false;
    }
    if (femObject->getParams().plist.findParameter(POISSON_RATIO_PARAMETER) == 0 && !femObject->getMesh().is1D())
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified Poisson's ratio!"));
        return false;
    }
    if (femObject->getParams().plist.findParameter(BOUNDARY_CONDITION_PARAMETER) == 0)
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified boundary conditions!"));
        return false;
    }
    if (femObject->getParams().plist.findParameter(VOLUME_LOAD_PARAMETER) == 0 && femObject->getParams().plist.findParameter(SURFACE_LOAD_PARAMETER) == 0 &&
        femObject->getParams().plist.findParameter(CONCENTRATED_LOAD_PARAMETER) == 0 && femObject->getParams().plist.findParameter(PRESSURE_LOAD_PARAMETER) == 0)
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified loads!"));
        return false;
    }
    if (femObject->getParams().plist.findParameter(THICKNESS_PARAMETER) == 0 && (femObject->getMesh().is2D() || femObject->getMesh().isShell() || femObject->getMesh().isPlate()))
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified FE thickness!"));
        return false;
    }
    if (femObject->getParams().pMethod != Linear && (femObject->getParams().loadStep <= 0 || femObject->getParams().plist.findParameter(STRESS_STRAIN_CURVE_PARAMETER) == 0))
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified nonlinear parameters!"));
        return false;
    }
    if (femObject->getParams().fType == DynamicProblem)
    {
        if (femObject->getParams().theta <= 0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified the Wilson-Theta parameter!"));
            return false;
        }
        if (femObject->getParams().plist.findParameter(DENSITY_PARAMETER) == 0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified density!"));
            return false;
        }
        if (femObject->getParams().plist.findParameter(DAMPING_PARAMETER) == 0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified damping parameter!"));
            return false;
        }
        if (femObject->getParams().th == 0.0 || femObject->getParams().t0 < 0 || femObject->getParams().t1 <= 0 || (femObject->getParams().t0 >= femObject->getParams().t1))
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified time!"));
            return false;
        }


    }

    return true;
}


// Запуск расчета по функционалу Лагранжа (статика) или Гамильтона-Остроградского (динамика)
void TMainWindow::startSolvingProblem(void)
{
    TFEMObject* femObject = femProcessor->getFEMObject();
    QString htmlFile = QFileInfo(curFile).absolutePath() + "/" + QFileInfo(curFile).baseName() + ".html",
            qresFile = QFileInfo(curFile).absolutePath() + "/" + QFileInfo(curFile).baseName() + ".qres";

    try
    {
        // Запуск расчета
        femProcess();

        if (femObject->isCalculated())
        {
            // Отображение результатов
//            showResults(QFileInfo(curFile).absolutePath() + "/" + QFileInfo(curFile).baseName() + "." + QString("txt").toLower());
            QApplication::setOverrideCursor(Qt::BusyCursor);
            // femObject->saveResult(qresFile.toStdString());
            if (isAutoSaveResults)
                saveQRES(qresFile);
            showProtocol(htmlFile);
            QApplication::restoreOverrideCursor();
        }
    }
    catch (.../*TMessage&*/)
    {
//        cerr << serror(msg.GetErrorId()) << endl;
        cerr << S_ERR << endl;
    }
}

// Формирование протокола расчета (QRES)
void TMainWindow::showProtocol(QString fileName)
{
    QDateTime dt;
    TFEMObject* femObject = femProcessor->getFEMObject();
    QString webOut,
            tm;
    bool isFind = false;

    webOut = "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";

    // Получение времени и даты формирования отчета
    dt.setTime_t(femObject->getResult().getSolutionTime());
    webOut += "<h1>" + tr("The problem has been solving %1 at %2").arg(QString("%1.%2.%3").arg(dt.date().day(), 2, 10, QChar('0')).arg(dt.date().month(), 2, 10, QChar('0')).arg(dt.date().year(), 2, 10, QChar('0'))).arg(QString("%1:%2").arg(dt.time().hour(), 2, 10, QChar('0')).arg(dt.time().minute(), 2, 10, QChar('0'))) + "</h1>";
    webOut += tr("Object: <b>%1</b> (nodes: <b>%2</b>, finite elements: <b>%3</b>)<br>").arg(femObject->getObjectName().c_str()).arg(femObject->getMesh().getNumVertex()).arg(femObject->getMesh().getNumFE());
    webOut += tr("FE type: <b>%1</b><br>").arg(femObject->getMesh().feName().c_str());

    // Вывод метода аппроксимации по времени (в динамике)
    if (femObject->getParams().fType == DynamicProblem)
        webOut += (((femObject->getParams().tMethod == Wilson) ? tr("Method for approximating the time: the method of Wilson") : tr("Method for approximating the time: the method Zinkevych")) + "<br>");

    // Вывод рез-тов по каждой функции
    webOut += ("<h2>" + tr("Results of calculation:") + "</h2>");
    webOut += QString("<table border=\"1\" cellpadding=\"4\" cellspacing=\"0\"><tr><th>%1</th><th>%2</th><th>%3</th></tr>").arg(tr("Function")).arg(tr("Min")).arg("Max");
    if (femObject->getParams().fType == StaticProblem)
        for (unsigned i = 0; i < femObject->getResult().size(); i++)
            webOut += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(femObject->getResult()[i].getName().c_str()).arg(femObject->getResult()[i].min(),int(femObject->getParams().width),'e',int(femObject->getParams().precision)).arg(femObject->getResult()[i].max(), int(femObject->getParams().width),'e',int(femObject->getParams().precision));
    else
    {
        tm = femObject->getResult()[0].getName().substr(femObject->getResult()[0].getName().find('(') + 1,femObject->getResult()[0].getName().find(')') - femObject->getResult()[0].getName().find('(') - 1).c_str();
        webOut += "<tr><td colspan=\"3\"><center><b>" + QString(femObject->getParams().names[3].c_str()) + "=" + tm + "</b></center></td></tr>";
        for (unsigned i = 0; i < femObject->getResult().size(); i++)
        {
            if (tm == QString(femObject->getResult()[i].getName().substr(femObject->getResult()[i].getName().find('(') + 1,femObject->getResult()[i].getName().find(')') - femObject->getResult()[i].getName().find('(') - 1).c_str()))
                webOut += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(femObject->getResult()[i].getName().substr(femObject->getResult()[i].getName().find('>') + 1,femObject->getResult()[i].getName().find("(") - femObject->getResult()[i].getName().find('>') - 1).c_str()).arg(femObject->getResult()[i].min(),int(femObject->getParams().width),'e',int(femObject->getParams().precision)).arg(femObject->getResult()[i].max(),int(femObject->getParams().width),'e',int(femObject->getParams().precision));
            else
            {
                tm = femObject->getResult()[i].getName().substr(femObject->getResult()[i].getName().find('(') + 1,femObject->getResult()[i].getName().find(')') - femObject->getResult()[i].getName().find('(') - 1).c_str();
                webOut += "<tr><td colspan=\"3\"><center><b>" + QString(femObject->getParams().names[3].c_str()) + "=" + tm + "</b></center></td></tr>";
                i--;
            }
        }
    }
    webOut += "</table><br>";

    if (femObject->getNotes().size())
    {
        webOut += "<h2>" + tr("Problem notes:") + "</h2>";
        // Вывод примечаний к расчету
        for (auto it: femObject->getNotes())
            webOut += tr(it.c_str()) + "<br>";
    }

    // Вывод параметров расчета задачи для текущего объекта
    sayParams(webOut);

    // Добавление закладки с результатами
    for (int i = 0; i < tabWidget->count(); i++)
        if (qobject_cast<QTextEdit*>(tabWidget->widget(i)))
        {
            isFind = true;
            qobject_cast<QTextEdit*>(tabWidget->widget(i))->setText(webOut);
        }
    if (!isFind)
    {
        QTextEdit* ed = new QTextEdit(webOut);

        ed->setReadOnly(true);
        tabWidget->addTab(ed,tr("Results"));
//        qobject_cast<QTextEdit*>(tabWidget->widget(tabWidget->count() - 1))->setText(webOut);
    }
    // Сохраняем протокол расчета задачи
    if (isAutoSaveProtocol)
    {
        QFile file(fileName);
        QTextStream out(&file);

        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::critical(this, tr("Error"), tr("Error opening file %1").arg(fileName));
            return;
        }
        out.setDevice(&file);
        out << webOut;
        file.close();
    }
}

// Вывод параметров расчета задачи
void TMainWindow::sayParams(QString& webOut)
{
    TFEMObject* femObject = femProcessor->getFEMObject();
    unsigned num;
    QString predicate;

    // --------------- базовые параметры ---------------
    webOut += "<h2>" + tr("Problem parameters:") + "</h2>";
    webOut += ((femObject->getParams().fType == StaticProblem) ? tr("Solution method: <b>%1</b>").arg(tr("static (Lagrange)")) : tr("Solution method: <b>%1</b>").arg(tr("dynamic (Ostrogradsky)"))) + "<br>";

    webOut += tr("Computational accuracy: <b>%1</b>").arg(femObject->getParams().eps, int(femObject->getParams().width), 'e', int(femObject->getParams().precision)) + "<br><br>";
    webOut += tr("Elastic characteristics");
    sayParam(webOut, tr("Young's modulus"), YOUNG_MODULUS_PARAMETER, false);
    sayParam(webOut, tr("Poisson's ratio"), POISSON_RATIO_PARAMETER, false);

    if (femObject->getParams().fType == DynamicProblem)
    {
        sayParam(webOut, tr("Density"), DENSITY_PARAMETER, false);
        sayParam(webOut, tr("Damping parameter"), DAMPING_PARAMETER, false);
    }
    // Упруго-пластические параметры задачи
    if (femObject->getParams().pMethod != Linear)
    {
        webOut += "<br>" + ((femObject->getParams().pMethod == MVS) ? tr("Method of elastic-plastic analysis: <b>%1</b>").arg(tr("method of variable stiffness")) : tr("Method of elastic-plastic analysis: <b>%1</b>").arg(tr("method of elastic solutions Ilyushin"))) + "<br>";
        webOut += tr("Stress-strain curve:");

        for (auto it: femObject->getParams().plist)
            if (it.getType() == STRESS_STRAIN_CURVE_PARAMETER)
            {
                webOut += "<br>" + QString("<b>%1</b>: %2").arg(tr("Predicate")).arg(it.getPredicate().c_str()) + "<br>";
                webOut += QString("<table border=\"1\" cellpadding=\"4\" cellspacing=\"0\"><tr><th>%1</th><th>%2</th><th>%3</th></tr>").arg(tr("N")).arg(tr("Stress")).arg(tr("Deformation"));
                for (unsigned i = 0; i < it.getStressStrainCurve().size1(); i++)
                    webOut += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(i + 1).arg(it.getStressStrainCurve(i, 0), int(femObject->getParams().width), 'e', int(femObject->getParams().precision)).arg(it.getStressStrainCurve(i, 1), int(femObject->getParams().width), 'e', int(femObject->getParams().precision));
                webOut += "</table><br>";

            }
        webOut += "<br>" + tr("Load step");
        webOut += QString("<b>%1</b><br>").arg(femObject->getParams().loadStep, int(femObject->getParams().width),'e', int(femObject->getParams().precision));
    }

    // Толщина КЭ
    if (femObject->getMesh().isPlate() || femObject->getMesh().isShell() || femObject->getMesh().is1D() || femObject->getMesh().is2D())
        sayParam(webOut, tr("FE thickness"), THICKNESS_PARAMETER, false);


    // --------------- Термоупругие параметры ---------------
    sayParam(webOut, tr("Thermal expansion"), ALPHA_PARAMETER, false);
    sayParam(webOut, tr("Temperatire"), TEMPERATURE_PARAMETER, false);

    // --------------- Краевые условия задачи ---------------
    sayParam(webOut, tr("Boundary condition"), BOUNDARY_CONDITION_PARAMETER);

    // --------------- Объемная нагрузки ---------------
    sayParam(webOut, tr("Volume load"), VOLUME_LOAD_PARAMETER);

    // --------------- Поверхностная нагрузки ---------------
    sayParam(webOut, tr("Surface load"), SURFACE_LOAD_PARAMETER);

    // --------------- Сосредоточенная нагрузки ---------------
    sayParam(webOut, tr("Concentrated load"), CONCENTRATED_LOAD_PARAMETER);

    // --------------- Давление ---------------
    sayParam(webOut, tr("Pressure load"), PRESSURE_LOAD_PARAMETER, false);

    // --------------- Начальные и другие условия, зависящие от времени ---------------
    if (femObject->getParams().fType == DynamicProblem)
    {
        webOut += tr("time parameters - initial time: <b>%1</b>, the final moment: <b>%2</b>, step: <b>%3</b>").arg(femObject->getParams().t0).arg(femObject->getParams().t1).arg(femObject->getParams().th) + "<br>";
        num = 0;

        for (auto it: femObject->getParams().plist)
            if (it.getType() == INITIAL_CONDITION_PARAMETER)
            {
                num++;
                break;
            }
        if (num)
        {
            webOut += "<br>" + tr("initial conditions:") + "<br>";
            for (auto it: femObject->getParams().plist)
                if (it.getType() == INITIAL_CONDITION_PARAMETER)
                {
                    if ((it.getDirect() & FUN_U) == FUN_U)
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[4].c_str()).arg(it.getExpression().c_str()) + "<br>";
                    if ((it.getDirect() & FUN_V) == FUN_V)
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[5].c_str()).arg(it.getExpression().c_str()) + "<br>";
                    if ((it.getDirect() & FUN_W) == FUN_W)
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[6].c_str()).arg(it.getExpression().c_str()) + "<br>";
                    if ((it.getDirect() & FUN_UT) == FUN_UT)
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[19].c_str()).arg(it.getExpression().c_str()) + "<br>";
                    if ((it.getDirect() & FUN_VT) == FUN_VT)
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[20].c_str()).arg(it.getExpression().c_str()) + "<br>";
                    if ((it.getDirect() & FUN_WT) == FUN_WT)
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[21].c_str()).arg(it.getExpression().c_str()) + "<br>";
                    if ((it.getDirect() & FUN_UTT) == FUN_UTT)
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[22].c_str()).arg(it.getExpression().c_str()) + "<br>";
                    if ((it.getDirect() & FUN_VTT) == FUN_VTT)
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[23].c_str()).arg(it.getExpression().c_str()) + "<br>";
                    if ((it.getDirect() & FUN_WTT) == FUN_WTT)
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[24].c_str()).arg(it.getExpression().c_str()) + "<br>";
                }
        }
    }
    // --------------- Вспомогательные параметры ---------------
    if (femObject->getParams().variables.size())
    {
        webOut += "<br>" + tr("Variables:") + "<br>";
        for (auto it: femObject->getParams().variables)
            webOut += tr("<b>%1</b> = <b>%2</b>").arg(it.first.c_str()).arg(it.second) + "<br>";
    }
    webOut += "</p>";
}

// Вывод информации о конкретном парметре расчета в табличной форме
void TMainWindow::sayParam(QString& webOut, const QString& title, int param, bool isDirect)
{
    unsigned num = 0;
    QString predicate;
    TFEMObject* femObject = femProcessor->getFEMObject();

    for (auto it: femObject->getParams().plist)
        if (it.getType() == param)
        {
            num++;
            break;
        }
    if (num)
    {
        webOut += "<br>" + tr(title.toUtf8());
        num = 1;
        webOut += QString("<table border=\"1\" cellpadding=\"4\" cellspacing=\"0\"><tr><th>%1</th><th>%2</th><th>%3</th>").arg(tr("N")).arg(tr("Value")).arg(tr("Predicate"));
        if (isDirect)
        {
            webOut += QString("<th>%1</th>").arg(femObject->getParams().names[0].c_str());
            if (femObject->getMesh().getDimension() > 1)
                webOut += QString("<th>%1</th>").arg(femObject->getParams().names[1].c_str());
            if (femObject->getMesh().getDimension() > 2)
                webOut += QString("<th>%1</th>").arg(femObject->getParams().names[2].c_str());
            webOut += "</tr>";
        }
        for (auto it: femObject->getParams().plist)
        {
            if (it.getType() != param)
                continue;
            predicate = QString(it.getPredicate().c_str()).replace(QString("<="), QString("&lt;=")).replace(QString(">="), QString("&gt;="));
            webOut += QString("<tr><td>%1</td><td>%2</td><td>%3</td>").arg(num++).arg(it.getExpression().c_str()).arg(predicate);
            if (isDirect)
            {
                if ((it.getDirect() & DIR_X) == DIR_X)
                    webOut += QString("<td>%1</td>").arg("+");
                else
                    webOut += QString("<td>%1</td>").arg("");
                if ((it.getDirect() & DIR_Y) == DIR_Y)
                    webOut += QString("<td>%1</td>").arg("+");
                else if (femObject->getMesh().getFreedom() > 1)
                    webOut += QString("<td>%1</td>").arg("");
                if ((it.getDirect() & DIR_Z) == DIR_Z)
                    webOut += QString("<td>%1</td>").arg("+");
                else if (femObject->getMesh().getFreedom() > 2)
                    webOut += QString("<td>%1</td>").arg("");
            }
            webOut += "</tr>";
        }
        webOut += "</table><br>";
    }
}

void TMainWindow::slotAnalyseFunction(void)
{
    TFunListDialog* dlg = new TFunListDialog(this);

    dlg->setup(femProcessor->getFEMObject());
    dlg->changeLanguage();
    if (dlg->exec() == QDialog::Accepted)
        addFuncToAnalyse(dlg->getName(), "");
    delete dlg;
}

bool TMainWindow::saveQFPF(QString fileName)
{
    QDateTime dt;
    QFile file;
    QString val;
    QJsonDocument doc;
    QJsonObject main,
                header,
                mesh,
                params,
                out,
                time,
                thermal,
                nonlin,
                geometrical,
                physical;
    QJsonArray names,
               lbc,
               variables;
    TFEMObject* femObject = femProcessor->getFEMObject();

    // ------------- Заголовок -----------------
    dt.setTime_t(femObject->getResult().getSolutionTime());
    header.insert("Title", QJsonValue::fromVariant("QFEM problem file"));
    header.insert("Object", QJsonValue::fromVariant(femObject->getObjectName().c_str()));
    header.insert("DateTime", QJsonValue::fromVariant(dt.currentDateTime()));

    // ---------------- Сетка ------------------
    mesh.insert("File", QJsonValue::fromVariant(QString("%1").arg(femObject->getFileName().c_str())));
    main.insert("Mesh", mesh);
    main.insert("Header", header);

    // ------------- Параметры расчета ---------------------
    saveParam(main);

    // Формирование итогового документа
    doc.setObject(main);

    // Запись в файл
    file.setFileName(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
           return false;
    file.write(doc.toJson(), doc.toJson().length());
    file.close();
    return true;
}

bool TMainWindow::loadQFPF(QString fileName)
{
    QString jsonText,
            meshFile,
            fn;
    QFileInfo fi;
    QStringList lst;
    QFile file;
    QJsonDocument doc;
    QJsonObject obj;
    QJsonArray bc,
               arr;
    TFEMParams& params = femProcessor->getFEMObject()->getParams();
    int dir;
    string expr,
           pred;
    int type;
    matrix<double> ssc;


    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    jsonText = file.readAll();
    file.close();

    doc = QJsonDocument::fromJson(jsonText.toUtf8());
    obj = doc.object();

    // Проверяем заголовок
    if (obj.value(QString("Header"))["Title"] != "QFEM problem file")
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("Wrong format file %1").arg(fileName));
        return false;
    }

    // Сетка
    meshFile = obj.value(QString("Mesh"))["File"].toVariant().toString();

    // Проверяем наличие файла сетки
    fn = meshFile;
    fi.setFile(fn);
    if (!fi.exists())
    {
        // Такого файла нет
        if ((fn = fi.fileName()) != meshFile)
        {
            // В случае, если задан полный путь, выводим сообщение об ошибке
            QMessageBox::critical(this, tr("Error"), tr("Error reading file %1").arg(meshFile));
            return false;
        }
        // Пытаемся открыть файл в той же папке, где лежит файл fileName
        fi.setFile(fileName);
        meshFile = fi.canonicalPath() + "/" + fn;
    }


    // Загрузка сетки
    if (!loadMesh(meshFile))
        return false;


    // Базовые параметры
    params.fType = (obj.value(QString("Parameters"))["ProblemType"].toVariant().toString() == "Static") ? StaticProblem : DynamicProblem;
    params.eps = obj.value(QString("Parameters"))["Accuracy"].toVariant().toDouble();

    // Динамические параметры
    params.theta = obj.value(QString("Parameters"))["DynamicParameters"]["WilsonTheta"].toVariant().toDouble();
    params.t0 = obj.value(QString("Parameters"))["DynamicParameters"]["T0"].toVariant().toDouble();
    params.t1 = obj.value(QString("Parameters"))["DynamicParameters"]["T1"].toVariant().toDouble();
    params.th = obj.value(QString("Parameters"))["DynamicParameters"]["TH"].toVariant().toDouble();

    // Параметры вывода
    params.width = obj.value(QString("Parameters"))["OutputParameters"]["Width"].toVariant().toInt();
    params.precision = obj.value(QString("Parameters"))["OutputParameters"]["Precision"].toVariant().toInt();

    // Параметры нелинейного расчета
    params.loadStep = obj.value(QString("Parameters"))["Nonlinearity"]["LoadStep"].toVariant().toDouble();
    params.pMethod = PlasticityMethod(obj.value(QString("Parameters"))["Nonlinearity"]["CalculationMethod"].toVariant().toInt());

    // Краевые условия и прочие параметры
    bc = obj.value(QString("Parameters"))["BoundaryConditions"].toVariant().toJsonArray();
    if (bc.size())
        for (int i = 0; i < bc.size(); i++)
        {
            dir = bc.at(i)["Direct"].toVariant().toInt();
            expr = bc.at(i)["Expression"].toVariant().toString().toStdString();
            pred = bc.at(i)["Predicate"].toVariant().toString().toStdString();
            type = bc.at(i)["Type"].toVariant().toInt();
            if (type == STRESS_STRAIN_CURVE_PARAMETER)
            {
                if (!pForm->decodeStressStarinCurve(expr, ssc))
                    return false;
                params.plist.addStressStrainCurve(ssc, pred);
            }
            else
                params.plist.addParameter(type, expr, pred, dir);
        }

    // Названия
    bc = obj.value(QString("Parameters"))["Names"].toVariant().toJsonArray();
    for (unsigned i = 0; i < params.stdNames().size(); i++)
        params.names[i] = bc.at(int(i)).toVariant().toString().toStdString();

    // Вспомогательные переменные
    bc = obj.value(QString("Parameters"))["Variables"].toVariant().toJsonArray();
    if (bc.size())
        for (int i = 0; i < bc.size(); i++)
        {
            lst = bc.at(i).toVariant().toString().split(' ');
            params.variables[lst.at(0).toStdString()] = lst.at(1).toDouble();
        }
    return true;
}

void TMainWindow::addFuncToAnalyse(QString funName, QString expression)
{
    TFEMObject* femObject = femProcessor->getFEMObject();
    bool isFind = false;
    unsigned ind_f = unsigned(femObject->getResult().index(funName.toStdString())),
             ind_d = getTimeDeltaIndex(funName);

    // Проверка наличия такой функции в уже открытых закладках
    for (int i = 0; i < tabWidget->count(); i++)
        if (tabWidget->tabText(i).replace("&", "") == funName)
        {
            isFind = true;
            tabWidget->setCurrentIndex(i);
        }
    if (!isFind)
    {
        tabWidget->addTab(new TGLFunction(&femObject->getMesh(), &femObject->getResult(ind_f).getResults(), &femObject->getResult(ind_d + 0).getResults(), &femObject->getResult(ind_d + 1).getResults(), &femObject->getResult(ind_d + 2).getResults(), expression, this), funName);
        tabWidget->setCurrentIndex(tabWidget->count() - 1);
    }
}

// Определение индекса компоненты вектора перемещений (U) в общем списке функция для соответствующего момента времени
unsigned TMainWindow::getTimeDeltaIndex(QString funName)
{
    int pos1,
        pos2;
    QString t,
            tU;
    int ret = 0;

    QApplication::setOverrideCursor(Qt::BusyCursor);
    if (femProcessor->getFEMObject()->getParams().fType == DynamicProblem)
        if ((pos1 = funName.lastIndexOf("(")) != -1)
        {
            pos2 = funName.lastIndexOf(")");
            t = funName.mid(pos1, pos2 - pos1 + 1);
            tU = QString(femProcessor->getFEMObject()->getParams().names[4].c_str()) + t;
            ret = femProcessor->getFEMObject()->getResult().index(tU.toStdString());
        }
    QApplication::restoreOverrideCursor();
    return unsigned(ret);
}

void TMainWindow::slotSetupImageParams(void)
{
    TFEMObject* femObject = femProcessor->getFEMObject();
    int type = (qobject_cast<TGLFunction*>(tabWidget->currentWidget())) ? ( (qobject_cast<TGLParameter*>(tabWidget->currentWidget())) ? V_PARAM : V_FUNC ) : V_MESH;

    if (qobject_cast<TGLParameter*>(tabWidget->currentWidget()))
        if (qobject_cast<TGLParameter*>(tabWidget->currentWidget())->getType() == VOLUME_LOAD_PARAMETER || qobject_cast<TGLParameter*>(tabWidget->currentWidget())->getType() == SURFACE_LOAD_PARAMETER ||
            qobject_cast<TGLParameter*>(tabWidget->currentWidget())->getType() == CONCENTRATED_LOAD_PARAMETER || qobject_cast<TGLParameter*>(tabWidget->currentWidget())->getType() == PRESSURE_LOAD_PARAMETER ||
            qobject_cast<TGLParameter*>(tabWidget->currentWidget())->getType() == BOUNDARY_CONDITION_PARAMETER)
            type = V_MESH;
    if (qobject_cast<TGLMesh*>(tabWidget->currentWidget()))
    {
        iDlg->changeLanguage();
        iDlg->setImageParams(qobject_cast<TGLMesh*>(tabWidget->currentWidget())->getImageParams(),int(femObject->getMesh().getFreedom()), type);
        if (iDlg->exec() == QDialog::Accepted)
            if (qobject_cast<TGLMesh*>(tabWidget->currentWidget()))
            {
                qobject_cast<TGLMesh*>(tabWidget->currentWidget())->setImageParams(iDlg->getImageParams());
                qobject_cast<TGLMesh*>(tabWidget->currentWidget())->repaint();
            }
    }
}

void TMainWindow::repaintResults(void)
{
    QString name,
            exp;
    TGLFunction* ptr;
    bool isCalc = femProcessor->getFEMObject()->isCalculated();

    for (int i = 1; i < tabWidget->count(); i++)
        if ((ptr = qobject_cast<TGLFunction*>(tabWidget->widget(i))) != nullptr)
        {
            if (isCalc)
            {
                if ((exp = ptr->getExpression()).length())
                    calcExpression(exp, name);
                ptr->repaint();
            }
            else
            {
                delete tabWidget->widget(i);
                tabWidget->removeTab(i);
                i--;
            }
        }
}

// Запись результатов расчета в JSON
void TMainWindow::saveResults(QJsonObject &main)
{
    TResultList &result = femProcessor->getFEMObject()->getResult();
    QJsonObject res;
    QJsonArray resultObj;

    for (unsigned i = 0; i < result.size(); i++)
    {
        QJsonArray arr;

        res.insert("Function", result[i].getName().c_str());
        res.insert("Time", result[i].getTime());
        for (unsigned j = 0; j < result[i].getResults().size(); j++)
            arr.push_back(result[i].getResults(j));
        res.insert("Values", arr);
        resultObj.push_back(res);
    }
    main.insert("Results", resultObj);
}
// Запись параметров расчета в JSON
void TMainWindow::saveParam(QJsonObject &main)
{
    QString val;
    QJsonObject paramObj,
                out,
                time,
                thermal,
                nonlin,
                geometrical,
                physical;
    QJsonArray names,
               lbc,
               variables;
    TFEMParams &params = femProcessor->getFEMObject()->getParams();
    
    // Тип задачи
    paramObj.insert("ProblemType", (params.fType == StaticProblem) ? "Static" : "Dynamic");

    // Точность вычислений
    paramObj.insert("Accuracy", params.eps);

    // Параметры для динамического расчета
    time.insert("T0", params.t0);
    time.insert("T1", params.t1);
    time.insert("TH", params.th);
    time.insert("WilsonTheta", params.theta);
    paramObj.insert("DynamicParameters", time);

    // Параметры вывода
    out.insert("Width", params.width);
    out.insert("Precision", params.precision);
    paramObj.insert("OutputParameters", out);

    // Параметры нелинейного расчета
    nonlin.insert("CalculationMethod", params.pMethod);
    nonlin.insert("LoadStep", params.loadStep);
    paramObj.insert("Nonlinearity", nonlin);

    // Краевые условия и прочие параметры
    for (auto it: params.plist)
    {
        QJsonObject bc;

        bc.insert("Type", it.getType());
        bc.insert("Direct", it.getDirect());
        bc.insert("Predicate", it.getPredicate().c_str());
        if (it.getType() == STRESS_STRAIN_CURVE_PARAMETER)
        {
            val = "{";
            for (unsigned i = 0; i < it.getStressStrainCurve().size1(); i++)
                val += QString("{%1, %2}%3 ").arg(it.getStressStrainCurve(i, 0)).arg(it.getStressStrainCurve(i, 1)).arg((i < it.getStressStrainCurve().size1() - 1) ? "," : "");
            val += "}";
            bc.insert("Expression", val);
        }
        else
            bc.insert("Expression", it.getExpression().c_str());

        lbc.push_back(bc);
    }
    paramObj.insert("BoundaryConditions", lbc);

    // Названия
    for (unsigned i = 0; i < params.stdNames().size(); i++)
        names.push_back(params.names[i].c_str());
    paramObj.insert("Names", names);

    // Вспомагательные параметры (для парсера)
    for (auto it: params.variables)
        variables.push_back(QString("%1 %2").arg(it.first.c_str()).arg(it.second));
    paramObj.insert("Variables", variables);

    main.insert("Parameters", paramObj);
}

// Запись сетки в JSON-объект
void TMainWindow::saveMesh(QJsonObject &main)
{
    QJsonObject meshObj;
    QJsonArray x,
               fe,
               be;
    TMesh &mesh = femProcessor->getFEMObject()->getMesh();
    
    // ---------------- Сетка ------------------
    meshObj.insert("FEType", QJsonValue::fromVariant(mesh.getTypeFE()));
    for (unsigned i = 0; i < mesh.getNumVertex(); i++)
    {
        QJsonArray arr;

        for (unsigned j = 0; j < mesh.getDimension(); j++)
            arr.push_back(mesh.getX(i, j));
        x.push_back(arr);
    }
    meshObj.insert("Coordinates", x);
    for (unsigned i = 0; i < mesh.getNumFE(); i++)
    {
        QJsonArray arr;

        for (unsigned j = 0; j < mesh.getSizeFE(); j++)
            arr.push_back(QJsonValue::fromVariant(mesh.getFE(i, j)));
        fe.push_back(arr);
    }
    meshObj.insert("FE", fe);
    for (unsigned i = 0; i < mesh.getNumBE(); i++)
    {
        QJsonArray arr;

        for (unsigned j = 0; j < mesh.getSizeBE(); j++)
            arr.push_back(QJsonValue::fromVariant(mesh.getBE(i, j)));
        be.push_back(arr);
    }
    meshObj.insert("BE", be);
    
    main.insert("Mesh", meshObj);
}

// Запись примечаний к расчету в JSON-объект
void TMainWindow::saveNotes(QJsonObject &main)
{
    list<string> &notes = femProcessor->getFEMObject()->getNotes();
    QJsonArray str;

    for (auto it: notes)
        str.push_back(QString("%1").arg(it.c_str()));
    main.insert("Notes", str);
}

bool TMainWindow::saveQRES(QString fileName)
{
    QFile file;
    QDateTime dt;
    QJsonDocument doc;
    QJsonObject main,
                header;

    // ------------- Заголовок -----------------
    dt.setTime_t(femProcessor->getFEMObject()->getResult().getSolutionTime());
    header.insert("Title", "QFEM results file");
    header.insert("Object", femProcessor->getFEMObject()->getObjectName().c_str());
    header.insert("DateTime", dt.currentDateTime().toString());
    main.insert("Header", header);

    // ---------------- Сетка ------------------
    saveMesh(main);
    // ---------------- Параметры ----------------
    saveParam(main);
    // ---------------- Результаты расчета ----------------
    saveResults(main);
    // ---------------- Примечания к расчету ----------------
    saveNotes(main);

    // Формирование итогового документа
    doc.setObject(main);

    // Запись в файл
    file.setFileName(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    file.write(doc.toJson(), doc.toJson().length());
    file.close();
    return true;
}

// Чтение сетки из JSON-объекта
void TMainWindow::loadMesh(const QJsonObject &meshObj)
{
    QJsonArray ja_x = meshObj["Coordinates"].toArray(),
               ja_fe = meshObj["FE"].toArray(),
               ja_be = meshObj["BE"].toArray();
    unsigned fe_type = unsigned(meshObj["FEType"].toInt()),
             fe_dim,
             fe_size,
             be_size;
    matrix<double> x;
    matrix<unsigned> fe,
                     be;
    TMesh &mesh = femProcessor->getFEMObject()->getMesh();

    mesh.getDataFE(mesh.convertFEType(FEType(fe_type)), fe_size, be_size, fe_dim);
    x.resize(ja_x.size(), fe_dim);
    for (unsigned i = 0; i < unsigned(ja_x.size()); i++)
    {
        QJsonArray arr = ja_x[i].toArray();

        for (unsigned j = 0; j < fe_dim; j++)
            x[i][j] = arr[j].toDouble();
    }
    fe.resize(ja_fe.size(), fe_size);
    for (unsigned i = 0; i < unsigned(ja_fe.size()); i++)
    {
        QJsonArray arr = ja_fe[i].toArray();

        for (unsigned j = 0; j < fe_size; j++)
            fe[i][j] = unsigned(arr[j].toInt());
    }
    if (fe_type == FE3D3S || fe_type == FE3D4S || fe_type == FE3D6S)
        be = fe;
    else
    {
        be.resize(ja_be.size(), be_size);
        for (unsigned i = 0; i < unsigned(ja_be.size()); i++)
        {
            QJsonArray arr = ja_be[i].toArray();

            for (unsigned j = 0; j < be_size; j++)
                be[i][j] = unsigned(arr[j].toInt());
        }
    }
    mesh.setMesh(FEType(fe_type), x, fe, be);
}

// Чтение параметров из JSON-объекта
void TMainWindow::loadParam(const QJsonObject &paramObj)
{
    QJsonArray lbc = paramObj["BoundaryConditions"].toArray(),
               nm = paramObj["Names"].toArray(),
               var = paramObj["Variables"].toArray();
    TFEMParams &params = femProcessor->getFEMObject()->getParams();

    // Тип задачи
    params.fType = (paramObj["ProblemType"].toString() == "Static") ? StaticProblem : DynamicProblem;

    // Точность вычислений
    params.eps = paramObj["Accuracy"].toDouble();

    // Параметры для динамического расчета
    params.t0 = (paramObj["DynamicParameters"].toObject())["T0"].toDouble();
    params.t1 = (paramObj["DynamicParameters"].toObject())["T1"].toDouble();
    params.th = (paramObj["DynamicParameters"].toObject())["TH"].toDouble();
    params.theta = (paramObj["DynamicParameters"].toObject())["WilsonTheta"].toDouble();

    // Параметры вывода
    params.width = (paramObj["OutputParameters"].toObject())["Width"].toInt();
    params.precision = (paramObj["OutputParameters"].toObject())["Precision"].toInt();

    // Параметры нелинейного расчета
    params.pMethod = PlasticityMethod((paramObj["Nonlinearity"].toObject())["CalculationMethod"].toInt());
    params.loadStep = (paramObj["Nonlinearity"].toObject())["CalculationMethod"].toDouble();

    // Краевые условия и прочие параметры
    params.plist.clear();
    for (auto value: lbc)
    {
        int type = static_cast<QJsonValue>(value)["Type"].toInt(),
            direct = static_cast<QJsonValue>(value)["Direct"].toInt();
        QString predicate = static_cast<QJsonValue>(value)["Predicate"].toString(),
                expression = static_cast<QJsonValue>(value)["Expression"].toString();
        TParameter p(type, expression.toStdString(), predicate.toStdString(), direct);

        params.plist.push_back(p);
    }

    // Названия
    params.names.clear();
    for (auto value: nm)
        params.names.push_back(value.toString().toStdString());

    // Вспомагательные параметры (для парсера)
    params.variables.clear();
    for (auto value: var)
    {
        QStringList sl = value.toString().split(" ");

        params.variables.insert(pair<string, double>(sl[0].toStdString(), sl[1].toDouble()));
    }
}

// Чтение результатов из JSON-объекта
void TMainWindow::loadResults(const QJsonArray &resultArr)
{
    QJsonArray arr;
    QString name;
    double t;
    vector<double> res;
    TResultList &result = femProcessor->getFEMObject()->getResult();

    result.clear();
    for (auto value: resultArr)
    {
        res.clear();
        name = value["Function"].toString();
        t = value["Time"].toDouble();
        arr = value["Values"].toArray();
        for (auto v: arr)
            res.push_back(v.toDouble());
        result.setResult(res, name.toStdString(), t);
    }
}

// Чтение примечаний из JSON-объекта
void TMainWindow::loadNotes(const QJsonArray &notesArr)
{
    QJsonArray arr;
    QString str;
    list<string> &notes = femProcessor->getFEMObject()->getNotes();

    notes.clear();
    for (auto value: notesArr)
        notes.push_back(value.toString().toStdString());
}

bool TMainWindow::loadRES(QString fileName)
{
    bool ret;
    QString htmlFile = QFileInfo(fileName).absolutePath() + "/" +  QFileInfo(fileName).baseName() + ".html";

    QApplication::setOverrideCursor(Qt::BusyCursor);
    ret = femProcessor->getFEMObject()->loadResult(fileName.toStdString());
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    if (!ret)
    {
        QMessageBox::critical(this, tr("Error"), tr("Error opening file %1").arg(fileName));
        return false;
    }
    showProtocol(htmlFile);
    checkMenuState();
    return true;
}

bool TMainWindow::loadQRES(QString fileName)
{
    QString val,
            htmlFile = QFileInfo(fileName).absolutePath() + "/" +  QFileInfo(fileName).baseName() + ".html";
    QFile file;
    QJsonDocument doc;
    QJsonObject obj;

//    cout << endl;
    // Чтение из файла
    file.setFileName(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(this, tr("Error"), tr("Error opening file %1").arg(fileName));
        return false;
    }
    val = file.readAll();
    file.close();

    doc = QJsonDocument::fromJson(val.toUtf8());
    obj = doc.object();

    // Чтение сетки
    loadMesh(obj["Mesh"].toObject());
    // Чтение параметров
    loadParam(obj["Parameters"].toObject());
    // Чтение результатов
    loadResults(obj["Results"].toArray());
    // Чтение примечаний
    loadNotes(obj["Notes"].toArray());

    femProcessor->getFEMObject()->setProcessCalculated(true);
    showProtocol(htmlFile);
    checkMenuState();
    return true;
}

void TMainWindow::slotAddFunction(void)
{
    TVCDialog *vdlg = new TVCDialog(this);
    QString name;

    vdlg->changeLanguage();
    if (vdlg->exec() == QDialog::Accepted)
    {
        if (calcExpression(vdlg->getExpression(), name))
            addFuncToAnalyse(name, vdlg->getExpression());
    }
    delete vdlg;
}

bool TMainWindow::calcExpression(QString expression, QString& name)
{
    TFEMObject *femObject = femProcessor->getFEMObject();
    TParser parser;
    unsigned n = unsigned(femObject->getMesh().getNumVertex()),
             m = unsigned(femObject->getResult().size());
    vector<double> results(n);
    QString exp;

    // Выделяем имя новой переменной
    name = QString("%1").arg(expression.left(int(expression.toStdString().find("="))).trimmed());
    if (femObject->getResult().index(name.toStdString()) != -1)
    {
        QMessageBox::critical(this, tr("Error"), tr("This function is is already exists!"));
        return false;
    }
    // Выделяем выражение
    if (!expression.contains('='))
    {
        QMessageBox::critical(this, tr("Error"), tr("Invalid expression!"));
        return false;
    }
    exp = expression.mid(int(expression.toStdString().find("=") + 1)).trimmed();
    if (!exp.trimmed().length())
    {
        QMessageBox::critical(this, tr("Error"), tr("Invalid expression!"));
        return false;
    }
    // Парсим выражение
    parser.set_variables(femObject->getParams().variables);
    for (unsigned i = 0; i < n; i++)
    {
        for (unsigned j = 0; j < m; j++)
            parser.set_variable(femObject->getResult()[j].getName(),femObject->getResult()[j].getResults()[i]);
        for (unsigned j = 0; j < femObject->getMesh().getDimension(); j++)
            parser.set_variable(femObject->getParams().names[j],femObject->getMesh().getX(i, j));
        parser.set_expression(exp.toStdString().c_str());
        if (parser.get_error() != NO_ERR)
        {
            QMessageBox::critical(this, tr("Error"), tr("Invalid expression!"));
            return false;
        }
        results[i] = parser.run();
//        if ((results[i] = parser.run()) == 0.0)
//            return false;
    }
    femObject->getResult().addResult(reinterpret_cast<double*>(&(results[0])), unsigned(results.size()), name.toStdString());
    return true;
}


void TMainWindow::setupLanguage(void)
{
    QString translatorFileName = QLatin1String("qt_") + QLocale::system().name(),
            translatorFile;
    QTranslator* dlgTranslator = new QTranslator(qApp);

    // Локализация (стандартных диалогов, e.t.c, ...)
    if (dlgTranslator->load(translatorFileName, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        qApp->installTranslator(dlgTranslator);

    translator = new QTranslator(qApp);
    // Перевод на указанный язык
    switch (langNo)
    {
        case 0: // English
            translatorFile = "";
            break;
        case 1: // Russian
            translatorFile = "QFEM_RU";
    }

    if (translator->load(translatorFile))
    {
        qApp->installTranslator(translator);
        ui->retranslateUi(this);
        femProcessor->getFEMObject()->setLanguage(langNo);
    }
}

void TMainWindow::slotAppSettings(void)
{
    TAppSetupDialog* dlg = new TAppSetupDialog(langNo, numThread, isAutoSaveResults, isAutoScroll, isAutoSaveProtocol, this);

    dlg->changeLanguage();
    if (dlg->exec() != QDialog::Accepted)
        return;

    switch ((langNo = dlg->getLangNo()))
    {
        case 0: // English
            qApp->removeTranslator(translator);
            break;
        case 1: // Russian
            translator->load("QFEM_RU");
            qApp->installTranslator(translator);
            break;
    }
    ui->retranslateUi(this);
    femProcessor->getFEMObject()->setLanguage(langNo);
    isAutoScroll = dlg->getIsAutoScroll();
    isAutoSaveProtocol = dlg->getIsAutoSaveProtocol();
    isAutoSaveResults = dlg->getIsAutoSaveResults();
    numThread = dlg->getNumThread();
}

void TMainWindow::slotSaveResults(void)
{
//    QString fileName = QFileDialog::getSaveFileName(this, tr("Saving results"), QString(QFileInfo(curFile).absolutePath() + "/" + QFileInfo(curFile).baseName() + "." + QString("qres").toLower()), tr("QFEM result files (*.qres)"));
    QString fileName = QFileDialog::getSaveFileName(this, tr("Saving results"), QString(QFileInfo(curFile).absolutePath() + "/" + QFileInfo(curFile).baseName() + "." + QString("qres").toLower()), tr("QFEM result files (*.qres);; QFEM report files (*.txt)"));
    QFileInfo info(fileName);

    if (!fileName.isEmpty())
    {
        if (info.suffix().toUpper() == "TXT")
            femProcessor->getFEMObject()->printResult(fileName.toStdString());
        else if (info.suffix().toUpper() == "QRES")
            saveQRES(fileName);
    }
}

void TMainWindow::slotAboutProgramm(void)
{
    THelpDialog* hdlg = new THelpDialog(0,this);

    hdlg->exec();
}

void TMainWindow::slotHelpProgramm(void)
{
    THelpDialog* hdlg = new THelpDialog(1,this);

    hdlg->exec();
}

void TMainWindow::slotDataCopy(void)
{
    QClipboard *clipboard = QApplication::clipboard();
    QTextCursor cursor;

    if (qobject_cast<TGLMesh*>(tabWidget->currentWidget()))
    {
        qobject_cast<TGLMesh*>(tabWidget->currentWidget())->loadPaint();
        clipboard->setImage(qobject_cast<TGLMesh*>(tabWidget->currentWidget())->grabFrameBuffer(true));
    }
    else
    {
        cursor = qobject_cast<QTextEdit*>(tabWidget->currentWidget())->textCursor();
        qobject_cast<QTextEdit*>(tabWidget->currentWidget())->selectAll();
        qobject_cast<QTextEdit*>(tabWidget->currentWidget())->copy();
        cursor.clearSelection();
        qobject_cast<QTextEdit*>(tabWidget->currentWidget())->setTextCursor(cursor);
    }

}

void TMainWindow::slotShowParam(int type)
{
    bool isFind = false;
    QTextCursor textCursor = terminal->textCursor(),
                saveCursor = textCursor;
    QString tabName = paramName(type).c_str();

    textCursor.clearSelection();
    terminal->setTextCursor(textCursor);

    pb->reset();
    pb->show();
    ui->actionStart->setEnabled(false);
    ui->actionStop->setEnabled(true);
    bcProcessor->setType(type);
    bcProcessor->moveToThread(thread);
    connect(thread, SIGNAL(started()), bcProcessor, SLOT(start()));
    connect(bcProcessor, SIGNAL(finished()), thread, SLOT(terminate()));
    thread->start();
    while (thread->isRunning())
        QApplication::processEvents();
    disconnect(thread, SIGNAL(started()), bcProcessor, SLOT(start()));
    disconnect(bcProcessor, SIGNAL(finished()), thread, SLOT(terminate()));
    ui->actionStart->setEnabled(true);
    ui->actionStop->setEnabled(false);
    pb->hide();
    terminal->setTextCursor(saveCursor);

    // Отображаем параметр
    // Проверка наличия такой функции в уже открытых закладках
    for (int i = 0; i < tabWidget->count(); i++)
        if (tabWidget->tabText(i).replace("&", "") == tabName)
        {
            isFind = true;
            qobject_cast<TGLParameter*>(tabWidget->widget(i))->redraw(bcProcessor->getVertex());
            tabWidget->setCurrentIndex(i);
        }
    if (!isFind)
    {
        tabWidget->addTab(new TGLParameter(&femProcessor->getFEMObject()->getMesh(), bcProcessor->getVertex(), type, this), tabName);
        tabWidget->setCurrentIndex(tabWidget->count() - 1);
    }

}
