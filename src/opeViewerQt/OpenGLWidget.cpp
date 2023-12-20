//
// Created by chudonghao on 2023/12/15.
//

#include "OpenGLWidget.h"

namespace opeViewerQt
{

OpenGLWidget::OpenGLWidget(QWidget *parent, const Qt::WindowFlags &f) : Base(parent, f)
{
    setup();
}

OpenGLWidget::~OpenGLWidget()
{
}

} // namespace opeViewerQt
