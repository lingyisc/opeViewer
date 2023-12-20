//
// Created by chudonghao on 2023/12/18.
//

#include "OpenGLQuerySupport.h"

#include <osg/Drawable>

namespace opeViewer
{

OpenGLQuerySupport::OpenGLQuerySupport() : _extensions(0)
{
}

void OpenGLQuerySupport::initialize(osg::State *state, osg::Timer_t /*startTick*/)
{
    _extensions = state->get<osg::GLExtensions>();
}

EXTQuerySupport::EXTQuerySupport() : _previousQueryTime(0.0)
{
}

void EXTQuerySupport::checkQuery(osg::Stats *stats, osg::State * /*state*/, osg::Timer_t startTick)
{
    for (QueryFrameNumberList::iterator itr = _queryFrameNumberList.begin(); itr != _queryFrameNumberList.end();)
    {
        GLuint query = itr->first;
        GLint available = 0;
        _extensions->glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
        if (available)
        {
            GLuint64 timeElapsed = 0;
            _extensions->glGetQueryObjectui64v(query, GL_QUERY_RESULT, &timeElapsed);

            double timeElapsedSeconds = double(timeElapsed) * 1e-9;
            double currentTime = osg::Timer::instance()->delta_s(startTick, osg::Timer::instance()->tick());
            double estimatedEndTime = (_previousQueryTime + currentTime) * 0.5;
            double estimatedBeginTime = estimatedEndTime - timeElapsedSeconds;

            stats->setAttribute(itr->second, "GPU draw begin time", estimatedBeginTime);
            stats->setAttribute(itr->second, "GPU draw end time", estimatedEndTime);
            stats->setAttribute(itr->second, "GPU draw time taken", timeElapsedSeconds);

            itr = _queryFrameNumberList.erase(itr);
            _availableQueryObjects.push_back(query);
        }
        else
        {
            ++itr;
        }
    }
    _previousQueryTime = osg::Timer::instance()->delta_s(startTick, osg::Timer::instance()->tick());
}

GLuint EXTQuerySupport::createQueryObject()
{
    if (_availableQueryObjects.empty())
    {
        GLuint query;
        _extensions->glGenQueries(1, &query);
        return query;
    }
    else
    {
        GLuint query = _availableQueryObjects.back();
        _availableQueryObjects.pop_back();
        return query;
    }
}

void EXTQuerySupport::beginQuery(unsigned int frameNumber, osg::State * /*state*/)
{
    GLuint query = createQueryObject();
    _extensions->glBeginQuery(GL_TIME_ELAPSED, query);
    _queryFrameNumberList.push_back(QueryFrameNumberPair(query, frameNumber));
}

void EXTQuerySupport::endQuery(osg::State * /*state*/)
{
    _extensions->glEndQuery(GL_TIME_ELAPSED);
}

void EXTQuerySupport::initialize(osg::State *state, osg::Timer_t startTick)
{
    OpenGLQuerySupport::initialize(state, startTick);
    _previousQueryTime = osg::Timer::instance()->delta_s(startTick, osg::Timer::instance()->tick());
}

void ARBQuerySupport::initialize(osg::State *state, osg::Timer_t startTick)
{
    OpenGLQuerySupport::initialize(state, startTick);
}

void ARBQuerySupport::beginQuery(unsigned int frameNumber, osg::State * /*state*/)
{
    QueryPair query;
    if (_availableQueryObjects.empty())
    {
        _extensions->glGenQueries(1, &query.first);
        _extensions->glGenQueries(1, &query.second);
    }
    else
    {
        query = _availableQueryObjects.back();
        _availableQueryObjects.pop_back();
    }
    _extensions->glQueryCounter(query.first, GL_TIMESTAMP);
    _queryFrameList.push_back(ActiveQuery(query, frameNumber));
}

void ARBQuerySupport::endQuery(osg::State * /*state*/)
{
    _extensions->glQueryCounter(_queryFrameList.back().queries.second, GL_TIMESTAMP);
}

void ARBQuerySupport::checkQuery(osg::Stats *stats, osg::State *state, osg::Timer_t /*startTick*/)
{
    for (QueryFrameList::iterator itr = _queryFrameList.begin(); itr != _queryFrameList.end();)
    {
        GLint available = 0;
        // If the end query is available, the begin query must be too.
        _extensions->glGetQueryObjectiv(itr->queries.second, GL_QUERY_RESULT_AVAILABLE, &available);
        if (available)
        {
            QueryPair queries = itr->queries;
            GLuint64 beginTimestamp = 0;
            GLuint64 endTimestamp = 0;
            _extensions->glGetQueryObjectui64v(queries.first, GL_QUERY_RESULT, &beginTimestamp);
            _extensions->glGetQueryObjectui64v(queries.second, GL_QUERY_RESULT, &endTimestamp);
            GLuint64 gpuTimestamp = state->getGpuTimestamp();
            // Have any of the timestamps wrapped around?
            int tbits = state->getTimestampBits();
            if (tbits < 64)
            {
                // If the high bits on any of the timestamp bits are
                // different then the counters may have wrapped.
                const int hiShift = (tbits - 1);
                const GLuint64 one = 1;
                const GLuint64 hiMask = one << hiShift;
                const GLuint64 sum = (beginTimestamp >> hiShift) + (endTimestamp >> hiShift) + (gpuTimestamp >> hiShift);
                if (sum == 1 || sum == 2)
                {
                    const GLuint64 wrapAdd = one << tbits;
                    // Counter wrapped between begin and end?
                    if (beginTimestamp > endTimestamp)
                    {
                        endTimestamp += wrapAdd;
                    }
                    else if (gpuTimestamp < beginTimestamp && beginTimestamp - gpuTimestamp > (hiMask >> 1))
                    {
                        gpuTimestamp += wrapAdd;
                    }
                    else if (endTimestamp < gpuTimestamp && gpuTimestamp - endTimestamp > (hiMask >> 1))
                    {
                        beginTimestamp += wrapAdd;
                        endTimestamp += wrapAdd;
                    }
                }
            }
            GLuint64 timeElapsed = endTimestamp - beginTimestamp;
            double timeElapsedSeconds = double(timeElapsed) * 1e-9;
            double gpuTick = state->getGpuTime();
            double beginTime = 0.0;
            double endTime = 0.0;
            if (beginTimestamp > gpuTimestamp)
                beginTime = gpuTick + double(beginTimestamp - gpuTimestamp) * 1e-9;
            else
                beginTime = gpuTick - double(gpuTimestamp - beginTimestamp) * 1e-9;
            if (endTimestamp > gpuTimestamp)
                endTime = gpuTick + double(endTimestamp - gpuTimestamp) * 1e-9;
            else
                endTime = gpuTick - double(gpuTimestamp - endTimestamp) * 1e-9;
            stats->setAttribute(itr->frameNumber, "GPU draw begin time", beginTime);
            stats->setAttribute(itr->frameNumber, "GPU draw end time", endTime);
            stats->setAttribute(itr->frameNumber, "GPU draw time taken", timeElapsedSeconds);
            itr = _queryFrameList.erase(itr);
            _availableQueryObjects.push_back(queries);
        }
        else
        {
            ++itr;
        }
    }
}

} // namespace opeViewer
