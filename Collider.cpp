#include "Collider.h"

void Collider::Initialize() {

worldTransform.Initialize();
	count++;
}

void Collider::UpdateWorldTransform() {

	aabb_.max = Add(worldTransform.translation_, aabbRadius_);
	aabb_.min = Subtract(worldTransform.translation_, aabbRadius_);

	// ワールド座標をワールドトランスフォームに適応
	worldTransform.translation_ = GetCenterPosition();

	worldTransform.UpdateMatrix();
}

void Collider::Draw(Model* model, const ViewProjection& viewProjection) {

	model->Draw(worldTransform, viewProjection);

}

void Collider::SetTypeID(uint32_t typeID) { typeID_ = typeID; };
