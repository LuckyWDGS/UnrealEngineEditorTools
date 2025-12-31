// Fill out your copyright notice in the Description page of Project Settings.

#include "Net/LengthFieldBasedFrameDecoder.h"
#include "Net/Bytebuf.h"


FLengthFieldBasedFrameDecoder::FLengthFieldBasedFrameDecoder(
	int32 _maxFrameLength,
	int32 _lengthFieldOffset,
	int32 _lengthFieldLength) :
	byteOrder(ByteOrder::BIG_ENDIAN),
	maxFrameLength(_maxFrameLength),
	lengthFieldOffset(_lengthFieldOffset),
	lengthFieldLength(_lengthFieldLength),
	lengthAdjustment(0),
	initialBytesToStrip(0),
	failFast(true)
{
}

FLengthFieldBasedFrameDecoder::FLengthFieldBasedFrameDecoder(
	int32 _maxFrameLength,
	int32 _lengthFieldOffset,
	int32 _lengthFieldLength,
	int32 _lengthAdjustment,
	int32 _initialBytesToStrip) :
	byteOrder(ByteOrder::BIG_ENDIAN),
	maxFrameLength(_maxFrameLength),
	lengthFieldOffset(_lengthFieldOffset),
	lengthFieldLength(_lengthFieldLength),
	lengthAdjustment(_lengthAdjustment),
	initialBytesToStrip(_initialBytesToStrip),
	failFast(true)
{
}

FLengthFieldBasedFrameDecoder::FLengthFieldBasedFrameDecoder(
	int32 _maxFrameLength,
	int32 _lengthFieldOffset,
	int32 _lengthFieldLength,
	int32 _lengthAdjustment,
	int32 _initialBytesToStrip,
	bool _failFast) :
	byteOrder(ByteOrder::BIG_ENDIAN),
	maxFrameLength(_maxFrameLength),
	lengthFieldOffset(_lengthFieldOffset),
	lengthFieldLength(_lengthFieldLength),
	lengthAdjustment(_lengthAdjustment),
	initialBytesToStrip(_initialBytesToStrip),
	failFast(_failFast)
{
}

FLengthFieldBasedFrameDecoder::FLengthFieldBasedFrameDecoder(
	ByteOrder _byteOrder,
	int32 _maxFrameLength,
	int32 _lengthFieldOffset,
	int32 _lengthFieldLength,
	int32 _lengthAdjustment,
	int32 _initialBytesToStrip,
	bool _failFast) :
	byteOrder(_byteOrder),
	maxFrameLength(_maxFrameLength),
	lengthFieldOffset(_lengthFieldOffset),
	lengthFieldLength(_lengthFieldLength),
	lengthAdjustment(_lengthAdjustment),
	initialBytesToStrip(_initialBytesToStrip),
	failFast(_failFast)
{
}

bool FLengthFieldBasedFrameDecoder::Init()
{
	if (maxFrameLength <= 0 || lengthFieldOffset < 0 || initialBytesToStrip < 0)
	{
		delete this;
		return false;
	}

	if (lengthFieldOffset > maxFrameLength - lengthFieldLength)
	{
		UE_LOG(LogTemp, Error,
			TEXT("maxFrameLength (%d) must be equal to or greater than lengthFieldOffset (%d) + lengthFieldLength (%d)"),
			maxFrameLength, lengthFieldOffset, lengthFieldLength);
		delete this;
		return false;
	}

	discardingTooLongFrame = false;
	tooLongFrameLength = 0.f;
	bytesToDiscard = 0.f;
	frameLengthInt = -1;

	this->lengthFieldEndOffset = lengthFieldOffset + lengthFieldLength;
	return true;
}

FLengthFieldBasedFrameDecoder::~FLengthFieldBasedFrameDecoder()
{
	UE_LOG(LogTemp, Log,
		TEXT("~FLengthFieldBasedFrameDecoder()"));
}

bool FLengthFieldBasedFrameDecoder::decode(FByteBuf& in, TSharedRef<FByteBuf> outbuf)
{
	uint64 frameLength = 0;
	if (frameLengthInt == -1) {
		if (discardingTooLongFrame) {
			DiscardingTooLongFrame(in);
		}
		if (in.readableBytes() < lengthFieldEndOffset) {
			// UE_LOG(LogTemp, Log, TEXT("return 1"));
			return false;
		}
		int32 actualLengthFieldOffset = in.GetReaderIndex() + lengthFieldOffset;
		frameLength = getUnadjustedFrameLength(in, actualLengthFieldOffset, lengthFieldLength, byteOrder);
		if (frameLength < 0) {
			UE_LOG(LogTemp, Log, TEXT("RPC :frameLength < 0: return 2"));
			failOnNegativeLengthField(in, frameLength, lengthFieldEndOffset);
			return false;
		}
		frameLength += lengthAdjustment + lengthFieldEndOffset;
		if (frameLength < lengthFieldEndOffset) {
			failOnFrameLengthLessThanLengthFieldEndOffset(in, frameLength, lengthFieldEndOffset);
		}
		if (frameLength > maxFrameLength)
		{
			if (frameLength/10 > maxFrameLength || frameLength > in.GetMaxCapacity()) {
				in.clear();
				UE_LOG(LogTemp, Error, TEXT("data error try clear "), frameLength);
				return false;

			}else
			{
				exceededFrameLength(in, frameLength);
				UE_LOG(LogTemp, Log, TEXT("RPC :frameLength > maxFrameLength: return 3, frameLength = %d "), frameLength);
				return false;
			}

		}
		frameLengthInt = (int32)frameLength;
	}
	if (in.readableBytes() < frameLengthInt) {
		UE_LOG(LogTemp, Log, TEXT("RPC :in.readableBytes() < frameLengthInt: return 4"));
		return false;
	}
	if (initialBytesToStrip > frameLengthInt) {
		failOnFrameLengthLessThanInitialBytesToStrip(in, frameLength, initialBytesToStrip);
		UE_LOG(LogTemp, Log, TEXT("RPC :initialBytesToStrip > frameLengthInt: return 5"));
		return false;
	}
	in.skipBytes(initialBytesToStrip);
	int32 actualFrameLength = frameLengthInt - initialBytesToStrip;
	in.readBytes(actualFrameLength, outbuf);
	frameLengthInt = -1;
	// UE_LOG(LogTemp, Log, TEXT("return 6"));
	in.discardReadedBytes();
	return true;
}

