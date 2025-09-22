#-------------------------------------------------
#
# Project created by QtCreator 2010-12-30T09:27:42
#
#-------------------------------------------------

QT += core gui opengl widgets openglwidgets
TARGET = QFEM
TEMPLATE = app
DEFINES += BOOST_ALL_NO_LIB


INCLUDEPATH += core


#LIBS += -lopengl32
#msvc:LIBS += -lOpenGL32
#msvc:QMAKE_CXXFLAGS += /permissive-

#msvc:QMAKE_CXXFLAGS+= /openmp
#gcc:QMAKE_CXXFLAGS+= -fopenmp
#gcc:QMAKE_LFLAGS += -fopenmp

win32 {
    INCLUDEPATH += 'C:/Program Files (x86)/Intel/oneAPI/mkl/2025.2/include/' \
                   ../../eigen
    LIBS += -L'C:/Program Files (x86)/Intel/oneAPI/mkl/2025.2/lib/' -lmkl_core -lmkl_intel_lp64 -lmkl_sequential
}

unix {
    INCLUDEPATH += /opt/intel/oneapi/mkl/2025.2/include/ \
                   /usr/include/eigen3/
    LIBS += -L/opt/intel/oneapi/mkl/2025.2/lib/ -lmkl_intel_lp64 -lmkl_sequential -lmkl_core
}


SOURCES += gui/main.cpp\
    gui/bcprocessor.cpp \
    gui/funcview.cpp \
    gui/mainwindow.cpp \
    gui/meshview.cpp \
    gui/paramview.cpp \
    gui/problemsetupform.cpp \
    gui/fldialog.cpp \
    gui/setupimagedialog.cpp \
    gui/vcdialog.cpp \
    gui/gbdialog.cpp \
    gui/appsetupdialog.cpp \
    gui/helpdialog.cpp \
    gui/femprocessor.cpp \
    core/solver/bccsolver.cpp \
    core/solver/psolver.cpp \
    core/sparse/sparse_bccs.cpp \
    core/sparse/sparse_csr.cpp \
    core/solver/lzhsolver.cpp \
    core/solver/eigensolver.cpp \
    core/mesh/mesh.cpp \
    core/parser/parser.cpp \
    core/object/object.cpp \
    core/object/params.cpp

HEADERS  += gui/mainwindow.h \
    gui/bcprocessor.h \
    gui/funcview.h \
    gui/meshview.h \
    gui/paramview.h \
    gui/problemsetupform.h \
    gui/fldialog.h \
    gui/redirector.h \
    gui/setupimagedialog.h \
    gui/imageparams.h \
    gui/vcdialog.h \
    gui/gbdialog.h \
    gui/qmsg.h \
    gui/appesetupdialog.h \
    gui/helpdialog.h \
    gui/femprocessor.h \
    gui/terminal.h \
    core/solver/bccsolver.h \
    core/solver/psolver.h \
    core/sparse/sparse_bccs.h \
    core/sparse/sparse_csr.h \
    core/solver/lzhsolver.h \
    core/parser/defs.h \
    core/solver/eigensolver.h \
    core/solver/solver.h \
    core/analyse/analyse.h \
    core/msg/msg.h \
    core/mesh/mesh.h \
    core/parser/node.h \
    core/parser/parser.h \
    core/object/object.h \
    core/object/params.h \
    core/object/plist.h \
    core/util/matrix.h \
    core/fem/fem.h \
    core/fem/femstatic.h \
    core/fem/femstatic_mvs.h \
    core/fem/femdynamic.h \
    core/fe/shape.h \
    core/fe/fe.h \
    core/fe/fe1d.h \
    core/fe/fe2d.h \
    core/fe/fe2dp.h \
    core/fe/fe3d.h \
    core/fe/fe3ds.h

FORMS    += gui/mainwindow.ui \
    gui/problemsetupform.ui \
    gui/funlistdialog.ui \
    gui/setupimagedialog.ui \
    gui/vcdialog.ui \
    gui/gbdialog.ui \
    gui/appsetupdialog.ui \
    gui/helpdialog.ui

RESOURCES += gui/QFEM.qrc

win32 {
    RC_FILE += gui/QFEM.rc
}

TRANSLATIONS += gui/QFEM_RU.ts

OTHER_FILES += \
    gui/QFEM_RU.ts


# win:message(in scope win)
# unix:message(in scope unix)

