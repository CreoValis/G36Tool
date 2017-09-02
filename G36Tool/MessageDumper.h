#pragma once

#include <ostream>

#include "Message.h"

namespace SWatch
{

class MessageDumper
{
public:
	inline MessageDumper(std::ostream &Target) : Target(Target) { }

	void operator()(bool CrcValid, const unsigned char *Begin, const unsigned char *End);

private:
	std::ostream &Target;

	void OnMessage(const SWatch::Msg::BootBanner &);
	void OnMessage(const SWatch::Msg::BootInfo &);
	void OnMessage(const SWatch::Msg::Log &Msg);
	void OnMessage(const SWatch::Msg::ExtLog &Msg);
	void OnMessage(const SWatch::Msg::MessageBase &Msg);

	template<class MsgType>
	void OnTypedMessage(const MsgType &Msg)
	{
		if (Msg.size() >= MsgType::min_size)
			OnMessage(Msg);
		else
			Target << "[Err:size(" << Msg.size() << ")<min_size(" << MsgType::min_size << ")]\n";
	}
};

} //SWatch
