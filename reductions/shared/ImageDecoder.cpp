/*
 * Copyright (C) 2008-2009 Torch Mobile, Inc.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

//#include "config.h"

#include "ImageDecoder.h"
#include "GIFImageDecoder.h"
#include "JPEGImageDecoder.h"
#include "PNGImageDecoder.h"

#if 0
#include "ICOImageDecoder.h"
#include "BMPImageDecoder.h"
#include "XBMImageDecoder.h"
#endif

namespace WebCore {

ImageDecoder* ImageDecoder::create(const QByteArray* data)
{
    // We need at least 4 bytes to figure out what kind of image we're dealing with.
    int length = data->size();
    if (length < 4)
        return 0;

    const unsigned char* uContents = (const unsigned char*)data->data();
    const char* contents = data->data();

    // GIFs begin with GIF8(7 or 9).
    if (strncmp(contents, "GIF8", 4) == 0)
        return new GIFImageDecoder();

    // Test for PNG.
    if (uContents[0]==0x89 &&
        uContents[1]==0x50 &&
        uContents[2]==0x4E &&
        uContents[3]==0x47)
        return new PNGImageDecoder();

    // JPEG
    if (uContents[0]==0xFF &&
        uContents[1]==0xD8 &&
        uContents[2]==0xFF)
        return new JPEGImageDecoder();

#if 0
    // BMP
    if (strncmp(contents, "BM", 2) == 0)
        return new BMPImageDecoder();

    // ICOs always begin with a 2-byte 0 followed by a 2-byte 1.
    // CURs begin with 2-byte 0 followed by 2-byte 2.
    if (!memcmp(contents, "\000\000\001\000", 4) ||
        !memcmp(contents, "\000\000\002\000", 4))
        return new ICOImageDecoder();

    // XBMs require 8 bytes of info.
    if (length >= 8 && strncmp(contents, "#define ", 8) == 0)
        return new XBMImageDecoder();
#endif

    // Give up. We don't know what the heck this is.
    return 0;
}

RGBA32Buffer::RGBA32Buffer()
    : m_duration(0)
    , m_disposalMethod(DisposeNotSpecified)
    , m_status(FrameEmpty)
    , m_hasAlpha(false)
{
}

void RGBA32Buffer::clear()
{
    m_bytes.clear();
    m_status = FrameEmpty;
    // NOTE: Do not reset other members here; clearFrameBufferCache()
    // calls this to free the bitmap data, but other functions like
    // initFrameBuffer() and frameComplete() may still need to read
    // other metadata out of this frame later.
}

void RGBA32Buffer::zeroFill()
{
    m_bytes.fill(0);
    m_hasAlpha = true;
}

void RGBA32Buffer::copyBitmapData(const RGBA32Buffer& other)
{
    if (this == &other)
        return;

    m_bytes = other.m_bytes;
    m_size = other.m_size;
    setHasAlpha(other.m_hasAlpha);
}

bool RGBA32Buffer::setSize(int newWidth, int newHeight)
{
    // NOTE: This has no way to check for allocation failure if the
    // requested size was too big...
    m_bytes.resize(newWidth * newHeight * sizeof(PixelData));
    m_size = IntSize(newWidth, newHeight);

    // Zero the image.
    zeroFill();

    return true;
}

bool RGBA32Buffer::hasAlpha() const
{
    return m_hasAlpha;
}

void RGBA32Buffer::setHasAlpha(bool alpha)
{
    m_hasAlpha = alpha;
}

void RGBA32Buffer::setStatus(FrameStatus status)
{
    m_status = status;
}

RGBA32Buffer& RGBA32Buffer::operator=(const RGBA32Buffer& other)
{
    if (this == &other)
        return *this;

    copyBitmapData(other);
    setStatus(other.status());
    setDuration(other.duration());
    setDisposalMethod(other.disposalMethod());
    return *this;
}

int RGBA32Buffer::width() const
{
    return m_size.width();
}

int RGBA32Buffer::height() const
{
    return m_size.height();
}

}
