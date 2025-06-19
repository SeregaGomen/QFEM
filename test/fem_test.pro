TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
DEFINES += BOOST_ALL_NO_LIB

linux {
    QMAKE_LFLAGS += -pthread
}

msvc:QMAKE_CXXFLAGS += /permissive-


INCLUDEPATH += ../core \
               ../../../eigen

#msvc:QMAKE_CXXFLAGS+= /openmp
#gcc:QMAKE_CXXFLAGS+= -fopenmp
#gcc:QMAKE_LFLAGS += -fopenmp

win32 {
    INCLUDEPATH += ../../../intel/compilers_and_libraries_2025.1/windows/mkl/include/
    LIBS += -L$$PWD/../../../intel/compilers_and_libraries_2025.1/windows/mkl/lib/intel64_win/ -lmkl_core -lmkl_intel_lp64 -lmkl_sequential
}

unix {
    INCLUDEPATH += /opt/intel/oneapi/mkl/2025.1/include/
    LIBS += -L/opt/intel/oneapi/mkl/2025.1/lib/intel64/ -lmkl_intel_lp64 -lmkl_sequential -lmkl_core
}

SOURCES += main.cpp \
    ../core/mesh/mesh.cpp \
    ../core/object/object.cpp \
    ../core/object/params.cpp \
    ../core/parser/parser.cpp \
    ../core/solver/bccsolver.cpp \
    ../core/solver/psolver.cpp \
    ../core/solver/eigensolver.cpp \
    ../core/sparse/sparse_bccs.cpp \
    ../core/sparse/sparse_csr.cpp \

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
    ../core/parser/defs.h \
    ../core/parser/node.h \
    ../core/parser/parser.h \
    ../core/solver/bccsolver.h \
    ../core/solver/solver.h \
    ../core/solver/eigensolver.h \
    ../core/sparse/sparse_bccs.h \
    ../core/sparse/sparse_csr.h \
    ../core/solver/psolver.h \
    ../core/util/list.h \
    ../core/util/matrix.h
