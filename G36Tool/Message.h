#pragma once

#include <cstdint>
#include <tuple>

namespace SWatch
{

namespace Msg
{

namespace detail
{

inline const unsigned char *StripPaddedText(const unsigned char *Begin, const unsigned char *End)
{
	//Skip the 0 padding at the end of the text.
	while (End > Begin)
	{
		--End;
		if (*End)
		{
			End++;
			break;
		}
	}

	return End;
}

} //detail

class MessageBase
{
public:
	/**
	@param Begin Position in message after PayloadLength.
	@param End Position in message after Payload.*/
	inline MessageBase(const unsigned char *Begin, const unsigned char *End) : Begin(Begin), End(End)
	{ }

	inline unsigned char type() const { return (unsigned char)*Begin; }
	inline size_t size() const { return End - (Begin + 1); }
	inline std::tuple<const unsigned char *, const unsigned char *> raw_data() const { return std::make_tuple(Begin + 1, End); }

	static constexpr size_t header_size = 1;

protected:
	const unsigned char *Begin, *End;
};

template<unsigned char TypeCode, size_t MinSize>
class TypedMsg : public MessageBase
{
public:
	using MessageBase::MessageBase;

	static constexpr unsigned char type_code = TypeCode;
	static constexpr size_t min_size = MinSize;
	static constexpr size_t min_total_size = MessageBase::header_size + min_size;

protected:
	typedef TypedMsg<TypeCode, MinSize> TypedMsgType;
};

class BootBanner : public TypedMsg<0x78, 4>
{
public:
	inline BootBanner(const unsigned char *Begin, const unsigned char *End) : TypedMsgType(Begin, End)
	{ }

	inline uint32_t code() const { return *(const uint32_t *)std::get<0>(raw_data()); }
};

class BootInfo : public TypedMsg<0x83, 24>
{
public:
	inline BootInfo(const unsigned char *Begin, const unsigned char *End) : TypedMsgType(Begin, End)
	{ }
};

class Log : public TypedMsg<0x62, 8>
{
public:
	inline Log(const unsigned char *Begin, const unsigned char *End) : TypedMsgType(Begin, End)
	{ }

	inline uint32_t tick_count() const { return *(const uint32_t *)std::get<0>(raw_data()); }
	inline uint16_t code_1() const { return *(const uint16_t *)(std::get<0>(raw_data())+4); }
	inline uint16_t code_2() const { return *(const uint16_t *)(std::get<0>(raw_data())+6); }

	inline std::tuple<const unsigned char *, const unsigned char *> text() const
	{
		const unsigned char *Begin, *End;
		std::tie(Begin, End) = raw_data();
		Begin += 8;
		End = detail::StripPaddedText(Begin, End);
		return std::make_tuple(Begin, End);
	}
};

class ExtLog : public TypedMsg<0x65, 12>
{
public:
	inline ExtLog(const unsigned char *Begin, const unsigned char *End) : TypedMsgType(Begin, End)
	{ }

	inline uint32_t tick_count() const { return *(const uint32_t *)(std::get<0>(raw_data())+4); }
	inline uint32_t code_1() const { return *(const uint32_t *)std::get<0>(raw_data()); }
	inline uint32_t code_2() const { return *(const uint32_t *)(std::get<0>(raw_data()) + 8); }

	inline std::tuple<const unsigned char *, const unsigned char *> text() const
	{
		const unsigned char *Begin, *End;
		std::tie(Begin, End) = raw_data();
		Begin += 12;
		End = detail::StripPaddedText(Begin, End);
		return std::make_tuple(Begin, End);
	}
};

} //Msg

} //SWatch
