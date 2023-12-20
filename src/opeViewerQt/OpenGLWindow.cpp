//
// Created by chudonghao on 2023/12/15.
//

#include "OpenGLWindow.h"

namespace opeViewerQt
{

OpenGLWindow::OpenGLWindow(QOpenGLContext *shareContext, UpdateBehavior updateBehavior, QWindow *parent) : Base(shareContext, updateBehavior, parent)
{
    setup();
}

OpenGLWindow::OpenGLWindow(UpdateBehavior updateBehavior, QWindow *parent) : OpenGLWindow(nullptr, updateBehavior, parent)
{
}

OpenGLWindow::~OpenGLWindow()
{
}

} // namespace opeViewerQt
