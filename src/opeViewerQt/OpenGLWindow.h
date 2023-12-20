//
// Created by chudonghao on 2023/12/15.
//

#ifndef INC_2023_12_15_72CAE12A4E1D4DE0B477E7C3A954694G_H_
#define INC_2023_12_15_72CAE12A4E1D4DE0B477E7C3A954694G_H_

#include <QOpenGLWindow>

#include "WindowBase.hpp"

namespace opeViewerQt
{

class GraphicsWindow;

class OpenGLWindow : public WindowBase<QOpenGLWindow, opeViewer::Window>
{
    Q_OBJECT
    using Base = WindowBase;

  public:
    explicit OpenGLWindow(UpdateBehavior updateBehavior = UpdateBehavior::NoPartialUpdate, QWindow *parent = nullptr);

    explicit OpenGLWindow(QOpenGLContext *shareContext, UpdateBehavior updateBehavior = UpdateBehavior::NoPartialUpdate, QWindow *parent = nullptr);

    ~OpenGLWindow() override;
};

} // namespace opeViewerQt

#endif // INC_2023_12_15_72CAE12A4E1D4DE0B477E7C3A954694G_H_
