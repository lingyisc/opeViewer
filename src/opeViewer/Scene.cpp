//
// Created by chudonghao on 2023/12/18.
//

#include "Scene.h"

#include <osgDB/DatabasePager>
#include <osgDB/ImagePager>

namespace opeViewer
{

struct SceneSingleton
{
    SceneSingleton()
    {
    }

    inline void add(Scene *scene)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        _cache.push_back(scene);
    }

    inline void remove(Scene *scene)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        for (SceneCache::iterator itr = _cache.begin(); itr != _cache.end(); ++itr)
        {
            if (scene == itr->get())
            {
                _cache.erase(itr);
                break;
            }
        }
    }

    inline Scene *getScene(osg::Node *node)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        for (SceneCache::iterator itr = _cache.begin(); itr != _cache.end(); ++itr)
        {
            Scene *scene = itr->get();
            if (scene && scene->getSceneData() == node)
                return scene;
        }
        return 0;
    }

    typedef std::vector<osg::observer_ptr<Scene>> SceneCache;
    SceneCache _cache;
    OpenThreads::Mutex _mutex;
};

static SceneSingleton &getSceneSingleton()
{
    static SceneSingleton s_sceneSingleton;
    return s_sceneSingleton;
}

// Use a proxy to force the initialization of the SceneSingleton during static initialization
OSG_INIT_SINGLETON_PROXY(SceneSingletonProxy, getSceneSingleton())

Scene::Scene() : osg::Object(true)
{
    setDatabasePager(osgDB::DatabasePager::create());
    setImagePager(new osgDB::ImagePager);
    setStats(new osg::Stats("Scene"));
    getSceneSingleton().add(this);
}

Scene::Scene(const Scene &r, const osg::CopyOp &copyop)
{
    throw std::runtime_error("Not implemented");
}

Scene::~Scene()
{
    getSceneSingleton().remove(this);
}

void Scene::setSceneData(osg::Node *node)
{
    _sceneData = node;
}

osg::Node *Scene::getSceneData()
{
    return _sceneData.get();
}

const osg::Node *Scene::getSceneData() const
{
    return _sceneData.get();
}

void Scene::setDatabasePager(osgDB::DatabasePager *dp)
{
    _databasePager = dp;
}

osgDB::DatabasePager *Scene::getDatabasePager()
{
    return _databasePager.get();
}

const osgDB::DatabasePager *Scene::getDatabasePager() const
{
    return _databasePager.get();
}

void Scene::setImagePager(osgDB::ImagePager *ip)
{
    _imagePager = ip;
}

osgDB::ImagePager *Scene::getImagePager()
{
    return _imagePager.get();
}

const osgDB::ImagePager *Scene::getImagePager() const
{
    return _imagePager.get();
}

void Scene::addUpdateOperation(osg::Operation *operation)
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

void Scene::removeUpdateOperation(osg::Operation *operation)
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

void Scene::setStats(osg::Stats *stats)
{
    _stats = stats;
}

osg::Stats *Scene::getStats()
{
    return _stats.get();
}

const osg::Stats *Scene::getStats() const
{
    return _stats.get();
}

bool Scene::requiresUpdateSceneGraph() const
{
    // check if the database pager needs to update the scene
    if (getDatabasePager()->requiresUpdateSceneGraph())
        return true;

    // check if the image pager needs to update the scene
    if (getImagePager()->requiresUpdateSceneGraph())
        return true;

    // check if scene graph needs update traversal
    if (_sceneData.valid() && (_sceneData->getUpdateCallback() || (_sceneData->getNumChildrenRequiringUpdateTraversal() > 0)))
        return true;

    if (_updateOperations.valid() && _updateOperations->getNumOperationsInQueue() > 0)
        return true;

    return false;
}

void Scene::updateSceneGraph(osg::NodeVisitor &updateVisitor)
{
    if (!_sceneData)
        return;

    if (getDatabasePager())
    {
        // synchronize changes required by the DatabasePager thread to the scene graph
        getDatabasePager()->updateSceneGraph((*updateVisitor.getFrameStamp()));
    }

    if (getImagePager())
    {
        // synchronize changes required by the DatabasePager thread to the scene graph
        getImagePager()->updateSceneGraph(*(updateVisitor.getFrameStamp()));
    }

    if (getSceneData())
    {
        updateVisitor.setImageRequestHandler(getImagePager());
        getSceneData()->accept(updateVisitor);
    }

    if (_updateOperations)
    {
        _updateOperations->runOperations(this);
    }
}

bool Scene::requiresRedraw() const
{
    // check if the database pager needs a redraw
    if (getDatabasePager()->requiresRedraw())
        return true;

    return false;
}

Scene *Scene::getScene(osg::Node *node)
{
    return getSceneSingleton().getScene(node);
}

Scene *Scene::getOrCreateScene(osg::Node *node)
{
    if (!node)
        return 0;

    Scene *scene = getScene(node);
    if (!scene)
    {
        scene = new Scene;
        scene->setSceneData(node);
    }

    return scene;
}

} // namespace opeViewer
