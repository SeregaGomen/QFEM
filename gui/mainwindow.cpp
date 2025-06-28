#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QClipboard>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QSettings>
#include <QCloseEvent>
#include <QDockWidget>
#include <QProgressBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <ctime>

#include "problemsetupform.h"
#include "appesetupdialog.h"
#include "vcdialog.h"
#include "mainwindow.h"
#include "setupimagedialog.h"
#include "fldialog.h"
#include "paramview.h"
#include "gbdialog.h"
#include "qmsg.h"
#include "helpdialog.h"
#include "femprocessor.h"
#include "bcprocessor.h"
#include "terminal.h"
#include "ui_mainwindow.h"
#include "parser/parser.h"
#include "object/object.h"

#include "meshview.h"
#include "funcview.h"
#include "imageparams.h"

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
    delete ui;
}

void TMainWindow::init(void)
{
////    QTextCodec* codec =  QTextCodec::codecForName("UTF-8");
////    QTextCodec::setCodecForLocale(codec);
//    auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());

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
    connect(dock, SIGNAL(visibilityChanged(bool)), this, SLOT(slotCloseTerminal()));

    //////////////////////
    // Перехват cout и cerr
    myCout = new QStdRedirector<>(std::cout, this);
//    connect(myCout, SIGNAL(messageChanged(QString)), terminal, SLOT(insertPlainText(QString)));
    connect(myCout, SIGNAL(messageChanged(QString)), this, SLOT(slotMsg(QString)));
//    if (langCode == 1)
//        cout.imbue(std::locale("ru_RU.utf8"));
//    cout << put_time(localtime(&now), "%d-%m-%Y %X") << endl;
    cout << QDateTime::currentDateTime().toString("d-MM-yyyy hh:mm:ss").toStdString() << endl;

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

    connect(ui->actionRotate, SIGNAL(triggered(bool)), this, SLOT(slotSetRotate()));



    connect(iDlg, SIGNAL(sendAutoRotateState(bool)), this, SLOT(slotSetRotate(bool)));
    connect(iDlg, SIGNAL(sendShowMesh()), this, SLOT(slotShowMesh()));
    connect(iDlg, SIGNAL(sendShowSurface()), this, SLOT(slotShowSurface()));
    connect(iDlg, SIGNAL(sendShowSurfaceAndMesh()), this, SLOT(slotShowSurfaceAndMesh()));




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
    connect(dynamic_cast<QObject*>(msg), SIGNAL(signalSetFormat(QString)), pb, SLOT(slotSetFormat(QString)));



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
    //bool isEnabled = (qobject_cast<TGLMesh*>(tabWidget->widget(nTab)) == nullptr) ? false : true;
    bool isEnabled = (qobject_cast<TMeshView*>(tabWidget->widget(nTab)) == nullptr) ? false : true;

    ui->actionInfo->setEnabled(isEnabled);
    ui->actionRotate->setEnabled(isEnabled);
    ui->actionMesh->setEnabled(isEnabled);
    ui->actionSurface->setEnabled(isEnabled);
    ui->actionMesh->setEnabled(isEnabled);
    ui->actionSurfaceAndMesh->setEnabled(isEnabled);
    ui->actionRestore->setEnabled(isEnabled);
//    ui->actionAnalyse->setEnabled(isEnabled);
    ui->actionAnalyse->setEnabled(femProcessor->isCalculated());
    ui->actionSetupImage->setEnabled(isEnabled);
    if (/*tabWidget->currentIndex() == 0 and*/ !isUntitled)
        ui->actionObjectParameters->setEnabled(true);
    else
        ui->actionObjectParameters->setEnabled(false);
}

