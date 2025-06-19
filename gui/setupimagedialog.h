#ifndef SETUPIMAGEDIALOG_H
#define SETUPIMAGEDIALOG_H

#include <QDialog>
//#include "imageparams.h"

class QXmlStreamWriter;
class TImageParams;
class TMeshView;

namespace Ui {
    class TSetupImageDialog;
}

using namespace std;

enum class ImageType { mesh, func, param };

class TSetupImageDialog : public QDialog
{
    Q_OBJECT

public slots:
    void accept(void);

public:
    explicit TSetupImageDialog(QWidget *parent = nullptr);
    ~TSetupImageDialog();
    void setup(ImageType);
    void clear(void);
    void initParams(void);
    void write(QDataStream& file);
    void read(QDataStream& file);
    // TImageParams& getImageParams(void)
    // {
    //     return params;
    // }
    void setImageParams(TImageParams* r, int pf, ImageType type)
    {
        dimension = pf;
        params = r;
        setup(type);
    }
    void setGLWidget(TMeshView* widget)
    {
        glWidget = widget;
    }
    void changeLanguage(void);

private slots:
    void slotTransformation(int);
    void slotScale(int);
    void slotChangeBkgColor(void);
    void slotChangeNumColor(int);
    void slotRotateX(int);
    void slotRotateY(int);
    void slotRotateZ(int);
    void slotTranslateX(int);
    void slotTranslateY(int);
    void slotIsMesh(void);
    void slotIsSurface(void);
    void slotIsSurfaceMesh(void);
    void slotIsAxis(void);
    void slotIsLight(void);
    void slotChangeAlpha(int);
    void slotIsAutoRotate(void);
    void slotIsShowLegend(void);
    void slotIsGrayscale(void);
    void slotIsColor(void);

private:
    QColor bkgColor;
    Ui::TSetupImageDialog *ui;
    TImageParams *params = nullptr;
    TMeshView* glWidget = nullptr;
    int dimension;
};

#endif // SETUPIMAGEDIALOG_H
