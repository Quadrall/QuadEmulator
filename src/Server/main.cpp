/*
* Liam Ashdown
* Copyright (C) 2019
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

#include "HabboSocket.h"
#include "Hotel.h"
#include "RoomManager.h"
#include "Database/DatabaseTypes.h"
#include "TriggerEventManager.h"
#include "Platform/ProcessPriority.h"

DatabaseType RoomDatabase;
DatabaseType UserDatabase;

bool LoadDatabase();

int main()
{
    /// Initialize our logger
    static plog::ColorConsoleAppender<plog::TxtFormatter> l_ConsoleAppender;
    plog::init(plog::verbose, &l_ConsoleAppender);

    LOG_NONE << " ______     ______   ______     ______     ______     ______     ______   ______     __   __     ______ ";
    LOG_NONE << "/\\  ___\\   /\\__  _\\ /\\  ___\\   /\\  ___\\   /\\  == \\   /\\  ___\\   /\\__  _\\ /\\  __ \\   /\\ ' - .\\ \ / \\  ___\\     ";
    LOG_NONE << "\\ \\___  \\  \\/_/\\ \\/ \\ \\  __\\   \\ \\  __\\   \\ \\  __<   \\ \\___  \\  \\/_/\\ \\/ \\ \\ \\/\\ \\  \\ \\ \\-.  \\  \\ \\  __\\     ";
    LOG_NONE << " \\/\\_____\\    \\ \\_\\  \\ \\_____\\  \\ \\_____\\  \\ \\_\\ \\_\\  \\/\\_____\\    \\ \\_\\  \\ \\_____\\  \\ \\_\\\\'\\_\\  \\ \\_____\\   ";
    LOG_NONE << "  \\/_____/     \\/_/   \\/_____/   \\/_____/   \\/_/ /_/   \\/_____/     \\/_/   \\/_____/   \\/_/ \\/_/   \\/_____/    ";
    LOG_NONE << "                                                                    Powered by Boost & MySQL";

    if (!sConfig->SetFile("server.conf"))
    {
        LOG_ERROR << "Could not find server.conf";
        Sleep(5000);
        return 1;
    }

    if (!LoadDatabase())
    {
        Sleep(5000);
        return -1;
    }

    SetProcessPriority(sConfig->GetIntDefault("UseProcessors", 0), sConfig->GetIntDefault("ProcessPriority", 0));
    
    uint32 l_StartTime = sHotelTimer->GetServerTime();

    sHotel->LoadConfigs();
    sOpcode->InitializePackets();
    sTriggerMgr->LoadTriggerEvents();
    sItemMgr->LoadPublicRoomItems();
    sRoomMgr->LoadRoomModels();
    sRoomMgr->LoadRoomUrls();
    sRoomMgr->LoadRoomCategories();
    sRoomMgr->LoadRoomWalkWays();
    sRoomMgr->LoadRooms();
    sRoomMgr->LoadRoomRights();
    sRoomMgr->LoadVotedUsers();

    SteerStone::Listener<SteerStone::HabboSocket> l_Listener(sConfig->GetStringDefault("BindIP", "127.0.0.1"), sHotel->GetIntConfig(SteerStone::IntConfigs::CONFIG_SERVER_PORT),
        5);

    uint32 l_EndTime = sHotelTimer->GetTimeDifference(l_StartTime, sHotelTimer->GetServerTime());

    LOG_INFO << "Booted up server in " << l_EndTime <<  " milliseconds";

    LOG_INFO << "Successfully booted up SteerStone! Listening on " << sConfig->GetStringDefault("BindIP", "127.0.0.1") << " " << sHotel->GetIntConfig(SteerStone::IntConfigs::CONFIG_SERVER_PORT);

    uint32 l_RealCurrTime = 0;
    uint32 l_RealPrevTime = sHotelTimer->Tick();
    uint32 l_PrevSleepTime = 0;

    while (!SteerStone::Hotel::StopWorld())
    {
        l_RealCurrTime = sHotelTimer->GetServerTime();

        uint32 const& l_Diff = sHotelTimer->Tick();

        sHotel->UpdateWorld(l_Diff);

        l_RealPrevTime = l_RealCurrTime;

        /// Update the world every 500 ms
        if (l_Diff <= UPDATE_WORLD_TIMER + l_PrevSleepTime)
        {
            l_PrevSleepTime = UPDATE_WORLD_TIMER + l_PrevSleepTime - l_Diff;
            std::this_thread::sleep_for(std::chrono::milliseconds((uint32)(l_PrevSleepTime)));
        }
        else
            l_PrevSleepTime = 0;
    }
   
    sHotel->CleanUp();

    return 0;
}

bool LoadDatabase()
{
    if (UserDatabase.StartUp(sConfig->GetStringDefault("UserDatabaseInfo").c_str(), sConfig->GetIntDefault("UserDatabaseInfo.MySQLConnections", 1)) ||
        RoomDatabase.StartUp(sConfig->GetStringDefault("RoomDatabaseInfo").c_str(), sConfig->GetIntDefault("RoomDatabaseInfo.MySQLConnections", 1)))
        return false;

    return true;
}