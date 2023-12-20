//
// Created by chudonghao on 2023/12/19.
//

#include "GraphicsWindowQt.h"

#include <QApplication>
#include <QOpenGLWidget>
#include <QOpenGLWindow>
#include <QScreen>

namespace opeViewerQt
{

GraphicsWindowQt::GraphicsWindowQt(QOpenGLWidget *widget) : _widget(widget)
{
    _state = new osg::State;
    _state->setGraphicsContext(this);
    _traits = new Traits;
}

GraphicsWindowQt::GraphicsWindowQt(QOpenGLWindow *window) : _window(window)
{
    _state = new osg::State;
    _state->setGraphicsContext(this);
    _traits = new Traits;
}

bool GraphicsWindowQt::realizeImplementation()
{
    auto format = _window ? _window->format() : _widget->format();

    // TODO shared context
    updateFrom(*_traits, format);
    updateGeometryFrom(*_traits, _window), /*or*/ updateGeometryFrom(*_traits, _widget);

    setupStateContextID();
    return GraphicsWindowEmbedded::realizeImplementation();
}

void GraphicsWindowQt::resizedImplementation(int x, int y, int width, int height)
{
    updateGeometryFrom(*_traits, _window), /*or*/ updateGeometryFrom(*_traits, _widget);

    GraphicsWindowEmbedded::resizedImplementation(_traits->x, _traits->y, _traits->width, _traits->height);
}

void updateFrom(osg::GraphicsContext::Traits &traits, const QSurfaceFormat &format)
{
    traits.red = format.redBufferSize();
    traits.green = format.greenBufferSize();
    traits.blue = format.blueBufferSize();
    traits.alpha = format.alphaBufferSize();
    traits.stencil = format.stencilBufferSize();
    traits.depth = format.depthBufferSize();
    traits.sampleBuffers = format.samples() > 1 ? 1 : 0;
    traits.samples = format.samples();
    traits.doubleBuffer = format.swapBehavior() == QSurfaceFormat::DoubleBuffer;
    traits.vsync = format.swapInterval() > 0;

    traits.glContextVersion = std::to_string(format.majorVersion()) + "." + std::to_string(format.minorVersion());
    // traits.glContextFlags = ds->getGLContextFlags();
    // traits.glContextProfileMask = ds->getGLContextProfileMask();

    // traits.swapMethod = ds->getSwapMethod();

    // traits.stereo
}

void updateGeometryFrom(osg::GraphicsContext::Traits &traits, const QWidget *widget)
{
    if (!widget)
    {
        return;
    }

    QScreen *screen = widget->windowHandle() && widget->windowHandle()->screen() ? widget->windowHandle()->screen() : qApp->screens().front();
    traits.x = widget->x() * screen->devicePixelRatio();
    traits.y = widget->y() * screen->devicePixelRatio();
    traits.width = widget->width() * screen->devicePixelRatio();
    traits.height = widget->height() * screen->devicePixelRatio();
}

void opeViewerQt::updateGeometryFrom(osg::GraphicsContext::Traits &traits, const QWindow *window)
{
    if (!window)
    {
        return;
    }

    QScreen *screen = window->screen() ? window->screen() : qApp->screens().front();
    traits.x = window->x() * screen->devicePixelRatio();
    traits.y = window->y() * screen->devicePixelRatio();
    traits.width = window->width() * screen->devicePixelRatio();
    traits.height = window->height() * screen->devicePixelRatio();
}

} // namespace opeViewerQt
