#-------------------------------------------------
#
# Project created by QtCreator 2010-12-30T09:27:42
#
#-------------------------------------------------

QT += core gui opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QFEM
TEMPLATE = app

INCLUDEPATH += ../core \
               ../../../../eigen



win32 {
    INCLUDEPATH += ../../../../intel/compilers_and_libraries_2019.5.281/windows/mkl/include/
    LIBS += -L$$PWD/../../../../intel/compilers_and_libraries_2019.5.281/windows/mkl/lib/intel64_win/ -lmkl_core -lmkl_intel_lp64 -lmkl_sequential
}

unix {
    INCLUDEPATH +=../../../../../intel/compilers_and_libraries_2019.5.281/linux/mkl/include/
    LIBS += -L$$PWD/../../../../../intel/mkl/lib/intel64/ -lmkl_intel_lp64 -lmkl_sequential -lmkl_core
}


SOURCES += main.cpp\
    ../core/solver/eigensolver.cpp \
    mainwindow.cpp \
    setuptaskdialog.cpp \
    fldialog.cpp \
    glfun.cpp \
    setupimagedialog.cpp \
    vcdialog.cpp \
    gbdialog.cpp \
    appsetupdialog.cpp \
    helpdialog.cpp \
    femprocessor.cpp \
    lcprocessor.cpp \
    glmesh.cpp \
    ../core/mesh/mesh.cpp \
    ../core/parser/binary.cpp \
    ../core/parser/parser.cpp \
    ../core/parser/tree.cpp \
    ../core/parser/unary.cpp \
    ../core/object/object.cpp \
    ../core/object/params.cpp \
    ../core/solver/bccsolver.cpp \
    ../core/sparse/sparse32.cpp \
    ../core/fe/shape.cpp

HEADERS  += mainwindow.h \
    ../core/solver/eigensolver.h \
    ../core/solver/solver.h \
    ../core/sparse/sparse32.h \
    qstdredirector.h \
    setuptaskdialog.h \
    fldialog.h \
    glfun.h \
    setupimagedialog.h \
    imageparams.h \
    vcdialog.h \
    gbdialog.h \
    qmsg.h \
    appesetupdialog.h \
    helpdialog.h \
    femprocessor.h \
    lcprocessor.h \
    terminal.h \
    lclist.h \
    glmesh.h \
    ../core/fem/femtstatic_t.h \
    ../core/analyse/analyse.h \
    ../core/msg/msg.h \
    ../core/mesh/mesh.h \
    ../core/parser/binary.h \
    ../core/parser/node.h \
    ../core/parser/parser.h \
    ../core/parser/real.h \
    ../core/parser/tree.h \
    ../core/parser/unary.h \
    ../core/object/object.h \
    ../core/object/params.h \
    ../core/object/plist.h \
    ../core/util/matrix.h \
    ../core/fem/fem.h \
    ../core/fem/femstatic.h \
    ../core/fem/femstatic_mvs.h \
    ../core/fem/femdynamic.h \
    ../core/solver/bccsolver.h \
    ../core/fe/shape.h \
    ../core/fe/fe.h \
    ../core/fe/fe1d.h \
    ../core/fe/fe2d.h \
    ../core/fe/fe2dp.h \
    ../core/fe/fe3d.h \
    ../core/fe/fe3ds.h

FORMS    += mainwindow.ui \
    setuptaskdialog.ui \
    funlistdialog.ui \
    setupimagedialog.ui \
    vcdialog.ui \
    gbdialog.ui \
    appsetupdialog.ui \
    helpdialog.ui

RESOURCES += QFEM.qrc

win32 {
    RC_FILE += QFEM.rc
}

TRANSLATIONS += QFEM_RU.ts

OTHER_FILES += \
    QFEM_RU.ts


# win:message(in scope win)
# unix:message(in scope unix)
