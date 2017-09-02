#include <iostream>
#include <iomanip>

#include <fstream>

#include "MessageExtractor.h"
#include "Message.h"

void OnMessage(const SWatch::Msg::BootBanner &)
{
	std::cout << "[BootBanner]\n";
}

void OnMessage(const SWatch::Msg::BootInfo &)
{
	std::cout << "[BootInfo]\n";
}

void OnMessage(const SWatch::Msg::Log &Msg)
{
	auto Text = Msg.text();

	std::cout << std::setw(8) << Msg.tick_count() << "ms  " <<
		std::setw(4) << std::hex << Msg.code_1() << "  " <<
		std::setw(4) << Msg.code_2() << std::dec << "  ";
	std::cout.write((const char *)std::get<0>(Text), std::get<1>(Text) - std::get<0>(Text));
	std::cout << "\n";
}

void OnMessage(const SWatch::Msg::ExtLog &Msg)
{
	auto Text = Msg.text();

	std::cout << std::setw(8) << Msg.tick_count() << "ms  " <<
		std::setw(4) << std::hex << Msg.code_1() << "  " <<
		std::setw(4) << Msg.code_2() << std::dec << "  ";
	std::cout.write((const char *)std::get<0>(Text), std::get<1>(Text) - std::get<0>(Text));
	std::cout << "\n";
}

void OnMessage(const SWatch::Msg::MessageBase &Msg)
{
	static const char HexA[] = { "0123456789abcdef" };

	std::cout << "x" << std::hex << (int)Msg.type() << std::dec << "  hex(";
	const unsigned char *Begin, *End;
	std::tie(Begin, End) = Msg.raw_data();
	for (; Begin != End; ++Begin)
	{
		auto Val = *Begin;
		std::cout.write(HexA + (Val >> 4), 1);
		std::cout.write(HexA + (Val & 0xF), 1);
		std::cout.write(" ", 1);
	}

	std::cout << ")\n";
}

template<class MsgType>
void OnTypedMessage(const MsgType &Msg)
{
	if (Msg.size() >= MsgType::min_size)
		OnMessage(Msg);
	else
		std::cout << "[size(" << Msg.size() << ")<min_size(" << MsgType::min_size << ")]\n";
}

void OnRawMessage(bool CrcValid, const unsigned char *Begin, const unsigned char *End)
{
	if (!CrcValid)
	{
		std::cout << "[CRCMismatch]\n";
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

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: " << *argv << " filename_or_serialport\n";
		return 1;
	}

	SWatch::MessageExtractor Parser;

	unsigned char ReadBuff[128];
	std::ifstream InS(argv[1], std::ios_base::binary);
	while (InS)
	{
		size_t ReadCount = InS.read((char *)ReadBuff, sizeof(ReadBuff)).gcount();
		Parser.Process(ReadBuff, ReadBuff + ReadCount, OnRawMessage);
	}

	return 0;
}
