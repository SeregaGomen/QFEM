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
#include <fstream>

#include "appesetupdialog.h"
#include "vcdialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "setuptaskdialog.h"
#include "setupimagedialog.h"
#include "fldialog.h"
#include "glfun.h"
#include "gbdialog.h"
#include "qmsg.h"
#include "helpdialog.h"
#include "femprocessor.h"
#include "lcprocessor.h"
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
    delete lcProcessor;
    delete myCout;
    delete myCerr;
    delete terminal;
    delete dock;
    delete tabWidget;
    delete tDlg;
    delete iDlg;
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

    isAutoSaveProtocol = isAutoScroll = isBlackBkg = isUntitled = true;

    femProcessor = new TFEMProcessor();
    lcProcessor = new TLCProcessor(femProcessor->getFEMObject());

    setupRecentActions();
    readSettings();
    setupLanguage();
    createRecentMenu();

    tDlg = new TSetupTaskDialog(femProcessor->getFEMObject(), this);
    iDlg = new TSetupImageDialog(this);

    tabWidget = new QTabWidget();
    tabWidget->setTabsClosable(false);

    setCentralWidget(tabWidget);

    thread = new QThread(this);

    terminal = new TTerminal(thread, this);
    terminal->setFontWeight( QFont::DemiBold );
    terminal->setTextColor( QColor( "gray" ) );
    terminal->setStyleSheet((isBlackBkg) ? "QTextEdit { background-color: rgb(0, 0, 0) }" : "QTextEdit { background-color: rgb(255, 255, 255) }");
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
    connect(dynamic_cast<QObject*>(msg),SIGNAL(signalSetValue(int)),pb,SLOT(setValue(int)));
    connect(dynamic_cast<QObject*>(msg),SIGNAL(signalSetMinimum(int)),pb,SLOT(setMinimum(int)));
    connect(dynamic_cast<QObject*>(msg),SIGNAL(signalSetMaximum(int)),pb,SLOT(setMaximum(int)));

    connect(dynamic_cast<QObject*>(msg),SIGNAL(signalSetTextVisible(bool)),pb,SLOT(slotSetTextVisible(bool)));
    connect(dynamic_cast<QObject*>(msg),SIGNAL(signalSetFormat(const QString&)),pb,SLOT(slotSetFormat(const QString&)));
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
        tabWidget->removeTab(nTab);
    else
    {
        if (thread->isRunning())
        {
            QApplication::setOverrideCursor(Qt::BusyCursor);
            femProcessor->stop();
            lcProcessor->stop();
            while (thread->isRunning())
                QApplication::processEvents();
            QApplication::restoreOverrideCursor();
        }
        femProcessor->clear();
        lcProcessor->clear();
        pb->setValue(0);
        i = tabWidget->count() - 1;
        while (tabWidget->count() > 0)
        {
            if (qobject_cast<TGLMesh*>(tabWidget->widget(i)))
                delete tabWidget->widget(i);
            tabWidget->removeTab(i--);
        }
        iDlg->clear();
        tDlg->clear();
        setWindowTitle("QFEM");
        isUntitled = true;
        checkMenuState();
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
    QString fileName = QFileDialog::getOpenFileName(this,tr("Opening a document"),windowFilePath(),tr("QFEM Problem files (*.qfpf);; Mesh files (*.trp *.trpa *.vol *.mesh);; Result files (*.qres)"));

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
    else if (QFileInfo(fileName).completeSuffix().toUpper() == "QFPF")
        isOk = loadJSON(fileName);

    if (isOk)
    {
        tabWidget->insertTab(0, new TGLMesh(&(femProcessor->getFEMObject()->getMesh()), &lcProcessor->getLCVertex(), this), tr("Object"));
        tabWidget->setTabsClosable(true);
        tabWidget->setCurrentIndex(0);

//        qobject_cast<TGLMesh*>(tabWidget->widget(0))->setImageParams(iDlg->getImageParams());

        QApplication::setOverrideCursor(Qt::BusyCursor);
        statusBar()->showMessage(tr("File successfully downloaded"),5000);
        updateRecentFileActions(fileName);
        slotSetRotate(0);
        setCurrentFile(fileName);
        QApplication::restoreOverrideCursor();


        if (iDlg->getImageParams().isLimit || iDlg->getImageParams().isLoad)
        {
            lcProcess();
            if (tabWidget->count())
                qobject_cast<TGLMesh*>(tabWidget->widget(0))->repaint();
        }

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

    isBlackBkg = settings.value("black").toBool();
    isAutoScroll = settings.value("scroll").toBool();
    isAutoSaveProtocol = settings.value("protocol").toBool();
    ui->actionTerminal->setChecked(isTerminal);
    files = settings.value("recentFileList").toStringList();
    langNo = settings.value("lang").toInt();
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
    settings.setValue("black", isBlackBkg);
    settings.setValue("scroll", isAutoScroll);
    settings.setValue("protocol", isAutoSaveProtocol);
}

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
    TFEMObject* femObject = femProcessor->getFEMObject();

    if (QFileInfo(curFile).suffix().toUpper() == "QRES")
    {
        QString name = QString(QFileInfo(curFile).absolutePath() + "/" +  QFileInfo(curFile).baseName() + ".trpa"),
//                meshFile = QFileDialog::getSaveFileName(this,tr("Save mesh"),windowFilePath(),tr("Mesh files (*.trpa)")); // Запрос имени файла для сетки
                meshFile = QFileDialog::getSaveFileName(this,tr("Save mesh"),name,tr("Mesh files (*.trpa)")); // Запрос имени файла для сетки

        if (meshFile.isEmpty())
            return;
        // Формирование файла с сеткой в случае, если исходный файл - QRES
        femObject->getMesh().write(meshFile.toStdString());
        femObject->setFileName(meshFile.toStdString());
    }

    if (saveJSON(fileName))
    {
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File successfully saved"),5000);
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
    lcProcessor->clear();
    isUntitled = true;
    setWindowTitle("QFEM");
    tDlg->clear();
    iDlg->clear();
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
    lcProcessor->moveToThread(thread);
    connect(thread, SIGNAL(started()), lcProcessor, SLOT(start()));
    connect(lcProcessor, SIGNAL(finished()), thread, SLOT(terminate()));
    thread->start();
    while (thread->isRunning())
        QApplication::processEvents();
    disconnect(thread, SIGNAL(started()), lcProcessor, SLOT(start()));
    disconnect(lcProcessor, SIGNAL(finished()), thread, SLOT(terminate()));
    ui->actionStart->setEnabled(true);
    ui->actionStop->setEnabled(false);
    pb->hide();
    tDlg->setBCChanged(false);

    terminal->setTextCursor(saveCursor);
}


void TMainWindow::slotSetupTaskParams(void)
{
    tDlg->changeLanguage();
    tDlg->setup();
    if (tDlg->exec() == QDialog::Accepted)
    {
        if (iDlg->getImageParams().isLoad || iDlg->getImageParams().isLimit)
            if (!qobject_cast<TGLMesh*>(tabWidget->widget(0))->isSelectedVertex() || tDlg->isBCChanged())
            {
//                pb->show();
//                ui->actionStart->setEnabled(false);
//                ui->actionStop->setEnabled(true);
//                qobject_cast<GLObjWidget*>(tabWidget->widget(0))->setSelectedVertex();
//                ui->actionStart->setEnabled(true);
//                ui->actionStop->setEnabled(false);
//                pb->hide();
//                tDlg->setLimitChanged(false);
//                qobject_cast<GLObjWidget*>(tabWidget->widget(0))->repaint();
                lcProcess();
                qobject_cast<TGLMesh*>(tabWidget->widget(0))->repaint();
            }
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
    ui->actionObjectParameters->setEnabled(false);

//    QApplication::setOverrideCursor(Qt::BusyCursor);
    startSolvingProblem();
    repaintResults();
    qApp->beep();
//    QApplication::restoreOverrideCursor();

    ui->actionObjectParameters->setEnabled(true);
    ui->actionStart->setEnabled(true);
    ui->actionStop->setEnabled(false);
    checkMenuState();

    terminal->setTextCursor(saveCursor);
}

// Остановка процедуры расчета задачи (или любого вычислительного процесса)
void TMainWindow::slotStopProcess(void)
{
    lcProcessor->stop();
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

    return true;
}

// Запуск расчета по функционалу Лагранжа (статика) или Гамильтона-Остроградского (динамика)
void TMainWindow::startSolvingProblem(void)
{
    TFEMObject* femObject = femProcessor->getFEMObject();
    QString htmlFile = QFileInfo(curFile).absolutePath() + "/" + QFileInfo(curFile).baseName() + ".html",
            qresFile = QFileInfo(curFile).absolutePath() + "/" + QFileInfo(curFile).baseName() + ".qres";

    if (!checkParams())
        return;

    try
    {
        // Запуск расчета
        femProcess();

        if (femObject->isCalculated())
        {
            // Отображение результатов
//            showResults(QFileInfo(curFile).absolutePath() + "/" + QFileInfo(curFile).baseName() + "." + QString("txt").toLower());
            QApplication::setOverrideCursor(Qt::BusyCursor);
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
    TFEMObject* femObject = femProcessor->getFEMObject();
    int d = femObject->getResult().getSolutionTime().tm_mday,
        m = femObject->getResult().getSolutionTime().tm_mon + 1,
        y = femObject->getResult().getSolutionTime().tm_year + 1900,
        hour = femObject->getResult().getSolutionTime().tm_hour,
        min = femObject->getResult().getSolutionTime().tm_min;
    QString webOut,
            tm;
    bool isFind = false;

    webOut = "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">";

    // Получение времени и даты формирования отчета
    webOut += "<h1>";
    webOut += tr("The problem has been solving %1 at %2").arg(QString("%1.%2.%3").arg(d,2,10,QChar('0')).arg(m,2,10,QChar('0')).arg(y,2,10,QChar('0'))).arg(QString("%1:%2").arg(hour,2,10,QChar('0')).arg(min,2,10,QChar('0')));
    webOut += "</h1>";

    webOut += tr("Object: <b>%1</b> (nodes: <b>%2</b>, finite elements: <b>%3</b>)").arg(femObject->getObjectName().c_str()).arg(femObject->getMesh().getNumVertex()).arg(femObject->getMesh().getNumFE());
    webOut += "<br>";

    webOut += tr("FE type: <b>%1</b>").arg(femObject->getMesh().feName().c_str());
    webOut += "<br>";

    // Вывод метода аппроксимации по времени (в динамике)
    if (femObject->getParams().fType == DynamicProblem)
    {
        if (femObject->getParams().tMethod == Wilson)
            webOut += tr("Method for approximating the time: the method of Wilson");
        else if (femObject->getParams().tMethod == Zinkevich)
            webOut += tr("Method for approximating the time: the method Zinkevych");
        webOut += "<br>";
    }


    // Вывод рез-тов по каждой функции
    webOut += "<br>";
    webOut += tr("Results of calculation");
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

    // Вывод примечаний к расчету
    webOut += "<br>";
    for (auto it = femObject->getNotes().begin(); it != femObject->getNotes().end(); it++)
    {
        webOut += it->c_str();
        webOut += "<br>";
    }
//    webOut += "<br>";

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
    webOut += "<h2>";
    webOut += tr("Problem parameters:");
    webOut += "</h2>";
//    webOut += "<p style = \"margin-left:1em;margin-right:-400px;\">";
    if (femObject->getParams().fType == StaticProblem)
        webOut += tr("Solution method: <b>%1</b>").arg(tr("static (Lagrange)"));
    else
        webOut += tr("Solution method: <b>%1</b>").arg(tr("dynamic (Ostrogradsky)"));
    webOut += "<br>";

    webOut += tr("Computational accuracy: <b>%1</b>").arg(femObject->getParams().eps, int(femObject->getParams().width), 'e', int(femObject->getParams().precision));
    webOut += "<br><br>";
    webOut += tr("Elastic characteristics");
    sayParam(webOut, "Young's modulus", YOUNG_MODULUS_PARAMETER, false);
    sayParam(webOut, "Poisson's ratio", POISSON_RATIO_PARAMETER, false);

    if (femObject->getParams().fType == DynamicProblem)
    {
        sayParam(webOut, "Density", DENSITY_PARAMETER, false);
        sayParam(webOut, "Damping parameter", DAMPING_PARAMETER, false);
    }
    // Упруго-пластические параметры задачи
    if (femObject->getParams().pMethod != Linear)
    {
        webOut += "<br>";
        if (femObject->getParams().pMethod == MVS)
            webOut += tr("Method of elastic-plastic analysis: <b>%1</b>").arg(tr("method of variable stiffness"));
        else
            webOut += tr("Method of elastic-plastic analysis: <b>%1</b>").arg(tr("method of elastic solutions Ilyushin"));
        webOut += "<br>";

        webOut += tr("Stress-strain curve:");

        for (auto it = femObject->getParams().plist.begin(); it != femObject->getParams().plist.end(); it++)
            if (it->getType() == STRESS_STRAIN_CURVE_PARAMETER)
            {
                webOut += "<br>";
                webOut += QString("<b>%1</b>: %2").arg(tr("Predicate")).arg(it->getPredicate().c_str());
                webOut += "<br>";

                webOut += QString("<table border=\"1\" cellpadding=\"4\" cellspacing=\"0\"><tr><th>%1</th><th>%2</th><th>%3</th></tr>").arg(tr("N")).arg(tr("Stress")).arg(tr("Deformation"));
                for (unsigned i = 0; i < it->getStressStrainCurve().size1(); i++)
                    webOut += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>").arg(i + 1).arg(it->getStressStrainCurve(i, 0), int(femObject->getParams().width), 'e', int(femObject->getParams().precision)).arg(it->getStressStrainCurve(i, 1), int(femObject->getParams().width), 'e', int(femObject->getParams().precision));
                webOut += "</table><br>";

            }
        webOut += "<br>";
        webOut += tr("Load step");
        webOut += QString("<b>%1</b>").arg(femObject->getParams().loadStep, int(femObject->getParams().width),'e', int(femObject->getParams().precision));
        webOut += "<br>";

    }

    // Толщина КЭ
    if (femObject->getMesh().isPlate() || femObject->getMesh().isShell() || femObject->getMesh().is1D() || femObject->getMesh().is2D())
        sayParam(webOut, "FE thickness", THICKNESS_PARAMETER, false);


    // --------------- Краевые условия задачи ---------------
    sayParam(webOut, "Boundary condition", BOUNDARY_CONDITION_PARAMETER);

    // --------------- Объемная нагрузки ---------------
    sayParam(webOut, "Volume load", VOLUME_LOAD_PARAMETER);

    // --------------- Поверхностная нагрузки ---------------
    sayParam(webOut, "Surface load", SURFACE_LOAD_PARAMETER);

    // --------------- Сосредоточенная нагрузки ---------------
    sayParam(webOut, "Concentrated load", CONCENTRATED_LOAD_PARAMETER);

    // --------------- Давление ---------------
    sayParam(webOut, "Pressure load", PRESSURE_LOAD_PARAMETER, false);

    // --------------- Начальные и другие условия, зависящие от времени ---------------
    if (femObject->getParams().fType == DynamicProblem)
    {
        webOut += tr("time parameters - initial time: <b>%1</b>, the final moment: <b>%2</b>, step: <b>%3</b>").arg(femObject->getParams().t0).arg(femObject->getParams().t1).arg(femObject->getParams().th);
        webOut += "<br>";
        num = 0;

        for (auto it = femObject->getParams().plist.begin(); it != femObject->getParams().plist.end(); it++)
            if (it->getType() == INITIAL_CONDITION_PARAMETER)
            {
                num++;
                break;
            }
        if (num)
        {
            webOut += "<br>";
            webOut += tr("initial conditions:");
            webOut += "<br>";
            for (auto it = femObject->getParams().plist.begin(); it != femObject->getParams().plist.end(); it++)
                if (it->getType() == INITIAL_CONDITION_PARAMETER)
                {
                    if ((it->getDirect() & FUN_U) == FUN_U)
                    {
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[4].c_str()).arg(it->getExpression().c_str());
                        webOut += "<br>";
                    }
                    if ((it->getDirect() & FUN_V) == FUN_V)
                    {
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[5].c_str()).arg(it->getExpression().c_str());
                        webOut += "<br>";
                    }
                    if ((it->getDirect() & FUN_W) == FUN_W)
                    {
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[6].c_str()).arg(it->getExpression().c_str());
                        webOut += "<br>";
                    }
                    if ((it->getDirect() & FUN_UT) == FUN_UT)
                    {
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[19].c_str()).arg(it->getExpression().c_str());
                        webOut += "<br>";
                    }
                    if ((it->getDirect() & FUN_VT) == FUN_VT)
                    {
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[20].c_str()).arg(it->getExpression().c_str());
                        webOut += "<br>";
                    }
                    if ((it->getDirect() & FUN_WT) == FUN_WT)
                    {
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[21].c_str()).arg(it->getExpression().c_str());
                        webOut += "<br>";
                    }
                    if ((it->getDirect() & FUN_UTT) == FUN_UTT)
                    {
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[22].c_str()).arg(it->getExpression().c_str());
                        webOut += "<br>";
                    }
                    if ((it->getDirect() & FUN_VTT) == FUN_VTT)
                    {
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[23].c_str()).arg(it->getExpression().c_str());
                        webOut += "<br>";
                    }
                    if ((it->getDirect() & FUN_WTT) == FUN_WTT)
                    {
                        webOut += tr("function: <b>%1</b>, value: <b>%2</b>").arg(femObject->getParams().names[24].c_str()).arg(it->getExpression().c_str());
                        webOut += "<br>";
                    }
                }
        }
    }
    // --------------- Вспомогательные параметры ---------------
    if (femObject->getParams().variables.size())
    {
        webOut += "<br>";
        webOut += tr("variables:");
        webOut += "<br>";
        for (auto it = femObject->getParams().variables.begin(); it != femObject->getParams().variables.end(); ++it)
        {
            webOut += tr("name: <b>%1</b>, value: <b>%2</b>").arg(it->first.c_str()).arg(it->second);
            webOut += "<br>";
        }
    }
    webOut += "</p>";
}

