#!/usr/bin/env python

import sqlite3, sys, optparse

def parse():
    parser = optparse.OptionParser(version = "Store all data to disk",
                                   usage = "%prog [options]")
    parser.add_option("-d", "--db", help = "The crawl database to be used",
                      action = "store", dest = "db_name", default="crawl_db.db")
    parser.add_option("-o", "--output", help = "Basename of the files..",
                      action = "store", dest = "output", default="dump-")
    return parser.parse_args(sys.argv)

(opts, args) = parse()
connection = sqlite3.connect(opts.db_name)

i = 0
cursor = connection.execute("SELECT url, data FROM responses")
for row in cursor:
    out = open("%s-%.12d" % (opts.output, i), "w")
    out.write(row[0])
    out.write("\n")
    if row[1]:
        out.write(row[1])
    i = i + 1
