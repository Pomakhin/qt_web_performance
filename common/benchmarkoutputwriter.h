/*
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

#ifndef BENCHMARKOUTPUTWRITER_H
#define BENCHMARKOUTPUTWRITER_H

#include <QFile>
#include <QString>

class Benchmark;
class QIODevice;

struct SummaryResult {
    SummaryResult()
        : mean(0)
        , average(0)
        , size(0)
    {}

    long long mean;
    long long average;
    int size;
};

class BenchmarkOutputWriter
{
public:
    BenchmarkOutputWriter(const QString& path = QString());
    virtual ~BenchmarkOutputWriter();
    virtual void writeOutput(const Benchmark& parent) = 0;
protected:
    QFile* output;
};

class BenchmarkOutputHuman : public BenchmarkOutputWriter
{
public:
    BenchmarkOutputHuman(const QString& path = QString());
    void writeOutput(const Benchmark& parent);
private:
    SummaryResult benchmarkOutputHumanReadableImpl(const Benchmark& parent, const QString& indent = QString());
};

class BenchmarkOutputCsv : public BenchmarkOutputWriter
{
public:
    BenchmarkOutputCsv(const QString& path = QString());
    void writeOutput(const Benchmark& parent);
};

#endif // BENCHMARKOUTPUTWRITER_H
