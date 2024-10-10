#pragma once
#include "Model.h"
#include "WorldTransform.h"
#include "ViewProjection.h"
#include "MathFunctions.h"

enum class MapChipType {
	kBlank, // 空白
	kBlock, // ブロック
};

struct  MapChipData {
	std::vector<std::vector<MapChipType>> data;
};



/// <summary>
/// マップチップフィールド
/// </summary> 

class MapChipField {
public:
	struct IndexSet {
		uint32_t xIndex;
		uint32_t zIndex;
	};

	struct MapObject {
		std::unique_ptr<WorldTransform> worldTransformBlocks_;
		AABB collAABB;
	};

public:

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
	void Draw(const ViewProjection& viewProjection);


	void ResetMapChipData();
	//読み込み
	void LoadMapChipCsv(const std::string& filePath);
	//マップチップ種別の取得
	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t zIndex);
	//マップチップ座標の取得
	Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t zIndex,float posY);

	uint32_t GetNumBlockVirtical();

	uint32_t GetNumBlockHorizontal();


	//座標からマップチップ番号を計算
	IndexSet GetMapChipIndexSetByPosition(const Vector3& position);

	bool IsMapAABB(AABB &charcter, IndexSet &index);
	bool IsMapAABB(AABB &charcter);

	void IsMapY(float &posY, float radY, IndexSet &index);
	void IsMapY(AABB& charcter, float& posY, float radY);
	void IsMapY2(AABB& charcter, float& posY, float radY);

private:
	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	// 1ブロックの個数
	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 20;


	MapChipData mapChipData_;

	// ブロックの3Dモデル
	Model* blockModel_ = nullptr;

	// ブロック用のワールドトランスフォームをユニークポインタで管理。
	// 複数並べるために配列にする
	std::vector<std::vector<std::unique_ptr<MapObject>>> mapWorld_;
	
	//AABB collAABB[kNumBlockHorizontal][kNumBlockVirtical];
	Vector3 rad = {0.5f, 0.5f, 0.5f};
};
