//
// Created by chudonghao on 2023/12/19.
//

#ifndef INC_2023_12_19_DE3136157A2849488FEEFDC07DDF1EAD_H_
#define INC_2023_12_19_DE3136157A2849488FEEFDC07DDF1EAD_H_

#include <QOpenGLWindow>

#include <opeViewer/GraphicsWindowEmbedded.h>

namespace opeViewerQt
{

class GraphicsWindowQt : public opeViewer::GraphicsWindowEmbedded
{
    QOpenGLWidget *_widget{};
    QOpenGLWindow *_window{};

  public:
    explicit GraphicsWindowQt(QOpenGLWidget *widget);

    explicit GraphicsWindowQt(QOpenGLWindow *window);

    bool realizeImplementation() override;

    void resizedImplementation(int x, int y, int width, int height) override;
};

void updateFrom(osg::GraphicsContext::Traits &traits, const QSurfaceFormat &format);

void updateGeometryFrom(osg::GraphicsContext::Traits &traits, const QWidget *widget);

void updateGeometryFrom(osg::GraphicsContext::Traits &traits, const QWindow *window);

} // namespace opeViewerQt

#endif // INC_2023_12_19_DE3136157A2849488FEEFDC07DDF1EAD_H_
