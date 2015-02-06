/*
 * Copyright (C) 2010 Holger Hans Peter Freyther
 * Copyright (C) 2010 Benjamin Poulain, Nokia
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "benchmark.h"
#include "benchmark_p.h"

#include <QEventLoop>
#include <QSignalSpy>
#include <QTimer>

Benchmark::Benchmark(const QString& testName, const QString& dataName)
    : m_empty(false)
    , m_testName(testName)
    , m_dataName(dataName)
{
}

void Benchmark::addBenchmark(const Benchmark& benchmark)
{
    if (!m_results.isEmpty())
        qFatal("You can not add benchmarks and results.");
    m_benchmarks.append(benchmark);
}

void Benchmark::addResult(long long result)
{
    if (!m_benchmarks.isEmpty())
        qFatal("You can not add benchmarks and results.");

    m_results.append(result);
}

QSharedPointer<BenchmarkOutputWriter> outWriter(0);

void benchmarkOutput()
{
    outWriter->writeOutput(*benchmark_parent);
}

bool waitForSignal(QObject* obj, const char* signal, int timeout)
{
    QEventLoop loop;
    QObject::connect(obj, signal, &loop, SLOT(quit()));
    QTimer timer;
    QSignalSpy timeoutSpy(&timer, SIGNAL(timeout()));
    if (timeout > 0) {
        QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
        timer.setSingleShot(true);
        timer.start(timeout);
    }
    loop.exec();
    return timeoutSpy.isEmpty();
}

Benchmark* benchmark_parent = 0;
