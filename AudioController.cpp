#include "AudioController.h"
#include <cassert>

void AudioController::Initialize() {

	HRESULT result;

	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	result = xAudio2->CreateMasteringVoice(&masterVoice);

}

SoundData AudioController::SoundLoadWave(const char* filename) {

	//ファイル入力ストリームのインスタンス
	std::ifstream file;

	//.wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);

	//ファイルオープン失敗を検出する
	assert(file.is_open());

	///
	/// RIFFチャンクの読み込み
	///

	//RIFFヘッダーの読み込み
	RiffHeader riff;

	file.read((char*)&riff, sizeof(riff));

	//ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id,"RIFF",4) != 0) {
		assert(0);
	}

	//タイプがWAVEかチェック
	if (strncmp(riff.type,"WAVE",4) != 0) {
		assert(0);
	}

	///
	/// fmtチャンクの読み込み
	///

	//Formatチャンクの読み込み
	FormatChunk format = {};

	//チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));

	if (strncmp(format.chunk.id,"fmt ",4) != 0) {
		assert(0);
	}

	//チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));

	file.read((char*)&format.fmt, format.chunk.size);

	///
	/// dataチャンクの読み込み
	///

	//Dataチャンクの読み込み
	ChunkHeader data;

	file.read((char*)&data, sizeof(data));


	//bextチャンクを検出した場合
	if (strncmp(data.id, "bext", 4) == 0) {

		//読み取り位置をbextチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);

		//再読み込み
		file.read((char*)&data, sizeof(data));
	}

	//junkチャンクを検出した場合
	if (strncmp(data.id, "junk", 4) == 0) {

		//読み取り位置をjunkチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);

		//再読み込み
		file.read((char*)&data, sizeof(data));
	}

	//JUNKチャンクを検出した場合
	if (strncmp(data.id,"JUNK",4) == 0) {

		//読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);

		//再読み込み
		file.read((char*)&data, sizeof(data));
	}

	//JUNKチャンクを検出した場合
	if (strncmp(data.id, "LIST", 4) == 0) {

		//読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);

		//再読み込み
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id,"data",4) != 0) {
		assert(0);
	}

	//Dataチャンクのデータ部 (波形データ) の読み込み
	char* pBuffer = new char[data.size];

	file.read(pBuffer, data.size);

	//Waveファイルを閉じる
	file.close();

	//returnするための音声データ;
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}

void AudioController::ResetXAudio2() {

	xAudio2.Reset();
}

void AudioController::SoundUnload(SoundData* soundData) {

	//バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}

SoundSource AudioController::CreateSoundSource(SoundSource source, bool isLoop) {

	HRESULT result;

	//波形フォーマットをもとにSoundVoiceの生成
	result = xAudio2->CreateSourceVoice(&source.pSourceVoice, &source.soundData.wfex);
	assert(SUCCEEDED(result));

	//再生する波形データの設定
	source.buf.pAudioData = source.soundData.pBuffer;
	source.buf.AudioBytes = source.soundData.bufferSize;
	source.buf.Flags = XAUDIO2_END_OF_STREAM;

	if (isLoop) {
		source.buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	SoundSource resultSource;

	resultSource.pSourceVoice = source.pSourceVoice;
	resultSource.buf = source.buf;
	resultSource.soundData = source.soundData;

	return resultSource;
}

void AudioController::PlaySoundWave(SoundSource source) {

	HRESULT result;

	result = source.pSourceVoice->SubmitSourceBuffer(&source.buf);
	result = source.pSourceVoice->Start();
}

void AudioController::StopSoundWave(SoundSource source) {

	xAudio2->CommitChanges(source.pSourceVoice->Stop());

}