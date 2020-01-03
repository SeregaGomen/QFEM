TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

linux {
QMAKE_LFLAGS += -pthread
}



INCLUDEPATH += ../core \
               ../../../../eigen


SOURCES += main.cpp \
    ../core/fe/shape.cpp \
    ../core/mesh/mesh.cpp \
    ../core/object/object.cpp \
    ../core/object/params.cpp \
    ../core/parser/binary.cpp \
    ../core/parser/parser.cpp \
    ../core/parser/tree.cpp \
    ../core/parser/unary.cpp \
    ../core/solver/bccsolver.cpp \
    ../core/solver/elltsolver.cpp \
    ../core/sparse/sparse32.cpp \
    ../core/util/util.cpp

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
    ../core/solver/bccsolver.h \
    ../core/solver/elltsolver.h \
    ../core/sparse/private.h \
    ../core/sparse/sparse32.h \
    ../core/util/list.h \
    ../core/util/matrix.h \
    ../core/util/util.h
