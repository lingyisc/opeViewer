//
// Created by chudonghao on 2023/12/18.
//

#include "Renderer.h"

#include <osg/GraphicsContext>
#include <osgDB/DatabasePager>
#include <osgDB/ImagePager>
#include <osgUtil/GLObjectsVisitor>
#include <osgUtil/SceneView>
#include <osgUtil/Statistics>

#include "OpenGLQuerySupport.h"
#include "Scene.h"
#include "Viewport.h"
#include "Window.h"

// #define DEBUG_MESSAGE OSG_NOTICE
#define DEBUG_MESSAGE OSG_DEBUG

namespace opeViewer
{

Renderer::Renderer(osg::Camera *camera) : /*osg::Referenced(true),*/ osg::GraphicsOperation("Renderer", true), _camera(camera)
{
}

Renderer::~Renderer()
{
}

void Renderer::operator()(osg::Object *object)
{
    osg::GraphicsContext *context = dynamic_cast<osg::GraphicsContext *>(object);
    if (context)
    {
        operator()(context);
    }
}

void Renderer::operator()(osg::GraphicsContext *context)
{
    struct CollectStats
    {
        Renderer *thiz;
        osgUtil::SceneView *sceneView{};
        OpenGLQuerySupport *querySupport{};

        osg::Stats *stats{};
        osg::State *state{};
        osg::Timer_t startTick{};
        unsigned int frameNumber{};
        bool acquireGPUStats{};

        osg::Timer_t beforeCullTick{};
        osg::Timer_t afterCullTick{};
        osg::Timer_t beforeDrawTick{};
        osg::Timer_t afterDrawTick{};

        void init(Renderer *thiz, osgUtil::SceneView *sceneView, OpenGLQuerySupport *querySupport)
        {
            auto camera = sceneView->getCamera();
            auto viewport = dynamic_cast<Viewport *>(camera->getView());
            auto window = viewport ? viewport->getWindow() : nullptr;
            auto fs = sceneView->getFrameStamp();

            this->thiz = thiz;
            this->sceneView = sceneView;
            this->querySupport = querySupport;
            this->state = sceneView->getState();
            this->stats = camera->getStats();
            this->frameNumber = fs ? fs->getFrameNumber() : 0;
            this->acquireGPUStats = stats && querySupport && stats->collectStats("gpu");
            this->startTick = window ? window->getStartTick() : 0;
        }

        void beforeCull()
        {
            if (acquireGPUStats)
            {
                querySupport->checkQuery(stats, state, startTick);
            }

            beforeCullTick = osg::Timer::instance()->tick();
        }

        void afterCull()
        {
            afterCullTick = osg::Timer::instance()->tick();
        }

        void beforeDraw()
        {
            // do draw traversal
            if (acquireGPUStats)
            {
                querySupport->checkQuery(stats, state, startTick);
                querySupport->beginQuery(frameNumber, state);
            }

            beforeDrawTick = osg::Timer::instance()->tick();
        }

        void afterDraw()
        {
            if (acquireGPUStats)
            {
                querySupport->endQuery(state);
                querySupport->checkQuery(stats, state, startTick);
            }

            afterDrawTick = osg::Timer::instance()->tick();
        }

        void collect()
        {
            thiz->stats();

            if (stats && stats->collectStats("rendering"))
            {
                DEBUG_MESSAGE << "Collecting rendering stats" << std::endl;

                stats->setAttribute(frameNumber, "Cull traversal begin time", osg::Timer::instance()->delta_s(startTick, beforeCullTick));
                stats->setAttribute(frameNumber, "Cull traversal end time", osg::Timer::instance()->delta_s(startTick, afterCullTick));
                stats->setAttribute(frameNumber, "Cull traversal time taken", osg::Timer::instance()->delta_s(beforeCullTick, afterCullTick));

                stats->setAttribute(frameNumber, "Draw traversal begin time", osg::Timer::instance()->delta_s(startTick, beforeDrawTick));
                stats->setAttribute(frameNumber, "Draw traversal end time", osg::Timer::instance()->delta_s(startTick, afterDrawTick));
                stats->setAttribute(frameNumber, "Draw traversal time taken", osg::Timer::instance()->delta_s(beforeDrawTick, afterDrawTick));
            }
        }
    } stats;

    DEBUG_MESSAGE << "Renderer() " << this << std::endl;

    if (!_initialized)
    {
        initialize(context->getState());
    }

    updateSceneView(_sceneView, context->getState());

    if (_compileOnNextDraw)
    {
        compile();
    }

    stats.init(this, _sceneView, _querySupport);

    stats.beforeCull();

    _sceneView->inheritCullSettings(*(_sceneView->getCamera()));
    _sceneView->cull();

    stats.afterCull();

#if 0
    if (state->getDynamicObjectCount()==0 && state->getDynamicObjectRenderingCompletedCallback())
    {
        state->getDynamicObjectRenderingCompletedCallback()->completed(state);
    }
#endif

    stats.beforeDraw();

    _sceneView->draw();

    stats.afterDraw();

    stats.collect();

    DEBUG_MESSAGE << "end Renderer() " << this << std::endl;
}

void Renderer::resizeGLObjectBuffers(unsigned int i)
{
    GraphicsOperation::resizeGLObjectBuffers(i);

    if (_sceneView)
    {
        _sceneView->resizeGLObjectBuffers(i);
    }
}

void Renderer::releaseGLObjects(osg::State *state) const
{
    GraphicsOperation::releaseGLObjects(state);

    if (_sceneView)
    {
        _sceneView->releaseGLObjects(state);
    }
}

osg::Camera *Renderer::getCamera()
{
    return _camera.get();
}

void Renderer::setCompileOnNextDraw(bool compileOnNextDraw)
{
    _compileOnNextDraw = compileOnNextDraw;
}

void Renderer::setStatsCallback(StatsCallback *statsCallback)
{
    _statsCallback = statsCallback;
}

Renderer::StatsCallback *Renderer::getStatsCallback() const
{
    return _statsCallback.get();
}

void Renderer::statsImplementation(osgUtil::SceneView *sceneView)
{
    auto stats = getCamera()->getStats();
    auto frameNumber = sceneView->getFrameStamp()->getFrameNumber();

    if (stats && stats->collectStats("scene"))
    {
        osgUtil::Statistics sceneStats;
        sceneView->getStats(sceneStats);

        stats->setAttribute(frameNumber, "Visible vertex count", static_cast<double>(sceneStats._vertexCount));
        stats->setAttribute(frameNumber, "Visible number of drawables", static_cast<double>(sceneStats.numDrawables));
        stats->setAttribute(frameNumber, "Visible number of fast drawables", static_cast<double>(sceneStats.numFastDrawables));
        stats->setAttribute(frameNumber, "Visible number of lights", static_cast<double>(sceneStats.nlights));
        stats->setAttribute(frameNumber, "Visible number of render bins", static_cast<double>(sceneStats.nbins));
        stats->setAttribute(frameNumber, "Visible depth", static_cast<double>(sceneStats.depth));
        stats->setAttribute(frameNumber, "Number of StateGraphs", static_cast<double>(sceneStats.numStateGraphs));
        stats->setAttribute(frameNumber, "Visible number of impostors", static_cast<double>(sceneStats.nimpostor));
        stats->setAttribute(frameNumber, "Number of ordered leaves", static_cast<double>(sceneStats.numOrderedLeaves));

        unsigned int totalNumPrimitiveSets = 0;
        const osgUtil::Statistics::PrimitiveValueMap &pvm = sceneStats.getPrimitiveValueMap();
        for (osgUtil::Statistics::PrimitiveValueMap::const_iterator pvm_itr = pvm.begin(); pvm_itr != pvm.end(); ++pvm_itr)
        {
            totalNumPrimitiveSets += pvm_itr->second.first;
        }
        stats->setAttribute(frameNumber, "Visible number of PrimitiveSets", static_cast<double>(totalNumPrimitiveSets));

        osgUtil::Statistics::PrimitiveCountMap &pcm = sceneStats.getPrimitiveCountMap();
        stats->setAttribute(frameNumber, "Visible number of GL_POINTS", static_cast<double>(pcm[GL_POINTS]));
        stats->setAttribute(frameNumber, "Visible number of GL_LINES", static_cast<double>(pcm[GL_LINES]));
        stats->setAttribute(frameNumber, "Visible number of GL_LINE_STRIP", static_cast<double>(pcm[GL_LINE_STRIP]));
        stats->setAttribute(frameNumber, "Visible number of GL_LINE_LOOP", static_cast<double>(pcm[GL_LINE_LOOP]));
        stats->setAttribute(frameNumber, "Visible number of GL_TRIANGLES", static_cast<double>(pcm[GL_TRIANGLES]));
        stats->setAttribute(frameNumber, "Visible number of GL_TRIANGLE_STRIP", static_cast<double>(pcm[GL_TRIANGLE_STRIP]));
        stats->setAttribute(frameNumber, "Visible number of GL_TRIANGLE_FAN", static_cast<double>(pcm[GL_TRIANGLE_FAN]));
        stats->setAttribute(frameNumber, "Visible number of GL_QUADS", static_cast<double>(pcm[GL_QUADS]));
        stats->setAttribute(frameNumber, "Visible number of GL_QUAD_STRIP", static_cast<double>(pcm[GL_QUAD_STRIP]));
        stats->setAttribute(frameNumber, "Visible number of GL_POLYGON", static_cast<double>(pcm[GL_POLYGON]));
    }
}

void Renderer::setupSceneView(osgUtil::SceneView *sceneView)
{
    // 创建帧戳
    sceneView->setFrameStamp(new osg::FrameStamp);

    // 设置状态集
    osg::Camera *masterCamera = _camera->getView() ? _camera->getView()->getCamera() : _camera.get();
    osg::StateSet *global_stateset = nullptr;
    osg::StateSet *secondary_stateset = nullptr;
    if (_camera != masterCamera)
    {
        global_stateset = masterCamera->getOrCreateStateSet();
        secondary_stateset = _camera->getStateSet();
    }
    else
    {
        global_stateset = _camera->getOrCreateStateSet();
    }

    osg::DisplaySettings *ds = _camera->getDisplaySettings() ? _camera->getDisplaySettings() : osg::DisplaySettings::instance().get();

    // TODO
    //_serializeDraw = ds ? ds->getSerializeDrawDispatch() : false;

    auto view = _camera->getView();
    unsigned int sceneViewOptions = osgUtil::SceneView::HEADLIGHT;
    if (view)
    {
        switch (view->getLightingMode())
        {
        case (osg::View::NO_LIGHT):
            sceneViewOptions = 0;
            break;
        case (osg::View::SKY_LIGHT):
            sceneViewOptions = osgUtil::SceneView::SKY_LIGHT;
            break;
        case (osg::View::HEADLIGHT):
            sceneViewOptions = osgUtil::SceneView::HEADLIGHT;
            break;
        }
    }

    sceneView->setAutomaticFlush(true /*TODO*/);
    sceneView->setGlobalStateSet(global_stateset);
    sceneView->setSecondaryStateSet(secondary_stateset);

    sceneView->setDefaults(sceneViewOptions);

    if (ds && ds->getUseSceneViewForStereoHint())
    {
        sceneView->setDisplaySettings(ds);
    }
    else
    {
        sceneView->setResetColorMaskToAllOn(false);
    }

    sceneView->setCamera(_camera.get(), false);

    {
        // assign CullVisitor::Identifier so that the double buffering of SceneView doesn't interfer
        // with code that requires a consistent knowledge and which effective cull traversal to taking place
        osg::ref_ptr<osgUtil::CullVisitor::Identifier> leftEyeIdentifier = new osgUtil::CullVisitor::Identifier();
        osg::ref_ptr<osgUtil::CullVisitor::Identifier> rightEyeIdentifier = new osgUtil::CullVisitor::Identifier();

        sceneView->getCullVisitor()->setIdentifier(leftEyeIdentifier.get());
        sceneView->setCullVisitorLeft(sceneView->getCullVisitor()->clone());
        sceneView->getCullVisitorLeft()->setIdentifier(leftEyeIdentifier.get());
        sceneView->setCullVisitorRight(sceneView->getCullVisitor()->clone());
        sceneView->getCullVisitorRight()->setIdentifier(rightEyeIdentifier.get());
    }
}

osgUtil::SceneView *Renderer::createSceneView()
{
    osg::ref_ptr<osgUtil::SceneView> sceneView = new osgUtil::SceneView();
    setupSceneView(sceneView);
    return sceneView.release();
}

void Renderer::initialize(osg::State *state)
{
    if (!_initialized)
    {
        _initialized = true;

        _sceneView = createSceneView();

        auto viewport = dynamic_cast<Viewport *>(_camera->getView());
        auto window = viewport ? viewport->getWindow() : nullptr;
        auto startTick = window ? window->getStartTick() : 0;

        osg::GLExtensions *ext = state->get<osg::GLExtensions>();
        if (ext->isARBTimerQuerySupported && state->getTimestampBits() > 0)
            _querySupport = new ARBQuerySupport();
        else if (ext->isTimerQuerySupported)
            _querySupport = new EXTQuerySupport();
        if (_querySupport.valid())
            _querySupport->initialize(state, startTick);
    }
}

void Renderer::compile()
{
    DEBUG_MESSAGE << "Renderer::compile()" << std::endl;

    _compileOnNextDraw = false;

    _sceneView->getState()->checkGLErrors("Before Renderer::compile");

    if (_sceneView->getSceneData())
    {
        osgUtil::GLObjectsVisitor glov;
        glov.setState(_sceneView->getState());

        // collect stats if required
        osg::View *view = _camera.valid() ? _camera->getView() : 0;
        osg::Stats *stats = view ? view->getStats() : 0;
        if (stats && stats->collectStats("compile"))
        {
            osg::ElapsedTime elapsedTime;

            glov.compile(*(_sceneView->getSceneData()));

            double compileTime = elapsedTime.elapsedTime();

            const osg::FrameStamp *fs = _sceneView->getFrameStamp();
            unsigned int frameNumber = fs ? fs->getFrameNumber() : 0;

            stats->setAttribute(frameNumber, "compile", compileTime);

            OSG_NOTICE << "Compile time " << compileTime * 1000.0 << "ms" << std::endl;
        }
        else
        {
            glov.compile(*(_sceneView->getSceneData()));
        }
    }

    _sceneView->getState()->checkGLErrors("After Renderer::compile");
}

void Renderer::updateSceneView(osgUtil::SceneView *sceneView, osg::State *state)
{
    osg::Camera *masterCamera = _camera->getView() ? _camera->getView()->getCamera() : _camera.get();

    osg::StateSet *global_stateset = 0;
    osg::StateSet *secondary_stateset = 0;
    if (_camera != masterCamera)
    {
        global_stateset = masterCamera->getOrCreateStateSet();
        secondary_stateset = _camera->getStateSet();
    }
    else
    {
        global_stateset = _camera->getOrCreateStateSet();
    }

    if (sceneView->getGlobalStateSet() != global_stateset)
    {
        sceneView->setGlobalStateSet(global_stateset);
    }

    if (sceneView->getSecondaryStateSet() != secondary_stateset)
    {
        sceneView->setSecondaryStateSet(secondary_stateset);
    }

    if (sceneView->getState() != state)
    {
        sceneView->setState(state);
    }

    Viewport *viewport = dynamic_cast<Viewport *>(_camera->getView());
    Scene *scene = viewport ? viewport->getScene() : nullptr;

    sceneView->setAutomaticFlush(true /*TODO*/);

    osgDB::DatabasePager *databasePager = scene ? scene->getDatabasePager() : nullptr;
    sceneView->getCullVisitor()->setDatabaseRequestHandler(databasePager);

    osgDB::ImagePager *imagePager = scene ? scene->getImagePager() : nullptr;
    sceneView->getCullVisitor()->setImageRequestHandler(imagePager);

    if (viewport && viewport->getFrameStamp())
    {
        (*sceneView->getFrameStamp()) = *(viewport->getFrameStamp());
    }
    else if (state->getFrameStamp())
    {
        (*sceneView->getFrameStamp()) = *(state->getFrameStamp());
    }

    if (viewport)
    {
        sceneView->setFusionDistance(viewport->getFusionDistanceMode(), viewport->getFusionDistanceValue());
    }

    osg::DisplaySettings *ds = _camera->getDisplaySettings() ? _camera->getDisplaySettings() : osg::DisplaySettings::instance().get();

    if (ds && ds->getUseSceneViewForStereoHint())
    {
        sceneView->setDisplaySettings(ds);
    }

    if (viewport && viewport->getWindow() && state)
    {
        state->setStartTick(viewport->getWindow()->getStartTick());
    }
}

void Renderer::stats()
{
    if (_statsCallback)
    {
        return _statsCallback->statsImplementation(this, _sceneView);
    }
    else
    {
        statsImplementation(_sceneView);
    }
}

} // namespace opeViewer
