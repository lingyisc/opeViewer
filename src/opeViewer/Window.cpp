//
// Created by chudonghao on 2023/12/19.
//

#include "Window.h"

#include <osg/FrameStamp>
#include <osg/Stats>
#include <osg/TextureCubeMap>
#include <osg/TextureRectangle>
#include <osgDB/DatabasePager>
#include <osgDB/ImagePager>
#include <osgDB/Registry>
#include <osgGA/CameraManipulator>
#include <osgGA/EventVisitor>
#include <osgUtil/IncrementalCompileOperation>
#include <osgUtil/Statistics>
#include <osgUtil/UpdateVisitor>

#include "GraphicsWindow.h"
#include "Scene.h"
#include "Viewport.h"

namespace opeViewer
{

namespace
{

void generateSlavePointerData(osg::Camera *camera, osgGA::GUIEventAdapter &event)
{
    osg::GraphicsContext *gw = event.getGraphicsContext();
    if (!gw)
        return;

    // What type of Camera is it?
    // 1) Master Camera : do nothin extra
    // 2) Slave Camera, Relative RF, Same scene graph as master : transform coords into Master Camera and add to PointerData list
    // 3) Slave Camera, Relative RF, Different scene graph from master : do nothing extra?
    // 4) Slave Camera, Absolute RF, Same scene graph as master : do nothing extra?
    // 5) Slave Camera, Absolute RF, Different scene graph : do nothing extra?
    // 6) Slave Camera, Absolute RF, Different scene graph but a distortion correction subgraph depending upon RTT Camera (slave or master)
    //                              : project ray into RTT Camera's clip space, and RTT Camera's is Relative RF and sharing same scene graph as master then transform coords.

    // if camera isn't the master it must be a slave and could need reprojecting.

    auto view = camera->getView();
    if (!view)
        return;

    osg::Camera *view_masterCamera = view->getCamera();
    if (camera != view_masterCamera)
    {
        float x = event.getX();
        float y = event.getY();

        bool invert_y = event.getMouseYOrientation() == osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS;
        if (invert_y && gw->getTraits())
            y = gw->getTraits()->height - 1 - y;

        double master_min_x = -1.0;
        double master_max_x = 1.0;
        double master_min_y = -1.0;
        double master_max_y = 1.0;

        osg::Matrix masterCameraVPW = view_masterCamera->getViewMatrix() * view_masterCamera->getProjectionMatrix();
        if (view_masterCamera->getViewport())
        {
            osg::Viewport *viewport = view_masterCamera->getViewport();
            master_min_x = viewport->x();
            master_min_y = viewport->y();
            master_max_x = viewport->x() + viewport->width() - 1;
            master_max_y = viewport->y() + viewport->height() - 1;
            masterCameraVPW *= viewport->computeWindowMatrix();
        }

        // slave Camera tahnks to sharing the same View
        osg::View::Slave *slave = view->findSlaveForCamera(camera);
        if (slave)
        {
            if (camera->getReferenceFrame() == osg::Camera::RELATIVE_RF && slave->_useMastersSceneData)
            {
                osg::Viewport *viewport = camera->getViewport();
                osg::Matrix localCameraVPW = camera->getViewMatrix() * camera->getProjectionMatrix();
                if (viewport)
                    localCameraVPW *= viewport->computeWindowMatrix();

                osg::Matrix matrix(osg::Matrix::inverse(localCameraVPW) * masterCameraVPW);
                osg::Vec3d new_coord = osg::Vec3d(x, y, 0.0) * matrix;
                // OSG_NOTICE<<"    pointer event new_coord.x()="<<new_coord.x()<<" new_coord.y()="<<new_coord.y()<<std::endl;
                event.addPointerData(new osgGA::PointerData(view_masterCamera, new_coord.x(), master_min_x, master_max_x, new_coord.y(), master_min_y, master_max_y));
            }
            else if (!slave->_useMastersSceneData)
            {
                // Are their any RTT Camera's that this Camera depends upon for textures?

                osg::ref_ptr<osgUtil::LineSegmentIntersector> ray = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, x, y);
                osgUtil::IntersectionVisitor iv(ray.get());
                camera->accept(iv);
                if (ray->containsIntersections())
                {
                    osg::Vec3 tc;
                    osg::Texture *texture = ray->getFirstIntersection().getTextureLookUp(tc);
                    if (texture)
                    {
                        // look up Texture in RTT Camera's.
                        for (unsigned int i = 0; i < view->getNumSlaves(); ++i)
                        {
                            osg::Camera *slave_camera = view->getSlave(i)._camera.get();
                            if (slave_camera)
                            {
                                osg::Camera::BufferAttachmentMap::const_iterator ba_itr = slave_camera->getBufferAttachmentMap().find(osg::Camera::COLOR_BUFFER);
                                if (ba_itr != slave_camera->getBufferAttachmentMap().end())
                                {
                                    if (ba_itr->second._texture == texture)
                                    {
                                        osg::TextureRectangle *tr = dynamic_cast<osg::TextureRectangle *>(ba_itr->second._texture.get());
                                        osg::TextureCubeMap *tcm = dynamic_cast<osg::TextureCubeMap *>(ba_itr->second._texture.get());
                                        if (tr)
                                        {
                                            event.addPointerData(new osgGA::PointerData(slave_camera, tc.x(), 0.0f, static_cast<float>(tr->getTextureWidth()), tc.y(), 0.0f, static_cast<float>(tr->getTextureHeight())));
                                        }
                                        else if (tcm)
                                        {
                                            OSG_INFO << "  Slave has matched texture cubemap" << ba_itr->second._texture.get() << ", " << ba_itr->second._face << std::endl;
                                        }
                                        else
                                        {
                                            event.addPointerData(new osgGA::PointerData(slave_camera, tc.x(), 0.0f, 1.0f, tc.y(), 0.0f, 1.0f));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void generatePointerData(Window *window, osgGA::GUIEventAdapter &event)
{
    osg::GraphicsContext *gw = event.getGraphicsContext();
    if (!gw || window->getGraphicsContext() != gw)
        return;

    float x = event.getX();
    float y = event.getY();

    bool invert_y = event.getMouseYOrientation() == osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS;
    if (invert_y && gw->getTraits())
        y = gw->getTraits()->height - 1 - y;

    event.addPointerData(new osgGA::PointerData(gw, x, 0, gw->getTraits()->width - 1, y, 0, gw->getTraits()->height - 1));

    event.setMouseYOrientationAndUpdateCoords(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);

    typedef std::vector<osg::Camera *> CameraVector;
    CameraVector activeCameras;

    for (auto viewport : window->getViewports())
    {
        osg::Camera *camera = viewport->getCamera();
        if (camera->getAllowEventFocus() && camera->getRenderTargetImplementation() == osg::Camera::FRAME_BUFFER)
        {
            osg::Viewport *viewport = camera->getViewport();
            if (viewport && x >= viewport->x() && y >= viewport->y() && x < (viewport->x() + viewport->width()) && y < (viewport->y() + viewport->height()))
            {
                activeCameras.push_back(camera);
            }
        }
    }

    std::sort(activeCameras.begin(), activeCameras.end(), osg::CameraRenderOrderSortOp());

    osg::Camera *camera = activeCameras.empty() ? 0 : activeCameras.back();

    if (camera)
    {
        osg::Viewport *viewport = camera->getViewport();

        event.addPointerData(new osgGA::PointerData(camera, (x - viewport->x()) / (viewport->width() - 1) * 2.0f - 1.0f, -1.0, 1.0, (y - viewport->y()) / (viewport->height() - 1) * 2.0f - 1.0f, -1.0, 1.0));

        auto view = camera->getView();
        osg::Camera *view_masterCamera = view ? view->getCamera() : 0;

        // if camera isn't the master it must be a slave and could need reprojecting.
        if (view && camera != view_masterCamera)
        {
            generateSlavePointerData(camera, event);
        }
    }
}

void reprojectPointerData(osgGA::GUIEventAdapter &source_event, osgGA::GUIEventAdapter &dest_event)
{
    auto gw = dest_event.getGraphicsContext();
    if (!gw)
        return;

    float x = dest_event.getX();
    float y = dest_event.getY();

    bool invert_y = dest_event.getMouseYOrientation() == osgGA::GUIEventAdapter::Y_INCREASING_DOWNWARDS;
    if (invert_y && gw->getTraits())
        y = gw->getTraits()->height - 1 - y;

    dest_event.addPointerData(new osgGA::PointerData(gw, x, 0, gw->getTraits()->width - 1, y, 0, gw->getTraits()->height - 1));

    dest_event.setMouseYOrientationAndUpdateCoords(osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS);

    osg::Object *object = (source_event.getNumPointerData() >= 2) ? source_event.getPointerData(1)->object.get() : 0;
    osg::Camera *camera = object ? object->asCamera() : 0;
    osg::Viewport *viewport = camera ? camera->getViewport() : 0;

    if (!viewport)
        return;

    dest_event.addPointerData(new osgGA::PointerData(camera, (x - viewport->x()) / (viewport->width() - 1) * 2.0f - 1.0f, -1.0, 1.0, (y - viewport->y()) / (viewport->height() - 1) * 2.0f - 1.0f, -1.0, 1.0));

    auto view = camera->getView();
    osg::Camera *view_masterCamera = view ? view->getCamera() : 0;

    // if camera isn't the master it must be a slave and could need reprojecting.
    if (view && camera != view_masterCamera)
    {
        generateSlavePointerData(camera, dest_event);
    }
}

} // namespace

Window::Window() : Window(nullptr)
{
}

Window::Window(GraphicsWindow *graphicsContext) : _graphicsContext(graphicsContext)
{
    _startTick = osg::Timer::instance()->tick();
    _frameStamp = new osg::FrameStamp();
    _eventVisitor = new osgGA::EventVisitor;
    _updateVisitor = new osgUtil::UpdateVisitor;

    _accumulateEventState = new osgGA::GUIEventAdapter;

    _stats = new osg::Stats("Window");
}

Window::~Window()
{
}

osg::Object *Window::cloneType() const
{
    throw std::runtime_error("Not supported");
}

osg::Object *Window::clone(const osg::CopyOp &op) const
{
    throw std::runtime_error("Not supported");
}

const char *Window::libraryName() const
{
    return "opeViewer";
}

const char *Window::className() const
{
    return "Window";
}

void Window::requestContinuousUpdate(bool needed)
{
    _requestContinuousUpdate = needed;
}

void Window::requestWarpPointer(float x, float y)
{
    throw std::logic_error("not implemented");
}

bool Window::computeIntersections(const osgGA::GUIEventAdapter &adapter, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask mask)
{
    throw std::logic_error("not implemented");
}

bool Window::computeIntersections(const osgGA::GUIEventAdapter &adapter, const osg::NodePath &path, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask mask)
{
    throw std::logic_error("not implemented");
}

osg::GraphicsContext *Window::getGraphicsContext() const
{
    return _graphicsContext;
}

bool Window::addViewport(Viewport *viewport)
{
    if (_addViewportCallback)
    {
        return _addViewportCallback->addViewportImplementation(this, viewport);
    }
    else
    {
        return addViewportImplementation(viewport);
    }
}

bool Window::removeViewport(Viewport *viewport)
{
    if (_removeViewportCallback)
    {
        return _removeViewportCallback->removeViewportImplementation(this, viewport);
    }
    else
    {
        return removeViewportImplementation(viewport);
    }
}

void Window::setAddViewportCallback(Window::AddViewportCallback *callback)
{
    _addViewportCallback = callback;
}

Window::AddViewportCallback *Window::getAddViewportCallback()
{
    return _addViewportCallback;
}

const Window::AddViewportCallback *Window::getAddViewportCallback() const
{

    return _addViewportCallback;
}

bool Window::addViewportImplementation(Viewport *viewport)
{
    if (auto iter = std::find(_viewports.begin(), _viewports.end(), viewport); iter == _viewports.end())
    {
        _viewports.push_back(viewport);

        viewport->setWindow(this);

        // FIXME GL对象相关的管理函数怎么调用，如：resizeGLObjectBuffers releaseGLObjects
        //       \see osg::GraphicsContext::removeCamera
        //       \see osgViewer::CompositeViewer::addView
        //       \see osgViewer::CompositeViewer::removeView
        //       \see Window::removeViewportImplementation

        // if (viewport->getSceneData())
        // {
        //     // TODO camera?
        //     // update the scene graph so that it has enough GL object buffer memory for the graphics contexts that will be using it.
        //     viewport->getSceneData()->resizeGLObjectBuffers(osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts());
        // }
        // // TODO slave?

        viewport->setFrameStamp(_frameStamp.get());

        if (_inited)
        {
            viewport->init(_graphicsContext->getTraits()->width, _graphicsContext->getTraits()->height);
        }

        return true;
    }

    return false;
}

void Window::setRemoveViewportCallback(Window::RemoveViewportCallback *callback)
{
    _removeViewportCallback = callback;
}

Window::RemoveViewportCallback *Window::getRemoveViewportCallback()
{
    return _removeViewportCallback;
}

const Window::RemoveViewportCallback *Window::getRemoveViewportCallback() const
{
    return _removeViewportCallback;
}

bool Window::removeViewportImplementation(Viewport *viewport)
{
    if (auto iter = std::find(_viewports.begin(), _viewports.end(), viewport); iter != _viewports.end())
    {
        viewport->setFrameStamp(nullptr);

        // for (unsigned int i = 0; i < viewport->getNumSlaves(); ++i)
        // {
        //     viewport->getSlave(i)._camera->releaseGLObjects(_graphicsContext->getState());
        // }
        // viewport->getCamera()->releaseGLObjects(_graphicsContext->getState());

        viewport->setWindow(nullptr);

        _viewports.erase(iter);
        _viewportsRequestContinuousUpdate.erase(viewport);

        return true;
    }

    return false;
}

std::vector<Viewport *> Window::getViewports() const
{
    return {_viewports.begin(), _viewports.end()};
}

bool Window::contains(const Viewport *viewport) const
{
    return std::find(_viewports.begin(), _viewports.end(), viewport) != _viewports.end();
}

osg::Timer_t Window::getStartTick() const
{
    return _startTick;
}

double Window::elapsedTime()
{
    return osg::Timer::instance()->delta_s(_startTick, osg::Timer::instance()->tick());
}

osg::FrameStamp *Window::getFrameStamp() const
{
    return _frameStamp;
}

Window::ThreadingModel Window::getThreadingModel() const
{
    return _threadingModel;
}

void Window::setIncrementalCompileOperation(osgUtil::IncrementalCompileOperation *incrementalCompileOperation)
{
    _incrementalCompileOperation = incrementalCompileOperation;
}

osgUtil::IncrementalCompileOperation *Window::getIncrementalCompileOperation() const
{
    return _incrementalCompileOperation;
}

std::vector<Scene *> Window::getScenes(bool onlyValid)
{
    std::vector<Scene *> scenes;
    std::set<Scene *> sceneSet;

    for (auto &viewport : _viewports)
    {
        if (viewport->getScene() && (!onlyValid || viewport->getScene()->getSceneData()))
        {
            if (sceneSet.count(viewport->getScene()) == 0)
            {
                sceneSet.insert(viewport->getScene());
                scenes.push_back(viewport->getScene());
            }
        }
    }

    return scenes;
}

void Window::setRunFrameScheme(FrameScheme fs)
{
    _runFrameScheme = fs;
}

Window::FrameScheme Window::getRunFrameScheme() const
{
    return _runFrameScheme;
}

osg::Stats *Window::getStats() const
{
    return _stats;
}

void Window::setStatsCallback(StatsCallback *statsCallback)
{
    _statsCallback = statsCallback;
}

Window::StatsCallback *Window::getStatsCallback()
{
    return _statsCallback.get();
}

const Window::StatsCallback *Window::getStatsCallback() const
{
    return _statsCallback.get();
}

void Window::addEventHandler(osgGA::EventHandler *eventHandler)
{
    EventHandlers::iterator itr = std::find(_eventHandlers.begin(), _eventHandlers.end(), eventHandler);
    if (itr == _eventHandlers.end())
    {
        _eventHandlers.push_back(eventHandler);
    }
}

void Window::removeEventHandler(osgGA::EventHandler *eventHandler)
{
    EventHandlers::iterator itr = std::find(_eventHandlers.begin(), _eventHandlers.end(), eventHandler);
    if (itr != _eventHandlers.end())
    {
        _eventHandlers.erase(itr);
    }
}

Window::EventHandlers &Window::getEventHandlers()
{
    return _eventHandlers;
}

void Window::addUpdateOperation(osg::Operation *operation)
{
    if (!operation)
    {
        return;
    }

    if (!_updateOperations)
    {
        _updateOperations = new osg::OperationQueue;
    }

    _updateOperations->add(operation);
}

void Window::removeUpdateOperation(osg::Operation *operation)
{
    if (!operation)
    {
        return;
    }

    if (_updateOperations.valid())
    {
        _updateOperations->remove(operation);
    }
}

bool Window::event(osgGA::GUIEventAdapter &ea)
{
    // 更新参考时间
    _frameStamp->setReferenceTime(elapsedTime());

    double beginEvent = elapsedTime();

    bool focusMode = false;
    bool updateFocus = false;

    // 创建PointerData并预处理Focus
    switch (ea.getEventType())
    {
    case osgGA::GUIEventAdapter::KEYDOWN:
    case osgGA::GUIEventAdapter::KEYUP:
        focusMode = true;
        if (_accumulateEventState->getPointerDataList().empty())
        {
            generatePointerData(this, *_accumulateEventState);
        }
        ea.copyPointerDataFrom(*_accumulateEventState);
        break;
    case osgGA::GUIEventAdapter::PUSH:
    case osgGA::GUIEventAdapter::DOUBLECLICK:
    case osgGA::GUIEventAdapter::SCROLL:
        updateFocus = true;
    case osgGA::GUIEventAdapter::RELEASE:
        focusMode = true;
        generatePointerData(this, ea);
        _accumulateEventState->copyPointerDataFrom(ea);
        break;
    case osgGA::GUIEventAdapter::MOVE:
    case osgGA::GUIEventAdapter::DRAG:
        focusMode = true;
        if (_accumulateEventState->getNumPointerData() < 2)
        {
            generatePointerData(this, ea);
        }
        else
        {
            reprojectPointerData(*_accumulateEventState, ea);
        }
        break;
    default:
        ea.copyPointerDataFrom(*_accumulateEventState);
        break;
    }

    // 更新Focus
    if (updateFocus || !_focusedViewport)
    {
        osgGA::PointerData *pd = ea.getNumPointerData() > 0 ? ea.getPointerData(ea.getNumPointerData() - 1) : nullptr;
        osg::Object *object = pd ? pd->object.get() : nullptr;
        osg::Camera *camera = object ? object->asCamera() : nullptr;
        Viewport *focusedViewport = camera ? dynamic_cast<Viewport *>(camera->getView()) : nullptr;

        if (_focusedViewport != focusedViewport && focusedViewport)
        {
            _focusedViewport = focusedViewport;
        }
    }

    // 更新事件处理器
    _eventVisitor->setFrameStamp(getFrameStamp());
    _eventVisitor->setTraversalNumber(getFrameStamp()->getFrameNumber());

    auto traverseSceneAndCamera = [this, &ea](Viewport *viewport) {
        _eventVisitor->setActionAdapter(viewport);

        if (viewport && viewport->getSceneData())
        {
            _eventVisitor->reset();
            _eventVisitor->addEvent(&ea);
            viewport->getSceneData()->accept(*_eventVisitor);

            // Do EventTraversal for slaves with their own subgraph
            for (unsigned int i = 0; i < viewport->getNumSlaves(); ++i)
            {
                osg::View::Slave &slave = viewport->getSlave(i);
                osg::Camera *camera = slave._camera.get();
                if (camera && !slave._useMastersSceneData)
                {
                    camera->accept(*_eventVisitor);
                }
            }

            // call any camera event callbacks, but only traverse that callback, don't traverse its subgraph
            // leave that to the scene update traversal.
            osg::NodeVisitor::TraversalMode tm = _eventVisitor->getTraversalMode();
            _eventVisitor->setTraversalMode(osg::NodeVisitor::TRAVERSE_NONE);

            if (viewport->getCamera())
            {
                viewport->getCamera()->accept(*_eventVisitor);
            }

            for (unsigned int i = 0; i < viewport->getNumSlaves(); ++i)
            {
                osg::View::Slave &slave = viewport->getSlave(i);
                osg::Camera *camera = viewport->getSlave(i)._camera.get();
                if (camera && slave._useMastersSceneData)
                {
                    camera->accept(*_eventVisitor);
                }
            }

            _eventVisitor->setTraversalMode(tm);
        }
    };

    auto traverseEventHandlers = [this, &ea](Viewport *viewport) {
        _eventVisitor->setActionAdapter(viewport);
        for (auto hitr = viewport->getEventHandlers().begin(); hitr != viewport->getEventHandlers().end(); ++hitr)
        {
            (*hitr)->handle(&ea, viewport, _eventVisitor.get());
        }
    };

    auto traverseCameraManipulator = [this, &ea](Viewport *viewport) {
        _eventVisitor->setActionAdapter(viewport);
        if (viewport->getCameraManipulator())
        {
            viewport->getCameraManipulator()->handle(&ea, viewport, _eventVisitor.get());
        }
    };

    // 遍历事件处理器
    for (auto &eventHandler : _eventHandlers)
    {
        _eventVisitor->setActionAdapter(this);
        eventHandler->handle(&ea, this, _eventVisitor.get());
    }

    // 遍历视口
    // 1. 遍历场景和相机
    // 2. 遍历事件处理器
    // 3. 遍历相机操作器
    if (focusMode && _focusedViewport.get())
    {
        traverseSceneAndCamera(_focusedViewport.get());
        traverseEventHandlers(_focusedViewport.get());
        traverseCameraManipulator(_focusedViewport.get());
    }
    else
    {
        for (auto &viewport : _viewports)
        {
            traverseSceneAndCamera(viewport);
        }
        for (auto &viewport : _viewports)
        {
            traverseEventHandlers(viewport);
        }
        for (auto &viewport : _viewports)
        {
            traverseCameraManipulator(viewport);
        }
    }

    double endEvent = elapsedTime();

    if (_stats && _stats->collectStats("event"))
    {
        double beginEventTraversal = beginEvent;
        double endEventTraversal = endEvent;
        double timeTaken{};

        // 使用第一个事件的事件戳
        if (!_stats->getAttribute(_frameStamp->getFrameNumber(), "Event traversal begin time", beginEventTraversal))
        {
            _stats->setAttribute(_frameStamp->getFrameNumber(), "Event traversal begin time", beginEventTraversal);
        }

        // 使用最后一个事件的事件戳
        _stats->setAttribute(_frameStamp->getFrameNumber(), "Event traversal end time", endEventTraversal);

        // 累加事件时间
        _stats->getAttribute(_frameStamp->getFrameNumber(), "Event traversal time taken", timeTaken);
        _stats->setAttribute(_frameStamp->getFrameNumber(), "Event traversal time taken", timeTaken + endEvent - beginEvent);
    }

    return false;
}

bool Window::checkNeedToDoFrame()
{
    if (_runFrameScheme == CONTINUOUS)
    {
        return true;
    }

    if (_requestContinuousUpdate)
    {
        return true;
    }

    if (_updateOperations && !_updateOperations->empty())
    {
        return true;
    }

    if (!_viewportsRequestContinuousUpdate.empty())
    {
        return true;
    }

    for (auto viewport : _viewports)
    {
        if (viewport->requiresUpdateSceneGraph() || viewport->requiresRedraw())
        {
            return true;
        }
    }

    return false;
}

void Window::requestRedraw(Viewport *viewport)
{
    requestRedraw();
}

void Window::requestContinuousUpdate(Viewport *viewport, bool needed)
{
    // 记录需要持续更新的Viewport
    if (needed && contains(viewport))
    {
        _viewportsRequestContinuousUpdate.insert(viewport);
    }
    else
    {
        _viewportsRequestContinuousUpdate.erase(viewport);
    }
}

void Window::requestWarpPointer(Viewport *viewport, float x, float y)
{
    throw std::logic_error("not implemented");
}

void Window::statsImplementation()
{
    auto stats = getStats();
    if (!stats || !stats->collectStats("scene"))
    {
        return;
    }

    auto scenes = getScenes();
    auto frameNumber = getFrameStamp()->getFrameNumber();

    for (auto &scene : scenes)
    {
        osg::Stats *stats = scene->getStats();
        osg::Node *sceneRoot = scene->getSceneData();
        if (sceneRoot && stats)
        {
            osgUtil::StatsVisitor statsVisitor;
            sceneRoot->accept(statsVisitor);
            statsVisitor.totalUpStats();

            unsigned int unique_primitives = 0;
            osgUtil::Statistics::PrimitiveCountMap::iterator pcmitr;
            for (pcmitr = statsVisitor._uniqueStats.GetPrimitivesBegin(); pcmitr != statsVisitor._uniqueStats.GetPrimitivesEnd(); ++pcmitr)
            {
                unique_primitives += pcmitr->second;
            }

            stats->setAttribute(frameNumber, "Number of unique StateSet", static_cast<double>(statsVisitor._statesetSet.size()));
            stats->setAttribute(frameNumber, "Number of unique Group", static_cast<double>(statsVisitor._groupSet.size()));
            stats->setAttribute(frameNumber, "Number of unique Transform", static_cast<double>(statsVisitor._transformSet.size()));
            stats->setAttribute(frameNumber, "Number of unique LOD", static_cast<double>(statsVisitor._lodSet.size()));
            stats->setAttribute(frameNumber, "Number of unique Switch", static_cast<double>(statsVisitor._switchSet.size()));
            stats->setAttribute(frameNumber, "Number of unique Geode", static_cast<double>(statsVisitor._geodeSet.size()));
            stats->setAttribute(frameNumber, "Number of unique Drawable", static_cast<double>(statsVisitor._drawableSet.size()));
            stats->setAttribute(frameNumber, "Number of unique Geometry", static_cast<double>(statsVisitor._geometrySet.size()));
            stats->setAttribute(frameNumber, "Number of unique Vertices", static_cast<double>(statsVisitor._uniqueStats._vertexCount));
            stats->setAttribute(frameNumber, "Number of unique Primitives", static_cast<double>(unique_primitives));

            unsigned int instanced_primitives = 0;
            for (pcmitr = statsVisitor._instancedStats.GetPrimitivesBegin(); pcmitr != statsVisitor._instancedStats.GetPrimitivesEnd(); ++pcmitr)
            {
                instanced_primitives += pcmitr->second;
            }

            stats->setAttribute(frameNumber, "Number of instanced Stateset", static_cast<double>(statsVisitor._numInstancedStateSet));
            stats->setAttribute(frameNumber, "Number of instanced Group", static_cast<double>(statsVisitor._numInstancedGroup));
            stats->setAttribute(frameNumber, "Number of instanced Transform", static_cast<double>(statsVisitor._numInstancedTransform));
            stats->setAttribute(frameNumber, "Number of instanced LOD", static_cast<double>(statsVisitor._numInstancedLOD));
            stats->setAttribute(frameNumber, "Number of instanced Switch", static_cast<double>(statsVisitor._numInstancedSwitch));
            stats->setAttribute(frameNumber, "Number of instanced Geode", static_cast<double>(statsVisitor._numInstancedGeode));
            stats->setAttribute(frameNumber, "Number of instanced Drawable", static_cast<double>(statsVisitor._numInstancedDrawable));
            stats->setAttribute(frameNumber, "Number of instanced Geometry", static_cast<double>(statsVisitor._numInstancedGeometry));
            stats->setAttribute(frameNumber, "Number of instanced Vertices", static_cast<double>(statsVisitor._instancedStats._vertexCount));
            stats->setAttribute(frameNumber, "Number of instanced Primitives", static_cast<double>(instanced_primitives));
        }
    }
}

void Window::resized(int oldWidth, int oldHeight, int width, int height)
{
    for (auto &viewport : _viewports)
    {
        viewport->resized(oldWidth, oldHeight, width, height);
    }
}

void Window::init()
{
    if (_inited)
    {
        return;
    }
    _inited = true;

    for (auto viewport : _viewports)
    {
        viewport->init(_graphicsContext->getTraits()->width, _graphicsContext->getTraits()->height);
    }
}

void Window::frame()
{
    // 更新参考时间
    _frameStamp->setReferenceTime(elapsedTime());

    // 更新和绘制
    updateTraversal();
    renderingTraversals();
}

void Window::updateSimulationTime(double simulationTime)
{
    if (simulationTime == USE_ELAPSED_TIME)
    {
        _frameStamp->setSimulationTime(elapsedTime());
    }
    else
    {
        _frameStamp->setSimulationTime(simulationTime);
    }
}

void Window::advance()
{
    if (_stats && _stats->collectStats("frame_rate"))
    {
        double beginFrame{};
        // 获取上一帧的开始时间
        _stats->getAttribute(_frameStamp->getFrameNumber(), "Reference time", beginFrame);

        // 以当前时间作为上一帧的结束时间、下一帧的开始时间
        double endFrame = elapsedTime();

        // 记录上一帧的帧率
        double deltaFrameTime = endFrame - beginFrame;
        _stats->setAttribute(_frameStamp->getFrameNumber(), "Frame duration", deltaFrameTime);
        _stats->setAttribute(_frameStamp->getFrameNumber(), "Frame rate", 1.0 / deltaFrameTime);

        // 记录下一帧的开始时间
        _stats->setAttribute(_frameStamp->getFrameNumber() + 1, "Reference time", endFrame);
    }

    _frameStamp->setFrameNumber(_frameStamp->getFrameNumber() + 1);
}

void Window::updateTraversal()
{
    double beginUpdateTraversal = elapsedTime();

    _updateVisitor->reset();
    _updateVisitor->setFrameStamp(getFrameStamp());
    _updateVisitor->setTraversalNumber(getFrameStamp()->getFrameNumber());

    auto scenes = getScenes();
    for (auto &scene : scenes)
    {
        scene->updateSceneGraph(*_updateVisitor);
    }

    // if we have a shared state manager prune any unused entries
    if (osgDB::Registry::instance()->getSharedStateManager())
        osgDB::Registry::instance()->getSharedStateManager()->prune();

    // update the Registry object cache.
    osgDB::Registry::instance()->updateTimeStampOfObjectsInCacheWithExternalReferences(*getFrameStamp());
    osgDB::Registry::instance()->removeExpiredObjectsInCache(*getFrameStamp());

    if (_incrementalCompileOperation)
    {
        // merge subgraphs that have been compiled by the incremental compiler operation.
        _incrementalCompileOperation->mergeCompiledSubgraphs(getFrameStamp());
    }

    if (_updateOperations)
    {
        _updateOperations->runOperations(this);
    }

    for (auto viewport : _viewports)
    {
        {
            // Do UpdateTraversal for slaves with their own subgraph
            for (unsigned int i = 0; i < viewport->getNumSlaves(); ++i)
            {
                osg::View::Slave &slave = viewport->getSlave(i);
                osg::Camera *camera = slave._camera.get();
                if (camera && !slave._useMastersSceneData)
                {
                    camera->accept(*_updateVisitor);
                }
            }

            // call any camera update callbacks, but only traverse that callback, don't traverse its subgraph
            // leave that to the scene update traversal.
            osg::NodeVisitor::TraversalMode tm = _updateVisitor->getTraversalMode();
            _updateVisitor->setTraversalMode(osg::NodeVisitor::TRAVERSE_NONE);

            if (viewport->getCamera())
                viewport->getCamera()->accept(*_updateVisitor);

            for (unsigned int i = 0; i < viewport->getNumSlaves(); ++i)
            {
                osg::View::Slave &slave = viewport->getSlave(i);
                osg::Camera *camera = slave._camera.get();
                if (camera && slave._useMastersSceneData)
                {
                    camera->accept(*_updateVisitor);
                }
            }

            _updateVisitor->setTraversalMode(tm);
        }

        if (viewport->getCameraManipulator())
        {
            viewport->setFusionDistance(viewport->getCameraManipulator()->getFusionDistanceMode(), viewport->getCameraManipulator()->getFusionDistanceValue());

            viewport->getCameraManipulator()->updateCamera(*(viewport->getCamera()));
        }
        viewport->updateSlaves();
    }

    if (_stats && _stats->collectStats("update"))
    {
        double endUpdateTraversal = elapsedTime();

        // update current frames stats
        _stats->setAttribute(_frameStamp->getFrameNumber(), "Update traversal begin time", beginUpdateTraversal);
        _stats->setAttribute(_frameStamp->getFrameNumber(), "Update traversal end time", endUpdateTraversal);
        _stats->setAttribute(_frameStamp->getFrameNumber(), "Update traversal time taken", endUpdateTraversal - beginUpdateTraversal);
    }
}

void Window::renderingTraversals()
{
    double beginRenderingTraversals = elapsedTime();

    auto scenes = getScenes();
    for (auto scene : scenes)
    {
        osgDB::DatabasePager *dp = scene->getDatabasePager();
        if (dp)
        {
            dp->signalBeginFrame(_frameStamp);
        }

        osgDB::ImagePager *ip = scene->getImagePager();
        if (ip)
        {
            ip->signalBeginFrame(_frameStamp);
        }

        if (scene->getSceneData())
        {
            // fire off a build of the bounding volumes while we
            // are still running single threaded.
            scene->getSceneData()->getBound();
        }
    }

    _graphicsContext->makeCurrent();
    viewportsRenderingTraversals();
    _graphicsContext->runOperations();
    _graphicsContext->swapBuffers();
    _graphicsContext->releaseContext();

    for (auto &scene : scenes)
    {
        osgDB::DatabasePager *dp = scene->getDatabasePager();
        if (dp)
        {
            dp->signalEndFrame();
        }

        osgDB::ImagePager *ip = scene->getImagePager();
        if (ip)
        {
            ip->signalEndFrame();
        }
    }

    if (_stats && _stats->collectStats("update"))
    {
        auto frameNumber = _frameStamp->getFrameNumber();
        double endRenderingTraversals = elapsedTime();

        // update current frames stats
        _stats->setAttribute(frameNumber, "Rendering traversals begin time ", beginRenderingTraversals);
        _stats->setAttribute(frameNumber, "Rendering traversals end time ", endRenderingTraversals);
        _stats->setAttribute(frameNumber, "Rendering traversals time taken", endRenderingTraversals - beginRenderingTraversals);
    }

    if (_stats)
    {
        stats();
    }
}

void Window::viewportsRenderingTraversals()
{
    if (!_graphicsContext->getCameras().empty())
    {
        throw std::logic_error("don't call osg::Camera::setGraphicsContext any more");
    }

    std::vector<osg::Camera *> mainCameras(_viewports.size());
    std::transform(_viewports.begin(), _viewports.end(), mainCameras.begin(), [](Viewport *viewport) { return viewport->getCamera(); });
    std::sort(mainCameras.begin(), mainCameras.end(), osg::CameraRenderOrderSortOp());

    for (auto mainCamera : mainCameras)
    {
        auto view = mainCamera->getView();
        if (view->getNumSlaves())
        {
            std::vector<osg::Camera *> cameras(view->getNumSlaves() + 1);
            for (auto i = 0; i < view->getNumSlaves(); ++i)
            {
                cameras[i] = view->getSlave(i)._camera;
            }
            cameras.back() = mainCamera;
            std::sort(cameras.begin(), cameras.end(), osg::CameraRenderOrderSortOp());

            for (auto camera : cameras)
            {
                if (camera->getRenderer())
                {
                    (*camera->getRenderer())(_graphicsContext);
                }
            }
        }
        else
        {
            if (mainCamera->getRenderer())
            {
                (*mainCamera->getRenderer())(_graphicsContext);
            }
        }
    }
}

void Window::stats()
{
    if (_statsCallback)
    {
        _statsCallback->statsImplementation(this);
    }
    else
    {
        statsImplementation();
    }
}

} // namespace opeViewer
