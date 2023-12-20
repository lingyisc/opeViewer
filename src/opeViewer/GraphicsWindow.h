//
// Created by chudonghao on 2023/12/18.
//

#ifndef INC_2023_12_18_3447C60048E049FCB0AF38874D0210D7_H_
#define INC_2023_12_18_3447C60048E049FCB0AF38874D0210D7_H_

#include <osg/GraphicsContext>

namespace opeViewer
{

class Viewport;

/// 图形窗口，持有一个OpenGL上下文，持有若干视口
class GraphicsWindow : public osg::GraphicsContext
{
  public:
    GraphicsWindow();

    ~GraphicsWindow() override;

    /// 重写了osg::GraphicsContext::runOperations()方法，相机过程不再在这里渲染
    void runOperations() override;

    void resizedImplementation(int x, int y, int width, int height) override;
};

} // namespace opeViewer

#endif // INC_2023_12_18_3447C60048E049FCB0AF38874D0210D7_H_
