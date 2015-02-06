"""
 Copyright (C) 2010 Holger Hans Peter Freyther

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
"""

from mod_python import apache
import sqlite3

def send_header(req, header):
    list = ("%s" % header).split("\n")
    for header in list:
        if header[-1:] == '\r':
            header = header[:-1]
        if len(header) == 0:
            continue

        (tag, value) = header.split(': ', 1)
        if tag.lower() == "content-length":
            req.set_content_length(int(value))
        elif tag.lower() == "content-type":
            req.content_type = value
        elif tag.lower() == "set-cookie":
            req.headers_out.add('set-cookie', value)
        else:
            req.headers_out[tag] = value

def send_response(req, query):
    req.status = query[0]
    send_header(req, query[1])
    if query[0] != 204:
        req.write(query[2])
    return True

def handle_uri(connection, req, uri):
    uri = "http://" + req.hostname + req.unparsed_uri
    cur = connection.execute("SELECT response, header, data FROM responses WHERE url like ?", (uri,))
    for row in cur:
        rc = send_response(req, row)
        if rc:
            return rc
    return None

def handler(req):
    # connect to the database
    db_file = req.get_options().get('qtwebkit.db', '')
    connection = sqlite3.connect(db_file)

    # look up the file... we do it in various runs
    # 1.) try the full URL and see if it matches
    # 2.) remove the fragment and query part
    # 3.) send out a 404
    rc = handle_uri(connection, req, "http://" + req.hostname + req.unparsed_uri)
    if rc:
        return apache.OK

    if req.parsed_uri[apache.URI_QUERY] and req.parsed_uri[apache.URI_PATH]:
        uri = "http://" + req.hostname + req.parsed_uri[apache.URI_PATH] + "?" + req.parsed_uri[apache.URI_QUERY]
        rc = handle_uri(connection, req, uri)
        if rc:
            return apache.OK

    if req.parsed_uri[apache.URI_PATH]:
        uri = "http://" + req.hostname + req.parsed_uri[apache.URI_PATH]
        rc = handle_uri(connection, req, uri)
        if rc:
            return apache.OK


    req.log_error("Could not find URI: hostname: %s uri: %s" % (req.hostname, req.unparsed_uri))
    req.status = apache.HTTP_NOT_FOUND
    req.content_type = "text/plain"
    req.write("http://%s%s not found" % (req.hostname, req.unparsed_uri))
    return apache.OK
