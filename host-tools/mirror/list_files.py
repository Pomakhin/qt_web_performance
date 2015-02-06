#!/usr/bin/env python

import sqlite3, sys, optparse

def parse():
    parser = optparse.OptionParser(version = "List URLs/files fetched",
                                   usage = "%prog [options] file query")
    parser.add_option("-d", "--db", help = "The crawl database to be used",
                      action = "store", dest = "db_name", default="crawl_db.db")
    return parser.parse_args(sys.argv)

(opts, args) = parse()
connection = sqlite3.connect(opts.db_name)

urls = []
cursor = connection.execute("SELECT url FROM responses")
for row in cursor:
    url = str(row[0])

    if not url in urls:
        urls.append(url)

urls.sort()
for url in urls:
    print "%s" % (url)

