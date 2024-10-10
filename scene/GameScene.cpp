#include "GameScene.h"
#include "AxisIndicator.h"
#include "ImGuiManager.h"
#include "PrimitiveDrawer.h"
#include "TextureManager.h"
#include <MathFunctions.h>
#include <cassert>

GameScene::GameScene() {}

GameScene::~GameScene() {
	// 2D
	delete sprite_;
	//
	//delete model_;
	// 3Dモデル
	delete blockModel_;
	// 3D天球
	delete modelSkydome_;
	//
	delete playerModel;
	// 天球クラス
	delete skydome_;
	//
	delete mapChipField_;

	
	// マップチップクラス
	//delete mapChipField_;

	// カメラコントローラ
	delete cameraController_;

	//// ワールドトランスフォーム
	//for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
	//	for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
	//		delete worldTransformBlock;
	//	}
	//}
	//worldTransformBlocks_.clear();

	// デバックカメラ
	delete debugCamera_;

	
}

//初期化
void GameScene::Initialize() {

	dxCommon_ = DirectXCommon::GetInstance();
	input_ = Input::GetInstance();
	audio_ = Audio::GetInstance();

	// マップチップ
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");
	mapChipField_->Initialize();
	

	// 3Dモデルの生成
	//blockModel_ = Model::Create();


	// ワールドトランスフォームの初期化
	worldTransform_.Initialize();

	// ビュープロジェクションの初期化

	// viewProjection_.farZ = 10;
	viewProjection_.Initialize();


	player_ = std::make_unique<Player>();
	// プレイヤークラス生成
	
	// 座標をマップチップ番号で指定
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(0,0,3);
	playerPosition = {0, 1, 0};
	playerModel = Model::CreateFromOBJ("player", true);
	// プレイヤー初期化
	player_->Initialize(playerModel, &viewProjection_, playerPosition);
	
	player_->SetMapChipField(mapChipField_);

	
	// 天球クラス生成
	skydome_ = new Skydome();

	// デバックカメラの生成
	debugCamera_ = new DebugCamera(100, 100);

	// 天球3Dモデルの生成
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);

	// 天球初期化
	skydome_->Initialize(modelSkydome_, &viewProjection_);

	blockModel_ = Model::CreateFromOBJ("block", true);

	//カメラコントローラ
	cameraController_ = new CameraController();
	cameraController_->Initialize(&viewProjection_);
	cameraController_->SetTarget(player_.get());
	cameraController_->Reset();
	//移動範囲の指定
	cameraController_->SetMovableArea({-100, 100, -100, 100});

	viewProjection_.translation_ = {7.4f, 9.3f, -22.0f};
	viewProjection_.rotation_.x = 0.31f;
	
}

void GameScene::Update() {


	ImGui::Begin("camera");
	ImGui::DragFloat3("translation", &viewProjection_.translation_.x,0.1f);
	ImGui::DragFloat3("rotate", &viewProjection_.rotation_.x,0.01f);
	ImGui::End();

	// プレイヤー更新処理
	player_->Update();
	

	// カメラコントローラ
	//cameraController_->Update();

	mapChipField_->Update();


	// 天球更新処理
	skydome_->Update();

	

#ifdef _DEBUG
	if (input_->TriggerKey(DIK_SPACE)) {
		if (isDebugCameraActive_ == false) {

			isDebugCameraActive_ = true;

		} else if (isDebugCameraActive_ == true) {

			isDebugCameraActive_ = false;
		}
	}

#endif //  _DEBUG

	if (isDebugCameraActive_) {
		// デバックカメラの更新
		debugCamera_->Update();

		viewProjection_.matView = debugCamera_->GetViewProjection().matView;

		viewProjection_.matProjection = debugCamera_->GetViewProjection().matProjection;

		// ビュープロジェクション行列の転送
		viewProjection_.TransferMatrix();
	} else {

		// ビュープロジェクション行列の更新と転送
		viewProjection_.UpdateMatrix();
	}
}

void GameScene::Draw() {

	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

#pragma region 背景スプライト描画
	// 背景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに背景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();
	// 深度バッファクリア
	dxCommon_->ClearDepthBuffer();
#pragma endregion

#pragma region 3Dオブジェクト描画
	// 3Dオブジェクト描画前処理
	Model::PreDraw(commandList);

	/// <summary>
	/// ここに3Dオブジェクトの描画処理を追加できる
	/// </summary>

	// 3Dモデル描画

	// 自キャラの描画
	player_->Draw();

	// スカイドーム描画
	skydome_->Draw();


	mapChipField_->Draw(viewProjection_);



	// 3Dオブジェクト描画後処理
	Model::PostDraw();

#pragma endregion

#pragma region 前景スプライト描画
	// 前景スプライト描画前処理
	Sprite::PreDraw(commandList);

	/// <summary>
	/// ここに前景スプライトの描画処理を追加できる
	/// </summary>

	// スプライト描画後処理
	Sprite::PostDraw();

#pragma endregion
}

void GameScene::GenerateBloks() 
{


}

void GameScene::CheckAllCollisions()
{

	// 衝突マネージャのリセット
	collisionManager_->Reset();
	// コライダーをリストに登録
	collisionManager_->AddCollider(player_.get());

	// 衝突判定
	collisionManager_->CheckAllCollisions();
}
