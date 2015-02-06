#!/usr/bin/env python

import sqlite3, sys, optparse

def parse():
    parser = optparse.OptionParser(version = "Update the HTTP headers",
                                   usage = "%prog [options] URL")
    parser.add_option("-d", "--db", help = "The crawl database to be used",
                      action = "store", dest = "db_name", default="crawl_db.db")
    return parser.parse_args(sys.argv)

(opts, _) = parse()
connection = sqlite3.connect(opts.db_name)


def to_num(length):
    if not length:
        return 0
    return int(length)

def header_content_length(old_header, new_length):
    """
    Update the Content Length
    """
    length = len("content-length: ")

    start = old_header.find("content-length: ")
    end = old_header.find("\r", start)

    old_length = old_header[start + length:end]
    if int(old_length) == new_length:
        return old_header
    new_entry = "content-length: %d" % new_length
    return old_header.replace(old_header[start:end], new_entry, 1)

def header_connection_close(old_header):
    """
    Remove the Connection: Something from the line
    """
    length = len("connection: ")

    start = old_header.lower().find("connection: ")
    if start < 0:
        return old_header
    end = old_header.find("\n", start)


    return "%s%s" % (old_header[0:start], old_header[end + 1:])

def update_header():
    cur = connection.execute("SELECT url, header, LENGTH(data) from responses")
    for row in cur:
        old_header = str(row[1])

        new_header = header_content_length(old_header, to_num(row[2]))
        new_header = header_connection_close(new_header)

        if old_header == new_header:
            continue

        connection.execute("UPDATE responses SET header = ? WHERE url like ?", [new_header, row[0]])

def replace_math_random(header):
    """
    Remove occurences of Math.random and replace it with
    a constant number. Currently 0.5 but there is no special
    reason for doing so.
    """

    if not "Math.random()" in str(header):
        return header

    return buffer(str(header).replace("Math.random()", "0.5"))

def replace_get_time(header):
    """
    Replace new Date().getTime()...
    """

    if not "new Date().getTime()" in str(header):
        return header
    return buffer(str(header).replace("new Date().getTime()", "1263154443910"))

def update_content():
    cur = connection.execute("SELECT url, data from responses")
    for row in cur:
        old_data = str(row[1])
        data = old_data

        data = replace_math_random(data)
        data = replace_get_time(data)

        if old_data == data:
            continue

        connection.execute("UPDATE responses SET data = ? WHERE url like ?", [data, row[0]])

update_content()
update_header()
connection.commit()
