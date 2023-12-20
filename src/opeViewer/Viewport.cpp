//
// Created by chudonghao on 2023/12/18.
//

#include "Viewport.h"

#include <iostream>

#include <osgDB/DatabasePager>
#include <osgDB/ImagePager>
#include <osgGA/CameraManipulator>
#include <osgUtil/Optimizer>

#include "Renderer.h"
#include "Scene.h"
#include "Window.h"

namespace opeViewer
{

Viewport::Viewport()
{
    _frameStamp = new osg::FrameStamp;
    _frameStamp->setFrameNumber(0);
    _frameStamp->setReferenceTime(0);
    _frameStamp->setSimulationTime(0);

    _scene = new Scene;

    _stats = new osg::Stats("Viewport");

    // need to attach a Renderer to the master camera which has been default constructed
    _camera->setStats(_stats);
}

Viewport::~Viewport()
{
}

void Viewport::setWindow(Window *window)
{
    if (_window && window)
    {
        throw std::runtime_error("Window already set");
    }
    _window = window;
}

Window *Viewport::getWindow() const
{
    return _window;
}

void Viewport::requestRedraw()
{
    if (_window)
    {
        _window->requestRedraw(this);
    }
}

void Viewport::requestContinuousUpdate(bool needed)
{
    if (_window)
    {
        _window->requestContinuousUpdate(this, needed);
    }
}

void Viewport::requestWarpPointer(float x, float y)
{
    if (_window)
    {
        _window->requestWarpPointer(this, x, y);
    }
}

bool Viewport::computeIntersections(const osgGA::GUIEventAdapter &adapter, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask mask)
{
    throw std::logic_error("not implemented");
}

bool Viewport::computeIntersections(const osgGA::GUIEventAdapter &adapter, const osg::NodePath &path, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask mask)
{
    throw std::logic_error("not implemented");
}

bool Viewport::requiresUpdateSceneGraph() const
{
    // check if there are camera update callbacks
    if (_camera->getUpdateCallback())
    {
        return true;
    }

    // check if the scene requires an update traversal
    if (_scene.valid() && _scene->requiresUpdateSceneGraph())
    {
        return true;
    }

    return false;
}

bool Viewport::requiresRedraw() const
{ // check if the scene requires a redraw
    if (_scene.valid() && _scene->requiresRedraw())
    {
        return true;
    }

    return false;
}

void Viewport::setScene(Scene *scene)
{
    _scene = scene;

    prepareSceneData();

    // TODO
    // computeActiveCoordinateSystemNodePath();

    assignSceneDataToCameras();
}

Scene *Viewport::getScene()
{
    return _scene.get();
}

Scene *Viewport::getScene() const
{
    return _scene.get();
}

void Viewport::setSceneData(osg::Node *node)
{
    if (_setSceneDataCallback)
    {
        _setSceneDataCallback->setSceneDataImplementation(this, node);
    }
    else
    {
        setSceneDataImplementation(node);
    }
}

osg::Node *Viewport::getSceneData()
{
    return _scene.valid() ? _scene->getSceneData() : 0;
}

const osg::Node *Viewport::getSceneData() const
{
    return _scene.valid() ? _scene->getSceneData() : 0;
}

void Viewport::setSetSceneDataCallback(Viewport::SetSceneDataCallback *callback)
{
    _setSceneDataCallback = callback;
}

Viewport::SetSceneDataCallback *Viewport::getSetSceneDataCallback()
{
    return _setSceneDataCallback;
}

const Viewport::SetSceneDataCallback *Viewport::getSetSceneDataCallback() const
{
    return _setSceneDataCallback;
}

void Viewport::setSceneDataImplementation(osg::Node *node)
{
    if (_scene && node == _scene->getSceneData())
    {
        return;
    }

    if (!_scene)
    {
        _scene = new Scene;
    }

    osg::ref_ptr<Scene> scene = Scene::getScene(node);

    if (scene)
    {
        OSG_INFO << "Viewport::setSceneData() Sharing scene " << scene.get() << std::endl;
        _scene = scene;
    }
    else
    {
        if (_scene->referenceCount() != 1)
        {
            // we are not the only reference to the Scene so we cannot reuse it.
            _scene = new Scene;
            OSG_INFO << "Viewport::setSceneData() Allocating new scene" << _scene.get() << std::endl;
        }
        else
        {
            OSG_INFO << "Viewport::setSceneData() Reusing existing scene" << _scene.get() << std::endl;
        }

        _scene->setSceneData(node);
    }

    prepareSceneData();

    // TODO
    // computeActiveCoordinateSystemNodePath();

    assignSceneDataToCameras();
}

void Viewport::setDisplaySettings(osg::DisplaySettings *ds)
{
    _displaySettings = ds;
}

osg::DisplaySettings *Viewport::getDisplaySettings()
{
    return _displaySettings.get();
}

const osg::DisplaySettings *Viewport::getDisplaySettings() const
{
    return _displaySettings.get();
}

void Viewport::setFusionDistance(osgUtil::SceneView::FusionDistanceMode mode, float value)
{
    _fusionDistanceMode = mode;
    _fusionDistanceValue = value;
}

osg::GraphicsOperation *Viewport::createRenderer(osg::Camera *camera)
{
    if (camera->getRenderer())
    {
        return camera->getRenderer();
    }
    return new Renderer(camera);
}

osgUtil::SceneView::FusionDistanceMode Viewport::getFusionDistanceMode() const
{
    return _fusionDistanceMode;
}

float Viewport::getFusionDistanceValue() const
{
    return _fusionDistanceValue;
}

void Viewport::addEventHandler(osgGA::EventHandler *eventHandler)
{
    EventHandlers::iterator itr = std::find(_eventHandlers.begin(), _eventHandlers.end(), eventHandler);
    if (itr == _eventHandlers.end())
    {
        _eventHandlers.push_back(eventHandler);
    }
}

void Viewport::removeEventHandler(osgGA::EventHandler *eventHandler)
{
    EventHandlers::iterator itr = std::find(_eventHandlers.begin(), _eventHandlers.end(), eventHandler);
    if (itr != _eventHandlers.end())
    {
        _eventHandlers.erase(itr);
    }
}

Viewport::EventHandlers &Viewport::getEventHandlers()
{
    return _eventHandlers;
}

const Viewport::EventHandlers &Viewport::getEventHandlers() const
{
    return _eventHandlers;
}

void Viewport::setCameraManipulator(osgGA::CameraManipulator *manipulator, bool resetPosition)
{
    _cameraManipulator = manipulator;

    if (_cameraManipulator.valid())
    {
        // TODO
        // _cameraManipulator->setCoordinateFrameCallback(new ViewerCoordinateFrameCallback(this));

        if (getSceneData())
            _cameraManipulator->setNode(getSceneData());

        if (resetPosition)
        {
            osg::ref_ptr<osgGA::GUIEventAdapter> dummyEvent = new osgGA::GUIEventAdapter;
            _cameraManipulator->home(*dummyEvent, *this);
        }
    }
}

osgGA::CameraManipulator *Viewport::getCameraManipulator()
{
    return _cameraManipulator.get();
}

const osgGA::CameraManipulator *Viewport::getCameraManipulator() const
{
    return _cameraManipulator.get();
}

void Viewport::init(int width, int height)
{
    if (_initCallback)
    {
        _initCallback->initImplementation(this, width, height);
    }
    else
    {
        initImplementation(width, height);
    }
}

void Viewport::setInitCallback(Viewport::InitCallback *callback)
{
    _initCallback = callback;
}

Viewport::InitCallback *Viewport::getInitCallback()
{
    return _initCallback;
}

const Viewport::InitCallback *Viewport::getInitCallback() const
{
    return _initCallback;
}

void Viewport::initImplementation(int width, int height)
{
    // 必须得有一个Viewport
    // \note 先设置Viewport，再创建Renderer（可能在创建过程中使用相机宽高）
    if (!_camera->getViewport())
    {
        _camera->setViewport(0, 0, width, height);
    }

    // 必须得有一个Renderer
    if (!_camera->getRenderer())
    {
        _camera->setRenderer(createRenderer(getCamera()));
    }

    // 和Viewport使用相同的Stats
    if (!_camera->getStats())
    {
        _camera->setStats(_stats);
    }

    // 和Viewport使用相同的DisplaySettings
    if (!_camera->getDisplaySettings())
    {
        _camera->setDisplaySettings(_displaySettings);
    }

    osg::ref_ptr<osgGA::GUIEventAdapter> initEvent = new osgGA::GUIEventAdapter;
    initEvent->setEventType(osgGA::GUIEventAdapter::FRAME);

    if (_cameraManipulator.valid())
    {
        _cameraManipulator->init(*initEvent, *this);
    }
}

void Viewport::resized(int oldWidth, int oldHeight, int width, int height)
{
    if (_resizedCallback.valid())
        _resizedCallback->resizedImplementation(this, oldWidth, oldHeight, width, height);
    else
        resizedImplementation(oldWidth, oldHeight, width, height);
}

void Viewport::setResizedCallback(ResizedCallback *rc)
{
    _resizedCallback = rc;
}

Viewport::ResizedCallback *Viewport::getResizedCallback()
{
    return _resizedCallback.get();
}

const Viewport::ResizedCallback *Viewport::getResizedCallback() const
{
    return _resizedCallback.get();
}

void Viewport::resizedImplementation(int oldWidth, int oldHeight, int width, int height)
{
    std::set<osg::Viewport *> processedViewports;

    double widthChangeRatio = double(width) / oldWidth;
    double heigtChangeRatio = double(height) / oldHeight;
    double aspectRatioChange = widthChangeRatio / heigtChangeRatio;

    auto processCamera = [&](osg::Camera *camera) {
        // resize doesn't affect Cameras set up with FBO's.
        if (camera->getRenderTargetImplementation() == osg::Camera::FRAME_BUFFER_OBJECT)
        {
            return;
        }

        osg::Viewport *viewport = camera->getViewport();
        if (viewport && processedViewports.find(viewport) == processedViewports.end())
        {
            processedViewports.insert(viewport);

            if (viewport->x() == 0 && viewport->y() == 0 && viewport->width() >= oldWidth && viewport->height() >= oldHeight)
            {
                viewport->setViewport(0, 0, width, height);
            }
            else
            {
                viewport->x() = static_cast<osg::Viewport::value_type>(double(viewport->x()) * widthChangeRatio);
                viewport->y() = static_cast<osg::Viewport::value_type>(double(viewport->y()) * heigtChangeRatio);
                viewport->width() = static_cast<osg::Viewport::value_type>(double(viewport->width()) * widthChangeRatio);
                viewport->height() = static_cast<osg::Viewport::value_type>(double(viewport->height()) * heigtChangeRatio);
            }
        }

        if (aspectRatioChange == 1.0)
        {
            return;
        }

        osg::View::Slave *slave = findSlaveForCamera(camera);

        if (slave)
        {
            if (camera->getReferenceFrame() != osg::Transform::RELATIVE_RF)
            {
                switch (camera->getProjectionResizePolicy())
                {
                case (osg::Camera::HORIZONTAL):
                    camera->getProjectionMatrix() *= osg::Matrix::scale(1.0 / aspectRatioChange, 1.0, 1.0);
                    break;
                case (osg::Camera::VERTICAL):
                    camera->getProjectionMatrix() *= osg::Matrix::scale(1.0, aspectRatioChange, 1.0);
                    break;
                default:
                    break;
                }
            }
        }
        else
        {
            osg::Camera::ProjectionResizePolicy policy = camera->getProjectionResizePolicy();
            switch (policy)
            {
            case (osg::Camera::HORIZONTAL):
                camera->getProjectionMatrix() *= osg::Matrix::scale(1.0 / aspectRatioChange, 1.0, 1.0);
                break;
            case (osg::Camera::VERTICAL):
                camera->getProjectionMatrix() *= osg::Matrix::scale(1.0, aspectRatioChange, 1.0);
                break;
            default:
                break;
            }
        }
    };

    processCamera(_camera);
    for (auto &slave : _slaves)
    {
        processCamera(slave._camera);
    }
}

void Viewport::prepareSceneData()
{
    if (getSceneData())
    {
#if defined(OSG_GLES2_AVAILABLE)
        osgUtil::ShaderGenVisitor sgv;
        getSceneData()->getOrCreateStateSet();
        getSceneData()->accept(sgv);
#endif

        // now make sure the scene graph is set up with the correct DataVariance to protect the dynamic elements of
        // the scene graph from being run in parallel.
        osgUtil::Optimizer::StaticObjectDetectionVisitor sodv;
        getSceneData()->accept(sodv);

        // make sure that existing scene graph objects are allocated with thread safe ref/unref
        if (_window && _window->getThreadingModel() != Window::SingleThreaded)
        {
            getSceneData()->setThreadSafeRefUnref(true);
        }

        // update the scene graph so that it has enough GL object buffer memory for the graphics contexts that will be using it.
        getSceneData()->resizeGLObjectBuffers(osg::DisplaySettings::instance()->getMaxNumberOfGraphicsContexts());
    }
}

void Viewport::assignSceneDataToCameras()
{
    // OSG_NOTICE<<"View::assignSceneDataToCameras()"<<std::endl;

    if (_scene.valid() && _scene->getDatabasePager() && _window)
    {
        _scene->getDatabasePager()->setIncrementalCompileOperation(_window->getIncrementalCompileOperation());
    }

    osg::Node *sceneData = _scene.valid() ? _scene->getSceneData() : 0;

    if (_cameraManipulator.valid())
    {
        _cameraManipulator->setNode(sceneData);

        osg::ref_ptr<osgGA::GUIEventAdapter> dummyEvent = new osgGA::GUIEventAdapter;
        _cameraManipulator->home(*dummyEvent, *this);
    }

    if (_camera.valid())
    {
        _camera->removeChildren(0, _camera->getNumChildren());
        if (sceneData)
        {
            _camera->addChild(sceneData);
        }

        Renderer *renderer = dynamic_cast<Renderer *>(_camera->getRenderer());
        if (renderer)
        {
            renderer->setCompileOnNextDraw(true);
        }
    }

    for (unsigned i = 0; i < getNumSlaves(); ++i)
    {
        Slave &slave = getSlave(i);
        if (slave._camera.valid() && slave._useMastersSceneData)
        {
            slave._camera->removeChildren(0, slave._camera->getNumChildren());
            if (sceneData)
            {
                slave._camera->addChild(sceneData);
            }

            Renderer *renderer = dynamic_cast<Renderer *>(slave._camera->getRenderer());
            if (renderer)
            {
                renderer->setCompileOnNextDraw(true);
            }
        }
    }
}

} // namespace opeViewer
