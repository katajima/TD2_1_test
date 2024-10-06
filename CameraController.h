#pragma once
#include <ViewProjection.h>
#include<MathFunctions.h>

// 前方宣言
class Player;

/// <summary>
/// カメラコントローラ
/// </summary>

class CameraController {
private:
	// ビュープロジェクション
	ViewProjection* viewProjection_ = nullptr;

	// プレイヤー
	Player* target_ = nullptr;

	// 追従対象とカメラの座標の差（オフセット）
	Vector3 targetOffset_ = {0, 0, -15.0f};

	// 矩形
	struct Rect {
		float left = 0.0f;   // 左端
		float right = 1.0f;  // 右端
		float bottom = 0.0f; // 下端
		float top = 1.0f;    // 上端
	};

	// カメラ移動範囲
	Rect movableArea_ = {0, 100, 0, 100};

	//カメラ目標座標
	Vector3 cameraTarget_{};


	//座標補間割合
	static inline const float kInterpolationRate = 1.0f;

	// 速度掛け率
	static inline const float kVeloctiyBias = 4.5f;

	// 追従対象の各方向へのカメラ移動制限 余白
	static inline const Rect kMargin = {-6, 6, -6, 6};
	

public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(ViewProjection* viewProjection);
	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	//
	void SetTarget(Player* target) { target_ = target; }

	//
	void Reset();

	// 移動範囲指定
	void SetMovableArea(Rect area) { movableArea_ = area; };

	~CameraController();

};
