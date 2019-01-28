/*
* Liam Ashdown
* Copyright (C) 2018
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _QuadEmu_QuadEngine_h_
#define _QuadEmu_QuadEngine_h_
#include "../Server/Protocol/Listener.h"
#include "../Common/SharedDefines.h"
#endif /* _QuadEmu_QuadEngine_ */

class QuadEngine
{
public:
    QuadEngine() {}
    ~QuadEngine() {}

    void Boot();
    void LoadPublicRoomsPort();

protected:
    std::vector<std::shared_ptr<Listener>> publicRoomListener;
    boost::asio::io_service io_service;
};
