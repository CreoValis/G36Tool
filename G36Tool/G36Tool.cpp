#include <iostream>
#include <iomanip>

#include <fstream>

#include "MessageExtractor.h"
#include "MessageDumper.h"

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: " << *argv << " filename_or_serialport\n";
		return 1;
	}

	SWatch::MessageExtractor Parser;
	SWatch::MessageDumper Dumper(std::cout);

	unsigned char ReadBuff[128];
	std::ifstream InS(argv[1], std::ios_base::binary);
	while (InS)
	{
		size_t ReadCount = InS.read((char *)ReadBuff, sizeof(ReadBuff)).gcount();
		Parser.Process(ReadBuff, ReadBuff + ReadCount, [&Dumper](bool CrcValid, const unsigned char *Begin, const unsigned char *End) {
			Dumper(CrcValid, Begin, End);
		});
	}

	return 0;
}
