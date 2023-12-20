//
// Created by chudonghao on 2023/12/18.
//

#ifndef INC_2023_12_18_CCB2436FDA1949C3AE24DA8FBB690A67_H_
#define INC_2023_12_18_CCB2436FDA1949C3AE24DA8FBB690A67_H_

#include <osg/GraphicsThread>

namespace osgUtil
{
class SceneView;
}

namespace opeViewer
{

class OpenGLQuerySupport;

/// 绘制器，属于相机
class Renderer : public osg::GraphicsOperation
{
  public:
    // 每帧调用
    struct StatsCallback : osg::Referenced
    {
        virtual void statsImplementation(Renderer *renderer, osgUtil::SceneView *sceneView) = 0;
    };

  protected:
    bool _initialized{};

    /// Renderer属于相机，所以用observer_ptr
    osg::observer_ptr<osg::Camera> _camera;
    osg::ref_ptr<osgUtil::SceneView> _sceneView;
    osg::ref_ptr<OpenGLQuerySupport> _querySupport;
    bool _compileOnNextDraw{true};

    osg::ref_ptr<StatsCallback> _statsCallback;

  public:
    explicit Renderer(osg::Camera *camera);

    ~Renderer() override;

    void operator()(osg::Object *object) override;

    void operator()(osg::GraphicsContext *context) override;

    osg::Camera *getCamera();

    void setCompileOnNextDraw(bool compileOnNextDraw);

    void setStatsCallback(StatsCallback *statsCallback);

    StatsCallback *getStatsCallback() const;

    virtual void statsImplementation(osgUtil::SceneView *sceneView);

  protected:
    void setupSceneView(osgUtil::SceneView *sceneView);

    virtual osgUtil::SceneView *createSceneView();

    virtual void initialize(osg::State *);

    void compile();

    void updateSceneView(osgUtil::SceneView *sceneView, osg::State *state);

    void stats();
};

} // namespace opeViewer

#endif // INC_2023_12_18_CCB2436FDA1949C3AE24DA8FBB690A67_H_
