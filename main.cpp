#include <Novice.h>
#include <iostream>
#include <algorithm>
#include "AudioController.h"
#include "imgui.h"

const char kWindowTitle[] = "LE2B_17_ナガイ_コハク_TR1_AudioController";

struct Vector2 {
	float x, y;
};

float Dot(Vector2 v1, Vector2 v2) {

	float result;

	result = (v1.x * v2.x) + (v1.y * v2.y);

	return result;
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// ライブラリの初期化
	Novice::Initialize(kWindowTitle, 1280, 720);

	// キー入力結果を受け取る箱
	char keys[256] = { 0 };
	char preKeys[256] = { 0 };


	//オーディオコントローラー
	AudioController audioController;

	//サイレンの音
	SoundSource siren;

	//車の走行音
	SoundSource carNoize;


	//プレイヤーの速度
	Vector2 playerVelocity{ 0.0f,0.0f };

	//プレイヤーの座標
	Vector2 playerPosition{ 1280.0f / 2.0f ,720.0f / 2.0f };

	//救急車の速度
	Vector2 ambulanceVelocity{ 0.0f,0.0f };

	//救急車の座標
	Vector2 ambulancePosition{ 1280.0f / 2.0f - 1280.0f / 2.0f ,720.0f / 2.0f };


	//半径
	float radius = 30.0f;

	//減速度
	float attenuation = 0.1f;

	//加速度
	float acceleration = 0.3f;

	//速度の最大値
	float maxVelocity = 5.0f;


	//音速
	float sonic = 50.0f;

	//音量
	float volume = 0.5f;

	//周波数の元レート
	float sourceRate;

	//周波数の新レート
	float targetRate = 1.0f;


	//オーディオコントローラーの初期化
	audioController.Initialize();


	//サイレンのデータ読み込み
	siren.soundData = audioController.SoundLoadWave("Resources/siren.wav");

	//サイレンのデータ作成
	siren = audioController.CreateSoundSource(siren, true);

	//サイレンの周波数を取得
	siren.pSourceVoice->GetFrequencyRatio(&sourceRate);


	//走行音の読み込み
	carNoize.soundData = audioController.SoundLoadWave("Resources/carunoise-loop.wav");

	//走行音のデータ作成
	carNoize = audioController.CreateSoundSource(carNoize, true);


	//サイレンと走行音の音を鳴らす
	audioController.PlaySoundWave(siren);
	audioController.PlaySoundWave(carNoize);


	// ウィンドウの×ボタンが押されるまでループ
	while (Novice::ProcessMessage() == 0) {
		// フレームの開始
		Novice::BeginFrame();

		// キー入力を受け取る
		memcpy(preKeys, keys, 256);
		Novice::GetHitKeyStateAll(keys);

		///
		/// ↓更新処理ここから
		///

		//ImGuiの起動
		ImGui::Begin("Debug");

		//スペースを押したら音が止まる
		if (keys[DIK_SPACE] && !preKeys[DIK_SPACE]) {
			siren.pSourceVoice->Stop();
			carNoize.pSourceVoice->Stop();
		}

		//プレイヤーの移動
		//左右移動
		if (keys[DIK_A] || keys[DIK_D]) {
			if (keys[DIK_A]) {
				playerVelocity.x -= acceleration;
			}

			if (keys[DIK_D]) {
				playerVelocity.x += acceleration;
			}

		} else {

			//操作しなかったら減速
			playerVelocity.x *= (1.0f - attenuation);
		}

		//上下移動
		if (keys[DIK_W] || keys[DIK_S]) {
			if (keys[DIK_W]) {
				playerVelocity.y -= acceleration;
			}

			if (keys[DIK_S]) {
				playerVelocity.y += acceleration;
			}
		} else {

			//操作しなかったら減速
			playerVelocity.y *= (1.0f - attenuation);
		}

		//移動量の制限
		playerVelocity.x = std::clamp(playerVelocity.x, -maxVelocity, maxVelocity);
		playerVelocity.y = std::clamp(playerVelocity.y, -maxVelocity, maxVelocity);

		//移動
		playerPosition.x += playerVelocity.x;
		playerPosition.y += playerVelocity.y;

		//救急車の移動
		//左右移動
		if (keys[DIK_LEFTARROW] || keys[DIK_RIGHTARROW]) {
			if (keys[DIK_LEFTARROW]) {
				ambulanceVelocity.x -= acceleration;
			}
			if (keys[DIK_RIGHTARROW]) {
				ambulanceVelocity.x += acceleration;
			}
		} else {

			//操作しなかったら減速
			ambulanceVelocity.x *= (1.0f - attenuation);
		}
		
		//上下移動
		if (keys[DIK_UPARROW] || keys[DIK_DOWNARROW]) {
			if (keys[DIK_UPARROW]) {
				ambulanceVelocity.y -= acceleration;
			}
			if (keys[DIK_DOWNARROW]) {
				ambulanceVelocity.y += acceleration;
			}
		} else {

			//操作しなかったら減速
			ambulanceVelocity.y *= (1.0f - attenuation);
		}

		//移動量の制限
		ambulanceVelocity.x = std::clamp(ambulanceVelocity.x, -maxVelocity, maxVelocity);
		ambulanceVelocity.y = std::clamp(ambulanceVelocity.y, -maxVelocity, maxVelocity);

		//移動
		ambulancePosition.x += ambulanceVelocity.x;
		ambulancePosition.y += ambulanceVelocity.y;


		//プレイヤーの移動方向
		float playerDirection = cosf(atan2f(playerVelocity.y, playerVelocity.x));

		//プレイヤーから救急車までの角度
		float PtoADirection = cosf(atan2f(ambulancePosition.y - playerPosition.y, ambulancePosition.x - playerPosition.x)) * playerDirection;

		//プレイヤーから救急車に向けての移動速度
		float playerV = PtoADirection * sqrtf(Dot(playerVelocity, playerVelocity));


		//救急車の移動方向
		float ambulanceDirection = cosf(atan2f(ambulanceVelocity.y, ambulanceVelocity.x));

		//救急車からプレイヤーまでの角度
		float AtoPDirection = cosf(atan2f(playerPosition.y - ambulancePosition.y, playerPosition.x - ambulancePosition.x)) * ambulanceDirection;

		//救急車からプレイヤーに向けての移動速度
		float ambulanceV = AtoPDirection * sqrtf(Dot(ambulanceVelocity, ambulanceVelocity));


		//ドップラー効果の計算
		targetRate = ((sonic - playerV) / (sonic + ambulanceV)) * sourceRate;

		//変更後の周波数
		float frequencyRatio = sourceRate / targetRate;

		//サイレンのピッチを指定した周波数で変更
		siren.pSourceVoice->SetFrequencyRatio(frequencyRatio);


		//音量の変更
		siren.pSourceVoice->SetVolume(volume);
		carNoize.pSourceVoice->SetVolume(volume);


		//プレイヤーのimgui
		if (ImGui::TreeNode("player")) {
			ImGui::InputFloat("velocityX", &playerVelocity.x);
			ImGui::InputFloat("velocityY", &playerVelocity.y);
			ImGui::InputFloat("playerDirection", &playerDirection);
			ImGui::InputFloat("PtoADirection", &PtoADirection);
			ImGui::InputFloat("playerV", &playerV);
			ImGui::TreePop();
		}

		//救急車のimgui
		if (ImGui::TreeNode("ambulance")) {
			ImGui::InputFloat("ambulanceDirection", &ambulanceDirection);
			ImGui::InputFloat("AtoPDirection", &AtoPDirection);
			ImGui::InputFloat("ambulanceV", &ambulanceV);
			ImGui::TreePop();
		}

		//その他のimgui
		ImGui::InputFloat("sourceRate", &sourceRate);
		ImGui::InputFloat("targetRate", &targetRate);
		ImGui::DragFloat("sonic", &sonic, 1.0f);
		ImGui::DragFloat("volume", &volume, 0.01f,0.0f,1.0f);

		//imguiの受付終了
		ImGui::End();

		///
		/// ↑更新処理ここまで
		///

		///
		/// ↓描画処理ここから
		///

		//プレイヤーの描画
		Novice::DrawEllipse(
			int(playerPosition.x),
			int(playerPosition.y),
			int(radius),
			int(radius),
			0.0f,
			BLACK,
			kFillModeSolid
		);

		//救急車の描画
		Novice::DrawEllipse(
			int(ambulancePosition.x),
			int(ambulancePosition.y),
			int(radius),
			int(radius),
			0.0f,
			RED,
			kFillModeSolid
		);

		///
		/// ↑描画処理ここまで
		///

		// フレームの終了
		Novice::EndFrame();

		// ESCキーが押されたらループを抜ける
		if (preKeys[DIK_ESCAPE] == 0 && keys[DIK_ESCAPE] != 0) {
			break;
		}
	}

	audioController.ResetXAudio2();
	audioController.SoundUnload(&siren.soundData);
	audioController.SoundUnload(&carNoize.soundData);

	// ライブラリの終了
	Novice::Finalize();
	return 0;
}
