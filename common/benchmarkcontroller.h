/*
 * Copyright (C) 2009 Holger Hans Peter Freyther
 * Copyright (C) 2010 Benjamin Poulain, Nokia
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef BENCHMARKCONTROLLER_H
#define BENCHMARKCONTROLLER_H

#include "benchmark_p.h"

class QString;

#if (QT_VERSION >= 0x040700)
#    include <QElapsedTimer>
#else
#    include <QTime>
#endif


// A benchmark controller is used to collect the data and create a Benchmark object containing the result
// AbstractBenchmarkController is the base class of controler, it is a controller that does not take any measurment
class AbstractBenchmarkController {
public:
    AbstractBenchmarkController(const QString& testName, const QString& dataName, Benchmark* parent, int iterations = defaultIterations);
    virtual ~AbstractBenchmarkController() { if (!m_aborted) m_parent->addBenchmark(m_benchmark); }
    virtual void next() { ++m_currentIteration; }
    int iterations() const { return m_iterations; }
    int currentIteration() const { return m_currentIteration; }

    void abort() { m_aborted = true; }

    static void setDefaultIterations(int i) { defaultIterations = i; }
    static int defaultIterations;

protected:
    Benchmark m_benchmark;
    Benchmark* const m_parent;

#if (QT_VERSION >= 0x040700)
    QElapsedTimer m_timer;
#else
    QTime m_timer;
#endif

private:
    int m_currentIteration;
    const int m_iterations;
    bool m_aborted;
};


// classing benchmark controller to get the time of a section
class BenchmarkController : public AbstractBenchmarkController {
public:
    BenchmarkController(const QString& testName, const QString& dataName, Benchmark* parent);

    void next();
    void timeNow();

private:
    long long timeElapsed() const;
    bool m_timed;
};


// used to measure sub-sections of the code which is run multiple time
class SubSectionBenchmarkController : public AbstractBenchmarkController {
public:
    SubSectionBenchmarkController(const QString& testName, const QString& dataName, Benchmark* parent);

    void next();
    void startSubMeasure();
    void stopSubMeasure();

private:
    bool m_running;
    unsigned int m_iterationTime;
};


// used to mesure average time spent on each frame
class TimePerFrameBenchmarkController : public AbstractBenchmarkController {
public:
    TimePerFrameBenchmarkController(const QString& testName, const QString& dataName, Benchmark* parent);

    void next();
    void newFrame();

private:
    unsigned int m_frameCount;
};

#endif // BENCHMARKCONTROLLER_H
