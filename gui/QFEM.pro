#-------------------------------------------------
#
# Project created by QtCreator 2010-12-30T09:27:42
#
#-------------------------------------------------

QT += core gui opengl widgets openglwidgets
TARGET = QFEM
TEMPLATE = app
DEFINES += BOOST_ALL_NO_LIB


INCLUDEPATH += ../core \
               ../../../eigen

LIBS += -lopengl32
#msvc:LIBS += -lOpenGL32
#msvc:QMAKE_CXXFLAGS += /permissive-

#msvc:QMAKE_CXXFLAGS+= /openmp
#gcc:QMAKE_CXXFLAGS+= -fopenmp
#gcc:QMAKE_LFLAGS += -fopenmp

win32 {
    INCLUDEPATH += 'C:/Program Files (x86)/Intel/oneAPI/mkl/2024.0/include/'
    LIBS += -L'C:/Program Files (x86)/Intel/oneAPI/mkl/2024.0/lib/' -lmkl_core -lmkl_intel_lp64 -lmkl_sequential
}

unix {
    INCLUDEPATH += /opt/intel/oneapi/mkl/2022.1.0/include/
    LIBS += -L/opt/intel/oneapi/mkl/2023.1.0/lib/ -lmkl_intel_lp64 -lmkl_sequential -lmkl_core
}


SOURCES += main.cpp\
    ../core/solver/bccsolver.cpp \
    ../core/solver/psolver.cpp \
    ../core/sparse/sparse_bccs.cpp \
    ../core/sparse/sparse_csr.cpp \
    bcprocessor.cpp \
    glparam.cpp \
    mainwindow.cpp \
    problemsetupform.cpp \
    fldialog.cpp \
    glfun.cpp \
    setupimagedialog.cpp \
    vcdialog.cpp \
    gbdialog.cpp \
    appsetupdialog.cpp \
    helpdialog.cpp \
    femprocessor.cpp \
    glmesh.cpp \
    ../core/solver/lzhsolver.cpp \
    ../core/solver/eigensolver.cpp \
    ../core/mesh/mesh.cpp \
    ../core/parser/parser.cpp \
    ../core/object/object.cpp \
    ../core/object/params.cpp

HEADERS  += mainwindow.h \
    ../core/solver/bccsolver.h \
    ../core/solver/psolver.h \
    ../core/sparse/sparse_bccs.h \
    ../core/sparse/sparse_csr.h \
    bcprocessor.h \
    glparam.h \
    problemsetupform.h \
    qstdredirector.h \
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
    terminal.h \
    glmesh.h \
    ../core/solver/lzhsolver.h \
    ../core/parser/defs.h \
    ../core/solver/eigensolver.h \
    ../core/solver/solver.h \
    ../core/analyse/analyse.h \
    ../core/msg/msg.h \
    ../core/mesh/mesh.h \
    ../core/parser/node.h \
    ../core/parser/parser.h \
    ../core/object/object.h \
    ../core/object/params.h \
    ../core/object/plist.h \
    ../core/util/matrix.h \
    ../core/fem/fem.h \
    ../core/fem/femstatic.h \
    ../core/fem/femstatic_mvs.h \
    ../core/fem/femdynamic.h \
    ../core/fe/shape.h \
    ../core/fe/fe.h \
    ../core/fe/fe1d.h \
    ../core/fe/fe2d.h \
    ../core/fe/fe2dp.h \
    ../core/fe/fe3d.h \
    ../core/fe/fe3ds.h

FORMS    += mainwindow.ui \
    problemsetupform.ui \
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

