#include "common_init.h"

#include <QTest>

Q_GLOBAL_STATIC(QList<QUrl>,globalUrlList)

void add_url_to_test_urls(const QUrl &url) {
    *globalUrlList() << url;
}

QList<QUrl> test_urls()
{
    if (globalUrlList()->isEmpty()) {
        // populate with the old standard test set
        /**
         * Created from do_mirror.sh with the following shell foo:
         *  for site in $WEBSITES; do echo 'QTest::newRow("rowXY") <<' QUrl(\"$site\"\);; done
         */

       // add_url_to_test_urls(QUrl("http://172.30.24.248/frontend/site/index?autotest=yes"));
          add_url_to_test_urls(QUrl("http://devlauncher.q1.ru/system/index?autotest=yes"));

//        add_url_to_test_urls(QUrl("http://www.google.com/ncr"));
//        add_url_to_test_urls(QUrl("http://maps.google.com"));
//        add_url_to_test_urls(QUrl("http://www.apple.com"));
//        add_url_to_test_urls(QUrl("http://amazon.com"));
//        add_url_to_test_urls(QUrl("http://ebay.com"));
//        add_url_to_test_urls(QUrl("http://bbc.co.uk"));
//        add_url_to_test_urls(QUrl("http://www.guardian.co.uk"));
//        add_url_to_test_urls(QUrl("http://www.google.com/news?ned=us"));
//        add_url_to_test_urls(QUrl("http://news.yahoo.com"));
//        add_url_to_test_urls(QUrl("http://flickr.com"));
//        add_url_to_test_urls(QUrl("http://www.youtube.com/?hl=en_US"));
//        add_url_to_test_urls(QUrl("http://myspace.com"));
//        add_url_to_test_urls(QUrl("http://www.twitter.com"));
//        add_url_to_test_urls(QUrl("http://twitter.com/BARACKOBAMA"));
//        add_url_to_test_urls(QUrl("http://reddit.com"));
//        add_url_to_test_urls(QUrl("http://www.facebook.com"));
//        add_url_to_test_urls(QUrl("http://www.gmail.com"));
//        add_url_to_test_urls(QUrl("http://mail.yahoo.com"));
//        add_url_to_test_urls(QUrl("http://hi5.com"));
//        add_url_to_test_urls(QUrl("http://www.dailymotion.com"));
//        add_url_to_test_urls(QUrl("http://youporn.com"));
//        add_url_to_test_urls(QUrl("http://www.microsoft.com/en/us/default.aspx"));
//        add_url_to_test_urls(QUrl("http://www.live.com"));
//        add_url_to_test_urls(QUrl("http://www.wikipedia.org"));
//        add_url_to_test_urls(QUrl("http://en.wikipedia.org"));
//        add_url_to_test_urls(QUrl("http://www.wordpress.com"));
//        add_url_to_test_urls(QUrl("http://www.rapidshare.com"));
//        add_url_to_test_urls(QUrl("http://www.blogger.com"));
//        add_url_to_test_urls(QUrl("http://www.bing.com"));
//        add_url_to_test_urls(QUrl("http://www.imdb.com"));
//        add_url_to_test_urls(QUrl("http://www.linkedin.com"));
//        add_url_to_test_urls(QUrl("http://en.wikipedia.org/wiki/Maxwell_equations"));
//        add_url_to_test_urls(QUrl("http://naver.com"));
//        add_url_to_test_urls(QUrl("http://kr.yahoo.com"));
//        add_url_to_test_urls(QUrl("http://www.google.co.kr/"));
//        add_url_to_test_urls(QUrl("http://www.auction.co.kr"));
//        add_url_to_test_urls(QUrl("http://www.youtube.com/?hl=ko-KR"));
    }
    return *globalUrlList();
}

void add_test_urls()
{
    QList<QUrl> row = test_urls();
    QTest::addColumn<QUrl>("url");

    for (int i = 0; i < row.count(); ++i)
        QTest::newRow(QString("row%1").arg(i+1).toLatin1()) << row[i];

}
