//
// Created by chudonghao on 2023/12/18.
//

#include <QApplication>

#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>

#define USE_WIDGET 1
#if USE_WIDGET
#include <opeViewerQt/OpenGLWidget.h>
using Window = opeViewerQt::OpenGLWidget;
#else
#include <opeViewerQt/OpenGLWindow.h>
using Window = opeViewerQt::OpenGLWindow;
#endif

#include <opeViewer/StatsHandler.h>
#include <opeViewer/Viewport.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    Window w;
    w.resize(800, 800);
    w.setRunFrameScheme(opeViewer::Window::ON_DEMAND);
    w.addEventHandler(new opeViewer::StatsHandler);

    {
        osg::ref_ptr<opeViewer::Viewport> viewport = new opeViewer::Viewport;
        w.addViewport(viewport);

        viewport->getCamera()->setClearColor(osg::Vec4(1, 0, 0, 1));
        viewport->getCamera()->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        viewport->getCamera()->setProjectionResizePolicy(osg::Camera::FIXED);
        viewport->getCamera()->setViewport(0, 0, 400, 400);
        viewport->setSceneData(osgDB::readNodeFile("avatar.osg"));
        viewport->setCameraManipulator(new osgGA::TrackballManipulator);
    }

    {
        osg::ref_ptr<opeViewer::Viewport> viewport = new opeViewer::Viewport;
        w.addViewport(viewport);

        viewport->getCamera()->setClearColor(osg::Vec4(0, 1, 0, 1));
        viewport->getCamera()->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        viewport->getCamera()->setProjectionResizePolicy(osg::Camera::FIXED);
        viewport->getCamera()->setViewport(400, 0, 400, 400);
        viewport->setSceneData(osgDB::readNodeFile("cow.osg"));
        viewport->setCameraManipulator(new osgGA::TrackballManipulator);
    }

    {
        osg::ref_ptr<opeViewer::Viewport> viewport = new opeViewer::Viewport;
        w.addViewport(viewport);

        viewport->getCamera()->setClearColor(osg::Vec4(0, 0, 1, 1));
        viewport->getCamera()->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        viewport->getCamera()->setProjectionResizePolicy(osg::Camera::FIXED);
        viewport->getCamera()->setViewport(0, 400, 400, 400);
        viewport->setSceneData(osgDB::readNodeFile("glider.osg"));
        viewport->setCameraManipulator(new osgGA::TrackballManipulator);
    }

    w.show();

    return QApplication::exec();
}
