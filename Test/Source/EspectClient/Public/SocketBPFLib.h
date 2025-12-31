// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include <Net/SocketObject.h>
#include "SocketBPFLib.generated.h"

/**
 *
 */

UCLASS()
class ESPECTCLIENT_API USocketBPFLib : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	~USocketBPFLib();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "EspectClient")
	static USocketBPFLib *GetSocketClientManager();
	static USocketBPFLib *SocketBPFLib;


	UFUNCTION(BlueprintCallable, Category = "EspectClient")
	static USocketObject* InitNetClient(const FString& TCPServerIP, const int32 TCPServerPort);
	USocketObject* InitNetClientNonStatic(const FString& TCPServerIP, const int32 TCPServerPort);


	// 获取网络端口
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"), Category = "EspectClient")
	static void GetAddressURL(const UObject *WorldContextObject, FString &IP, int32 &Port);


	UFUNCTION(BlueprintPure, Category = "EspectClient")
	USocketObject* GetSocketObject(){ return NetClientSocketObject; }


	UFUNCTION(BlueprintPure, Category = "EspectClient")
	static FString ConvertBytesToString(const TArray<uint8>& Data);

	UFUNCTION(BlueprintPure, Category = "EspectClient")
	static TArray<uint8> ConvertStringToBytes(const FString& Data);

	UFUNCTION(BlueprintPure, Category = "EspectClient")
	static FJsonLibraryValue ConvertBytesToJson(const TArray<uint8>& Data);

	UFUNCTION(BlueprintPure, Category = "EspectClient")
	static TArray<uint8> ConvertJsonToBytes(const FJsonLibraryValue& Data);

	// aes 加密
	// UFUNCTION(BlueprintCallable, Category = "Insidous|Math|DateTime")
	static FString AESEncrypt(const FString& data, const FString& key);

	// aes 解密
	// UFUNCTION(BlueprintCallable, Category = "Insidous|Math|DateTime")
	static FString AESDecrypt(const FString& data, const FString& key);

	static FString AESDecryptDytes(TArray<uint8> data, const FString& key);

private:
	USocketObject* NetClientSocketObject;

};
