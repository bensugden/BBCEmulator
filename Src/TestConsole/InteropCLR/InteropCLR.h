// InteropCLR.h

#pragma once

#include "msclr\marshal_cppstd.h"

using namespace System;

#include <string>

struct CRemoteMessagePacket
{
	std::string event;
};

namespace InteropCLR 
{

	public ref class Interop
	{
 		void SendEvent( System::String^ event_string )
 		{
 			CRemoteMessagePacket packet;
 			msclr::interop::marshal_context context;
 			packet.event = context.marshal_as<std::string>(event_string);
 			//NFrameWork::SendRemoteMessage( packet );
 		}
	};
}
