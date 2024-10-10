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
Player::~Player() 
{

	// delete mapChipField_;
}

// 自キャラの初期化
void Player::Initialize(Model* model, ViewProjection* viewProjection, const Vector3& position) {
	Collider::Initialize();
	Collider::SetAABBRabius(Vector3{0.25f,0.25f,0.25f});
	radEne_ = Collider::GetAABBRadius();
	

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
	

	// マップ上のプレイヤーの位置インデックスを取得
	MapChipField::IndexSet index = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);
	Vector2 mapIndex = Vector2(float(index.xIndex), float(index.zIndex));

	// プレイヤーのAABB（Axis-Aligned Bounding Box）を更新
	collAABB_.max = Add(worldTransform_.translation_, rad_);
	collAABB_.min = Subtract(worldTransform_.translation_, rad_);

	// ImGuiデバッグ用ウィンドウ
	ImGui::Begin("PlayAABB");
	ImGui::InputFloat3("max", &collAABB_.max.x);
	ImGui::InputFloat3("min", &collAABB_.min.x); // 
	ImGui::InputFloat3("uesEnemyRad", &radEne_.x); // 
	aabbEne_ = Collider::GetAABB();
	ImGui::InputFloat3("uesEnemyAABB.max", &aabbEne_.max.x); // 
	ImGui::InputFloat3("uesEnemyAABB.min", &aabbEne_.min.x); //
	ImGui::End();

	// マップチップに応じたY座標（高さ）の更新
	if (!isFlyColliderIgnored) {

		if (onGround_ == true) // 地面に着地している 
		{
			//mapChipField_->IsMapY2(collAABB_, worldTransform_.translation_.y, rad_.y);
		}
		else if (onGround_ == false && isJamp_ == true) // 着地していなくて、ジャンプしているとき 
		{
			mapChipField_->IsMapY(collAABB_, worldTransform_.translation_.y, rad_);
		}
		else if (!onGround_ && !isJamp_) // 着地していなくて、ジャンプしていないとき
		{
			isJamp_ = true;
		}

		// AABBによるマップとの衝突判定
		if (mapChipField_->IsMapAABB(collAABB_)) {
			velocity_.y = 0; // Y軸の速度をゼロにする（例えば地面に接触した時）
			isJamp_ = false; // ジャンプ状態を解除
			onGround_ = true;
		} else if (!mapChipField_->IsMapAABB(collAABB_)) {

			onGround_ = false;
		}
	}



	// プレイヤーの移動を行う
	Move(); // ① 移動処理（少しずつ移動して衝突判定を行うように修正する必要あり）

	// ImGuiデバッグ用ウィンドウ
	ImGui::Begin("Window");
	if (ImGui::Button("SetPos")) {
		worldTransform_.translation_ = mapChipField_->GetMapChipPositionByIndex(0,0,3);
	}
	ImGui::InputFloat3("velocity_", &velocity_.x);
	ImGui::InputFloat3("translation_", &worldTransform_.translation_.x);
	ImGui::InputFloat2("mapIndex", &mapIndex.x); // 修正: mapIndexを表示
	ImGui::Checkbox("jump", &isJamp_);
	ImGui::Checkbox("onGround", &onGround_);
	ImGui::End();

	Collider::UpdateWorldTransform();
	// ワールドトランスフォームの更新
	worldTransform_.UpdateMatrix();
}

// 移動
void Player::Move() {

	if (Input::GetInstance()->TriggerKey(DIK_RIGHT) && isJamp_ == false) {
		worldTransform_.translation_.x += 1;
		isJamp_ = true;
		velocity_.y = 1;
	}
	if (Input::GetInstance()->TriggerKey(DIK_LEFT) && isJamp_ == false) {
		worldTransform_.translation_.x -= 1;
		isJamp_ = true;
		velocity_.y = 1;
	}
	if (Input::GetInstance()->TriggerKey(DIK_UP) && isJamp_ == false) {
		worldTransform_.translation_.z += 1;
		isJamp_ = true;
		velocity_.y = 1;
	}
	if (Input::GetInstance()->TriggerKey(DIK_DOWN) && isJamp_ == false) {
		worldTransform_.translation_.z -= 1;
		isJamp_ = true;
		velocity_.y = 1;
	}

	if (isJamp_ == true) {
		velocity_.y -= 0.1f;
		if (velocity_.y <= -1.5f) {
			velocity_.y = -1.5f;
		}
		if (velocity_.y >= 0) {
			isFlyColliderIgnored = true;
		} else {
			isFlyColliderIgnored = false;
		}
	} else {
		isFlyColliderIgnored = false;
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
	}
}

// 描画
void Player::Draw() {

	// 3Dモデルを描画
	model_->Draw(worldTransform_, *viewProjection_);
}

Vector3 Player::GetCenterPosition() const {
	const Vector3 ofset = {0.0f, 0.0f, 0.0f};
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
