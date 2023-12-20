//
// Created by chudonghao on 2023/12/18.
//

#ifndef INC_2023_12_18_F357EA5F504C4C77ACF0EDAA255961FD_H_
#define INC_2023_12_18_F357EA5F504C4C77ACF0EDAA255961FD_H_

#include <list>

#include <osg/GLExtensions>
#include <osg/Referenced>
#include <osg/Stats>
#include <osg/Timer>

namespace opeViewer
{

/// 从osgViewer拷贝
class OpenGLQuerySupport : public osg::Referenced
{
  public:
    OpenGLQuerySupport();

    virtual void checkQuery(osg::Stats *stats, osg::State *state, osg::Timer_t startTick) = 0;

    virtual void beginQuery(unsigned int frameNumber, osg::State *state) = 0;
    virtual void endQuery(osg::State *state) = 0;
    virtual void initialize(osg::State *state, osg::Timer_t startTick);

  protected:
    const osg::GLExtensions *_extensions;
};

class EXTQuerySupport : public OpenGLQuerySupport
{
  public:
    EXTQuerySupport();
    void checkQuery(osg::Stats *stats, osg::State *state, osg::Timer_t startTick);
    virtual void beginQuery(unsigned int frameNumber, osg::State *state);
    virtual void endQuery(osg::State *state);
    virtual void initialize(osg::State *state, osg::Timer_t startTick);

  protected:
    GLuint createQueryObject();
    typedef std::pair<GLuint, unsigned int> QueryFrameNumberPair;
    typedef std::list<QueryFrameNumberPair> QueryFrameNumberList;
    typedef std::vector<GLuint> QueryList;

    QueryFrameNumberList _queryFrameNumberList;
    QueryList _availableQueryObjects;
    double _previousQueryTime;
};

class ARBQuerySupport : public OpenGLQuerySupport
{
  public:
    virtual void checkQuery(osg::Stats *stats, osg::State *state, osg::Timer_t startTick);

    virtual void beginQuery(unsigned int frameNumber, osg::State *state);
    virtual void endQuery(osg::State *state);
    virtual void initialize(osg::State *state, osg::Timer_t startTick);

  protected:
    typedef std::pair<GLuint, GLuint> QueryPair;

    struct ActiveQuery
    {
        ActiveQuery() : queries(0, 0), frameNumber(0)
        {
        }

        ActiveQuery(GLuint start_, GLuint end_, int frameNumber_) : queries(start_, end_), frameNumber(frameNumber_)
        {
        }

        ActiveQuery(const QueryPair &queries_, unsigned int frameNumber_) : queries(queries_), frameNumber(frameNumber_)
        {
        }

        QueryPair queries;
        unsigned int frameNumber;
    };

    typedef std::list<ActiveQuery> QueryFrameList;
    typedef std::vector<QueryPair> QueryList;
    QueryFrameList _queryFrameList;
    QueryList _availableQueryObjects;
};

} // namespace opeViewer

#endif // INC_2023_12_18_F357EA5F504C4C77ACF0EDAA255961FD_H_
