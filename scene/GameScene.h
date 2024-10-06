#pragma once

#include "Audio.h"
#include "DebugCamera.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "Model.h"
#include "Player.h"
#include "skydome.h"
#include "MapChipField.h"
#include "CameraController.h"
#include "Sprite.h"
#include "ViewProjection.h"
#include "WorldTransform.h"
#include "CollisionManager.h"
#include<vector>

/// <summary>
/// ゲームシーン
/// </summary>
class GameScene {

public: // メンバ関数
	/// <summary>
	/// コンストクラタ
	/// </summary>
	GameScene();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GameScene();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();


	void GenerateBloks();
	/// <summary>
	/// 衝突判定と応答
	/// </summary>
	void CheckAllCollisions();

private: // メンバ変数
	DirectXCommon* dxCommon_ = nullptr;
	Input* input_ = nullptr;
	Audio* audio_ = nullptr;

	
	//プレイヤー
	//  自キャラ
	std::unique_ptr<Player> player_;
	
	
	Model* playerModel = nullptr; 

	//プレイヤーテクスチャ
	uint32_t textureHandle_ = 0;

	// スプライト
	Sprite* sprite_ = nullptr;

	// ブロックの3Dモデル
	Model* blockModel_ = nullptr;

	// ワールドトランスフォーム
	WorldTransform worldTransform_;

	// ビュープロジェクション
	ViewProjection viewProjection_;

	

	MapChipField* mapChipField_;

	//デバックカメラ有効
	bool isDebugCameraActive_ = false;

	// デバッグカメラ
	DebugCamera* debugCamera_ = nullptr;

	// 天球の3Dモデル
	Model* modelSkydome_ = nullptr;
	// 天球クラス
	Skydome* skydome_ = nullptr;

	//カメラコントローラ
	CameraController* cameraController_ = nullptr;

	// ImGuiで値を入力する変数
	// float inputFloat3[3] = {0, 0, 0};

	/// <summary>
	/// ゲームシーン用
	/// </summary>
	// 衝突マネージャ
	std::unique_ptr<CollisionManager> collisionManager_;
};