void TMainWindow::slotCloseTab(int nTab)
{
    if (!tabWidget->count())
        return;


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
        int i = tabWidget->count() - 1;
        while (tabWidget->count() > 0)
        {
            //if (qobject_cast<TGLMesh*>(tabWidget->widget(i)))
            if (qobject_cast<TMeshView*>(tabWidget->widget(i)))
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
    //bool isEnabled = (qobject_cast<TGLMesh*>(tabWidget->currentWidget()) == nullptr) ? false : true;
    bool isEnabled = (qobject_cast<TMeshView*>(tabWidget->currentWidget()) == nullptr) ? false : true;

    ui->actionCopy->setEnabled(!isUntitled);
    ui->actionClose->setEnabled(!isUntitled);
    ui->actionSaveAs->setEnabled(!isUntitled);
    ui->actionRotate->setEnabled(!isUntitled && isEnabled);
    ui->actionMesh->setEnabled(!isUntitled && isEnabled);
    ui->actionSurface->setEnabled(!isUntitled && isEnabled);
    ui->actionMesh->setEnabled(!isUntitled && isEnabled);
    ui->actionSurfaceAndMesh->setEnabled(!isUntitled && isEnabled);
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
    if (qobject_cast<TMeshView*>(tabWidget->currentWidget()))
        qobject_cast<TMeshView*>(tabWidget->currentWidget())->stopTimer();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Opening a document"), windowFilePath(), tr("QFEM problem files (*.qfpf);;Mesh files (*.trp *.trpa *.vol *.mesh *.msh *.ele *.face *.node);;QFEM result files (*.qres *.res);;All files(*)"));
    if (qobject_cast<TMeshView*>(tabWidget->currentWidget()))
        qobject_cast<TMeshView*>(tabWidget->currentWidget())->startTimer();
    if (!fileName.isEmpty())
        loadFile(fileName);

}

void TMainWindow::loadFile(QString fileName)
{
    bool isOk = false;

    slotCloseTab(0);
//  if (QFileInfo(fileName).completeSuffix().toUpper() == "TRP" || QFileInfo(fileName).completeSuffix().toUpper() == "TRPA" || QFileInfo(fileName).completeSuffix().toUpper() == "VOL" || QFileInfo(fileName).completeSuffix().toUpper() == "MSH" || QFileInfo(fileName).completeSuffix().toUpper() == "MESH" || (QFileInfo(fileName).completeSuffix().toUpper() == "1.ELE" || QFileInfo(fileName).completeSuffix().toUpper() == "1.NODE" || QFileInfo(fileName).completeSuffix().toUpper() == "1.FACE"))
    if (QFileInfo(fileName).suffix().toUpper() == "TRP" || QFileInfo(fileName).suffix().toUpper() == "TRPA" || QFileInfo(fileName).suffix().toUpper() == "VOL" || QFileInfo(fileName).suffix().toUpper() == "MSH" || QFileInfo(fileName).suffix().toUpper() == "MESH" || (QFileInfo(fileName).suffix().toUpper() == "1.ELE" || QFileInfo(fileName).suffix().toUpper() == "1.NODE" || QFileInfo(fileName).suffix().toUpper() == "1.FACE"))
        isOk = loadMesh(fileName);
    else if (QFileInfo(fileName).suffix().toUpper() == "QRES")
        isOk = loadQRES(fileName);
    else if (QFileInfo(fileName).suffix().toUpper() == "RES")
        isOk = loadRES(fileName);
    else if (QFileInfo(fileName).suffix().toUpper() == "QFPF")
        isOk = loadQFPF(fileName);

    if (isOk)
    {
        //tabWidget->insertTab(0, new TGLMesh(&(femProcessor->getFEMObject()->getMesh()), this), tr("Object"));
        tabWidget->insertTab(0, new TMeshView(&(femProcessor->getFEMObject()->getMesh()), this), tr("Object"));
        tabWidget->setTabsClosable(true);
        tabWidget->setCurrentIndex(0);

//        qobject_cast<TGLMesh*>(tabWidget->widget(0))->setImageParams(iDlg->getImageParams());

        QApplication::setOverrideCursor(Qt::BusyCursor);
        statusBar()->showMessage(tr("File successfully downloaded"),5000);
        updateRecentFileActions(fileName);
        // slotSetRotate(0);
        setCurrentFile(fileName);
        pForm->setup();
        QApplication::restoreOverrideCursor();
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Error opening file: %1").arg(fileName));
        statusBar()->showMessage(tr("Error opening file"), 5000);
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
    numThread = (settings.value("thread").toInt() > 0) ? settings.value("thread").toInt() : 1;
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
    if (qobject_cast<TMeshView*>(tabWidget->currentWidget()))
        qobject_cast<TMeshView*>(tabWidget->currentWidget())->stopTimer();
    QString fileName = QString(QFileInfo(curFile).absolutePath() + "/" +  QFileInfo(curFile).baseName() + ".qfpf"),
            fullFileName = QFileDialog::getSaveFileName(this,tr("Saving the document"),fileName,tr("QFEM Problem files (*.qfpf)"));
    if (qobject_cast<TMeshView*>(tabWidget->currentWidget()))
        qobject_cast<TMeshView*>(tabWidget->currentWidget())->startTimer();
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

    if (femObject->getParams().plist.findParameter(ParamType::YoungModulus) == 0)
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified Young's modulus!"));
        return false;
    }
    if (femObject->getParams().plist.findParameter(ParamType::PoissonRatio) == 0 && !femObject->getMesh().is1D())
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified Poisson's ratio!"));
        return false;
    }
    if (femObject->getParams().plist.findParameter(ParamType::BoundaryCondition) == 0)
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified boundary conditions!"));
        return false;
    }
    if (femObject->getParams().plist.findParameter(ParamType::VolumeLoad) == 0 && femObject->getParams().plist.findParameter(ParamType::SurfaceLoad) == 0 &&
        femObject->getParams().plist.findParameter(ParamType::ConcentratedLoad) == 0 && femObject->getParams().plist.findParameter(ParamType::PressureLoad) == 0)
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified loads!"));
        return false;
    }
    if (femObject->getParams().plist.findParameter(ParamType::Thickness) == 0 && (femObject->getMesh().is2D() || femObject->getMesh().isShell() || femObject->getMesh().isPlate()))
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified FE thickness!"));
        return false;
    }
    if (femObject->getParams().pMethod != PlasticityMethod::Linear && (femObject->getParams().loadStep <= 0 || femObject->getParams().plist.findParameter(ParamType::StressStrainCurve) == 0))
    {
        QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified nonlinear parameters!"));
        return false;
    }
    if (femObject->getParams().fType == FEMType::DynamicProblem)
    {
        if (femObject->getParams().theta <= 0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified the Wilson-Theta parameter!"));
            return false;
        }
        if (femObject->getParams().plist.findParameter(ParamType::Density) == 0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified density!"));
            return false;
        }
        if (femObject->getParams().plist.findParameter(ParamType::Damping) == 0)
        {
            QMessageBox::critical(this, tr("Error"), tr("Incorrectly specified damping ratio!"));
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
        // Загрузка параметров
        if (!pForm->getParams())
            return;

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
////    dt.setTime_t(femObject->getResult().getSolutionTime());
    dt.setSecsSinceEpoch(femObject->getResult().getSolutionTime());
    webOut += "<h1>" + tr("The problem has been solving %1 at %2").arg(QString("%1.%2.%3").arg(dt.date().day(), 2, 10, QChar('0')).arg(dt.date().month(), 2, 10, QChar('0')).arg(dt.date().year(), 2, 10, QChar('0'))).arg(QString("%1:%2").arg(dt.time().hour(), 2, 10, QChar('0')).arg(dt.time().minute(), 2, 10, QChar('0'))) + "</h1>";
    webOut += tr("Object: <b>%1</b> (nodes: <b>%2</b>, finite elements: <b>%3</b>)<br>").arg(femObject->getObjectName().c_str()).arg(femObject->getMesh().getNumVertex()).arg(femObject->getMesh().getNumFE());
    webOut += tr("FE type: <b>%1</b><br>").arg(femObject->getMesh().feName().c_str());

    // Вывод метода аппроксимации по времени (в динамике)
    if (femObject->getParams().fType == FEMType::DynamicProblem)
        webOut += (((femObject->getParams().tMethod == TimeMethod::Wilson) ? tr("Method for approximating the time: the method of Wilson") : tr("Method for approximating the time: the method Zinkevych")) + "<br>");

    // Вывод рез-тов по каждой функции
    webOut += ("<h2>" + tr("Results of calculation:") + "</h2>") + tr("Parameters of the stress-strain state");
    webOut += QString("<table border=\"1\" cellpadding=\"4\" cellspacing=\"0\"><tr><th>%1</th><th>%2</th><th>%3</th></tr>").arg(tr("Function"), tr("Min"), tr("Max"));
    if (femObject->getParams().fType == FEMType::StaticProblem)
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
        for (auto &it: femObject->getNotes())
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
            tabWidget->setCurrentIndex(i);
            break;
        }
    if (!isFind)
    {
        QTextEdit* ed = new QTextEdit(webOut);

        ed->setReadOnly(true);
        tabWidget->addTab(ed, tr("Results"));
        tabWidget->setCurrentIndex(tabWidget->count() - 1);
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

    // --------------- базовые параметры ---------------
    webOut += "<h2>" + tr("Problem parameters:") + "</h2>";
    webOut += ((femObject->getParams().fType == FEMType::StaticProblem) ? tr("Solution method: <b>%1</b>").arg(tr("static (Lagrange)")) : tr("Solution method: <b>%1</b>").arg(tr("dynamic (Ostrogradsky)"))) + "<br>";

    webOut += tr("Computational accuracy: <b>%1</b>").arg(femObject->getParams().eps, int(femObject->getParams().width), 'e', int(femObject->getParams().precision)) + "<br>";
    // webOut += tr("Elastic characteristics");
    sayParam(webOut, tr("Young's modulus"), ParamType::YoungModulus, false);
    sayParam(webOut, tr("Poisson's ratio"), ParamType::PoissonRatio, false);

    if (femObject->getParams().fType == FEMType::DynamicProblem)
    {
        sayParam(webOut, tr("Density"), ParamType::Density, false);
        sayParam(webOut, tr("Damping ratio"), ParamType::Damping, false);
    }
    // Упруго-пластические параметры задачи
    if (femObject->getParams().pMethod != PlasticityMethod::Linear)
    {
        num = 1;
        webOut += "<br>" + ((femObject->getParams().pMethod == PlasticityMethod::MVS) ? tr("Method of elastic-plastic analysis: <b>%1</b>").arg(tr("method of variable stiffness")) : tr("Method of elastic-plastic analysis: <b>%1</b>").arg(tr("method of elastic solutions Ilyushin"))) + "<br>";
        webOut += tr("Stress-strain curve");

        for (auto it: femObject->getParams().plist)
            if (it.getType() == ParamType::StressStrainCurve)
            {
                webOut += QString("<br>%1. <b>%2</b>: %3").arg(num++).arg(tr("Predicate"), it.getPredicate().c_str());// + "<br>";
                webOut += QString("<table border=\"1\" cellpadding=\"4\" cellspacing=\"0\"><tr><th>%1</th><th>%2</th><th>%3</th></tr>").arg(tr("N"), tr("Stress"), tr("Deformation"));
                for (unsigned i = 0; i < it.getStressStrainCurve().size1(); i++)
                    webOut += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(i + 1).arg(it.getStressStrainCurve(i, 0), int(femObject->getParams().width), 'e', int(femObject->getParams().precision)).arg(it.getStressStrainCurve(i, 1), int(femObject->getParams().width), 'e', int(femObject->getParams().precision));
                webOut += "</table><br>";

            }
        webOut += "<br>" + tr("Load step") + QString("<b>%1</b><br>").arg(femObject->getParams().loadStep, int(femObject->getParams().width), 'e', int(femObject->getParams().precision));
    }

    // Толщина КЭ
    if (femObject->getMesh().isPlate() || femObject->getMesh().isShell() || femObject->getMesh().is1D() || femObject->getMesh().is2D())
        sayParam(webOut, tr("FE thickness"), ParamType::Thickness, false);


    // --------------- Термоупругие параметры ---------------
    sayParam(webOut, tr("Thermal expansion"), ParamType::Alpha, false);
    sayParam(webOut, tr("Temperatire"), ParamType::Temperature, false);

    // --------------- Краевые условия задачи ---------------
    sayParam(webOut, tr("Boundary condition"), ParamType::BoundaryCondition);

    // --------------- Объемная нагрузки ---------------
    sayParam(webOut, tr("Volume load"), ParamType::VolumeLoad);

    // --------------- Поверхностная нагрузки ---------------
    sayParam(webOut, tr("Surface load"), ParamType::SurfaceLoad);

    // --------------- Сосредоточенная нагрузки ---------------
    sayParam(webOut, tr("Concentrated load"), ParamType::ConcentratedLoad);

    // --------------- Давление ---------------
    sayParam(webOut, tr("Pressure load"), ParamType::PressureLoad, false);

    // --------------- Начальные и другие условия, зависящие от времени ---------------
    if (femObject->getParams().fType == FEMType::DynamicProblem)
    {
        webOut += tr("time parameters - initial time: <b>%1</b>, the final moment: <b>%2</b>, step: <b>%3</b>").arg(femObject->getParams().t0).arg(femObject->getParams().t1).arg(femObject->getParams().th) + "<br>";
        num = 0;

        for (auto &it: femObject->getParams().plist)
            if (it.getType() == ParamType::InitialCondition)
            {
                num++;
                break;
            }
        if (num)
        {
            webOut += "<br>" + tr("initial conditions:") + "<br>";
            for (auto &it: femObject->getParams().plist)
                if (it.getType() == ParamType::InitialCondition)
                {
                    if (contains(it.getInitialCondition(), InitialCondition::U))
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[4].c_str(), it.getExpression().c_str()) + "<br>";
                    if (contains(it.getInitialCondition(), InitialCondition::V))
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[5].c_str(), it.getExpression().c_str()) + "<br>";
                    if (contains(it.getInitialCondition(), InitialCondition::W))
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[6].c_str(), it.getExpression().c_str()) + "<br>";
                    if (contains(it.getInitialCondition(), InitialCondition::Ut))
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[22].c_str(), it.getExpression().c_str()) + "<br>";
                    if (contains(it.getInitialCondition(), InitialCondition::Vt))
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[23].c_str(), it.getExpression().c_str()) + "<br>";
                    if (contains(it.getInitialCondition(), InitialCondition::Wt))
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[24].c_str(), it.getExpression().c_str()) + "<br>";
                    if (contains(it.getInitialCondition(), InitialCondition::Utt))
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[25].c_str(), it.getExpression().c_str()) + "<br>";
                    if (contains(it.getInitialCondition(), InitialCondition::Vtt))
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[26].c_str(), it.getExpression().c_str()) + "<br>";
                    if (contains(it.getInitialCondition(), InitialCondition::Wtt))
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[27].c_str(), it.getExpression().c_str()) + "<br>";
                }
        }
    }
    // --------------- Вспомогательные параметры ---------------
    if (femObject->getParams().variables.size())
    {
        webOut += "<br>" + tr("Global variables") + "<br>";
        for (auto &it: femObject->getParams().variables)
            webOut += tr("<b>%1</b> = <b>%2</b>").arg(it.first.c_str()).arg(it.second) + "<br>";
    }
    webOut += "</p>";
}

