#include <iostream>
#include <iomanip>

#include <boost/asio.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "MessageExtractor.h"
#include "MessageDumper.h"

void DumpFromFile(const boost::filesystem::path &FN, SWatch::MessageExtractor &Parser, SWatch::MessageDumper &Dumper)
{
	unsigned char ReadBuff[128];
	boost::filesystem::ifstream InS(FN, std::ios_base::binary);
	while (InS)
	{
		size_t ReadCount = InS.read((char *)ReadBuff, sizeof(ReadBuff)).gcount();
		Parser.Process(ReadBuff, ReadBuff + ReadCount, [&Dumper](bool CrcValid, const unsigned char *Begin, const unsigned char *End) {
			Dumper(CrcValid, Begin, End);
		});
	}
}

void DumpFromSerialPort(const char *Name, SWatch::MessageExtractor &Parser, SWatch::MessageDumper &Dumper)
{
	boost::asio::io_service IOS;
	boost::asio::serial_port Port(IOS);

	std::string PrevErrorMsg;

	constexpr size_t ReadBuffSize = 2048;
	std::unique_ptr<unsigned char[]> ReadBuff(new unsigned char[ReadBuffSize]);
	while (true)
	{
		try
		{
			Port.open(Name);
			Port.set_option(boost::asio::serial_port_base::baud_rate(115200));
			Port.set_option(boost::asio::serial_port_base::character_size(8));
			Port.set_option(boost::asio::serial_port_base::stop_bits(boost::asio::serial_port_base::stop_bits::one));
			Port.set_option(boost::asio::serial_port_base::parity(boost::asio::serial_port::parity::none));
			Port.set_option(boost::asio::serial_port_base::flow_control(boost::asio::serial_port::flow_control::none));

			PrevErrorMsg.clear();
			while (true)
			{
				size_t ReadCount = Port.read_some(boost::asio::buffer(ReadBuff.get(), ReadBuffSize));
				Parser.Process(ReadBuff.get(), ReadBuff.get() + ReadCount, [&Dumper](bool CrcValid, const unsigned char *Begin, const unsigned char *End) {
					Dumper(CrcValid, Begin, End);
				});
				Dumper.flush();
			}
		}
		catch (const std::exception &Ex)
		{
			std::ostringstream ErrS;
			ErrS << "Caught exception(" << typeid(Ex).name() << "): " << Ex.what();
			std::string ErrMsg = ErrS.str();
			if (ErrMsg != PrevErrorMsg)
			{
				std::cerr << ErrMsg << std::endl;
				PrevErrorMsg = ErrMsg;
			}

			Port.close();
		}
	}
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: " << *argv << " filename_or_serialport\n"
			"prefix with ':' to explicitly use a serial port.\n";
		return 1;
	}

	SWatch::MessageExtractor Parser;
	SWatch::MessageDumper Dumper(std::cout);

	boost::filesystem::path SrcFN(argv[1]);
	auto SourceStatus = boost::filesystem::status(SrcFN);
	if ((*argv[1]!=':') &&
		(SourceStatus.type()!= boost::filesystem::file_not_found) &&
		(SourceStatus.type() != boost::filesystem::status_error) &&
		(boost::filesystem::status(SrcFN).type() != boost::filesystem::character_file))
		//Treat SrcFN as a normal file.
		DumpFromFile(SrcFN, Parser, Dumper);
	else
	{
		//Treat SrcFN as a serial port.
		const char *SourceFileName = argv[1];
		if (*SourceFileName == ':')
			++SourceFileName;

		DumpFromSerialPort(SourceFileName, Parser, Dumper);
	}

	return 0;
}
