/*
 *      vdr-plugin-robotv - roboTV server plugin for VDR
 *
 *      Copyright (C) 2016 Alexander Pipelka
 *
 *      https://github.com/pipelka/vdr-plugin-robotv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "parser.h"
#include "pes.h"

Parser::Parser(TsDemuxer* demuxer, int buffersize, int packetsize) : cRingBufferLinear(buffersize, packetsize), m_demuxer(demuxer), m_startup(true) {
    m_sampleRate = 0;
    m_bitRate = 0;
    m_channels = 0;
    m_duration = 0;
    m_headerSize = 0;
    m_frameType = StreamInfo::ftUNKNOWN;

    m_curPts = DVD_NOPTS_VALUE;
    m_curDts = DVD_NOPTS_VALUE;

    m_lastPts = DVD_NOPTS_VALUE;
    m_lastDts = DVD_NOPTS_VALUE;
}

Parser::~Parser() {
}

int Parser::parsePesHeader(uint8_t* buf, size_t len) {
    // parse PES header
    unsigned int hdr_len = PesPayloadOffset(buf);

    // PTS / DTS
    int64_t pts = PesHasPts(buf) ? PesGetPts(buf) : DVD_NOPTS_VALUE;
    int64_t dts = PesHasDts(buf) ? PesGetDts(buf) : DVD_NOPTS_VALUE;

    if(dts == DVD_NOPTS_VALUE) {
        dts = pts;
    }

    if(m_curDts == DVD_NOPTS_VALUE) {
        m_curDts = dts;
    }

    if(m_curPts == DVD_NOPTS_VALUE) {
        m_curPts = pts;
    }

    return hdr_len;
}

void Parser::sendPayload(unsigned char* payload, int length) {
    StreamPacket pkt;
    pkt.data      = payload;
    pkt.size      = length;
    pkt.duration  = m_duration;
    pkt.dts       = m_curDts;
    pkt.pts       = m_curPts;
    pkt.frameType = m_frameType;

    m_demuxer->sendPacket(&pkt);
}

void Parser::putData(unsigned char* data, int length, bool pusi) {
    // get PTS / DTS on PES start
    if(pusi) {
        int offset = parsePesHeader(data, length);
        data += offset;
        length -= offset;

        m_startup = false;
    }

    // put data
    if(!m_startup && length > 0 && data != NULL) {
        int put = Put(data, length);

        // reset buffer on overflow
        if(put < length) {
            esyslog("Parser buffer overflow - resetting");
            Clear();
        }
    }
}

void Parser::parse(unsigned char* data, int datasize, bool pusi) {
    // get available data
    int length = 0;
    uint8_t* buffer = Get(length);

    // do we have a sync ?
    int framesize = 0;

    if(length > m_headerSize && buffer != NULL && checkAlignmentHeader(buffer, framesize, true)) {
        // valid framesize ?
        if(framesize > 0 && length >= framesize + m_headerSize) {

            // check for the next frame (eliminate false positive header checks)
            int next_framesize = 0;

            if(!checkAlignmentHeader(&buffer[framesize], next_framesize, false)) {
                esyslog("next frame not found on expected position, searching ...");
            }
            else {
                // check if we should extrapolate the timestamps
                if(m_curPts == DVD_NOPTS_VALUE) {
                    m_curPts = PtsAdd(m_lastPts, m_duration);
                }

                if(m_curDts == DVD_NOPTS_VALUE) {
                    m_curDts = PtsAdd(m_lastDts, m_duration);
                }

                int length = parsePayload(buffer, framesize);
                sendPayload(buffer, length);

                // keep last timestamp
                m_lastPts = m_curPts;
                m_lastDts = m_curDts;

                // reset timestamps
                m_curPts = DVD_NOPTS_VALUE;
                m_curDts = DVD_NOPTS_VALUE;

                Del(framesize);
                putData(data, datasize, pusi);
                return;
            }
        }

    }

    // try to find sync
    int offset = findAlignmentOffset(buffer, length, 1, framesize);

    if(offset != -1) {
        isyslog("sync found at offset %i (streamtype: %s / %i bytes in buffer / framesize: %i bytes)", offset, m_demuxer->typeName(), Available(), framesize);
        Del(offset);
    }
    else if(length > m_headerSize) {
        Del(length - m_headerSize);
    }

    putData(data, datasize, pusi);
}

int Parser::parsePayload(unsigned char* payload, int length) {
    return length;
}

int Parser::findAlignmentOffset(unsigned char* buffer, int buffersize, int o, int& framesize) {
    framesize = 0;

    // seek sync
    while(o < (buffersize - m_headerSize) && !checkAlignmentHeader(buffer + o, framesize, false)) {
        o++;
    }

    // not found
    if(o >= buffersize - m_headerSize || framesize <= 0) {
        return -1;
    }

    return o;
}

bool Parser::checkAlignmentHeader(unsigned char* buffer, int& framesize, bool parse) {
    framesize = 0;
    return true;
}

int Parser::findStartCode(unsigned char* buffer, int buffersize, int offset, uint32_t startcode, uint32_t mask) {
    uint32_t sc = 0xFFFFFFFF;

    while(offset < buffersize) {

        sc = (sc << 8) | buffer[offset++];

        if((uint32_t)(sc & mask) == startcode) {
            return offset - 4;
        }
    }

    return -1;
}
