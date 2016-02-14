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

#ifndef ROBOTV_SERVER_H
#define ROBOTV_SERVER_H

#include <list>
#include <vdr/thread.h>

#include "config/config.h"

class cRoboTVClient;

class cRoboTVServer : public cThread {
protected:

    typedef std::list<cRoboTVClient*> ClientList;

    virtual void Action(void);
    void NewClientConnected(int fd);

    int           m_ServerPort;
    int           m_ServerFD;
    bool          m_IPv4Fallback;
    cString       m_AllowedHostsFile;
    ClientList    m_clients;

    static unsigned int m_IdCnt;

public:
    cRoboTVServer(int listenPort);
    virtual ~cRoboTVServer();
};

#endif // ROBOTV_SERVER_H
