//
// Created by chudonghao on 2023/12/15.
//

#ifndef INC_2023_12_15_72CAE12A4E1D4DE0B477E7C3A954694F_H_
#define INC_2023_12_15_72CAE12A4E1D4DE0B477E7C3A954694F_H_

#include <QOpenGLWidget>

#include "WindowBase.hpp"

namespace opeViewerQt
{

class GraphicsWindow;

class OpenGLWidget : public WindowBase<QOpenGLWidget, opeViewer::Window>
{
    Q_OBJECT
    using Base = WindowBase;

  public:
    explicit OpenGLWidget(QWidget *parent = nullptr, const Qt::WindowFlags &f = Qt::WindowFlags());

    ~OpenGLWidget() override;
};

} // namespace opeViewerQt

#endif // INC_2023_12_15_72CAE12A4E1D4DE0B477E7C3A954694F_H_
