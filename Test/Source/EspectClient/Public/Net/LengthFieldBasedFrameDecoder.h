// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Bytebuf.h"

enum class ByteOrder
{
	BIG_ENDIAN,
	LITTLE_ENDIAN,
	NATIVE_ORDER
};

class ESPECTCLIENT_API FLengthFieldBasedFrameDecoder
{
private:
	ByteOrder byteOrder;
	int32 maxFrameLength;
	int32 lengthFieldOffset;
	int32 lengthFieldLength;
	int32 lengthFieldEndOffset;
	int32 lengthAdjustment;
	int32 initialBytesToStrip;
	bool failFast;
	bool discardingTooLongFrame;
	long tooLongFrameLength;
	long bytesToDiscard;
	int32 frameLengthInt;

public:

	FLengthFieldBasedFrameDecoder(int32 _maxFrameLength, int32 _lengthFieldOffset, int32 _lengthFieldLength);

	FLengthFieldBasedFrameDecoder(int32 _maxFrameLength, int32 _lengthFieldOffset, int32 _lengthFieldLength,
	int32 _lengthAdjustment, int32 _initialBytesToStrip);

	FLengthFieldBasedFrameDecoder(int32 _maxFrameLength, int32 _lengthFieldOffset, int32 _lengthFieldLength,
	int32 _lengthAdjustment, int32 _initialBytesToStrip, bool _failFast);

	FLengthFieldBasedFrameDecoder(ByteOrder _byteOrder, int32 _maxFrameLength, int32 _lengthFieldOffset,
	int32 _lengthFieldLength, int32 _lengthAdjustment, int32 _initialBytesToStrip, bool _failFast);

	bool Init();

	~FLengthFieldBasedFrameDecoder();

public:
	bool decode(FByteBuf& in, TSharedRef<FByteBuf> outbuf);

	uint64 getUnadjustedFrameLength(FByteBuf& buf, int32 offset, int32 length, ByteOrder order);

private:

	void DiscardingTooLongFrame(FByteBuf& in);

	void failOnNegativeLengthField(FByteBuf& in, long frameLength, int32 lengthFieldEndOffset);

	void failOnFrameLengthLessThanLengthFieldEndOffset(FByteBuf& in, long frameLength, int32 lengthFieldEndOffset);

	void exceededFrameLength(FByteBuf& in, long frameLength);

	void failOnFrameLengthLessThanInitialBytesToStrip(FByteBuf& in, long frameLength, int32 initialBytesToStrip);

	void fail(long frameLength);

	void failIfNecessary(bool firstDetectionOfTooLongFrame);
};
