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

#include "benchmarkcontroller.h"

AbstractBenchmarkController::AbstractBenchmarkController(const QString& testName, const QString& dataName, Benchmark* parent, int iterations)
    : m_benchmark(testName, dataName)
    , m_parent(parent)
    , m_currentIteration(0)
    , m_iterations(iterations)
    , m_aborted(false)
{
}

int AbstractBenchmarkController::defaultIterations = 11;

BenchmarkController::BenchmarkController(const QString& testName, const QString& dataName, Benchmark* parent)
    : AbstractBenchmarkController(testName, dataName, parent)
    , m_timed(false)
{
    m_timer.start();
}

void BenchmarkController::next()
{
    if (!m_timed && currentIteration() != 0)
        m_benchmark.addResult(timeElapsed());
    m_timed = false;
    AbstractBenchmarkController::next();

    m_timer.start();
}

long long BenchmarkController::timeElapsed() const
{
    return m_timer.elapsed();
}

void BenchmarkController::timeNow()
{
    if (currentIteration() != 0)
        m_benchmark.addResult(timeElapsed());
    m_timed = true;
}

SubSectionBenchmarkController::SubSectionBenchmarkController(const QString& testName, const QString& dataName, Benchmark *parent)
    : AbstractBenchmarkController(testName, dataName, parent)
    , m_running(false)
    , m_iterationTime(0)
{
}

void SubSectionBenchmarkController::next()
{
    stopSubMeasure();
    if (currentIteration() != 0)
        m_benchmark.addResult(m_iterationTime);
    AbstractBenchmarkController::next();
    m_iterationTime = 0;
}

void SubSectionBenchmarkController::startSubMeasure()
{
    m_running = true;
    m_timer.start();
}

void SubSectionBenchmarkController::stopSubMeasure()
{
    if (m_running) {
        m_running = false;
        m_iterationTime += m_timer.elapsed();
    }
}

TimePerFrameBenchmarkController::TimePerFrameBenchmarkController(const QString& testName, const QString& dataName, Benchmark* parent)
    : AbstractBenchmarkController(testName, dataName, parent)
    , m_frameCount(0)
{
    m_timer.start();
}

void TimePerFrameBenchmarkController::next()
{
    if (currentIteration() != 0)
        m_benchmark.addResult(m_timer.elapsed() / m_frameCount);

    m_frameCount = 0;
    AbstractBenchmarkController::next();

    m_timer.start();
}

void TimePerFrameBenchmarkController::newFrame()
{
    ++m_frameCount;
}

