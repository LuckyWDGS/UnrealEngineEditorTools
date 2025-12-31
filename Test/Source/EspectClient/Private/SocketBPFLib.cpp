// Fill out your copyright notice in the Description page of Project Settings.

#include "SocketBPFLib.h"
#include "SocketSubsystem.h"

#include "Misc/AES.h"
#include "Misc/Base64.h"

#if WITH_SSL
#define UI UI_ST

#include <openssl/evp.h>
#include <openssl/aes.h>
#include <openssl/err.h>

#undef UI
#endif // WITH_SSL


USocketBPFLib* USocketBPFLib::SocketBPFLib;

USocketBPFLib::USocketBPFLib(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) {
	AddToRoot();
	SocketBPFLib = this;
}

USocketBPFLib::~USocketBPFLib()
{
	UE_LOG(LogTemp, Log, TEXT("~USocketBPFLib() ~~~~~~~~~~~~~~~~~~~~~~~~~~~~"));
}

USocketBPFLib* USocketBPFLib::GetSocketClientManager()
{
	return SocketBPFLib;
}

USocketObject* USocketBPFLib::InitNetClient(const FString& TCPServerIP, const int32 TCPServerPort)
{
	return USocketBPFLib::GetSocketClientManager()->InitNetClientNonStatic(TCPServerIP, TCPServerPort);
}

USocketObject* USocketBPFLib::InitNetClientNonStatic(const FString& TCPServerIP, const int32 TCPServerPort)
{
	NetClientSocketObject = NewObject<USocketObject>();
	if (NetClientSocketObject && NetClientSocketObject->InitSocket(TCPServerIP, TCPServerPort))
	{
		return NetClientSocketObject;
	}

	NetClientSocketObject->RemoveFromRoot();
	NetClientSocketObject = nullptr;
	return NetClientSocketObject;
}

void USocketBPFLib::GetAddressURL(const UObject* Context, FString& IP, int32& Port)
{
	Port = Context->GetWorld()->URL.Port;
	bool bCanBindAll = false;

#if PLATFORM_ANDROID
	ISocketSubsystem* sSS = ISocketSubsystem::Get(FName(TEXT("ANDROID")));
#else
	ISocketSubsystem* sSS = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
#endif

	TSharedPtr<class FInternetAddr> Addr = sSS->GetLocalHostAddr(*GLog, bCanBindAll);
	IP = Addr->ToString(false);
}


FString USocketBPFLib::ConvertBytesToString(const TArray<uint8>& Data)
{
	if (Data.Num() > 0 && Data.Last() == 0)
		return UTF8_TO_TCHAR(Data.GetData());

	TArray<uint8> ZeroTerminated(Data);
	ZeroTerminated.Add(0);
	return UTF8_TO_TCHAR(ZeroTerminated.GetData());
}
TArray<uint8> USocketBPFLib::ConvertStringToBytes(const FString& Data)
{
	TArray<uint8> Payload;

	FTCHARToUTF8 Converter(*Data);
	Payload.SetNum(Converter.Length());
	FMemory::Memcpy(Payload.GetData(), (uint8*)(ANSICHAR*)Converter.Get(), Payload.Num());

	return Payload;
}


FJsonLibraryValue USocketBPFLib::ConvertBytesToJson(const TArray<uint8>& Data)
{
	return FJsonLibraryValue::Parse(ConvertBytesToString(Data));
}
TArray<uint8> USocketBPFLib::ConvertJsonToBytes(const FJsonLibraryValue& Data)
{
	return ConvertStringToBytes(Data.Stringify());
}


FString USocketBPFLib::AESEncrypt(const FString& data, const FString& key)
{
#if WITH_SSL

    // Convert FString to byte array
    TArray<uint8> DataBytes;
    FTCHARToUTF8 Convert(*data);
    DataBytes.Append((uint8*)Convert.Get(), Convert.Length());

    // Generate a random nonce
    uint8 Nonce[12];
    FGuid guid;
    FPlatformMisc::CreateGuid(guid);
    FString GuidString = guid.ToString().Left(12);
    FTCHARToUTF8 ConvertGuid(*GuidString);
    FMemory::Memcpy(Nonce, ConvertGuid.Get(), 12);

    // Prepare the encryption context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(Nonce), NULL);
    EVP_EncryptInit_ex(ctx, NULL, NULL, (uint8*)TCHAR_TO_UTF8(*key), Nonce);

    // Encrypt the data
    TArray<uint8> Ciphertext;
    Ciphertext.SetNum(DataBytes.Num() + EVP_CIPHER_block_size(EVP_aes_256_gcm()));
    int OutLen;
    EVP_EncryptUpdate(ctx, Ciphertext.GetData(), &OutLen, DataBytes.GetData(), DataBytes.Num());

    // Finalize encryption
    int FinalLen;
    EVP_EncryptFinal_ex(ctx, Ciphertext.GetData() + OutLen, &FinalLen);
    Ciphertext.SetNum(OutLen + FinalLen);

    // Get the tag
    uint8 Tag[16];
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, sizeof(Tag), Tag);


    // Append the tag to the ciphertext
    Ciphertext.Append(Tag, sizeof(Tag));

    // Append the Nonce to the ciphertext
    Ciphertext.Append(Nonce, sizeof(Nonce));

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    // Convert to Base64 for easy storage
    FString EncryptedData = FBase64::Encode(Ciphertext);
    return EncryptedData;
