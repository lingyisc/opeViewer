//
// Created by chudonghao on 2023/12/20.
//

#include "DisplaySettings.h"

namespace opeViewerQt
{

void updateFrom(osg::DisplaySettings &ds, const QSurfaceFormat &sf)
{
    ds.setMinimumNumAlphaBits(sf.alphaBufferSize());
    ds.setMinimumNumStencilBits(sf.stencilBufferSize());
    ds.setDepthBuffer(sf.depthBufferSize() != 0);
    ds.setNumMultiSamples(sf.samples());
    ds.setGLContextVersion(std::to_string(sf.majorVersion()) + "." + std::to_string(sf.minorVersion()));
    // flags
    // profile
    // swap method
}

} // namespace opeViewerQt
