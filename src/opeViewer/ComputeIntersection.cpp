//
// Created by chudonghao on 2024/1/15.
//

#include "ComputeIntersection.h"

namespace opeViewer
{

bool computeIntersections(const osg::Camera *camera, osgUtil::Intersector::CoordinateFrame cf, float x, float y, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask traversalMask)
{
    /// \see osgViewer::View::computeIntersections

    if (!camera)
        return false;

    osg::ref_ptr<osgUtil::LineSegmentIntersector> picker = new osgUtil::LineSegmentIntersector(cf, x, y);
    osgUtil::IntersectionVisitor iv(picker.get());
    iv.setTraversalMask(traversalMask);

    const_cast<osg::Camera *>(camera)->accept(iv);

    if (picker->containsIntersections())
    {
        intersections = picker->getIntersections();
        return true;
    }
    else
    {
        intersections.clear();
        return false;
    }
}

bool computeIntersections(const osg::Camera *camera, osgUtil::Intersector::CoordinateFrame cf, float x, float y, const osg::NodePath &nodePath, osgUtil::LineSegmentIntersector::Intersections &intersections,
                          osg::Node::NodeMask traversalMask)
{
    /// \see osgViewer::View::computeIntersections

    if (!camera || nodePath.empty())
        return false;

    osg::Matrixd matrix;
    if (nodePath.size() > 1)
    {
        osg::NodePath prunedNodePath(nodePath.begin(), nodePath.end() - 1);
        matrix = osg::computeLocalToWorld(prunedNodePath);
    }

    matrix.postMult(camera->getViewMatrix());
    matrix.postMult(camera->getProjectionMatrix());

    double zNear = -1.0;
    double zFar = 1.0;
    if (cf == osgUtil::Intersector::WINDOW && camera->getViewport())
    {
        matrix.postMult(camera->getViewport()->computeWindowMatrix());
        zNear = 0.0;
        zFar = 1.0;
    }

    osg::Matrixd inverse;
    inverse.invert(matrix);

    osg::Vec3d startVertex = osg::Vec3d(x, y, zNear) * inverse;
    osg::Vec3d endVertex = osg::Vec3d(x, y, zFar) * inverse;

    osg::ref_ptr<osgUtil::LineSegmentIntersector> picker = new osgUtil::LineSegmentIntersector(osgUtil::Intersector::MODEL, startVertex, endVertex);

    osgUtil::IntersectionVisitor iv(picker.get());
    iv.setTraversalMask(traversalMask);
    nodePath.back()->accept(iv);

    if (picker->containsIntersections())
    {
        intersections = picker->getIntersections();
        return true;
    }
    else
    {
        intersections.clear();
        return false;
    }
}

bool computeIntersections(const osgGA::GUIEventAdapter &ea, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask traversalMask)
{
    /// \see osgViewer::View::computeIntersections

    if (ea.getNumPointerData() >= 1)
    {
        const osgGA::PointerData *pd = ea.getPointerData(ea.getNumPointerData() - 1);
        const osg::Camera *camera = pd->object.valid() ? pd->object->asCamera() : nullptr;
        if (camera)
        {
            return computeIntersections(camera, osgUtil::Intersector::PROJECTION, pd->getXnormalized(), pd->getYnormalized(), intersections, traversalMask);
        }
    }

    return false;
}

bool opeViewer::computeIntersections(const osgGA::GUIEventAdapter &ea, const osg::NodePath &nodePath, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask traversalMask)
{
    /// \see osgViewer::View::computeIntersections

    if (ea.getNumPointerData() >= 1)
    {
        const osgGA::PointerData *pd = ea.getPointerData(ea.getNumPointerData() - 1);
        const osg::Camera *camera = pd->object.valid() ? pd->object->asCamera() : nullptr;
        if (camera)
        {
            return computeIntersections(camera, osgUtil::Intersector::PROJECTION, pd->getXnormalized(), pd->getYnormalized(), nodePath, intersections, traversalMask);
        }
    }

    return false;
}

} // namespace opeViewer