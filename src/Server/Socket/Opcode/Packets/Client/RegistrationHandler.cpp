﻿/*
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
* along with this program.  If not, see <http:///www.gnu.org/licenses/>.
*/

#include "Opcode/Packets/Server/RegistrationPackets.h"
#include "Opcode/Packets/Server/LoginPackets.h"
#include "Habbo.h"
#include "Hotel.h"
#include "Common/SHA1.h"
#include "RoomManager.h"
#include "Database/DatabaseTypes.h"
#include <time.h>

namespace SteerStone
{    
    void HabboSocket::HandleGDate(ClientPacket* p_Packet)
    {
        HabboPacket::Login::GDate l_Packet;
        SendPacket(l_Packet.Write());
    }
    
    void HabboSocket::HandleApproveUsername(ClientPacket* p_Packet)
    {
        HabboPacket::Registration::ApproveNameReply l_Packet;
        l_Packet.Name = p_Packet->ReadString();

        l_Packet.ErrorCode = ApproveNameError::NAME_VALID;

        PreparedStatement* l_PreparedStatement = UserDatabase.GetPrepareStatement();
        l_PreparedStatement->PrepareStatement("SELECT user_name FROM account WHERE user_name = ?");
        l_PreparedStatement->SetString(0, l_Packet.Name.c_str());
        std::unique_ptr<PreparedResultSet> l_PreparedResultSet = l_PreparedStatement->ExecuteStatement();

        if (!l_PreparedResultSet)
        {
            if (l_Packet.Name.length() > 15)
                l_Packet.ErrorCode = ApproveNameError::NAME_TOO_LONG;
            else if (l_Packet.Name.length() < 3)
                l_Packet.ErrorCode = ApproveNameError::NAME_UNACCEPTABLE_TO_STAFF;
            else
            {
                std::string l_BobbaWords = sConfig->GetStringDefault("RegisterationBobbaWords");

                for (uint8 l_I = 0; l_I < l_BobbaWords.length(); l_I++)
                {
                    if (l_Packet.Name.find(l_BobbaWords[l_I]) != std::string::npos)
                    {
                        l_Packet.ErrorCode = ApproveNameError::NAME_UNACCEPTABLE_TO_STAFF_2;
                        break;
                    }
                }
            }
        }
        else
            l_Packet.ErrorCode = ApproveNameError::NAME_TAKEN;

        SendPacket(l_Packet.Write());

        UserDatabase.FreePrepareStatement(l_PreparedStatement);
    }
    
    void HabboSocket::HandleApprovePassword(ClientPacket* p_Packet)
    {
        HabboPacket::Registration::ApprovePasswordReply l_Packet;
        l_Packet.Name = p_Packet->ReadString();
        l_Packet.Password = p_Packet->ReadString();
        l_Packet.ErrorCode = ApprovePasswordError::PASSWORD_VALID;

        if (l_Packet.Password.length() < 6)
            l_Packet.ErrorCode = ApprovePasswordError::PASSWORD_TOO_SHORT;
        else if (l_Packet.Password.length() > 9)
            l_Packet.ErrorCode = ApprovePasswordError::PASSWORD_TOO_LONG;
        else
        {
            std::string l_BobbaWords = sConfig->GetStringDefault("RegisterationFilter");

            bool l_UsedNumber = false;

            /// Must contain atleast a number 0-9
            for (uint8 l_I = 0; l_I < 10; l_I++)
            {
                if (l_Packet.Password.find(l_BobbaWords[l_I]) == std::string::npos)
                {
                    l_UsedNumber = true;
                    break;
                }
            }

            if (!l_UsedNumber)
                l_Packet.ErrorCode = ApprovePasswordError::PASSWORD_REQUIRES_NUMBERS;

            /// Username similiar to password
            if (strstr(l_Packet.Name.c_str(), l_Packet.Password.c_str()))
                l_Packet.ErrorCode = ApprovePasswordError::PASSWORD_USER_NAME_SIMILIAR;
        }

        SendPacket(l_Packet.Write());
    }
    
