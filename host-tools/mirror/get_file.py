#!/usr/bin/env python

import sqlite3, sys, optparse

def parse():
    parser = optparse.OptionParser(version = "Print the content of a file to stdout",
                                   usage = "%prog [options] URL")
    parser.add_option("-d", "--db", help = "The crawl database to be used",
                      action = "store", dest = "db_name", default="crawl_db.db")
    parser.add_option("-o", "--output", help = "Write the file to output",
                      action = "store", dest = "output", default=None)
    return parser.parse_args(sys.argv)

(opts, args) = parse()
connection = sqlite3.connect(opts.db_name)

out = sys.stdout
if opts.output:
    out = open(opts.output, "w")

cursor = connection.execute("SELECT data FROM responses WHERE url like ?", [args[1]])
for row in cursor:
    if row[0]:
        out.write(row[0])
