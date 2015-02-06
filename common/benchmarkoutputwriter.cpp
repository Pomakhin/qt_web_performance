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

#include "benchmarkoutputwriter.h"

#include "benchmark.h"
#include <math.h>
#include <stdio.h>
#include <QDebug>

BenchmarkOutputWriter::BenchmarkOutputWriter(const QString &path)
{
    if (!path.isEmpty()) {
        output = new QFile(path);
        if (output->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            return;
        } else {
            qWarning() << "Error opening the output file, falling back to default. Error: " << output->errorString();
            delete output;
        }
    }
    output = new QFile();
    if(!output->open(stdout, QIODevice::WriteOnly)) {
        fprintf(stderr, "Error opening the output: %s\n", qPrintable(output->errorString()));
        qFatal("No output");
    }
}

BenchmarkOutputWriter::~BenchmarkOutputWriter()
{
    delete output;
}

static long long benchmarkMean(const Benchmark& _results)
{
    QList<long long> results = _results.results();
    qSort(results);

    return results.at(results.size() / 2);
}

static long long benchmarkAverage(const Benchmark& _results)
{
    long long res = 0;
    foreach(long long val, _results.results())
        res += val;

    return res / _results.results().size();
}

static double benchmarkStdDeviationBiased(const Benchmark& benchmark)
{
    long long mean = benchmarkMean(benchmark);
    long long squares = 0;

    foreach (long long res, benchmark.results()) {
        long long delta = res - mean;
        squares += (delta * delta);
        if (squares < 0)
            qFatal("numbers wrapped");
    }

    double variance = benchmark.results().size() - 1.0;
    variance = squares / variance;
    return sqrt(variance);
}

BenchmarkOutputHuman::BenchmarkOutputHuman(const QString &path)
    : BenchmarkOutputWriter(path)
{ }

void BenchmarkOutputHuman::writeOutput(const Benchmark& parent)
{
    benchmarkOutputHumanReadableImpl(parent);
}

SummaryResult BenchmarkOutputHuman::benchmarkOutputHumanReadableImpl(const Benchmark& parent, const QString& indent)
{
    SummaryResult result;

    const QString headerString = QString::fromLatin1("%1Printing result for: %2\n").arg(indent).arg(parent.testName());
    output->write(headerString.toLatin1());
    foreach (Benchmark bench, parent.benchmarks()) {
        if (!bench.benchmarks().isEmpty()) {
            SummaryResult sum = benchmarkOutputHumanReadableImpl(bench, indent + QChar('\t'));
            result.mean += sum.mean;
            result.average += sum.average;
        } else {
            long long run = benchmarkMean(bench);
            long long avg = benchmarkAverage(bench);
            int stddev = abs(benchmarkStdDeviationBiased(bench));
            QString outputString = QString::fromLatin1("%1\tbenchmark: %2\t%3\n"
                                                       "%1\t\tmean: %4 msecs +/- %5 msecs, +/- %6 %\n"
                                                       "%1\t\tavg:  %7 msecs\n");
            outputString = outputString.arg(indent).arg(bench.testName()).arg(bench.dataName()).arg(run).arg(stddev).arg(stddev * 100.0 / run).arg(avg);
            output->write(outputString.toLatin1());

            output->write(indent.toLatin1());
            output->write("\t\t\t");
            foreach (long long res, bench.results()) {
                output->write(QString::number(res).toLatin1());
                output->write(" ");
            }
            output->write("\n");
            result.mean += run;
            result.average += avg;
        }
        ++result.size;
    }

    if (indent.isEmpty()) {
        const QString totalString = QString::fromLatin1("Total mean: %1 msecs per test, avg: %2\n").arg(result.mean / result.size).arg(result.average / result.size);
        output->write(totalString.toLatin1());
    }
    output->flush();
    return result;
}

BenchmarkOutputCsv::BenchmarkOutputCsv(const QString &path)
    : BenchmarkOutputWriter(path)
{ }

void BenchmarkOutputCsv::writeOutput(const Benchmark& parent)
{
    foreach (Benchmark bench, parent.benchmarks()) {
        if (bench.benchmarks().isEmpty()) {
            long long run = benchmarkMean(bench);
            long long avg = benchmarkAverage(bench);
            int stddev = abs(benchmarkStdDeviationBiased(bench));
            QString outputString = QString::fromLatin1("\"%1\",\"%2\",%3,%4,%5\n");
            outputString = outputString.arg(bench.testName()).arg(bench.dataName()).arg(run).arg(stddev).arg(avg);
            output->write(outputString.toLatin1());
        }
    }
}
