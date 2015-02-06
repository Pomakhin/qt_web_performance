/*
 * Copyright (C) 2010 Guillaume Chereau
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

// Perfom load test of the javascript parser of webkit.  The test will
// attempt to parse a -huge- set of javascript fragments.  The data
// should be located in the file "data.txt".  All the fragments are
// separated by lines "NEW FRAGMENT".
//
// One way to automatically extract data from any web site is to
// modify the code of webkit, in the file
// "./JavaScriptCore/parser/Parser.cpp" before the call to jscyyparse,
// add a command to print the source in the provider variable to the
// standard output, preceded by a line "NEW FRAGMENT".

#include <QtTest/QtTest>
#include <QDebug>
#include <QApplication>

#include "benchmark.h"

#include <JSGlobalData.h>
#include <InitializeThreading.h>
#include <Parser.h>
#include <UString.h>
#include <Lexer.h>

#ifndef yyparse
extern int jscyyparse(void*);
#endif

// Class used to parse the JavaScript code fragments.
class Parser
{
public:
    Parser()
    {
        JSC::initializeThreading();
        m_globalData = JSC::JSGlobalData::create().releaseRef();
        Q_ASSERT(m_globalData->parser);
    }

    ~Parser()
    {
       m_globalData->heap.destroy();
       delete m_globalData;
    }

    int parse(const QString& fragment)
    {
        // Set up the parsing context.  Hopefully this is fast
        // compared to the actual parsing operation.
        JSC::UString string((const UChar*)fragment.constData(), fragment.size());
        WTF::PassRefPtr<JSC::UStringSourceProvider> provider = JSC::UStringSourceProvider::create(string, "");
        JSC::SourceCode code(provider, 0);
        m_globalData->lexer->setCode(code, m_globalData->parser->arena());
        int parseError = jscyyparse(m_globalData);
        // Free all the used memory.
        m_globalData->lexer->clear();
        m_globalData->parser->arena().reset();
        return parseError;
    }

private:
    JSC::JSGlobalData* m_globalData;
};

class tst_JavaScriptParsing : public QObject {
    Q_OBJECT

public:
    ~tst_JavaScriptParsing()
    {
         benchmarkOutput();
    }

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testParsing();

private:
    // Contains all the JavaScript code fragments.
    QStringList fragments;
    Parser parser;
};

void tst_JavaScriptParsing::initTestCase()
{
    // Load all the fragments from the data file.  In the file the
    // fragments are delimited by the line : "NEW FRAGMENT".
    QFile file("data.txt");
    if (!file.open(QIODevice::ReadOnly))
        QFAIL("Can not open the data file \"data.txt\".");
    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString all = in.readAll();
    fragments = all.split("\nNEW FRAGMENT\n");
}

void tst_JavaScriptParsing::cleanupTestCase()
{
    fragments.clear();
}

void tst_JavaScriptParsing::testParsing()
{
    WEB_BENCHMARK("javascript-parsing") {
        foreach(const QString& fragment, fragments) {
            parser.parse(fragment);
        }
    }
}

QTEST_MAIN(tst_JavaScriptParsing)
#include "main.moc"
