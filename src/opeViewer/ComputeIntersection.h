//
// Created by chudonghao on 2024/1/15.
//

#ifndef INC_2024_1_15_1AD1C82F53EC4FE2A3EF296CAE1B9DFD_H_
#define INC_2024_1_15_1AD1C82F53EC4FE2A3EF296CAE1B9DFD_H_

#include <osg/Camera>
#include <osgGA/GUIEventAdapter>
#include <osgUtil/LineSegmentIntersector>

namespace opeViewer
{

/// 计算射线命中
bool computeIntersections(const osg::Camera *, osgUtil::Intersector::CoordinateFrame cf, float x, float y, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask traversalMask);
/// 计算射线命中
bool computeIntersections(const osg::Camera *, osgUtil::Intersector::CoordinateFrame cf, float x, float y, const osg::NodePath &nodePath, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask traversalMask);
/// 在鼠标位置选择
bool computeIntersections(const osgGA::GUIEventAdapter &ea, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask traversalMask);
/// 在鼠标位置选择
bool computeIntersections(const osgGA::GUIEventAdapter &ea, const osg::NodePath &nodePath, osgUtil::LineSegmentIntersector::Intersections &intersections, osg::Node::NodeMask traversalMask);

} // namespace opeViewer

#endif // INC_2024_1_15_1AD1C82F53EC4FE2A3EF296CAE1B9DFD_H_
