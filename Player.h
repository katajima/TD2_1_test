#pragma once
#include "Model.h"
#include "WorldTransform.h"
#include "Input.h"
#include "Collider.h"
//#include "MapChipField.h"

class MapChipField;

///< summary>
/// 自クラス
///</summary>



float EaseOut(float t, float str, float end);


class Player : public Collider{
private:
	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	// ワールド変換データ
	WorldTransform worldTransform_;

	// ビュープロジェクション
	ViewProjection* viewProjection_ = nullptr;

	// 速度
	Vector3 velocity_ = {};

	bool isJamp = false;
	

	// 着地状態フラグ
	bool onGround_ = true;


	// モデル
	Model* model_ = nullptr;

	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;



public:
	///< summary>
	/// 初期化
	///</summary>
	///< param name = "model">モデル</param>
	///< param name = "textureHandle">テクスチャハンドル</param>
	///< param name = "viewProjection">ビュープロジェクション</param>
	void Initialize(Model* model, ViewProjection* viewProjection,const Vector3& position);

	///< summary>
	/// 更新
	///</summary>

	void Update();

	///< summary>
	/// 描画
	///</summary>

	void Draw();

	//　ゲットワールドトランスフォーム
	WorldTransform& GetWorldTransform();

	~Player();



	//　速度加算
	const Vector3& GetVeloctiy() const { return velocity_; };
	const float& GetVeloctiyX() const { return velocity_.x; };
	const float& GetVeloctiyY() const { return velocity_.y; };
	const float& GetVeloctiyZ() const { return velocity_.z; };

	//マップチップ
	void SetMapChipField(MapChipField* mapCipField) { mapChipField_ = mapCipField; };

	//移動
	void Move();

	Vector3 GetCenterPosition() const override;

	// 衝突を検出したら呼び出されるコールバック関数
	void OnCollision([[maybe_unused]] Collider* other) override;

	Vector3 rad{0.25f,0.5f,0.25f};
	AABB collAABB;


};
