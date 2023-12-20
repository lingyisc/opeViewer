//
// Created by chudonghao on 2023/12/19.
//

#include "EventFilter.h"

#include <unordered_map>

#include <QDebug>
#include <QEvent>
#include <QResizeEvent>
#include <osgGA/GUIEventAdapter>

#include <opeViewer/GraphicsWindow.h>
#include <opeViewer/Window.h>

namespace opeViewerQt
{

namespace
{
class KeyMap
{
    std::unordered_map<unsigned int, int> map;

  public:
    KeyMap()
    {
        map[Qt::Key_Escape] = osgGA::GUIEventAdapter::KEY_Escape;
        map[Qt::Key_Delete] = osgGA::GUIEventAdapter::KEY_Delete;
        map[Qt::Key_Home] = osgGA::GUIEventAdapter::KEY_Home;
        map[Qt::Key_Enter] = osgGA::GUIEventAdapter::KEY_KP_Enter;
        map[Qt::Key_End] = osgGA::GUIEventAdapter::KEY_End;
        map[Qt::Key_Return] = osgGA::GUIEventAdapter::KEY_Return;
        map[Qt::Key_PageUp] = osgGA::GUIEventAdapter::KEY_Page_Up;
        map[Qt::Key_PageDown] = osgGA::GUIEventAdapter::KEY_Page_Down;
        map[Qt::Key_Left] = osgGA::GUIEventAdapter::KEY_Left;
        map[Qt::Key_Right] = osgGA::GUIEventAdapter::KEY_Right;
        map[Qt::Key_Up] = osgGA::GUIEventAdapter::KEY_Up;
        map[Qt::Key_Down] = osgGA::GUIEventAdapter::KEY_Down;
        map[Qt::Key_Backspace] = osgGA::GUIEventAdapter::KEY_BackSpace;
        map[Qt::Key_Tab] = osgGA::GUIEventAdapter::KEY_Tab;
        map[Qt::Key_Space] = osgGA::GUIEventAdapter::KEY_Space;
        map[Qt::Key_Delete] = osgGA::GUIEventAdapter::KEY_Delete;
        map[Qt::Key_Alt] = osgGA::GUIEventAdapter::KEY_Alt_L;
        map[Qt::Key_Shift] = osgGA::GUIEventAdapter::KEY_Shift_L;
        map[Qt::Key_Control] = osgGA::GUIEventAdapter::KEY_Control_L;
        map[Qt::Key_Meta] = osgGA::GUIEventAdapter::KEY_Meta_L;
        map[Qt::Key_F1] = osgGA::GUIEventAdapter::KEY_F1;
        map[Qt::Key_F2] = osgGA::GUIEventAdapter::KEY_F2;
        map[Qt::Key_F3] = osgGA::GUIEventAdapter::KEY_F3;
        map[Qt::Key_F4] = osgGA::GUIEventAdapter::KEY_F4;
        map[Qt::Key_F5] = osgGA::GUIEventAdapter::KEY_F5;
        map[Qt::Key_F6] = osgGA::GUIEventAdapter::KEY_F6;
        map[Qt::Key_F7] = osgGA::GUIEventAdapter::KEY_F7;
        map[Qt::Key_F8] = osgGA::GUIEventAdapter::KEY_F8;
        map[Qt::Key_F9] = osgGA::GUIEventAdapter::KEY_F9;
        map[Qt::Key_F10] = osgGA::GUIEventAdapter::KEY_F10;
        map[Qt::Key_F11] = osgGA::GUIEventAdapter::KEY_F11;
        map[Qt::Key_F12] = osgGA::GUIEventAdapter::KEY_F12;
        map[Qt::Key_F13] = osgGA::GUIEventAdapter::KEY_F13;
        map[Qt::Key_F14] = osgGA::GUIEventAdapter::KEY_F14;
        map[Qt::Key_F15] = osgGA::GUIEventAdapter::KEY_F15;
        map[Qt::Key_F16] = osgGA::GUIEventAdapter::KEY_F16;
        map[Qt::Key_F17] = osgGA::GUIEventAdapter::KEY_F17;
        map[Qt::Key_F18] = osgGA::GUIEventAdapter::KEY_F18;
        map[Qt::Key_F19] = osgGA::GUIEventAdapter::KEY_F19;
        map[Qt::Key_F20] = osgGA::GUIEventAdapter::KEY_F20;
        map[Qt::Key_hyphen] = '-';
        map[Qt::Key_Equal] = '=';
        map[Qt::Key_division] = osgGA::GUIEventAdapter::KEY_KP_Divide;
        map[Qt::Key_multiply] = osgGA::GUIEventAdapter::KEY_KP_Multiply;
        map[Qt::Key_Minus] = '-';
        map[Qt::Key_Plus] = '+';
        map[Qt::Key_Insert] = osgGA::GUIEventAdapter::KEY_KP_Insert;
    }

    ~KeyMap()
    {
    }

    int operator()(Qt::Key key, QString text) const
    {
        if (auto itr = map.find(key); itr == map.end())
        {
            return text.size() == 1 ? int(*text.toLatin1().data()) : 0;
        }
        else
        {
            return itr->second;
        }
    }
};

class ButtonMap
{
  public:
    osgGA::GUIEventAdapter::MouseButtonMask operator[](Qt::MouseButton button) const
    {
        switch (button)
        {
        case Qt::LeftButton:
            return osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
        case Qt::MidButton:
            return osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;
        case Qt::RightButton:
            return osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
        case Qt::NoButton:
        default:
            return static_cast<osgGA::GUIEventAdapter::MouseButtonMask>(0);
        }
    }

