//
// Created by chudonghao on 2023/12/19.
//

#ifndef INC_2023_12_18_3447C60048E049FCB0AF38874D0210D8_H_
#define INC_2023_12_18_3447C60048E049FCB0AF38874D0210D8_H_

#include <QObject>
#include <osg/ref_ptr>

namespace osgGA
{
class GUIEventAdapter;
}

namespace opeViewer
{
class Window;
}

namespace opeViewerQt
{

/// 将Qt事件发送给OSG(Window)
class EventFilter : public QObject
{
    Q_OBJECT
    opeViewer::Window *_target{};
    osg::ref_ptr<osgGA::GUIEventAdapter> _accumulateEventState;
    qreal _devicePixelRatio{1};

  public:
    EventFilter(opeViewer::Window *target, QObject *parent);

    bool eventFilter(QObject *watched, QEvent *event) override;

    void setDevicePixelRatio(qreal devicePixelRatio);
};

} // namespace opeViewerQt

#endif // INC_2023_12_18_3447C60048E049FCB0AF38874D0210D8_H_
