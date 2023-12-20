//
// Created by chudonghao on 2023/12/19.
//

#ifndef INC_2023_12_19_B939489F5618414C85528C8E94C5B186_H_
#define INC_2023_12_19_B939489F5618414C85528C8E94C5B186_H_

#include "GraphicsWindow.h"

namespace opeViewer
{

class GraphicsWindowEmbedded : public GraphicsWindow
{
  public:
    /// \pre trait有效
    void setupStateContextID();

    bool valid() const override;

    bool realizeImplementation() override;

    bool isRealizedImplementation() const override;

    void closeImplementation() override;

    bool makeCurrentImplementation() override;

    bool makeContextCurrentImplementation(osg::GraphicsContext *readContext) override;

    bool releaseContextImplementation() override;

    void bindPBufferToTextureImplementation(GLenum buffer) override;

    void swapBuffersImplementation() override;
};

} // namespace opeViewer

#endif // INC_2023_12_19_B939489F5618414C85528C8E94C5B186_H_
