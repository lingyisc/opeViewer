//
// Created by chudonghao on 2023/12/19.
//

#include "GraphicsWindowEmbedded.h"

namespace opeViewer
{

void GraphicsWindowEmbedded::setupStateContextID()
{
    if (_traits && _traits->sharedContext.valid())
    {
        _state->setContextID(_traits->sharedContext->getState()->getContextID());
        // decrement在~GraphicsContext()中调用
        incrementContextIDUsageCount(_state->getContextID());
    }
    else
    {
        _state->setContextID(osg::GraphicsContext::createNewContextID());
    }
}

bool GraphicsWindowEmbedded::valid() const
{
    return true;
}

bool GraphicsWindowEmbedded::realizeImplementation()
{
    return true;
}

bool GraphicsWindowEmbedded::isRealizedImplementation() const
{
    return true;
}

void GraphicsWindowEmbedded::closeImplementation()
{
}

bool GraphicsWindowEmbedded::makeCurrentImplementation()
{
    return true;
}

bool GraphicsWindowEmbedded::makeContextCurrentImplementation(osg::GraphicsContext *readContext)
{
    return true;
}

bool GraphicsWindowEmbedded::releaseContextImplementation()
{
    return true;
}

void GraphicsWindowEmbedded::bindPBufferToTextureImplementation(GLenum buffer)
{
}

void GraphicsWindowEmbedded::swapBuffersImplementation()
{
}

} // namespace opeViewer
