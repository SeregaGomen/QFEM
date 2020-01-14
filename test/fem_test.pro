TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

linux {
QMAKE_LFLAGS += -pthread
}



INCLUDEPATH += ../core \
               ../../../eigen

msvc:QMAKE_CXXFLAGS+= /openmp
gcc:QMAKE_CXXFLAGS+= -fopenmp
gcc:QMAKE_LFLAGS += -fopenmp

win32 {
    INCLUDEPATH += ../../../intel/compilers_and_libraries_2019.5.281/windows/mkl/include/
    LIBS += -L$$PWD/../../../intel/compilers_and_libraries_2019.5.281/windows/mkl/lib/intel64_win/ -lmkl_core -lmkl_intel_lp64 -lmkl_sequential
}

unix {
    INCLUDEPATH +=../../../../intel/compilers_and_libraries_2019.5.281/linux/mkl/include/
    LIBS += -L$$PWD/../../../../intel/mkl/lib/intel64/ -lmkl_intel_lp64 -lmkl_sequential -lmkl_core
}

SOURCES += main.cpp \
    ../core/fe/shape.cpp \
    ../core/mesh/mesh.cpp \
    ../core/object/object.cpp \
    ../core/object/params.cpp \
    ../core/parser/binary.cpp \
    ../core/parser/parser.cpp \
    ../core/parser/tree.cpp \
    ../core/parser/unary.cpp \
    ../core/solver/eigensolver.cpp


HEADERS += \
    ../core/analyse/analyse.h \
    ../core/fe/fe.h \
    ../core/fe/fe1d.h \
    ../core/fe/fe2d.h \
    ../core/fe/fe2dp.h \
    ../core/fe/fe3d.h \
    ../core/fe/fe3ds.h \
    ../core/fe/shape.h \
    ../core/fem/fem.h \
    ../core/fem/femdynamic.h \
    ../core/fem/femstatic.h \
    ../core/fem/femstatic_mvs.h \
    ../core/mesh/mesh.h \
    ../core/msg/msg.h \
    ../core/object/object.h \
    ../core/object/params.h \
    ../core/object/plist.h \
    ../core/parser/binary.h \
    ../core/parser/node.h \
    ../core/parser/parser.h \
    ../core/parser/real.h \
    ../core/parser/tree.h \
    ../core/parser/unary.h \
    ../core/solver/solver.h \
    ../core/solver/eigensolver.h \
    ../core/util/list.h \
    ../core/util/matrix.h
