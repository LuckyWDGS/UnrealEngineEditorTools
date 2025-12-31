// Copyright Viligogo Group. All Rights Reserved.

#include "Net/ByteBuf.h"

FByteBuf::FByteBuf(FByteBuf& buf) :
	readerIndex(0),
	markedReaderIndex(0),
	markedWriterIndex(0),
	maxCapacity(buf.maxCapacity),
	checkBounds(true)
{
	bufdata.Init(0, buf.GetCapacity());
	FMemory::Memcpy(bufdata.GetData(), buf.GetData(), buf.GetCapacity());
	writerIndex = buf.GetCapacity();
	UE_LOG(LogTemp, Log, TEXT("COPY FByteBuf( %d )"), maxCapacity);
}

FByteBuf::FByteBuf(int32 _maxCapacity) :
	readerIndex(0),
	writerIndex(0),
	markedReaderIndex(0),
	markedWriterIndex(0),
	maxCapacity(_maxCapacity),
	checkBounds(true)
{
	check(maxCapacity >= 0);
	// UE_LOG(LogTemp, Log, TEXT("new FByteBuf( %d )"), maxCapacity);
}

FByteBuf::FByteBuf(int32 _initialCapacity, int32 _maxCapacity) :
	readerIndex(0),
	writerIndex(0),
	markedReaderIndex(0),
	markedWriterIndex(0),
	maxCapacity(_maxCapacity),
	checkBounds(true)
{

	check(_initialCapacity >= 0);
	check(_maxCapacity >= 0);
	check(_initialCapacity <= _maxCapacity);

	bufdata.Init(0, _initialCapacity);
	// UE_LOG(LogTemp, Log, TEXT("new FByteBuf( %d )"), maxCapacity);
}

FByteBuf::~FByteBuf()
{
	// UE_LOG(LogTemp, Log, TEXT("~FByteBuf( %d )"), maxCapacity);
}

void FByteBuf::Init(int32 _initialCapacity)
{
	clear();
	bufdata.Init(0, _initialCapacity);
}

void FByteBuf::skipBytes(int32 length)
{
	checkReadableBytes(length);
	readerIndex += length;
}

FByteBuf& FByteBuf::SetReaderIndex(int32 _readerIndex)
{
	if (checkBounds)
	{
		checkIndexBounds(_readerIndex, writerIndex, GetCapacity());
	}
	readerIndex = _readerIndex;
	return *this;
}

FByteBuf& FByteBuf::SetWriterIndex(int32 _writerIndex)
{
	if (checkBounds)
	{
		checkIndexBounds(readerIndex, _writerIndex, GetCapacity());
	}

	writerIndex = _writerIndex;
	return *this;
}

FByteBuf& FByteBuf::setIndex(int32 _readerIndex, int32 _writerIndex)
{
	if (checkBounds)
	{
		checkIndexBounds(_readerIndex, _writerIndex, GetCapacity());
	}

	readerIndex = _readerIndex;
	writerIndex = _writerIndex;
	return *this;
}

void FByteBuf::checkIndexBounds(int32 _readerIndex, int32 _writerIndex, int32 _capacity)
{
	ensureMsgf(!(_readerIndex < 0 || _readerIndex > _writerIndex || _writerIndex > _capacity),
		TEXT("readerIndex: %d, writerIndex: %d (expected: 0 <= readerIndex <= writerIndex <= capacity(%d))"),
		_readerIndex, _writerIndex, _capacity);
}

FByteBuf& FByteBuf::discardReadedBytes()
{
	FMemory::Memmove(bufdata.GetData(), bufdata.GetData() + readerIndex, writerIndex - readerIndex);
	writerIndex -= readerIndex;
	readerIndex = 0;
	return *this;
}

void FByteBuf::readBytes(int32 length, TSharedRef<FByteBuf> outbuf)
{
	outbuf->Init(length);
	FMemory::Memcpy(outbuf->GetData(), bufdata.GetData() + readerIndex, length);
	readerIndex += length;
}

void FByteBuf::readBytes(int32 length, TArray<uint8>* outbuf)
{
	outbuf->Init(0,length);
	FMemory::Memcpy(outbuf->GetData(), bufdata.GetData() + readerIndex, length);
	readerIndex += length;
}

void FByteBuf::checkReadableBytes(int32 minimumReadableBytes)
{
	check(minimumReadableBytes >= 0);
	checkReadableBytes0(minimumReadableBytes);
}

void FByteBuf::checkReadableBytes0(int32 minimumReadableBytes)
{
	ensureMsgf(!(checkBounds && readerIndex > writerIndex - minimumReadableBytes),
		TEXT("readerIndex(%d) + length(%d) exceeds writerIndex(%d)"),
		readerIndex, minimumReadableBytes, writerIndex);
}

bool FByteBuf::isOutOfBounds(int index, int length, int _capacity)
{
	return (index | length | _capacity | (index + length) | (_capacity - (index + length))) < 0;
}

void FByteBuf::checkIndex(int index, int fieldLength)
{
	if (checkBounds)
	{
		ensureMsgf(!isOutOfBounds(index, fieldLength, GetCapacity()),
			TEXT("Index: %d, length: %d (expected: range(0, %d))"),
			readerIndex, fieldLength, GetCapacity());
	}
}

FByteBuf& FByteBuf::writeByte(uint8 value)
{
	ensureMsgf(isWritable(1),
		TEXT("Out of MaxCapacity"));

	setByte(value);
	++writerIndex;
	return *this;
}

FByteBuf& FByteBuf::writeShort(uint16 value)
{
	ensureMsgf(isWritable(2),
		TEXT("Out of MaxCapacity"));

	writeByte( value >> 8 );
	writeByte( value );
	return *this;
}

FByteBuf& FByteBuf::writeInt(uint32 value)
{
	ensureMsgf(isWritable(4),
		TEXT("Out of MaxCapacity"));

	writeShort(value >> 16);
	writeShort(value);
	return *this;
}

FByteBuf& FByteBuf::writeBytes(uint8* value, uint32 len)
{
	ensureMsgf(isWritable(len),
		TEXT("Out of MaxCapacity"));

	FMemory::Memcpy(bufdata.GetData() + writerIndex, value, len);
	writerIndex += len;

	return *this;
}

FByteBuf& FByteBuf::writableBytes(const TArray<uint8>& ByteData)
{
	bufdata.Append(ByteData);
	writerIndex += ByteData.Num();
	return *this;
}

uint8 FByteBuf::readByte()
{
	++readerIndex;
	return bufdata[readerIndex-1];
}

uint16 FByteBuf::readShort()
{
	uint16 t1 = readByte();
	uint16 t2 = readByte();
	return (t1 << 8) | t2;
}

uint32 FByteBuf::readInt()
{
	uint32 t1 = readShort();
	uint32 t2 = readShort();
	return (t1 << 16) | t2;
}

FByteBuf& FByteBuf::setByte(uint8 value)
{
	bufdata[writerIndex] = value;
	return *this;
}