#endif // WITH_SSL
//    return FString();
}

FString USocketBPFLib::AESDecrypt(const FString& data, const FString& key)
{
#if WITH_SSL

    // Decode Base64
    TArray<uint8> Ciphertext;
    FBase64::Decode(data, Ciphertext);

    if (Ciphertext.Num() < 28) return FString();

    // Extract nonce from the ciphertext
    uint8 Nonce[12];
    FMemory::Memcpy(Nonce, Ciphertext.GetData() + Ciphertext.Num() - 12, 12);
    Ciphertext.RemoveAt(Ciphertext.Num() - 12, 12);

    // Extract tag from the ciphertext
    uint8 Tag[16];
    FMemory::Memcpy(Tag, Ciphertext.GetData() + Ciphertext.Num() - 16, 16);
    Ciphertext.SetNum(Ciphertext.Num() - 16);


    // Prepare the decryption context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(Nonce), NULL);
    EVP_DecryptInit_ex(ctx, NULL, NULL, (uint8*)TCHAR_TO_UTF8(*key), Nonce);

    // Decrypt the data
    TArray<uint8> DecryptedData;
    DecryptedData.SetNum(Ciphertext.Num());
    int OutLen;
    EVP_DecryptUpdate(ctx, DecryptedData.GetData(), &OutLen, Ciphertext.GetData(), Ciphertext.Num());

    // Set the expected tag value
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, sizeof(Tag), Tag);

    // Finalize decryption
    int FinalLen;
    if (EVP_DecryptFinal_ex(ctx, DecryptedData.GetData() + OutLen, &FinalLen) <= 0)
    {
        // Tag verification failed
        EVP_CIPHER_CTX_free(ctx);
        return FString(); // Return an empty string or handle the error as needed
    }
    DecryptedData.SetNum(OutLen + FinalLen);


    // Clean up
    EVP_CIPHER_CTX_free(ctx);
    DecryptedData.Add(0);

    // Convert byte array to FString
    FString DecryptedString = FString(UTF8_TO_TCHAR(DecryptedData.GetData()));
    return DecryptedString;
#endif // WITH_SSL
//return FString();
}

FString USocketBPFLib::AESDecryptDytes(TArray<uint8> Ciphertext, const FString& key)
{
#if WITH_SSL
    if(Ciphertext.Num() < 28) return FString();
    // Extract nonce from the ciphertext
    uint8 Nonce[12];
    FMemory::Memcpy(Nonce, Ciphertext.GetData() + Ciphertext.Num() - 12, 12);
    Ciphertext.RemoveAt(Ciphertext.Num() - 12, 12);

    // Extract tag from the ciphertext
    uint8 Tag[16];
    FMemory::Memcpy(Tag, Ciphertext.GetData() + Ciphertext.Num() - 16, 16);
    Ciphertext.SetNum(Ciphertext.Num() - 16);


    // Prepare the decryption context
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, sizeof(Nonce), NULL);
    EVP_DecryptInit_ex(ctx, NULL, NULL, (uint8*)TCHAR_TO_UTF8(*key), Nonce);

    // Decrypt the data
    TArray<uint8> DecryptedData;
    DecryptedData.SetNum(Ciphertext.Num());
    int OutLen;
    EVP_DecryptUpdate(ctx, DecryptedData.GetData(), &OutLen, Ciphertext.GetData(), Ciphertext.Num());

    // Set the expected tag value
    EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, sizeof(Tag), Tag);

    // Finalize decryption
    int FinalLen;
    if (EVP_DecryptFinal_ex(ctx, DecryptedData.GetData() + OutLen, &FinalLen) <= 0)
    {
        // Tag verification failed
        EVP_CIPHER_CTX_free(ctx);
        return FString(); // Return an empty string or handle the error as needed
    }
    DecryptedData.SetNum(OutLen + FinalLen);

    // Clean up
    EVP_CIPHER_CTX_free(ctx);

    DecryptedData.Add(0);

    // Convert byte array to FString
    FString DecryptedString = FString(UTF8_TO_TCHAR(DecryptedData.GetData()));
    return DecryptedString;
#endif // WITH_SSL
  //  return FString();
}
