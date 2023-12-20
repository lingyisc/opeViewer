//
// Created by chudonghao on 2023/12/21.
//

#ifndef INC_2023_12_21_EA8AFC0C6616411AAF3414F8CCE2587F_H_
#define INC_2023_12_21_EA8AFC0C6616411AAF3414F8CCE2587F_H_

#include <osgGA/GUIEventHandler>
#include <osgText/Text>

#include "Window.h"

namespace opeViewer
{

/**
 * Event handler for adding on screen stats reporting to Window.
 */
class StatsHandler : public osgGA::GUIEventHandler
{
  public:
    StatsHandler();

    enum StatsType
    {
        NO_STATS = 0,
        FRAME_RATE = 1 << 0,
        WINDOW_STATS = 1 << 1,
        VIEWPORT_STATS = 1 << 2,
        SCENE_STATS = 1 << 3,
    };

    void setKeyEventTogglesOnScreenStats(int key)
    {
        _keyEventTogglesOnScreenStats = key;
    }
    int getKeyEventTogglesOnScreenStats() const
    {
        return _keyEventTogglesOnScreenStats;
    }

    void setKeyEventPrintsOutStats(int key)
    {
        _keyEventPrintsOutStats = key;
    }
    int getKeyEventPrintsOutStats() const
    {
        return _keyEventPrintsOutStats;
    }

    double getBlockMultiplier() const
    {
        return _blockMultiplier;
    }

    void reset();

    Viewport *getViewport()
    {
        return _viewport.get();
    }

    const Viewport *getViewport() const
    {
        return _viewport.get();
    }

    virtual void collectWhichCamerasToRenderStatsFor(Window *window, std::vector<osg::Camera *> &cameras);

    virtual void setEnabled(size_t statsTypeMask);

    virtual bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &aa);

    /** Get the keyboard and mouse usage of this manipulator.*/
    virtual void getUsage(osg::ApplicationUsage &usage) const;

    /** This allows the user to register additional stats lines that will
        be added to the graph. The stats for these will be gotten from the
        viewer (viewer->getViewerStats()). The stats can be displayed in
        either or all of the following ways:

        - A numeric time beside the stat name
          Requires that an elapsed time be recorded in the viewer's stats
          for the "timeTakenName".
        - A bar in the top bar graph
          Requires that two times (relative to the viewer's start tick) be
          recorded in the viewer's stats for the "beginTimeName" and
          "endTimeName".
        - A line in the bottom graph
          Requires that an elapsed time be recorded in the viewer's stats
          for the "timeTakenName" and that the value be used as an average.

        If you don't want a numeric value and a line in the bottom line
        graph for your value, pass the empty string for timeTakenName. If
        you don't want a bar in the graph, pass the empty string for
        beginTimeName and endTimeName.

        @param label                 The label to be displayed to identify the line in the stats.
        @param textColor             Will be used for the text label, the numeric time and the bottom line graph.
        @param barColor              Will be used for the bar in the top bar graph.
        @param timeTakenName         The name to be queried in the viewer stats for the numeric value (also used for the bottom line graph).
        @param multiplier            The multiplier to apply to the numeric value before displaying it in the stats.
        @param average               Whether to use the average value of the numeric value.
        @param averageInInverseSpace Whether to average in inverse space (used for frame rate).
        @param beginTimeName         The name to be queried in the viewer stats for the begin time for the top bar graph.
        @param endTimeName           The name to be queried in the viewer stats for the end time for the top bar graph.
        @param maxValue              The value to use as maximum in the bottom line graph. Stats will be clamped between 0 and that value, and it will be the highest visible value in the graph.
      */
    void addUserStatsLine(const std::string &label, const osg::Vec4 &textColor, const osg::Vec4 &barColor, const std::string &timeTakenName, float multiplier, bool average, bool averageInInverseSpace, const std::string &beginTimeName,
                          const std::string &endTimeName, float maxValue);

    void removeUserStatsLine(const std::string &label);

  protected:
    void initialize(Window *window);

    void initialize(Window *window, osg::Vec3 &pos);

    virtual void setUpHUDCamera(Window *window);

    void setWindowSize(int width, int height);

    osg::Geometry *createBackgroundRectangle(const osg::Vec3 &pos, const float width, const float height, osg::Vec4 &color);

    osg::Geometry *createGeometry(const osg::Vec3 &pos, float height, const osg::Vec4 &colour, unsigned int numBlocks);

    osg::Geometry *createFrameMarkers(const osg::Vec3 &pos, float height, const osg::Vec4 &colour, unsigned int numBlocks);

    osg::Geometry *createTick(const osg::Vec3 &pos, float height, const osg::Vec4 &colour, unsigned int numTicks);

    void createTimeStatsLine(const std::string &lineLabel, osg::Vec3 pos, const osg::Vec4 &textColor, const osg::Vec4 &barColor, osg::Stats *viewerStats, osg::Stats *stats, const std::string &timeTakenName, float multiplier, bool average,
                             bool averageInInverseSpace, const std::string &beginTimeName, const std::string &endTimeName);

    void createCameraTimeStats(osg::Vec3 &pos, bool acquireGPUStats, osg::Stats *viewerStats, osg::Camera *camera);

    virtual void setUpScene(Window *window, osg::Vec3 &pos);

    void updateThreadingModelText();

    int _keyEventTogglesOnScreenStats;
    int _keyEventPrintsOutStats;

    size_t _statsTypeMask;
    int _statsTypeSize;

    bool _initialized;
    osg::ref_ptr<Viewport> _viewport;
    osg::ref_ptr<osg::Camera> _camera;

    osg::ref_ptr<osg::Switch> _switch;

    osg::ref_ptr<osg::Geode> _statsGeode;

    Window::ThreadingModel _threadingModel;
    osg::ref_ptr<osgText::Text> _threadingModelText;

    unsigned int _frameRateChildNum;
    unsigned int _windowChildNum;
    unsigned int _viewportChildNum;
    unsigned int _sceneChildNum;
    unsigned int _numBlocks;
    double _blockMultiplier;

    float _statsWidth;
    float _statsHeight;

    std::string _font;
    float _startBlocks;
    osg::Vec2 _leftTopPos;
    float _characterSize;
    float _lineHeight;

    struct UserStatsLine
    {
        std::string label;
        osg::Vec4 textColor;
        osg::Vec4 barColor;
        std::string timeTakenName;
        float multiplier;
        bool average;
        bool averageInInverseSpace;
        std::string beginTimeName;
        std::string endTimeName;
        float maxValue;

        UserStatsLine(const std::string &label_, const osg::Vec4 &textColor_, const osg::Vec4 &barColor_, const std::string &timeTakenName_, float multiplier_, bool average_, bool averageInInverseSpace_, const std::string &beginTimeName_,
                      const std::string &endTimeName_, float maxValue_)
            : label(label_), textColor(textColor_), barColor(barColor_), timeTakenName(timeTakenName_), multiplier(multiplier_), average(average_), averageInInverseSpace(averageInInverseSpace_), beginTimeName(beginTimeName_),
              endTimeName(endTimeName_), maxValue(maxValue_)
        {
        }
    };

    typedef std::vector<UserStatsLine> UserStatsLines;
    UserStatsLines _userStatsLines;
};

} // namespace opeViewer

#endif // INC_2023_12_21_EA8AFC0C6616411AAF3414F8CCE2587F_H_