uint64 FLengthFieldBasedFrameDecoder::getUnadjustedFrameLength(FByteBuf& buf, int32 offset, int32 length, ByteOrder order)
{
	uint64 frameLength = 0;
	switch (length) {
	case 1:
		frameLength = buf.getByte(offset);
		break;
	case 2:
		frameLength = buf.getShort(offset);
		break;
	case 4:
		frameLength = buf.getInt(offset);
		break;
	case 8:
		frameLength = buf.getLong(offset);
		break;
	default:
		UE_LOG(LogTemp, Error, TEXT("unsupported lengthFieldLength: %d (expected: 1, 2, 4, or 8)"), lengthFieldLength);
	}
	return frameLength;
}

void FLengthFieldBasedFrameDecoder::failOnNegativeLengthField(FByteBuf& in, long frameLength, int32 _lengthFieldEndOffset)
{
	in.skipBytes(_lengthFieldEndOffset);

	UE_LOG(LogTemp, Error,
		TEXT("negative pre - adjustment length field : %ld"),
		frameLength);
}

void FLengthFieldBasedFrameDecoder::failOnFrameLengthLessThanLengthFieldEndOffset(FByteBuf& in, long frameLength, int32 _lengthFieldEndOffset)
{
	in.skipBytes(_lengthFieldEndOffset);
	UE_LOG(LogTemp, Error,
		TEXT("Adjusted frame length (%ld) ) is less than lengthFieldEndOffset: %d"),
		frameLength, _lengthFieldEndOffset);
}

void FLengthFieldBasedFrameDecoder::exceededFrameLength(FByteBuf& in, long frameLength)
{
	long discard = frameLength - in.readableBytes();
	tooLongFrameLength = frameLength;

	if (discard < 0) {
		in.skipBytes((int32)frameLength);
	}
	else {
		discardingTooLongFrame = true;
		bytesToDiscard = discard;
		in.skipBytes(in.readableBytes());
	}
	failIfNecessary(true);
}

void FLengthFieldBasedFrameDecoder::DiscardingTooLongFrame(FByteBuf& in)
{
	long _bytesToDiscard = this->bytesToDiscard;
	UE_LOG(LogTemp, Log, TEXT("discardingTooLongFrame , _bytesToDiscard = %d "), _bytesToDiscard);
	int32 localBytesToDiscard = (int32)FMath::Min<long>(_bytesToDiscard, in.readableBytes());
	in.skipBytes(localBytesToDiscard);
	_bytesToDiscard -= localBytesToDiscard;
	this->bytesToDiscard = _bytesToDiscard;

	failIfNecessary(false);
}

void FLengthFieldBasedFrameDecoder::failOnFrameLengthLessThanInitialBytesToStrip(FByteBuf& in, long frameLength, int32 _initialBytesToStrip)
{
	in.skipBytes((int32)frameLength);

	UE_LOG(LogTemp, Error,
		TEXT("Adjusted frame length (%ld) ) is less than initialBytesToStrip: %d"),
		frameLength, _initialBytesToStrip);
}

void FLengthFieldBasedFrameDecoder::fail(long frameLength)
{
	if (frameLength > 0) {
		UE_LOG(LogTemp, Error,
			TEXT("Adjusted frame exceeds (%d): %ld discarded"),
			maxFrameLength, frameLength);
	}
	else {

		UE_LOG(LogTemp, Error,
			TEXT("Adjusted frame exceeds %d - discarding"),
			maxFrameLength);
	}
}

void FLengthFieldBasedFrameDecoder::failIfNecessary(bool firstDetectionOfTooLongFrame)
{
	if (bytesToDiscard == 0) {
		long _tooLongFrameLength = this->tooLongFrameLength;
		this->tooLongFrameLength = 0;
		discardingTooLongFrame = false;
		if (!failFast || firstDetectionOfTooLongFrame) {
			fail(_tooLongFrameLength);
		}
	}
	else {
		if (failFast && firstDetectionOfTooLongFrame) {
			fail(this->tooLongFrameLength);
		}
	}
}
