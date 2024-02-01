//
// Created by chudonghao on 2023/12/15.
//

#ifndef INC_2023_12_15_72CAE12A4E1D4DE0B477E7C3A954694H_H_
#define INC_2023_12_15_72CAE12A4E1D4DE0B477E7C3A954694H_H_

#include <QApplication>
#include <QObject>
#include <QScreen>
#include <QTimerEvent>

#include <osgGA/GUIActionAdapter>

#include <opeViewer/Window.h>

#include "EventFilter.h"
#include "GraphicsWindowQt.h"

namespace opeViewerQt
{

template <typename Base, typename Window>
class WindowBase : public Base, public Window
{
  protected:
    osg::ref_ptr<GraphicsWindowQt> _graphicsWindowQt{};
    int _updateTimer{};
    EventFilter *_eventFilter{};

  public:
    using Base::Base;

    /// 在构造函数中调用
    void setup();

    void requestRedraw() override;

  protected:
    using Window::resized;

    bool event(QEvent *event) override;

    void initializeGL() override;

    void resizeGL(int w, int h) override;

    void paintGL() override;

    void timerEvent(QTimerEvent *event) override;
};

template <typename Base, typename Window>
bool WindowBase<Base, Window>::event(QEvent *event)
{
    return _eventFilter->eventFilter(this, event) || Base::event(event);
}

template <typename Base, typename Window>
void WindowBase<Base, Window>::setup()
{
    _eventFilter = new EventFilter(this, nullptr);
    _eventFilter->setParent(this);
    _graphicsWindowQt = new GraphicsWindowQt(this);
    this->_graphicsContext = _graphicsWindowQt;
    _updateTimer = this->startTimer(1);

    this->connect(this, &Base::frameSwapped, [this] { this->advance(); });
}

template <typename Base, typename Window>
void WindowBase<Base, Window>::requestRedraw()
{
    // qDebug() << "update";
    this->update();
}

template <typename Base, typename Window>
void WindowBase<Base, Window>::initializeGL()
{
    _graphicsWindowQt->realize();
    _graphicsWindowQt->setDefaultFboId(this->defaultFramebufferObject());
    this->devicePixelRatioF();

    this->init();
    this->updateSimulationTime();
    this->advance();
}

template <typename Base, typename Window>
void WindowBase<Base, Window>::resizeGL(int w, int h)
{
    int oldWidth = this->_graphicsContext->getTraits()->width;
    int oldHeight = this->_graphicsContext->getTraits()->height;
    _graphicsWindowQt->resized(0, 0, w, h);
    int newWidth = this->_graphicsContext->getTraits()->width;
    int newHeight = this->_graphicsContext->getTraits()->height;

    this->resized(oldWidth, oldHeight, newWidth, newHeight);
}

template <typename Base, typename Window>
void WindowBase<Base, Window>::paintGL()
{
    _graphicsWindowQt->setDefaultFboId(this->defaultFramebufferObject());
    this->frame();
}

template <typename Base, typename Window>
void WindowBase<Base, Window>::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == _updateTimer)
    {
        this->updateSimulationTime();
        if (this->checkNeedToDoFrame())
        {
            // qDebug() << "update";
            this->update();
        }
    }
}

} // namespace opeViewerQt

#endif // INC_2023_12_15_72CAE12A4E1D4DE0B477E7C3A954694H_H_
