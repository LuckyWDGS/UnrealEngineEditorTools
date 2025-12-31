// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

class ESPECTCLIENT_API FByteBuf
{

public:
	FByteBuf(FByteBuf &buf);
	FByteBuf(int32 _maxCapacity);
	FByteBuf(int32 _initialCapacity, int32 _maxCapacity);
	~FByteBuf();

private:
	TArray<uint8> bufdata;
	int32 readerIndex;
	int32 writerIndex;
	int32 markedReaderIndex;
	int32 markedWriterIndex;
	int32 maxCapacity;
	bool checkBounds;

	uint8 _getByte(uint32 index) { return bufdata[index]; }

public:
	void Init(int32 _initialCapacity);

	int32 Num() { return bufdata.Num(); }
	int32 GetCapacity() { return bufdata.Num(); }

	uint8 *GetData() { return this->bufdata.GetData(); }
	TArray<uint8> *GetBuf() { return &this->bufdata; }

	int32 readableBytes() { return writerIndex - readerIndex; }

	int32 GetMaxCapacity() { return maxCapacity; }

	void SetMaxCapacity(int32 _maxCapacity) { this->maxCapacity = _maxCapacity; }

	int32 GetReaderIndex() { return readerIndex; }
	int32 GetWriterIndex() { return writerIndex; }

	void skipBytes(int32 length);

	FByteBuf &SetReaderIndex(int32 _readerIndex);
	FByteBuf &SetWriterIndex(int32 _writerIndex);
	FByteBuf &setIndex(int32 _readerIndex, int32 _writerIndex);
	void clear() { readerIndex = writerIndex = 0; }

	bool isReadable(int32 numBytes = 1) { return writerIndex - readerIndex >= numBytes; }
	bool isWritable(int32 numBytes = 1) { return GetMaxCapacity() - writerIndex >= numBytes; }

	int32 writableBytes() { return GetCapacity() - writerIndex; }
	int32 maxWritableBytes() { return GetMaxCapacity() - writerIndex; }

	FByteBuf &markReaderIndex()
	{
		markedReaderIndex = readerIndex;
		return *this;
	}
	FByteBuf &resetReaderIndex()
	{
		SetReaderIndex(markedReaderIndex);
		return *this;
	}
	FByteBuf &markWriterIndex()
	{
		markedWriterIndex = writerIndex;
		return *this;
	}
	FByteBuf &resetWriterIndex()
	{
		SetWriterIndex(markedWriterIndex);
		return *this;
	}

	FByteBuf &discardReadedBytes();

	void readBytes(int32 length, TSharedRef<FByteBuf> outbuf);

	void readBytes(int32 length, TArray<uint8>* outbuf);

	void checkIndexBounds(int32 _readerIndex, int32 _writerIndex, int32 _capacity);

	void checkReadableBytes(int32 minimumReadableBytes);
	void checkReadableBytes0(int32 minimumReadableBytes);

	uint8 getByte(uint32 index)
	{
		checkIndex(index);
		return _getByte(index);
	}

	uint16 getShort(uint32 index)
	{
		checkIndex(index, 2);
		return (uint16)bufdata[index] << 8 | (uint16)bufdata[index + 1];
	}

	uint32 getInt(uint32 index)
	{
		checkIndex(index, 4);
		return (uint32)bufdata[index] << 24 |
			   (uint32)bufdata[index + 1] << 16 |
			   (uint32)bufdata[index + 2] << 8 |
			   (uint32)bufdata[index + 3];
	}

	uint64 getLong(uint32 index)
	{
		checkIndex(index, 8);
		return (uint64)bufdata[index] << 56 |
			   (uint64)bufdata[index + 1] << 48 |
			   (uint64)bufdata[index + 2] << 40 |
			   (uint64)bufdata[index + 3] << 32 |
			   (uint64)bufdata[index + 4] << 24 |
			   (uint64)bufdata[index + 5] << 16 |
			   (uint64)bufdata[index + 6] << 8 |
			   (uint64)bufdata[index + 7];
	}

	bool isOutOfBounds(int index, int length, int _capacity);
	void checkIndex(int index, int fieldLength = 1);

	FByteBuf &writeByte(uint8 value);
	FByteBuf &writeShort(uint16 value);
	FByteBuf &writeInt(uint32 value);

	FByteBuf &writeBytes(uint8 *value, uint32 len);

	FByteBuf &writableBytes(const TArray<uint8> &ByteData);

	uint8 readByte();
	uint16 readShort();
	uint32 readInt();

	FByteBuf &setByte(uint8 value);
};