    void HabboSocket::HandleApproveEmail(ClientPacket* p_Packet)
    {
        std::string l_Email = p_Packet->ReadString();

        /// List of emails we accept
        char* l_AcceptedEmails[6] =
        {
            {"@hotmail.com"},
            {"@hotmail.co.uk"},
            {"@gmail.com"},
            {"@icloud.com"},
            {"@outlook.co.uk"},
            {"@outlook.com"}
        };

        /// Check if the email recieved matches our requirements
        for (int16 l_I = 0; l_I < 6; l_I++)
            if (l_Email.find(l_AcceptedEmails[l_I]) != std::string::npos)
            {
                /// Email is valid... continue
                HabboPacket::Registration::ApproveEmail l_Packet;
                SendPacket(l_Packet.Write());
                return;
            }

        /// Email is not valid
        HabboPacket::Registration::RejectedEmail l_Packet;
        SendPacket(l_Packet.Write());
    }
    
    void HabboSocket::HandleRegisteration(ClientPacket* p_Packet)
    {
        std::string l_Username;
        std::string l_Figure;
        std::string l_Gender;
        std::string l_Email;
        std::string l_Password;
        std::string l_Birthday;
        bool l_DirectEmail;

        for (uint8 l_I = 0; l_I < 6; l_I++)
        {
            uint32 l_Id = p_Packet->ReadBase64Int();
            std::string l_Content = p_Packet->ReadString();

            switch (l_Id)
            {
            case 2:
                l_Username = l_Content;
                break;
            case 4:
                l_Figure = l_Content;
                break;
            case 5:
                l_Gender = l_Content;
                break;
            case 7:
                l_Email = l_Content;
                break;
            case 8:
                l_Birthday = l_Content;
                break;
            default:
                break;
            }
        }

        p_Packet->ReadSkip(4);
        l_DirectEmail = p_Packet->ReadWiredBool();
        p_Packet->ReadSkip(1);
        l_Password = p_Packet->ReadString();

        if (l_Gender == "M")
            l_Gender = "Male";
        else
            l_Gender = "Female";

        PreparedStatement* l_PreparedStatement = UserDatabase.GetPrepareStatement();
        l_PreparedStatement->PrepareStatement("INSERT INTO account(user_name, hash_pass, email, figure, motto, console_motto, direct_mail, birthday, gender, credits, tickets, films, sound_enabled) VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        l_PreparedStatement->SetString(0, l_Username.c_str());
        l_PreparedStatement->SetString(1, (CalculateSHA1Hash(boost::to_upper_copy(l_Username) + ":" + boost::to_upper_copy(l_Password))).c_str());
        l_PreparedStatement->SetString(2, l_Email.c_str());
        l_PreparedStatement->SetString(3, l_Figure.c_str());
        l_PreparedStatement->SetString(4, sConfig->GetStringDefault("RegisterationMotto", "I'm a new user!"));
        l_PreparedStatement->SetString(5, sConfig->GetStringDefault("RegisterationConsoleMotto", "I'm looking for friends!"));
        l_PreparedStatement->SetBool(6, l_DirectEmail);
        l_PreparedStatement->SetString(7, l_Birthday.c_str());
        l_PreparedStatement->SetString(8, l_Gender.c_str());
        l_PreparedStatement->SetUint32(9, sHotel->GetIntConfig(CONFIG_REGISTERATION_CREDITS));
        l_PreparedStatement->SetUint32(10, sHotel->GetIntConfig(CONFIG_REGISTERATION_TICKETS));
        l_PreparedStatement->SetUint32(11, sHotel->GetIntConfig(CONFIG_REGISTERATION_FILMS));
        l_PreparedStatement->SetBool(12, sHotel->GetBoolConfig(BoolConfigs::CONFIG_REGISTERATION_SOUND));
        l_PreparedStatement->ExecuteStatement();
        UserDatabase.FreePrepareStatement(l_PreparedStatement);
    }

    void HabboSocket::HandleParentEmailRequired(ClientPacket* p_Packet)
    {
        HabboPacket::Registration::ParentEmailRequired l_Packet;
        l_Packet.RequireParentEmail = sHotel->GetBoolConfig(CONFIG_REGISTERATION_REQUIRE_PARENT_EMAIL);
        SendPacket(l_Packet.Write());
    }

    void HabboSocket::HandleValidateParentEmail(ClientPacket* p_Packet)
    {
        std::string l_Email = p_Packet->ReadString();

        /// List of emails we accept
        char* l_AcceptedEmails[6] =
        {
            {"@hotmail.com"},
            {"@hotmail.co.uk"},
            {"@gmail.com"},
            {"@icloud.com"},
            {"@outlook.co.uk"},
            {"@outlook.com"}
        };

        /// Check if the email recieved matches our requirements
        for (int16 l_I = 0; l_I < 6; l_I++)
            if (l_Email.find(l_AcceptedEmails[l_I]) != std::string::npos)
            {
                /// Email is valid... continue
                HabboPacket::Registration::ValidateParentEmail l_Packet;
                l_Packet.Validate = true;
                SendPacket(l_Packet.Write());
                return;
            }

        /// Email is not valid
        /// TODO; Add another database column to users, but will anyone ever use this parent email thingy?
        HabboPacket::Registration::ValidateParentEmail l_Packet;
        l_Packet.Validate = false;
        SendPacket(l_Packet.Write());
    }

    void HabboSocket::HandleClientAC(ClientPacket* p_Packet)
    {
        std::string l_DateOfBirth = p_Packet->ReadString();
        std::string l_MinimumDate = sConfig->GetStringDefault("RegisterationDateOfBirth");

        HabboPacket::Registration::ValidateAge l_Packet;
        l_Packet.Validate = true;
        SendPacket(l_Packet.Write());

        /// TODO; Fix
        /*struct tm l_TimeInfoDateOfBirth;
        struct tm l_TimeInfoMinimumDate;

        sscanf(l_DateOfBirth.c_str(), "%d.%d.%d", &l_TimeInfoDateOfBirth.tm_year, &l_TimeInfoDateOfBirth.tm_mon, &l_TimeInfoDateOfBirth.tm_mday);
        sscanf(l_MinimumDate.c_str(), "%d.%d.%d", &l_TimeInfoMinimumDate.tm_year, &l_TimeInfoMinimumDate.tm_mon, &l_TimeInfoMinimumDate.tm_mday);

        std::time_t l_TimeDateOfBirth = std::mktime(&l_TimeInfoDateOfBirth);
        std::time_t l_TimeMinimumDate = std::mktime(&l_TimeInfoMinimumDate);
        if (l_TimeDateOfBirth != (std::time_t)(-1) && l_TimeMinimumDate != (std::time_t)(-1))
        {
            double l_Difference = std::difftime(l_TimeMinimumDate, l_TimeDateOfBirth) / (60 * 60 * 24);
            std::cout << std::ctime(&l_TimeDateOfBirth);
            std::cout << std::ctime(&l_TimeMinimumDate);

            /// Is user date older than 13 years old?
            if (l_Difference > MINIMUM_AMOUNT_OF_DAYS)
            {
                HabboPacket::Registration::ValidateAge l_Packet;
                l_Packet.Validate = true;
                SendPacket(l_Packet.Write());
            }
            else
            {
                HabboPacket::Registration::ValidateAge l_Packet;
                l_Packet.Validate = false;
                SendPacket(l_Packet.Write());
            }
        }
        else ///< Shouldn't happen but handle this anyway just incase
        {
            LOG_WARNING << "Failed to compare two dates, rejecting user.";
            HabboPacket::Registration::ValidateAge l_Packet;
            l_Packet.Validate = false;
            SendPacket(l_Packet.Write());
        }*/
    }

    void HabboSocket::HandleCoppaRegGetRealTime(ClientPacket* p_Packet)
    {
        HabboPacket::Registration::CoppaGetRealTime l_Packet;
        SendPacket(l_Packet.Write());
    }
}
