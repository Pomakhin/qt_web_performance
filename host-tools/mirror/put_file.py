#!/usr/bin/env python

import sqlite3, sys, optparse

def parse():
    parser = optparse.OptionParser(version = "Put a file into the db",
                                   usage = "%prog [options] URL")
    parser.add_option("-d", "--db", help = "The crawl database to be used",
                      action = "store", dest = "db_name", default="crawl_db.db")
    parser.add_option("-i", "--input", help = "Read the file from..",
                      action = "store", dest = "input", default=None)
    return parser.parse_args(sys.argv)

(opts, args) = parse()
connection = sqlite3.connect(opts.db_name)

inp = sys.stdin
if opts.input:
    inp = open(opts.input, "r")

all = inp.read()

connection.execute("UPDATE responses SET data = ? WHERE url like ?", [all, args[1]])
connection.commit()
