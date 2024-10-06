#define NOMINMAX

#include "DebugText.h"
#include "ImGuiManager.h"
#include <MapChipField.h>
#include <MathFunctions.h>
#include <Player.h>
#include <algorithm>
#include <cassert>
#include <imgui.h>
#include <numbers>
#include <wingdi.h>

// ゲットワールドトランスフォーム
WorldTransform& Player::GetWorldTransform() { return worldTransform_; }

// デストラクタ
Player::~Player() {

	// delete mapChipField_;
}

// 自キャラの初期化
void Player::Initialize(Model* model, ViewProjection* viewProjection, const Vector3& position) {
	Collider::Initialize();

	// NULLポインタチェック
	assert(model);

	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;

	viewProjection_ = viewProjection;

	// ワールド変換の初期化
	worldTransform_.Initialize();
	// プレイヤー位置
	worldTransform_.translation_ = position;
	// 回転
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

// 更新
void Player::Update() {
	// プレイヤーの移動を行う
	Move(); // ① 移動処理（少しずつ移動して衝突判定を行うように修正する必要あり）

	// マップ上のプレイヤーの位置インデックスを取得
	MapChipField::IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);
	Vector2 mapIndex = Vector2(float(index.xIndex), float(index.zIndex));

	// プレイヤーのAABB（Axis-Aligned Bounding Box）を更新
	collAABB.max = Add(worldTransform_.translation_, rad);
	collAABB.min = Subtract(worldTransform_.translation_, rad);

	// ImGuiデバッグ用ウィンドウ
	ImGui::Begin("PlayAABB");
	ImGui::InputFloat3("max", &collAABB.max.x);
	ImGui::InputFloat3("min", &collAABB.min.x); // 修正: minの入力
	ImGui::End();

	// マップチップに応じたY座標（高さ）の更新
	// mapChipField_->IsMapY(worldTransform_.translation_.y, rad.y, index);
	if (onGround_ == true) {
		mapChipField_->IsMapY2(collAABB, worldTransform_.translation_.y, rad.y);
	} else if (onGround_ == false && isJamp == true) {
		mapChipField_->IsMapY(collAABB, worldTransform_.translation_.y, rad.y);
	} else if (!onGround_ && !isJamp) {
		isJamp = true;
	}
	
	// AABBによるマップとの衝突判定
	if (mapChipField_->IsMapAABB(collAABB)) {
		velocity_.y = 0; // Y軸の速度をゼロにする（例えば地面に接触した時）
		isJamp = false;  // ジャンプ状態を解除
		onGround_ = true;
	} else if (!mapChipField_->IsMapAABB(collAABB)) {

		onGround_ = false;
	}

	// ImGuiデバッグ用ウィンドウ
	ImGui::Begin("Window");
	ImGui::InputFloat3("velocity_", &velocity_.x);
	ImGui::InputFloat3("translation_", &worldTransform_.translation_.x);
	ImGui::InputFloat2("mapIndex", &mapIndex.x); // 修正: mapIndexを表示
	ImGui::Checkbox("jump", &isJamp);
	ImGui::Checkbox("onGround", &onGround_);
	ImGui::End();

	// ワールドトランスフォームの更新
	worldTransform_.UpdateMatrix();
}

// 移動
void Player::Move() {

	if (Input::GetInstance()->TriggerKey(DIK_RIGHT) && isJamp == false) {
		worldTransform_.translation_.x += 1;
		isJamp = true;
		velocity_.y = 1;
	}
	if (Input::GetInstance()->TriggerKey(DIK_LEFT) && isJamp == false) {
		worldTransform_.translation_.x -= 1;
		isJamp = true;
		velocity_.y = 1;
	}
	if (Input::GetInstance()->TriggerKey(DIK_UP) && isJamp == false) {
		worldTransform_.translation_.z += 1;
		isJamp = true;
		velocity_.y = 1;
	}
	if (Input::GetInstance()->TriggerKey(DIK_DOWN) && isJamp == false) {
		worldTransform_.translation_.z -= 1;
		isJamp = true;
		velocity_.y = 1;
	}

	if (isJamp == true) {
		velocity_.y -= 0.1f;
	}

	worldTransform_.translation_.y += velocity_.y;

	if (worldTransform_.translation_.x >= 19) {
		worldTransform_.translation_.x = 19;
	}
	if (worldTransform_.translation_.x <= 0) {
		worldTransform_.translation_.x = 0;
	}
	if (worldTransform_.translation_.z >= 19) {
		worldTransform_.translation_.z = 19;
	}
	if (worldTransform_.translation_.z <= 0) {
		worldTransform_.translation_.z = 0;
	}
	if (worldTransform_.translation_.y <= 1) {
		// worldTransform_.translation_.y = 1;
		// isJamp = false;
	}
}

// 描画
void Player::Draw() {

	// 3Dモデルを描画
	model_->Draw(worldTransform_, *viewProjection_);
}

Vector3 Player::GetCenterPosition() const {
	const Vector3 ofset = {0.0f, 1.5f, 0.0f};
	// ワールド座標に変換
	Vector3 worldPos = Transform(ofset, worldTransform_.matWorld_);

	return worldPos;
}

void Player::OnCollision([[maybe_unused]] Collider* other) {
	// 衝突判定の種別IDを取得
	uint32_t typeID = other->GetTypeID();
	if (typeID == static_cast<uint32_t>(CollisionTypeIdDef::kEnemy)) {
	}
}
