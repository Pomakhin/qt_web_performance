#include <QtTest/QtTest>
#include <sys/time.h>

class tst_Time : public QObject {
    Q_OBJECT
private Q_SLOTS:
    void testMonotonic();
    void testGettimeofday();
};

double currentTime()
{
    struct timeval now;
    struct timezone zone;

    gettimeofday(&now, &zone);
    return static_cast<double>(now.tv_sec) + (double)(now.tv_usec / 1000000.0);
}

double currentTimeMono()
{
    struct timespec tp;

    clock_gettime(CLOCK_MONOTONIC, &tp);
    return static_cast<double>(tp.tv_sec) + static_cast<double>(tp.tv_nsec / 1000000000.0);
}



void tst_Time::testGettimeofday()
{
    QBENCHMARK {
        currentTime();        
    }
}

void tst_Time::testMonotonic()
{
    QBENCHMARK {
        currentTimeMono();
    }
}

QTEST_MAIN(tst_Time)
#include "time.moc"
