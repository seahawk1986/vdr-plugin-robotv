/*
 *      vdr-plugin-robotv - RoboTV server plugin for VDR
 *
 *      Copyright (C) 2015 Alexander Pipelka
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

#ifndef ROBOTV_STREAMINFO_H
#define ROBOTV_STREAMINFO_H

#include <stdint.h>
#include <fstream>
#include <string>

#include "net/msgpacket.h"

class cStreamInfo {
public:

    enum Content {
        scNONE,
        scVIDEO,
        scAUDIO,
        scSUBTITLE,
        scTELETEXT,
        scSTREAMINFO
    };

    enum Type {
        stNONE,
        stMPEG2AUDIO,
        stAC3,
        stEAC3,
        stAAC,
        stLATM,
        stMPEG2VIDEO,
        stH264,
        stDVBSUB,
        stTELETEXT,
        stH265
    };

    enum FrameType {
        ftUNKNOWN,
        ftIFRAME,
        ftPFRAME,
        ftBFRAME,
        ftDFRAME
    };

public:

    cStreamInfo();

    cStreamInfo(int pid, Type type, const char* lang = NULL);

    virtual ~cStreamInfo();

    bool operator ==(const cStreamInfo& rhs) const;

    bool ismetaof(const cStreamInfo& rhs) const;

    bool operator !=(const cStreamInfo& rhs) const;

    const int GetPID() const {
        return m_pid;
    }

    void SetContent();

    static const Content GetContent(Type type);

    const Content GetContent() const {
        return m_content;
    }

    const Type GetType() const {
        return m_type;
    }

    const char* TypeName();

    static const char* TypeName(const cStreamInfo::Type& type);

    static const char* ContentName(const cStreamInfo::Content& content);

    void info() const;

    inline bool IsParsed() {
        return m_parsed;
    }

    void SetSubtitlingDescriptor(unsigned char SubtitlingType, uint16_t CompositionPageId, uint16_t AncillaryPageId);

protected:

    Content m_content;   // stream content (e.g. scVIDEO)
    Type m_type;         // stream type (e.g. stAC3)
    int m_pid;           // transport stream pid

    char m_language[4];  // ISO 639 3-letter language code (empty string if undefined)
    uint8_t m_audiotype; // ISO 639 audio type

    int m_fpsscale;      // scale of 1000 and a rate of 29970 will result in 29.97 fps
    int m_fpsrate;
    int m_height;        // height of the stream reported by the demuxer
    int m_width;         // width of the stream reported by the demuxer
    float m_aspect;      // display aspect of stream

    int m_channels;      // number of audio channels (e.g. 6 for 5.1)
    int m_samplerate;    // number of audio samples per second (e.g. 48000)
    int m_bitrate;       // audio bitrate (e.g. 160000)
    int m_bitspersample; // number of bits per audio sample (e.g. 16)
    int m_blockalign;    // number of bytes per audio block

    bool m_parsed;       // stream parsed flag (if all stream data is known)

    unsigned char m_subtitlingtype; // subtitling type
    uint16_t m_compositionpageid;   // composition page id
    uint16_t m_ancillarypageid;     // ancillary page id

    // decoder data
    uint8_t m_sps[128];  // SPS data (for decoder)
    uint8_t m_pps[128];  // PPS data (for decoder)
    uint8_t m_vps[128];  // VPS data (for decoder)

    int m_spsLength;     // SPS length
    int m_ppsLength;     // PPS length
    int m_vpsLength;     // VPS length

    friend MsgPacket& operator<< (MsgPacket& lhs, const cStreamInfo& rhs);
    friend MsgPacket& operator>> (MsgPacket& lhs, cStreamInfo& rhs);

private:

    void Initialize();

};

MsgPacket& operator>> (MsgPacket& lhs, cStreamInfo& rhs);
MsgPacket& operator<< (MsgPacket& lhs, const cStreamInfo& rhs);

#endif // ROBOTV_STREAMINFO_H
