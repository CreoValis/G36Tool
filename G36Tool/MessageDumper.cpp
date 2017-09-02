#include "MessageDumper.h"

#include <iomanip>

namespace SWatch
{

void MessageDumper::operator()(bool CrcValid, const unsigned char *Begin, const unsigned char *End)
{
	if (!CrcValid)
	{
		Target << "[Err:Checksum]\n";
		return;
	}

	SWatch::Msg::MessageBase Msg(Begin, End);
	switch (Msg.type())
	{
	case SWatch::Msg::BootBanner::type_code:
		OnTypedMessage((const SWatch::Msg::BootBanner &)Msg);
		break;
	case SWatch::Msg::BootInfo::type_code:
		OnTypedMessage((const SWatch::Msg::BootInfo &)Msg);
		break;
	case SWatch::Msg::Log::type_code:
		OnTypedMessage((const SWatch::Msg::Log &)Msg);
		break;
	case SWatch::Msg::ExtLog::type_code:
		OnTypedMessage((const SWatch::Msg::ExtLog &)Msg);
		break;
	default:
		OnMessage(Msg);
		break;
	}
}

void MessageDumper::OnMessage(const SWatch::Msg::BootBanner &)
{
	Target << "[BootBanner]\n";
}

void MessageDumper::OnMessage(const SWatch::Msg::BootInfo &)
{
	Target << "[BootInfo]\n";
}

void MessageDumper::OnMessage(const SWatch::Msg::Log &Msg)
{
	auto Text = Msg.text();

	Target << std::setw(8) << Msg.tick_count() << "ms  " <<
		std::setw(4) << std::hex << Msg.code_1() << "  " <<
		std::setw(4) << Msg.code_2() << std::dec << "  ";
	Target.write((const char *)std::get<0>(Text), std::get<1>(Text) - std::get<0>(Text));
	Target << "\n";
}

void MessageDumper::OnMessage(const SWatch::Msg::ExtLog &Msg)
{
	auto Text = Msg.text();

	Target << std::setw(8) << Msg.tick_count() << "ms  " <<
		std::setw(4) << std::hex << Msg.code_1() << "  " <<
		std::setw(4) << Msg.code_2() << std::dec << "  ";
	Target.write((const char *)std::get<0>(Text), std::get<1>(Text) - std::get<0>(Text));
	Target << "\n";
}

void MessageDumper::OnMessage(const SWatch::Msg::MessageBase &Msg)
{
	static const char HexA[] = { "0123456789abcdef" };

	Target << "x" << std::hex << (int)Msg.type() << std::dec << "  hex(";
	const unsigned char *Begin, *End;
	std::tie(Begin, End) = Msg.raw_data();
	for (; Begin != End; ++Begin)
	{
		auto Val = *Begin;
		Target.write(HexA + (Val >> 4), 1);
		Target.write(HexA + (Val & 0xF), 1);
		Target.write(" ", 1);
	}

	Target << ")\n";
}

} //SWatch