// Вывод информации о конкретном парметре расчета в табличной форме
void TMainWindow::sayParam(QString& webOut, const QString& title, ParamType param, bool isDirect)
{
    unsigned num = 0;
    QString predicate;
    TFEMObject* femObject = femProcessor->getFEMObject();

    for (auto &it: femObject->getParams().plist)
        if (it.getType() == param)
        {
            num++;
            break;
        }
    if (num)
    {
        webOut += "<br>" + tr(title.toUtf8());
        num = 1;
        webOut += QString("<table border=\"1\" cellpadding=\"4\" cellspacing=\"0\"><tr><th>%1</th><th>%2</th><th>%3</th>").arg(tr("N"), tr("Value"), tr("Predicate"));
        if (isDirect)
        {
            webOut += QString("<th>%1</th>").arg(femObject->getParams().names[0].c_str());
            if (femObject->getMesh().getDimension() > 1)
                webOut += QString("<th>%1</th>").arg(femObject->getParams().names[1].c_str());
            if (femObject->getMesh().getDimension() > 2 || femObject->getMesh().isPlate())
                webOut += QString("<th>%1</th>").arg(femObject->getParams().names[2].c_str());
            webOut += "</tr>";
        }
        for (auto &it: femObject->getParams().plist)
        {
            if (it.getType() != param)
                continue;
            predicate = QString(it.getPredicate().c_str()).replace(QString("<"), QString("&lt;")).replace(QString(">"), QString("&gt;"));
            //predicate = QString(it.getPredicate().c_str()).replace(QString("<="), QString("&lt;=")).replace(QString(">="), QString("&gt;=")).replace(QString("<"), QString("&lt;")).replace(QString(">"), QString("&gt;"));
            webOut += QString("<tr><td>%1</td><td>%2</td><td>%3</td>").arg(num++).arg(it.getExpression().c_str()).arg(predicate);
            if (isDirect)
            {
                if (contains(it.getDirect(), Direction::X))
                    webOut += QString("<td>%1</td>").arg("+");
                else
                    webOut += QString("<td>%1</td>").arg("");
                if (contains(it.getDirect(), Direction::Y))
                    webOut += QString("<td>%1</td>").arg("+");
                else if (femObject->getMesh().getFreedom() > 1)
                    webOut += QString("<td>%1</td>").arg("");
                if (contains(it.getDirect(), Direction::Z))
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
    QJsonDocument doc;
    QJsonObject main,
                header,
                mesh;
    TFEMObject* femObject = femProcessor->getFEMObject();

    // ------------- Заголовок -----------------
////    dt.setTime_t(femObject->getResult().getSolutionTime());
    dt.setSecsSinceEpoch(femObject->getResult().getSolutionTime());
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
    QJsonArray bc;
    TFEMParams& params = femProcessor->getFEMObject()->getParams();
    int dir;
    string expr,
           pred;
    ParamType type;
    matrix<double> ssc;


    file.setFileName(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, QObject::tr("Error"), QObject::tr("Error opening %1").arg(fileName));
        return false;
    }
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
    params.fType = (obj.value(QString("Parameters"))["ProblemType"].toVariant().toString() == "Static") ? FEMType::StaticProblem : FEMType::DynamicProblem;
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
            type = static_cast<ParamType>(bc.at(i)["Type"].toVariant().toInt());
            if (type == ParamType::StressStrainCurve)
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
    unsigned funIndex = unsigned(femObject->getResult().index(funName.toStdString()));
    vector<vector<double>*> delta;

    // Проверка наличия такой функции в уже открытых закладках
    for (int i = 0; i < tabWidget->count(); i++)
        if (tabWidget->tabText(i).replace("&", "") == funName)
        {
            isFind = true;
            tabWidget->setCurrentIndex(i);
        }
    if (!isFind)
    {
        for (auto i = 0u; i < femObject->getMesh().getFreedom(); i++)
            delta.push_back(&femObject->getResult(i).getResults());
        //tabWidget->addTab(new TGLFunction(&femObject->getMesh(), &femObject->getResult(ind_f).getResults(), &femObject->getResult(ind_d + 0).getResults(), &femObject->getResult(ind_d + 1).getResults(), &femObject->getResult(ind_d + 2).getResults(), expression, this), funName);
        tabWidget->addTab(new TFunctionView(&femObject->getMesh(), &femObject->getResult(funIndex).getResults(), delta, expression, this), funName);
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
    if (femProcessor->getFEMObject()->getParams().fType == FEMType::DynamicProblem)
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
    //ImageType type = (qobject_cast<TGLFunction*>(tabWidget->currentWidget())) ? ((qobject_cast<TParameterView*>(tabWidget->currentWidget())) ? ImageType::param : ImageType::func ) : ImageType::mesh;
    ImageType type = (qobject_cast<TFunctionView*>(tabWidget->currentWidget())) ? ((qobject_cast<TParameterView*>(tabWidget->currentWidget())) ? ImageType::param : ImageType::func ) : ImageType::mesh;

    if (qobject_cast<TParameterView*>(tabWidget->currentWidget()))
        if (qobject_cast<TParameterView*>(tabWidget->currentWidget())->getType() == ParamType::VolumeLoad || qobject_cast<TParameterView*>(tabWidget->currentWidget())->getType() == ParamType::SurfaceLoad ||
            qobject_cast<TParameterView*>(tabWidget->currentWidget())->getType() == ParamType::ConcentratedLoad || qobject_cast<TParameterView*>(tabWidget->currentWidget())->getType() == ParamType::PressureLoad or
            qobject_cast<TParameterView*>(tabWidget->currentWidget())->getType() == ParamType::BoundaryCondition)
            type = ImageType::mesh;
    //if (qobject_cast<TGLMesh*>(tabWidget->currentWidget()))
    if (qobject_cast<TMeshView*>(tabWidget->currentWidget()))
    {
        iDlg->changeLanguage();
        iDlg->setGLWidget(qobject_cast<TMeshView*>(tabWidget->currentWidget()));
        iDlg->setImageParams(qobject_cast<TMeshView*>(tabWidget->currentWidget())->getImageParams(), int(femObject->getMesh().getFreedom()), type);
        if (iDlg->exec() == QDialog::Accepted)
            //if (qobject_cast<TGLMesh*>(tabWidget->currentWidget()))
            if (qobject_cast<TMeshView*>(tabWidget->currentWidget()))
            {
                // qobject_cast<TGLMesh*>(tabWidget->currentWidget())->setImageParams(iDlg->getImageParams());
                // qobject_cast<TGLMesh*>(tabWidget->currentWidget())->repaint();
                qobject_cast<TMeshView*>(tabWidget->currentWidget())->repaint();
            }
    }
}

void TMainWindow::repaintResults(void)
{
    QString name,
            exp;
    // TGLFunction *ptr;
    TFunctionView *ptr;
    bool isCalc = femProcessor->getFEMObject()->isCalculated();

    for (int i = 1; i < tabWidget->count(); i++)
        //if ((ptr = qobject_cast<TGLFunction*>(tabWidget->widget(i))) != nullptr)
        if ((ptr = qobject_cast<TFunctionView*>(tabWidget->widget(i))) != nullptr)
        {
            if (isCalc)
            {
                if ((exp = ptr->getExpression()).length())
                    calcExpression(exp, name);
                ptr->refresh();
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
    TResults &result = femProcessor->getFEMObject()->getResult();
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
                nonlin;
    QJsonArray names,
               lbc,
               variables;
    TFEMParams &params = femProcessor->getFEMObject()->getParams();
    
    pForm->getParams();
    // Тип задачи
    paramObj.insert("ProblemType", (params.fType == FEMType::StaticProblem) ? "Static" : "Dynamic");

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
    nonlin.insert("CalculationMethod", static_cast<int>(params.pMethod));
    nonlin.insert("LoadStep", params.loadStep);
    paramObj.insert("Nonlinearity", nonlin);

    // Краевые условия и прочие параметры
    for (auto it: params.plist)
    {
        QJsonObject bc;

        bc.insert("Type", static_cast<int>(it.getType()));
        bc.insert("Direct", static_cast<int>(it.getDirect()));
        bc.insert("Predicate", it.getPredicate().c_str());
        if (it.getType() == ParamType::StressStrainCurve)
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
    for (auto &it: params.variables)
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
    meshObj.insert("FEType", QJsonValue::fromVariant(static_cast<int>(mesh.getTypeFE())));
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

    for (auto &it: notes)
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
////    dt.setTime_t(femProcessor->getFEMObject()->getResult().getSolutionTime());
    dt.setSecsSinceEpoch(femProcessor->getFEMObject()->getResult().getSolutionTime());
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

    mesh.getDataFE(mesh.convertFEType(FEType(fe_type)), be_size, fe_size, fe_dim);
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
    if (fe_type == static_cast<int>(FEType::fe3d3s) || fe_type == static_cast<int>(FEType::fe3d4s) || fe_type == static_cast<int>(FEType::fe3d6s))
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

void TMainWindow::loadParam(const QJsonObject &paramObj)
{
    QJsonArray lbc = paramObj["BoundaryConditions"].toArray(),
               nm = paramObj["Names"].toArray(),
               var = paramObj["Variables"].toArray();
    TFEMParams &params = femProcessor->getFEMObject()->getParams();

    // Тип задачи
    params.fType = (paramObj["ProblemType"].toString() == "Static") ? FEMType::StaticProblem : FEMType::DynamicProblem;

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
    for (auto i = 0; i < lbc.size(); i++)
    {
        ParamType type = static_cast<ParamType>(static_cast<QJsonValue>(lbc[i])["Type"].toInt());
        int direct = static_cast<QJsonValue>(lbc[i])["Direct"].toInt();
        QString predicate = static_cast<QJsonValue>(lbc[i])["Predicate"].toString(),
                expression = static_cast<QJsonValue>(lbc[i])["Expression"].toString();
        matrix<double> ssc;

        if (type == ParamType::StressStrainCurve)
        {
            pForm->decodeStressStarinCurve(expression.toStdString(), ssc);
            params.plist.addStressStrainCurve(ssc, predicate.toStdString());
        }
        else
            params.plist.addParameter(type, expression.toStdString(), predicate.toStdString(), direct);
    }

    // Названия
    params.names.clear();
    for (auto i = 0; i < nm.size(); i++)
        params.names.push_back(nm[i].toString().toStdString());

    // Вспомагательные параметры (для парсера)
    params.variables.clear();
    for (auto i = 0; i < var.size(); i++)
    {
        QStringList sl = var[i].toString().split(" ");

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
    TResults &result = femProcessor->getFEMObject()->getResult();

    result.clear();
    for (const QJsonValue &value: resultArr)
    {
        res.clear();
        name = value["Function"].toString();
        t = value["Time"].toDouble();
        arr = value["Values"].toArray();
        for (auto i = 0; i < arr.size(); i++)
            res.push_back(arr[i].toDouble());
        result.setResult(res, name.toStdString(), t);
    }
}

// Чтение примечаний из JSON-объекта
void TMainWindow::loadNotes(const QJsonArray &notesArr)
{
    auto &notes = femProcessor->getFEMObject()->getNotes();

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
        if (parser.get_error() != ErrorCode::Undefined)
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
    if (dlgTranslator->load(translatorFileName, QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
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
            (void)translator->load("QFEM_RU");
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
    if (qobject_cast<TMeshView*>(tabWidget->currentWidget()))
        qobject_cast<TMeshView*>(tabWidget->currentWidget())->stopTimer();
    QString fileName = QFileDialog::getSaveFileName(this, tr("Saving results"), QString(QFileInfo(curFile).absolutePath() + "/" + QFileInfo(curFile).baseName() + "." + QString("qres").toLower()), tr("QFEM result files (*.qres);; QFEM report files (*.txt)"));
    if (qobject_cast<TMeshView*>(tabWidget->currentWidget()))
        qobject_cast<TMeshView*>(tabWidget->currentWidget())->startTimer();
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

    //if (qobject_cast<TGLMesh*>(tabWidget->currentWidget()))
    if (qobject_cast<TMeshView*>(tabWidget->currentWidget()))
    {
        //qobject_cast<TGLMesh*>(tabWidget->currentWidget())->loadPaint();
        qobject_cast<TMeshView*>(tabWidget->currentWidget())->loadPaint();
        //clipboard->setImage(qobject_cast<TGLMesh*>(tabWidget->currentWidget())->grabFramebuffer());
        clipboard->setImage(qobject_cast<TMeshView*>(tabWidget->currentWidget())->grabFramebuffer());
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
    QString tabName = paramName(static_cast<ParamType>(type)).c_str();

    textCursor.clearSelection();
    terminal->setTextCursor(textCursor);

    pb->reset();
    pb->show();
    ui->actionStart->setEnabled(false);
    ui->actionStop->setEnabled(true);
    bcProcessor->setNumTheread(numThread);
    bcProcessor->setType(static_cast<ParamType>(type));
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
            qobject_cast<TParameterView*>(tabWidget->widget(i))->redraw(&bcProcessor->getVertex());
            tabWidget->setCurrentIndex(i);
        }
    if (!isFind)
    {
        tabWidget->addTab(new TParameterView(&femProcessor->getFEMObject()->getMesh(), &bcProcessor->getVertex(), static_cast<ParamType>(type), this), tabName);
        tabWidget->setCurrentIndex(tabWidget->count() - 1);
    }

}

void TMainWindow::slotSetRotate(void)
{
    qobject_cast<TMeshView*>(tabWidget->currentWidget())->getImageParams()->isAutoRotate = !qobject_cast<TMeshView*>(tabWidget->currentWidget())->getImageParams()->isAutoRotate;
    ui->actionRotate->setChecked(qobject_cast<TMeshView*>(tabWidget->currentWidget())->getImageParams()->isAutoRotate);
}

void TMainWindow::slotSetRotate(bool isRotate)
{
    ui->actionRotate->setChecked(isRotate);
}

void TMainWindow::slotShowSurface(void)
{
    if (qobject_cast<TMeshView*>(tabWidget->currentWidget()))
    {
        ui->actionSurface->setChecked(true);
        ui->actionMesh->setChecked(false);
        ui->actionSurfaceAndMesh->setChecked(false);

        qobject_cast<TMeshView*>(tabWidget->currentWidget())->getImageParams()->isSurface = true;
        qobject_cast<TMeshView*>(tabWidget->currentWidget())->getImageParams()->isMesh = false;
    }
}

void TMainWindow::slotShowMesh(void)
{
    ui->actionSurface->setChecked(false);
    ui->actionMesh->setChecked(true);
    ui->actionSurfaceAndMesh->setChecked(false);

    qobject_cast<TMeshView*>(tabWidget->currentWidget())->getImageParams()->isSurface = false;
    qobject_cast<TMeshView*>(tabWidget->currentWidget())->getImageParams()->isMesh = true;
}

void TMainWindow::slotShowSurfaceAndMesh(void)
{
    ui->actionSurface->setChecked(false);
    ui->actionMesh->setChecked(false);
    ui->actionSurfaceAndMesh->setChecked(true);

    qobject_cast<TMeshView*>(tabWidget->currentWidget())->getImageParams()->isSurface = true;
    qobject_cast<TMeshView*>(tabWidget->currentWidget())->getImageParams()->isMesh = true;
}
