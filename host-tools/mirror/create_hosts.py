#!/usr/bin/env python

import sqlite3, sys, optparse

def parse():
    parser = optparse.OptionParser(version = "Generate /etc/hosts entries",
                                   usage = "%prog [options] database")
    parser.add_option("-d", "--db", help = "The crawl database to be used",
                      action = "store", dest = "db_name", default="crawl_db.db")
    return parser.parse_args(sys.argv)

(opts, args) = parse()
connection = sqlite3.connect(opts.db_name)

urls = []
cursor = connection.execute("SELECT url FROM responses")
for row in cursor:
    url = str(row[0]).split("/")[2]

    # strip out the port
    if ':' in url:
        url = url.split(":", 1)[0]

    if not url in urls:
        urls.append(url)

for url in urls:
    print "%s\t%s" % (args[1], url)

