#!/usr/bin/env python

import sqlite3, sys, optparse

def parse():
    parser = optparse.OptionParser(version = "Delete URLs from the db",
                                   usage = "%prog [options]y")
    parser.add_option("-d", "--db", help = "The crawl database to be used",
                      action = "store", dest = "db_name", default="crawl_db.db")
    return parser.parse_args(sys.argv)

(opts, args) = parse()
connection = sqlite3.connect(opts.db_name)

lines = sys.stdin.readlines()
for line in lines:
    line = line.strip()
    connection.execute("DELETE FROM responses where url like ?", (line,))
connection.commit()