// Вывод информации о конкретном парметре расчета в табличной форме
void TMainWindow::sayParam(QString& webOut, const QString& title, int param, bool isDirect)
{
    unsigned num = 0;
    QString predicate;
    TFEMObject* femObject = femProcessor->getFEMObject();

    for (auto it = femObject->getParams().plist.begin(); it != femObject->getParams().plist.end(); it++)
        if (it->getType() == param)
        {
            num++;
            break;
        }
    if (num)
    {
        webOut += "<br>";
        webOut += tr(title.toUtf8());
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
        for (auto it = femObject->getParams().plist.begin(); it != femObject->getParams().plist.end(); it++)
        {
            if (it->getType() != param)
                continue;
            predicate = QString(it->getPredicate().c_str()).replace(QString("<="), QString("&lt;=")).replace(QString(">="), QString("&gt;="));
            webOut += QString("<tr><td>%1</td><td>%2</td><td>%3</td>").arg(num++).arg(it->getExpression().c_str()).arg(predicate);
            if (isDirect)
            {
                if ((it->getDirect() & DIR_X) == DIR_X)
                    webOut += QString("<td>%1</td>").arg("+");
                else
                    webOut += QString("<td>%1</td>").arg("");
                if ((it->getDirect() & DIR_Y) == DIR_Y)
                    webOut += QString("<td>%1</td>").arg("+");
                else if (femObject->getMesh().getFreedom() > 1)
                    webOut += QString("<td>%1</td>").arg("");
                if ((it->getDirect() & DIR_Z) == DIR_Z)
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
        addFuncToAnalyse(dlg->getName(),"");
    delete dlg;
}

bool TMainWindow::saveJSON(QString fileName)
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
    header.insert("Title", QJsonValue::fromVariant("QFEM problem file"));
    header.insert("Object", QJsonValue::fromVariant(femObject->getObjectName().c_str()));
    header.insert("DateTime", QJsonValue::fromVariant(dt.currentDateTime()));

    // ---------------- Сетка ------------------
    mesh.insert("File", QJsonValue::fromVariant(QString("%1").arg(femObject->getFileName().c_str())));

    // ------------- Параметры расчета ---------------------
    // Тип задачи
    params.insert("ProblemType", (femObject->getParams().fType == StaticProblem) ? QJsonValue::fromVariant("Static") : QJsonValue::fromVariant("Dynamic"));

    // Точность вычислений
    params.insert("Accuracy", QJsonValue::fromVariant(QString("%1").arg(femObject->getParams().eps)));

    // Параметры для динамического расчета
    time.insert("T0", QJsonValue::fromVariant(QString("%1").arg(femObject->getParams().t0)));
    time.insert("T1", QJsonValue::fromVariant(QString("%1").arg(femObject->getParams().t1)));
    time.insert("TH", QJsonValue::fromVariant(QString("%1").arg(femObject->getParams().th)));
    time.insert("WilsonTheta", QJsonValue::fromVariant(QString("%1").arg(femObject->getParams().theta)));
    params.insert("DynamicParameters", time);

    // Параметры вывода
    out.insert("Width", QJsonValue::fromVariant(QString("%1").arg(femObject->getParams().width)));
    out.insert("Precision", QJsonValue::fromVariant(QString("%1").arg(femObject->getParams().precision)));
    params.insert("OutputParameters", out);

    // Параметры нелинейного расчета
    nonlin.insert("CalculationMethod", QJsonValue::fromVariant(femObject->getParams().pMethod));
    nonlin.insert("LoadStep", QJsonValue::fromVariant(QString("%1").arg(femObject->getParams().loadStep)));
    params.insert("Nonlinearity", nonlin);

    // Краевые условия и прочие параметры
    for (auto it = femObject->getParams().plist.begin(); it != femObject->getParams().plist.end(); it++)
    {
        QJsonObject bc;

        bc.insert("Type", QJsonValue::fromVariant(QString("%1").arg(it->getType())));
        bc.insert("Direct", QJsonValue::fromVariant(QString("%1").arg(it->getDirect())));
        bc.insert("Predicate", QJsonValue::fromVariant(QString("%1").arg(it->getPredicate().c_str())));
        if (it->getType() == STRESS_STRAIN_CURVE_PARAMETER)
        {
            val = "{";
            for (unsigned i = 0; i < it->getStressStrainCurve().size1(); i++)
                val += QString("{%1, %2}%3 ").arg(it->getStressStrainCurve(i, 0)).arg(it->getStressStrainCurve(i, 1)).arg((i < it->getStressStrainCurve().size1() - 1) ? "," : "");
            val += "}";
            bc.insert("Expression", QJsonValue::fromVariant(val));
        }
        else
            bc.insert("Expression", QJsonValue::fromVariant(QString("%1").arg(it->getExpression().c_str())));

        lbc.push_back(bc);
    }
    params.insert("BoundaryConditions", lbc);

    // Названия
    for (unsigned i = 0; i < femObject->getParams().stdNames().size(); i++)
        names.push_back(QJsonValue::fromVariant(QString("%1").arg(femObject->getParams().names[i].c_str())));
    params.insert("Names", names);

    // Вспомагательные параметры (для парсера)
    for (auto it = femObject->getParams().variables.begin(); it != femObject->getParams().variables.end(); ++it)
        variables.push_back(QString("%1 %2").arg(it->first.c_str()).arg(it->second));
    params.insert("Variables", variables);

    // Формирование итогового документа
    main.insert("Header", header);
    main.insert("Mesh", mesh);
    main.insert("Parameters", params);
    doc.setObject(main);

    // Запись в файл
    file.setFileName(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
           return false;
    file.write(doc.toJson(), doc.toJson().length());
    file.close();
    return true;
}

bool TMainWindow::loadJSON(QString fileName)
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
    ParameterType type;
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
            type = ParameterType(bc.at(i)["Type"].toVariant().toInt());
            if (type == STRESS_STRAIN_CURVE_PARAMETER)
            {
                if (!tDlg->decodeStressStarinCurve(expr, ssc))
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

void TMainWindow::addFuncToAnalyse(QString funName,QString expression)
{
    TFEMObject* femObject = femProcessor->getFEMObject();
    bool isFind = false;

    // Проверка наличия такой функции в уже открытых закладках
    for (int i = 0; i < tabWidget->count(); i++)
        if (tabWidget->tabText(i).replace("&","") == funName)
        {
            isFind = true;
            tabWidget->setCurrentIndex(i);
        }
    if (!isFind)
    {
        tabWidget->addTab(new TGLFunction(&femObject->getMesh(),&femObject->getResult(), unsigned(femObject->getResult().index(funName.toStdString())),getTimeDeltaIndex(funName),expression,this),funName);
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
    bool isFunc = (qobject_cast<TGLFunction*>(tabWidget->currentWidget())) ? true : false;

    if (qobject_cast<TGLMesh*>(tabWidget->currentWidget()))
    {
        iDlg->changeLanguage();
        iDlg->setImageParams(qobject_cast<TGLMesh*>(tabWidget->currentWidget())->getImageParams(),int(femObject->getMesh().getFreedom()),isFunc);
        if (iDlg->exec() == QDialog::Accepted)
        {
            if ((iDlg->getImageParams().isLoad || iDlg->getImageParams().isLimit) && !qobject_cast<TGLMesh*>(tabWidget->widget(0))->isSelectedVertex())
            {
//                pb->show();
//                ui->actionStart->setEnabled(false);
//                ui->actionStop->setEnabled(true);
//                qobject_cast<GLObjWidget*>(tabWidget->widget(0))->setSelectedVertex();
//                ui->actionStop->setEnabled(false);
//                ui->actionStart->setEnabled(true);
//                pb->hide();
                lcProcess();
            }
            if (qobject_cast<TGLMesh*>(tabWidget->currentWidget()))
            {
                qobject_cast<TGLMesh*>(tabWidget->currentWidget())->setImageParams(iDlg->getImageParams());
                qobject_cast<TGLMesh*>(tabWidget->currentWidget())->repaint();
            }
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
                    calcExpression(exp,name);
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

bool TMainWindow::loadQRES(QString fileName)
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

void TMainWindow::slotAddFunction(void)
{
    TVCDialog* vdlg = new TVCDialog(this);
    QString name;

    vdlg->changeLanguage();
    if (vdlg->exec() == QDialog::Accepted)
    {
        if (calcExpression(vdlg->getExpression(),name))
            addFuncToAnalyse(name,vdlg->getExpression());
    }
    delete vdlg;
}

bool TMainWindow::calcExpression(QString expression ,QString& name)
{
    TFEMObject* femObject = femProcessor->getFEMObject();
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
        if ((results[i] = parser.run()) == 0.0)
            return false;
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
    TAppSetupDialog* dlg = new TAppSetupDialog(langNo, isBlackBkg, isAutoScroll, isAutoSaveProtocol, this);

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

    if ((isBlackBkg = dlg->getIsBlackBkg()) == true)
    {
        terminal->setStyleSheet("QTextEdit { background-color: rgb(0, 0, 0) }");
//        terminal->setPalette(QPalette(QPalette::Base, QColor("black")));
//        terminal->selectAll();
//        terminal->setTextColor(QColor("white"));
    }
    else
    {
        terminal->setStyleSheet("QTextEdit { background-color: rgb(255, 255, 255) }");
//        terminal->setPalette(QPalette(QPalette::Base, QColor("white")));
//        terminal->selectAll();
//        terminal->setTextColor(QColor("black"));
    }

    isAutoScroll = dlg->getIsAutoScroll();
    isAutoSaveProtocol = dlg->getIsAutoSaveProtocol();
}

void TMainWindow::slotSaveResults(void)
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("Saving results"),QString(QFileInfo(curFile).absolutePath() + "/" + QFileInfo(curFile).baseName() + "." + QString("txt").toLower()),tr("Results files (*.txt)"));

    if (!fileName.isEmpty())
        femProcessor->getFEMObject()->printResult(fileName.toStdString());
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
