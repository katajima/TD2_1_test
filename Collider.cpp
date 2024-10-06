#include "Collider.h"

void Collider::Initialize() {

worldTransform.Initialize();
	count++;
}

void Collider::UpdateWorldTransform() {

	// ワールド座標をワールドトランスフォームに適応
	worldTransform.translation_ = GetCenterPosition();

	worldTransform.UpdateMatrix();
}

void Collider::Draw(Model* model, const ViewProjection& viewProjection) {

	model->Draw(worldTransform, viewProjection);

}

void Collider::SetTypeID(uint32_t typeID) { typeID_ = typeID; };
