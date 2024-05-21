#pragma once
#include <Windows.h>
#include <chrono>
#include <cstdlib>
#include <d3d12.h>
#include <d3dx12.h>
#include <xaudio2.h>
#include <fstream>
#pragma comment(lib,"xaudio2.lib")

//チャンクヘッダ
struct ChunkHeader {
	char id[4];   //チャンク枚のID
	int32_t size; //チャンクサイズ
};

//RIFFヘッダチャンク
struct RiffHeader {
	ChunkHeader chunk; //"RIFF"
	char type[4];      //"WAVE"
};

//FMTチャンク
struct FormatChunk {
	ChunkHeader chunk; //"fmt"
	WAVEFORMATEX fmt;    //フォーマット
};

//音声データ
struct SoundData {
	//波形フォーマット
	WAVEFORMATEX wfex;
	//バッファの先頭アドレス
	BYTE* pBuffer;
	//バッファのサイズ
	unsigned int bufferSize;
};

struct SoundSource {
	//ソースボイス
	IXAudio2SourceVoice* pSourceVoice;
	//波形データの設定
	XAUDIO2_BUFFER buf{};
	//音声データ
	SoundData soundData;
};
class AudioController {

public:

	void Initialize();

	SoundData SoundLoadWave(const char* filename);

	void ResetXAudio2();

	void SoundUnload(SoundData* soundData);

	SoundSource CreateSoundSource(SoundSource source, bool isLoop);

	void PlaySoundWave(SoundSource source);

	void StopSoundWave(SoundSource source);

private:

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;

};