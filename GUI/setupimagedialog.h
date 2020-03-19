#ifndef SETUPIMAGEDIALOG_H
#define SETUPIMAGEDIALOG_H

#include <QDialog>
#include <fstream>
#include "imageparams.h"

class QXmlStreamWriter;

namespace Ui {
    class TSetupImageDialog;
}

using namespace std;

enum { V_MESH, V_FUNC, V_PARAM };

class TSetupImageDialog : public QDialog
{
    Q_OBJECT

public slots:
    void accept(void);

public:
    explicit TSetupImageDialog(QWidget *parent = nullptr);
    ~TSetupImageDialog();
    void setup(int);
    void clear(void);
    void initParams(void);
    void write(QDataStream& file)
    {
        params.write(file);
    }
    void read(QDataStream& file)
    {
        params.read(file);
    }
    TImageParams& getImageParams(void)
    {
        return params;
    }
    void setImageParams(TImageParams& r, int pf, int type)
    {
        dimension = pf;
        params = r;
        setup(type);
    }
    void changeLanguage(void);

private slots:
    void slotTransformation(int);
    void slotScale(int);
    void slotChangeBkgColor(void);
    void slotRotateX(int);
    void slotRotateY(int);
    void slotRotateZ(int);
    void slotTranslateX(int);
    void slotTranslateY(int);
    void slotTranslateZ(int);

private:
    QColor bkgColor;
    Ui::TSetupImageDialog *ui;
    TImageParams params;
    int dimension;
    bool reload(void);
    void setTransformValue(void);
    void setScaleValue(void);
    void setAngleX(void);
    void setAngleY(void);
    void setAngleZ(void);
    void setTranslateX(void);
    void setTranslateY(void);
    void setTranslateZ(void);
};

#endif // SETUPIMAGEDIALOG_H
