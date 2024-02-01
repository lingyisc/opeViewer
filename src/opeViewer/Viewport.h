//
// Created by chudonghao on 2023/12/18.
//

#ifndef INC_2023_12_18_D5047186FF2E481CA338576FCA276A91_H_
#define INC_2023_12_18_D5047186FF2E481CA338576FCA276A91_H_

#include <osg/View>
#include <osgGA/GUIActionAdapter>
#include <osgUtil/SceneView>

namespace osgDB
{
class DatabasePager;
class ImagePager;
} // namespace osgDB

namespace osgGA
{
class CameraManipulator;
} // namespace osgGA

namespace opeViewer
{

class Scene;
class Window;

/// 视口
class Viewport : public osg::View, public osgGA::GUIActionAdapter
{
  public:
    struct InitCallback : osg::Referenced
    {
        virtual void initImplementation(Viewport *viewport, int width, int height) = 0;
    };

    struct ResizedCallback : osg::Referenced
    {
        virtual void resizedImplementation(Viewport *viewport, int oldWidth, int oldHeight, int width, int height) = 0;
    };

    struct SetSceneDataCallback : public osg::Referenced
    {
        virtual void setSceneDataImplementation(Viewport *viewport, osg::Node *node) = 0;
    };

    using EventHandlers = std::list<osg::ref_ptr<osgGA::EventHandler>>;

  protected:
    Window *_window{};

    osg::ref_ptr<Scene> _scene;

    EventHandlers _eventHandlers;
    osg::ref_ptr<osgGA::CameraManipulator> _cameraManipulator;

    osg::ref_ptr<osg::DisplaySettings> _displaySettings;
    osgUtil::SceneView::FusionDistanceMode _fusionDistanceMode{osgUtil::SceneView::PROPORTIONAL_TO_SCREEN_DISTANCE};
    float _fusionDistanceValue{1.f};

    osg::ref_ptr<InitCallback> _initCallback;
    osg::ref_ptr<ResizedCallback> _resizedCallback;
    osg::ref_ptr<SetSceneDataCallback> _setSceneDataCallback;

  public:
    Viewport();

    ~Viewport() override;

    void setWindow(Window *window);

    Window *getWindow() const;

    void requestRedraw() override;

    void requestContinuousUpdate(bool needed) override;

    void requestWarpPointer(float x, float y) override;

    bool computeIntersections(const osgGA::GUIEventAdapter &ea, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask mask) override;

    bool computeIntersections(const osgGA::GUIEventAdapter &ea, const osg::NodePath &path, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask mask) override;

    virtual bool requiresUpdateSceneGraph() const;

    virtual bool requiresRedraw() const;

    Scene *getScene();

    Scene *getScene() const;

    void setSceneData(osg::Node *node);

    osg::Node *getSceneData();

    const osg::Node *getSceneData() const;

    void setSetSceneDataCallback(SetSceneDataCallback *callback);

    SetSceneDataCallback *getSetSceneDataCallback();

    const SetSceneDataCallback *getSetSceneDataCallback() const;

    virtual void setSceneDataImplementation(osg::Node *node);

    void setDisplaySettings(osg::DisplaySettings *ds);

    osg::DisplaySettings *getDisplaySettings();

    const osg::DisplaySettings *getDisplaySettings() const;

    void setFusionDistance(osgUtil::SceneView::FusionDistanceMode mode, float value);

    osgUtil::SceneView::FusionDistanceMode getFusionDistanceMode() const;

    float getFusionDistanceValue() const;

    void addEventHandler(osgGA::EventHandler *eventHandler);

    void removeEventHandler(osgGA::EventHandler *eventHandler);

    EventHandlers &getEventHandlers();

    /** Get the const View's list of EventHandlers.*/
    const EventHandlers &getEventHandlers() const;

    void setCameraManipulator(osgGA::CameraManipulator *manipulator, bool resetPosition = true);

    osgGA::CameraManipulator *getCameraManipulator();

    const osgGA::CameraManipulator *getCameraManipulator() const;

    void init(int width, int height);

    void setInitCallback(InitCallback *callback);

    InitCallback *getInitCallback();

    const InitCallback *getInitCallback() const;

    virtual void initImplementation(int width, int height);

    // 由Window调用，提示窗口大小变更
    void resized(int oldWidth, int oldHeight, int width, int height);

    void setResizedCallback(ResizedCallback *rc);

    ResizedCallback *getResizedCallback();

    const ResizedCallback *getResizedCallback() const;

    virtual void resizedImplementation(int oldWidth, int oldHeight, int width, int height);

  protected:
    osg::GraphicsOperation *createRenderer(osg::Camera *camera) override;

    void prepareSceneData();

    void assignSceneDataToCameras();
};

} // namespace opeViewer

#endif // INC_2023_12_18_D5047186FF2E481CA338576FCA276A91_H_
