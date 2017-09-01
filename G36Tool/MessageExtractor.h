#pragma once

#include <vector>
#include <tuple>

namespace SWatch
{

/**Extracts messages from the byte buffers it's given. The following format is used:
- Sync (2): 0x55 0x00
- PayloadLength (1)
- Payload (PayloadLength)
- Checksum (1) //XOR in the range [Sync, Checksum)
- Footer (1): 0x00
Thus, the minimal message size is 5 bytes. Because of the size of the PayloadLength field, the maximum size is 261.*/
class MessageExtractor
{
public:
	MessageExtractor() : ReqCount(0), StartOffset(0)
	{}

	/**Adds a chunk of data to the message consumer.
	@param Begin Start of the new data. The buffer's contents will be copied as needed.
	@param consumer A callable with the signature:
		consumer(bool CrcValid, const unsigned char *Begin, const unsigned char *End)*/
	template<class Consumer>
	void Process(const unsigned char *Begin, const unsigned char *End, Consumer consumer)
	{
		while (Begin!=End)
		{
			const unsigned char *MsgBegin, *MsgEnd, *ParseEnd;
			std::tie(MsgBegin, MsgEnd) = OnData(Begin, End, &ParseEnd);
			if ((MsgBegin) && (MsgEnd))
			{
				ProcessInternal(MsgBegin, MsgEnd, consumer);
				Begin = ParseEnd;
			}
			else
				break;
		}
	}

private:
	//Buffer for the last received message fragment.
	std::vector<unsigned char> MsgBuff;
	//Number of bytes remaining from the current message fragment.
	size_t ReqCount;
	size_t StartOffset;

	/**Extracts the next frame from the record stream. It possibly returns pointers from the range [Begin, End), but it
	might use the internal buffer MsgBuff. This can happen if a previous chunk of data ended in an incomplete frame, and
	it was stored for later processing. This is the only case where data is copied.*/
	std::tuple<const unsigned char *, const unsigned char *> OnData(const unsigned char *Begin, const unsigned char *End, const unsigned char **OutProcessEnd)
	{
		while (Begin != End)
		{
			if (!ReqCount)
			{
				StartOffset = 0;
				MsgBuff.clear();

				//Process the data from the received buffer.
				auto CurrFrame = GetNextFrame(Begin, End, ReqCount);

				//std::get<1>(CurrFrame) points to the next byte to process.
				Begin = std::get<1>(CurrFrame);

				//If we got a new frame, CurrFrame contains a non-zero length range.
				if (std::get<0>(CurrFrame) < std::get<1>(CurrFrame))
				{
					*OutProcessEnd = Begin;
					return CurrFrame;
				}
				else
				{
					if (ReqCount)
					{
						MsgBuff.assign(Begin, End);
						StartOffset = 0;
					}

					return std::tuple<const unsigned char *, const unsigned char *>(nullptr, nullptr);
				}
			}
			else
			{
				size_t AvailableLength = (size_t)(End - Begin);
				if (ReqCount <= AvailableLength)
				{
					MsgBuff.insert(MsgBuff.end(), Begin, Begin + ReqCount);
					Begin += ReqCount;
					ReqCount = 0;

					auto CurrFrame = GetNextFrame(MsgBuff.data() + StartOffset, MsgBuff.data() + MsgBuff.size(), ReqCount);

					//std::get<1>(CurrFrame) points to the next byte to process.
					StartOffset = (size_t)(std::get<1>(CurrFrame) - MsgBuff.data());

					//If we got a new frame, CurrFrame contains a non-zero length range.
					if (std::get<0>(CurrFrame) < std::get<1>(CurrFrame))
					{
						*OutProcessEnd = Begin;
						return CurrFrame;
					}
				}
				else
				{
					//Consume every available byte.
					MsgBuff.insert(MsgBuff.end(), Begin, End);
					ReqCount -= AvailableLength;
					*OutProcessEnd = End;
					return std::tuple<const unsigned char *, const unsigned char *>(nullptr, nullptr);
				}
			}
		}

		return std::tuple<const unsigned char *, const unsigned char *>(nullptr, nullptr);
	}

	std::tuple<const unsigned char *, const unsigned char *> GetNextFrame(const unsigned char *Begin, const unsigned char *End, size_t &OutReqCount)
	{
		enum
		{
			STATE_SYNC0,
			STATE_SYNC1,
			STATE_PAYLOADLENGTH,
		} SearchState = STATE_SYNC0;

		while (Begin != End)
		{
			if (SearchState == STATE_SYNC0)
			{
				if (*Begin == 0x55)
					SearchState = STATE_SYNC1;

				++Begin;
			}
			else if (SearchState == STATE_SYNC1)
			{
				if (*Begin == 0x00)
					SearchState = STATE_PAYLOADLENGTH;
				else
					SearchState = STATE_SYNC0;

				++Begin;
			}
			else if (SearchState == STATE_PAYLOADLENGTH)
			{
				unsigned char PayloadLength = *Begin++;
				size_t RequiredLength = PayloadLength + 2;
				size_t AvailableLength = (size_t)(End - Begin);

				if (AvailableLength >= RequiredLength)
					return std::make_tuple(Begin - 3, Begin + RequiredLength);
				else
				{
					OutReqCount = RequiredLength - AvailableLength;
					return std::make_tuple(Begin - 3, Begin - 3);
				}
			}
		}

		if (SearchState == STATE_SYNC1)
		{
			OutReqCount = 2;
			return std::make_tuple(Begin - 1, Begin - 1);
		}
		else if (SearchState == STATE_PAYLOADLENGTH)
		{
			OutReqCount = 1;
			return std::make_tuple(Begin - 2, Begin - 2);
		}
		else
		{
			OutReqCount = 0;
			return std::make_tuple(Begin, Begin);
		}
	}

	/**Processes a single message, from header to footer.*/
	template<class Consumer>
	void ProcessInternal(const unsigned char *Begin, const unsigned char *End, Consumer consumer)
	{
		if (End - Begin < 5)
			return;

		consumer(CalcChecksum(Begin, End - 1)==0,
			Begin + 3, End-2);
	}

	unsigned char CalcChecksum(const unsigned char *Begin, const unsigned char *End)
	{
		unsigned char Checksum=0;
		while (Begin != End)
			Checksum ^= *Begin++;

		return Checksum;
	}
};

} //SWatch
