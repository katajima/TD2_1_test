#pragma once
#include"MathFunctions.h"
#include"WorldTransform.h"
#include"ViewProjection.h"
#include"Model.h"
#include"CollisionTypeIdDef.h"

class Collider {
public:

	// 初期化
	void Initialize();

	// ワールドトランスフォームの初期化
	void UpdateWorldTransform();
	// 描画
	void Draw(Model* model, const ViewProjection& viewProjection);

	// 半径取得
	float GetRadius() { return radius_; }
	// 半径設定
	void SetRadius(float radius) { radius_ = radius; }

	// 衝突時に呼ばれる関数
	virtual void OnCollision([[maybe_unused]] Collider* other){};

	virtual Vector3 GetCenterPosition() const = 0;

	virtual ~Collider() = default; 

	// 種別IDの取得
	uint32_t GetTypeID() const { return typeID_; };
	// 種別IDの設定
	void SetTypeID(uint32_t typeID);
	
	void SetScale(Vector3 scale) { worldTransform.scale_ = scale; }

private:
	// 衝突判定
	float radius_ = 1.5f;
	int count = 0;
	// ワールドトランスフォーム
	WorldTransform worldTransform;

	// 種別ID
	uint32_t typeID_ = 0u;

};

