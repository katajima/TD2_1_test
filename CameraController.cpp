#include <CameraController.h>
#include <Player.h>
#include <fstream>
#include <sstream>
#include <imgui.h>
#define NOMINMAX

void CameraController::Initialize(ViewProjection* viewProjection) { viewProjection_ = viewProjection; }

void CameraController::Update() {
	

	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	// 追従対象とオフセットと追従対象速度からカメラの目標座標を計算
	cameraTarget_.x = targetWorldTransform.translation_.x + targetOffset_.x + target_->GetVeloctiyX() * kVeloctiyBias;
	//cameraTarget_.y = targetWorldTransform.translation_.y + targetOffset_.y + target_->GetVeloctiyY() * kVeloctiyBias;
	// cameraTarget_.z = targetWorldTransform.translation_.z + targetOffset_.z + (target_->GetVeloctiyZ() * kVeloctiyBias);

	// 座標補間によりゆったり追従
	viewProjection_->translation_.x = Lerp(viewProjection_->translation_.x, cameraTarget_.x, kInterpolationRate);
	//viewProjection_->translation_.y = Lerp(viewProjection_->translation_.y, cameraTarget_.y, kInterpolationRate);
	// viewProjection_->translation_.z = Lerp(viewProjection_->translation_.z, cameraTarget_.z, kInterpolationRate);

	

	// 追従対象が画面外に出ないように補正
	viewProjection_->translation_.x = max(viewProjection_->translation_.x, cameraTarget_.x + kMargin.left);
	viewProjection_->translation_.x = min(viewProjection_->translation_.x, cameraTarget_.x + kMargin.right);
	viewProjection_->translation_.y = max(viewProjection_->translation_.y, cameraTarget_.y + kMargin.bottom);
	viewProjection_->translation_.y = min(viewProjection_->translation_.y, cameraTarget_.y + kMargin.top);
	
	
	

	// 移動制限
	viewProjection_->translation_.x = max(viewProjection_->translation_.x, movableArea_.left);
	viewProjection_->translation_.x = min(viewProjection_->translation_.x, movableArea_.right);
	viewProjection_->translation_.y = max(viewProjection_->translation_.y, movableArea_.bottom);
	viewProjection_->translation_.y = min(viewProjection_->translation_.y, movableArea_.top);

	viewProjection_->UpdateMatrix();
}

void CameraController::Reset() {

	// 追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	viewProjection_->translation_.x = targetWorldTransform.translation_.x + targetOffset_.x;
	viewProjection_->translation_.y = targetWorldTransform.translation_.y + targetOffset_.y + -4;
	viewProjection_->translation_.z = targetWorldTransform.translation_.z + targetOffset_.z + -7;
}

CameraController::~CameraController() {

	//delete target_;

}

