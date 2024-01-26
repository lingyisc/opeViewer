//
// Created by chudonghao on 2023/12/19.
//

#ifndef INC_2023_12_19_EA8AFC0C6616411AAF3414F8CCE2587F_H_
#define INC_2023_12_19_EA8AFC0C6616411AAF3414F8CCE2587F_H_

#include <limits>
#include <list>
#include <vector>

#include <osg/Object>
#include <osg/Timer>
#include <osg/Vec4>
#include <osg/observer_ptr>
#include <osg/ref_ptr>
#include <osgGA/GUIActionAdapter>

namespace osg
{
class FrameStamp;
class GraphicsContext;
class Operation;
class OperationQueue;
class Stats;
} // namespace osg

namespace osgGA
{
class EventHandler;
class EventVisitor;
class GUIEventAdapter;
} // namespace osgGA

namespace osgUtil
{
class UpdateVisitor;
class IncrementalCompileOperation;
} // namespace osgUtil

namespace opeViewer
{

class GraphicsWindow;
class Viewport;
class Scene;

constexpr double USE_ELAPSED_TIME = std::numeric_limits<double>::infinity();

class Window : public osg::Object, public osgGA::GUIActionAdapter
{
  public:
    enum ThreadingModel
    {
        SingleThreaded,
    };

    enum FrameScheme
    {
        ON_DEMAND,
        CONTINUOUS
    };

    // 每帧调用
    struct StatsCallback : osg::Referenced
    {
        virtual void statsImplementation(Window *window) = 0;
    };

    struct AddViewportCallback : osg::Referenced
    {
        virtual bool addViewportImplementation(Window *window, Viewport *viewport) = 0;
    };

    struct RemoveViewportCallback : osg::Referenced
    {
        virtual bool removeViewportImplementation(Window *window, Viewport *viewport) = 0;
    };

    using EventHandlers = std::list<osg::ref_ptr<osgGA::EventHandler>>;

  protected:
    osg::GraphicsContext *_graphicsContext{};
    std::vector<osg::ref_ptr<Viewport>> _viewports;
    std::set<Viewport *> _viewportsRequestContinuousUpdate{};

    bool _inited{};

    osg::Timer_t _startTick{};
    osg::ref_ptr<osg::FrameStamp> _frameStamp;
    osg::ref_ptr<osgGA::EventVisitor> _eventVisitor;
    osg::ref_ptr<osgUtil::UpdateVisitor> _updateVisitor;
    // TODO
    osg::ref_ptr<osgUtil::IncrementalCompileOperation> _incrementalCompileOperation;

    ThreadingModel _threadingModel{ThreadingModel::SingleThreaded};

    // 仅用于保存指针等需要长久驻留的信息
    osg::ref_ptr<osgGA::GUIEventAdapter> _accumulateEventState;

    osg::observer_ptr<Viewport> _focusedViewport;

    FrameScheme _runFrameScheme{CONTINUOUS};

    osg::ref_ptr<osg::Stats> _stats;
    osg::ref_ptr<StatsCallback> _statsCallback;
    osg::ref_ptr<AddViewportCallback> _addViewportCallback;
    osg::ref_ptr<RemoveViewportCallback> _removeViewportCallback;

    // 窗口级别的事件处理器
    EventHandlers _eventHandlers;
    // 做一些窗口级别的更新
    osg::ref_ptr<osg::OperationQueue> _updateOperations;

    bool _requestContinuousUpdate{false};

  public:
    Object *cloneType() const override;

    Object *clone(const osg::CopyOp &op) const override;

    const char *libraryName() const override;

    const char *className() const override;

    void requestContinuousUpdate(bool needed) override;

    void requestWarpPointer(float x, float y) override;

    bool computeIntersections(const osgGA::GUIEventAdapter &ea, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask mask) override;

    bool computeIntersections(const osgGA::GUIEventAdapter &ea, const osg::NodePath &path, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask mask) override;

    osg::GraphicsContext *getGraphicsContext() const;

    bool addViewport(Viewport *viewport);

    bool removeViewport(Viewport *viewport);

    void setAddViewportCallback(AddViewportCallback *callback);

    AddViewportCallback *getAddViewportCallback();

    const AddViewportCallback *getAddViewportCallback() const;

    virtual bool addViewportImplementation(Viewport *viewport);

    void setRemoveViewportCallback(RemoveViewportCallback *callback);

    RemoveViewportCallback *getRemoveViewportCallback();

    const RemoveViewportCallback *getRemoveViewportCallback() const;

    virtual bool removeViewportImplementation(Viewport *viewport);

    std::vector<Viewport *> getViewports() const;

    bool contains(const Viewport *viewport) const;

    osg::Timer_t getStartTick() const;

    double elapsedTime();

    osg::FrameStamp *getFrameStamp() const;

    ThreadingModel getThreadingModel() const;

    void setIncrementalCompileOperation(osgUtil::IncrementalCompileOperation *incrementalCompileOperation);

    osgUtil::IncrementalCompileOperation *getIncrementalCompileOperation() const;

    /// 获取相关视口使用的场景
    std::vector<Scene *> getScenes(bool onlyValid = true);

    void setRunFrameScheme(FrameScheme fs);

    FrameScheme getRunFrameScheme() const;

    osg::Stats *getStats() const;

    void setStatsCallback(StatsCallback *statsCallback);

    StatsCallback *getStatsCallback();

    const StatsCallback *getStatsCallback() const;

    void addEventHandler(osgGA::EventHandler *eventHandler);

    void removeEventHandler(osgGA::EventHandler *eventHandler);

    EventHandlers &getEventHandlers();

    void addUpdateOperation(osg::Operation *operation);

    void removeUpdateOperation(osg::Operation *operation);

    // 分发来自窗口系统的事件
    virtual bool event(osgGA::GUIEventAdapter &ea);

    virtual bool checkNeedToDoFrame();

    using osgGA::GUIActionAdapter::requestRedraw;

    virtual void requestRedraw(Viewport *viewport);

    virtual void requestContinuousUpdate(Viewport *viewport, bool needed);

    virtual void requestWarpPointer(Viewport *viewport, float x, float y);

  protected:
    Window();

    // \note Window不持有GraphicsWindow
    explicit Window(GraphicsWindow *graphicsContext);

    virtual ~Window();

    virtual void resized(int oldWidth, int oldHeight, int width, int height);

    virtual void init();

    // 更新仿真时间
    virtual void updateSimulationTime(double simulationTime = USE_ELAPSED_TIME);

    // 标记下一帧开始了
    // \note 不同于osgViewerBase，advance需要手动调用
    virtual void advance();

    virtual void frame();

    virtual void updateTraversal();

    virtual void renderingTraversals();

    virtual void viewportsRenderingTraversals();

    void stats();

    virtual void statsImplementation();
};

} // namespace opeViewer

#endif // INC_2023_12_19_EA8AFC0C6616411AAF3414F8CCE2587F_H_
