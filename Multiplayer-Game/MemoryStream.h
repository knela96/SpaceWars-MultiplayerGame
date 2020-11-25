#pragma once

#include "ByteSwap.h"

enum class Endianness {
	LittleEndian,
	BigEndian
};

constexpr Endianness STREAM_ENDIANNESS = Endianness::BigEndian;
constexpr Endianness PLATFORM_ENDIANNESS = Endianness::LittleEndian;

class OutputMemoryStream
{
public:

	// Constructor
	OutputMemoryStream():
		mCapacity(DEFAULT_PACKET_SIZE), mHead(0)
	{ }

	// Destructor
	~OutputMemoryStream()
	{ }

	// Get pointer to the data in the stream
	const char *GetBufferPtr() const { return mBuffer; }
	uint32 GetCapacity() const { return mCapacity; }
	uint32 GetSize() const { return mHead; }

	// Clear the stream state
	void Clear() { mHead = 0; }

	// Write method
	void Write(const void *inData, size_t inByteCount);

	// Generic write for arithmetic types
	template< typename T >
	void Write( T inData )
	{
		static_assert(
				std::is_arithmetic< T >::value ||
				std::is_enum< T >::value,
				"Generic Write only supports primitive data types" );

		if( STREAM_ENDIANNESS == PLATFORM_ENDIANNESS )
		{
			Write( &inData, sizeof( inData ) );
		}
		else
		{
			T swappedData = ByteSwap( inData );
			Write( &swappedData, sizeof( swappedData ) );
		}
	}

	// Generic write for vectors of arithmetic types
	template< typename T >
	void Write( const std::vector< T >& inVector )
	{
		uint32 elementCount = static_cast<uint32>(inVector.size());
		Write( elementCount );
		for( const T& element : inVector )
		{
			Write( element );
		}
	}

	// Write for strings
	void Write( const std::string& inString )
	{
		uint32 elementCount = static_cast<uint32>(inString.size());
		Write( elementCount );
		Write( inString.data(), elementCount * sizeof( char ) );
	}

	// Generic operator <<
	template< typename T >
	OutputMemoryStream &operator<<(const T &data) {
		Write(data);
		return *this;
	}
	

private:

	char mBuffer[DEFAULT_PACKET_SIZE];
	uint32 mCapacity;
	uint32 mHead;
};

class InputMemoryStream
{
public:

	// Constructor
	InputMemoryStream() :
		mCapacity(DEFAULT_PACKET_SIZE), mSize(0), mHead(0)
	{ }

	// Destructor
	~InputMemoryStream()
	{ }

	// Get pointer to the data in the stream
	const char *GetBufferPtr() const { return mBuffer; }
	uint32 GetCapacity() const { return mCapacity; }
	uint32 GetSize() const { return mSize; }
	void   SetSize(uint32 size) { mSize = size; }
	uint32 RemainingByteCount() const { return mSize - mHead; }

	// Clear the stream state
	void Clear() { mHead = 0; }

	// Read method
	void Read(void *outData, size_t inByteCount) const;

	// Generic read for arithmetic types
	template< typename T >
	void Read( T& outData ) const
	{
		static_assert(
				std::is_arithmetic< T >::value ||
				std::is_enum< T >::value,
				"Generic Read only supports primitive data types" );

		if( STREAM_ENDIANNESS == PLATFORM_ENDIANNESS )
		{
			Read( &outData, sizeof( outData ) );
		}
		else
		{
			T unswappedData;
			Read( &unswappedData, sizeof( unswappedData ) );
			outData = ByteSwap(unswappedData);
		}
	}

	// Generic read for vectors of arithmetic types
	template< typename T >
	void Read( std::vector< T >& outVector ) const
	{
		uint32 elementCount;
		Read( elementCount );
		outVector.resize( elementCount );
		for( T& element : outVector )
		{
			Read( element );
		}
	}

	// Read for strings
	void Read( std::string& inString ) const
	{
		uint32 elementCount;
		Read( elementCount );
		inString.resize(elementCount);
		for (auto &character : inString) {
			Read(character);
		}
	}

	// Generic operator >>
	template< typename T >
	const InputMemoryStream &operator>>(T &data) const {
		Read(data);
		return *this;
	}

private:

	char mBuffer[DEFAULT_PACKET_SIZE];
	uint32 mCapacity;
	uint32 mSize;
	mutable uint32 mHead;
};
