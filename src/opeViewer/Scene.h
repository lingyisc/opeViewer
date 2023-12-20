//
// Created by chudonghao on 2023/12/18.
//

#ifndef INC_2023_12_18_C579260774414D9B8EBB247C01B48886_H_
#define INC_2023_12_18_C579260774414D9B8EBB247C01B48886_H_

#include <osg/NodeVisitor>
#include <osg/Referenced>
#include <osg/ref_ptr>

namespace osg
{
class Node;
class Operation;
class OperationQueue;
class Stats;
} // namespace osg

namespace osgDB
{
class DatabasePager;
class ImagePager;
} // namespace osgDB

namespace opeViewer
{

/// 场景
///
/// 主要包含场景和分页器
class Scene : public osg::Object
{
    friend class Viewport;

    osg::ref_ptr<osg::Node> _sceneData;
    osg::ref_ptr<osgDB::DatabasePager> _databasePager;
    osg::ref_ptr<osgDB::ImagePager> _imagePager;
    osg::ref_ptr<osg::OperationQueue> _updateOperations;
    osg::ref_ptr<osg::Stats> _stats;

  public:
    META_Object(opeViewer, Scene);

    void setSceneData(osg::Node *node);

    osg::Node *getSceneData();

    const osg::Node *getSceneData() const;

    void setDatabasePager(osgDB::DatabasePager *dp);

    osgDB::DatabasePager *getDatabasePager();

    const osgDB::DatabasePager *getDatabasePager() const;

    void setImagePager(osgDB::ImagePager *ip);

    osgDB::ImagePager *getImagePager();

    const osgDB::ImagePager *getImagePager() const;

    void addUpdateOperation(osg::Operation *operation);

    void removeUpdateOperation(osg::Operation *operation);

    void setStats(osg::Stats *stats);

    osg::Stats *getStats();

    const osg::Stats *getStats() const;

    virtual bool requiresUpdateSceneGraph() const;

    virtual void updateSceneGraph(osg::NodeVisitor &updateVisitor);

    virtual bool requiresRedraw() const;

    static Scene *getScene(osg::Node *node);

  protected:
    Scene();

    Scene(const Scene &r, const osg::CopyOp &copyop);

    virtual ~Scene();

    static Scene *getOrCreateScene(osg::Node *node);
};

} // namespace opeViewer

#endif // INC_2023_12_18_C579260774414D9B8EBB247C01B48886_H_