    int operator[](Qt::MouseButtons buttons) const
    {
        int masks = 0;
        if (Qt::LeftButton & buttons)
        {
            masks |= osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON;
        }
        if (Qt::MidButton & buttons)
        {
            masks |= osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON;
        }
        if (Qt::RightButton & buttons)
        {
            masks |= osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON;
        }
        return masks;
    }
};

KeyMap g_keyMap;
ButtonMap g_buttonMap;

int mapToOsg(Qt::Key key, QString text)
{
    return g_keyMap(key, text);
}

int mapToOsg(Qt::MouseButton button)
{
    return g_buttonMap[button];
}

int mapToOsg(Qt::MouseButtons buttons)
{
    return g_buttonMap[buttons];
}

void updateModKeyMask(const QInputEvent *event, osgGA::GUIEventAdapter *ea)
{
    int mask = 0;
    if (event->modifiers() & Qt::ShiftModifier)
    {
        mask |= osgGA::GUIEventAdapter::MODKEY_SHIFT;
    }
    if (event->modifiers() & Qt::ControlModifier)
    {
        mask |= osgGA::GUIEventAdapter::MODKEY_CTRL;
    }
    if (event->modifiers() & Qt::AltModifier)
    {
        mask |= osgGA::GUIEventAdapter::MODKEY_ALT;
    }
    ea->setModKeyMask(mask);
}

} // namespace

EventFilter::EventFilter(opeViewer::Window *target, QObject *parent) : QObject(parent), _target(target)
{
    _accumulateEventState = new osgGA::GUIEventAdapter;
}

bool EventFilter::eventFilter(QObject *watched, QEvent *event)
{
    using namespace osgGA;

    osg::ref_ptr<GUIEventAdapter> ea;
    switch (event->type())
    {
    case QEvent::Paint: {
        ea = new GUIEventAdapter(*_accumulateEventState);
        ea->setEventType(GUIEventAdapter::FRAME);
        break;
    }
    case QEvent::Resize: {
        auto resizeEvent = static_cast<QResizeEvent *>(event);
        _accumulateEventState->setWindowRectangle(0, 0, resizeEvent->size().width() * _devicePixelRatio, resizeEvent->size().height() * _devicePixelRatio /*,todo*/);
        ea = new GUIEventAdapter(*_accumulateEventState);
        ea->setEventType(GUIEventAdapter::RESIZE);
        break;
    }
    case QEvent::KeyPress:
    case QEvent::KeyRelease: {
        auto keyEvent = static_cast<QKeyEvent *>(event);
        if (auto key = mapToOsg(static_cast<Qt::Key>(keyEvent->key()), keyEvent->text()))
        {
            updateModKeyMask(keyEvent, _accumulateEventState);
            ea = new GUIEventAdapter(*_accumulateEventState);
            ea->setEventType(keyEvent->type() == QEvent::KeyPress ? GUIEventAdapter::KEYDOWN : GUIEventAdapter::KEYUP);
            ea->setKey(key);
        }
        break;
    }
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseMove: {
        auto _mapToOsg = [](Qt::MouseButtons buttons, QEvent::Type type) {
            switch (type)
            {
            case QEvent::MouseButtonPress:
                return osgGA::GUIEventAdapter::PUSH;
            case QEvent::MouseButtonRelease:
                return osgGA::GUIEventAdapter::RELEASE;
            case QEvent::MouseButtonDblClick:
                return osgGA::GUIEventAdapter::DOUBLECLICK;
            case QEvent::MouseMove:
                return buttons ? osgGA::GUIEventAdapter::DRAG : osgGA::GUIEventAdapter::MOVE;
            default:
                return static_cast<GUIEventAdapter::EventType>(0);
            }
        };

        auto mouseEvent = static_cast<QMouseEvent *>(event);
        auto button = mapToOsg(mouseEvent->button());
        auto buttons = mapToOsg(mouseEvent->buttons());
        if (button || buttons || event->type() == QEvent::MouseMove)
        {
            updateModKeyMask(mouseEvent, _accumulateEventState);
            _accumulateEventState->setX(mouseEvent->x() * _devicePixelRatio);
            _accumulateEventState->setY(mouseEvent->y() * _devicePixelRatio);

            ea = new GUIEventAdapter(*_accumulateEventState);
            ea->setEventType(_mapToOsg(mouseEvent->buttons(), event->type()));
            ea->setButton(button);
            ea->setButtonMask(buttons);
        }
        break;
    }
    case QEvent::Wheel: {
        auto wheelEvent = static_cast<QWheelEvent *>(event);
        updateModKeyMask(wheelEvent, _accumulateEventState);
        _accumulateEventState->setX(wheelEvent->x() * _devicePixelRatio);
        _accumulateEventState->setY(wheelEvent->y() * _devicePixelRatio);

        auto o = wheelEvent->orientation();
        auto d = wheelEvent->delta();
        auto motion = o == Qt::Vertical ? (d > 0 ? osgGA::GUIEventAdapter::SCROLL_UP : osgGA::GUIEventAdapter::SCROLL_DOWN) : (d > 0 ? osgGA::GUIEventAdapter::SCROLL_LEFT : osgGA::GUIEventAdapter::SCROLL_RIGHT);

        ea = new GUIEventAdapter(*_accumulateEventState);
        ea->setEventType(GUIEventAdapter::SCROLL);
        ea->setButtonMask(mapToOsg(wheelEvent->buttons()));
        ea->setScrollingMotion(motion);
        break;
    }
    default:
        return false;
    }

    if (!ea)
    {
        return false;
    }

    ea->setGraphicsContext(_target->getGraphicsContext());
    ea->setTime(_target->elapsedTime());
    return _target->event(*ea);
}

void EventFilter::setDevicePixelRatio(qreal devicePixelRatio)
{
    _devicePixelRatio = devicePixelRatio;
}

} // namespace opeViewerQt
