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

#ifndef BENCHMARK_P_H
#define BENCHMARK_P_H

#include <QString>
#include <QList>

/**
 * Data type for storing benchmark results
 */
class Benchmark {
public:
    Benchmark(const QString& testName, const QString& dataName = QString());

    QString testName() const { return m_testName; }
    QString dataName() const { return m_dataName; }
    bool isEmpty() const { return m_empty; }

    void addBenchmark(const Benchmark&);
    QList<Benchmark> benchmarks() const { return m_benchmarks; }

    void addResult(long long result);
    QList<long long> results() const { return m_results; }

private:
    bool m_empty;
    QString m_testName;
    QString m_dataName;
    QList<long long> m_results;
    QList<Benchmark> m_benchmarks;
};

#endif // BENCHMARK_P_H
