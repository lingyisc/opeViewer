//
// Created by chudonghao on 2023/12/18.
//

#include "GraphicsWindow.h"

#include "Viewport.h"

namespace opeViewer
{

GraphicsWindow::GraphicsWindow()
{
}

GraphicsWindow::~GraphicsWindow()
{
}

void GraphicsWindow::runOperations()
{
    for (GraphicsOperationQueue::iterator itr = _operations.begin(); itr != _operations.end();)
    {
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
            _currentOperation = *itr;

            if (!_currentOperation->getKeep())
            {
                itr = _operations.erase(itr);

                if (_operations.empty())
                {
                    _operationsBlock->set(false);
                }
            }
            else
            {
                ++itr;
            }
        }

        if (_currentOperation.valid())
        {
            // OSG_INFO<<"Doing op "<<_currentOperation->getName()<<" "<<this<<std::endl;

            // call the graphics operation.
            (*_currentOperation)(this);

            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
                _currentOperation = 0;
            }
        }
    }
}

void GraphicsWindow::resizedImplementation(int x, int y, int width, int height)
{
    if (!_cameras.empty())
    {
        throw std::logic_error("don't call osg::Camera::setGraphicsContext any more");
    }

    _traits->x = x;
    _traits->y = y;
    _traits->width = width;
    _traits->height = height;
}

} // namespace opeViewer
