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

#include "Habbo.h"
#include "Room.h"
#include "Opcode/Packets/Server/RoomPackets.h"

namespace SteerStone
{
    /// Constructor
    /// @p_Habbo :
    /// @p_RoomModel :
    WayPoints::WayPoints(Habbo* p_Habbo, RoomModel* p_RoomModel) : m_Habbo(p_Habbo), PathFinder(p_RoomModel), m_RoomModel(p_RoomModel)
    {
        m_ActivePath = false;
    }

    /// Deconstructor
    WayPoints::~WayPoints()
    {
    }

    /// SetEndPosition
    /// @p_X : End Position X
    /// @p_Y : End Position Y
    void WayPoints::SetEndPosition(int16 const p_X, int16 const p_Y)
    {
        m_EndX = p_X;
        m_EndY = p_Y;
    }

    /// CheckForInteractiveObjects
    /// Check if user is sitting or standing ontop any objects which user can interact with (automatically)
    void WayPoints::CheckForInteractiveObjects()
    {
        /// Get the current tile instance from Habbo position
        if (TileInstance* l_TileInstance = m_RoomModel->GetTileInstance(m_Habbo->GetPositionX(), m_Habbo->GetPositionY()))
        {
            if (Item* l_Item = l_TileInstance->GetItem())
            {
                /// If Habbo is ontop of an item which we can sit on, execute the sit function
                if (l_Item->GetBehaviour() == "can_sit_on_top")
                {
                    m_Habbo->GetRoom()->RemoveStatus(m_Habbo->GetRoomGUID(), Status::STATUS_DANCING);
                    m_Habbo->GetRoom()->AddStatus(m_Habbo->GetRoomGUID(), Status::STATUS_SITTING);
                    m_Habbo->UpdatePosition(l_Item->GetPositionX(), l_Item->GetPositionY(), l_Item->GetPositionZ(), l_Item->GetRotation());
                }
            }
        }
    }
} ///< NAMESPACE STEERSTONE