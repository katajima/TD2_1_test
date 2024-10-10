#include "imgui.h"
#include "string"
#include <MapChipField.h>
#include <assert.h>
#include <fstream>
#include <map>
#include <sstream>

namespace {

std::map<std::string, MapChipType> mapChipTable = {
    {"0", MapChipType::kBlank},
    {"1", MapChipType::kBlock},
};
}

void MapChipField::Initialize() {
	// マップの初期化
	mapWorld_.resize(kNumBlockVirtical);
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		mapWorld_[i].resize(kNumBlockHorizontal);
	}

	// ブロックの生成
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				// MapObjectの生成
				auto mapObject = std::make_unique<MapObject>();
				mapObject->worldTransformBlocks_ = std::make_unique<WorldTransform>();
				mapObject->worldTransformBlocks_->Initialize();
				mapObject->worldTransformBlocks_->translation_ = GetMapChipPositionByIndex(j, i,0);
				mapObject->collAABB.max = Add(mapObject->worldTransformBlocks_->translation_, rad_);
				mapObject->collAABB.min = Subtract(mapObject->worldTransformBlocks_->translation_, rad_);

				// マップワールドに格納
				mapWorld_[i][j] = std::move(mapObject);
			}
		}
	}

	blockModel_ = Model::CreateFromOBJ("block", true);
}

void MapChipField::Update() {
	ImGui::Begin("MapAABB");
	ImGui::DragFloat("pos00", &mapWorld_[0][0]->worldTransformBlocks_->translation_.y, 0.01f);
	ImGui::DragFloat("pos10", &mapWorld_[0][1]->worldTransformBlocks_->translation_.y, 0.01f);
	ImGui::End();

	static float time = 0.0f;            // ウェーブの時間を管理
	static bool hasWaveOccurred = false; // ウェーブが発生したかどうかのフラグ
	const float deltaTime = 0.016f;      // フレームごとの時間経過（例: 60fpsの場合）

	// ウェーブの設定
	const int waveOriginX = 5;        // ウェーブの中心となるX座標（インデックス）
	const int waveOriginZ = 5;        // ウェーブの中心となるZ座標（インデックス）
	const float waveSpeed = 2.0f;     // ウェーブの進行速度
	const float waveAmplitude = 1.0f; // ウェーブの高さ（振幅）
	//const float waveDuration = 1.0f;  // ウェーブの持続時間

	// ウェーブの広がりサイズを計算
	int waveSize = int(time * waveSpeed);

	// ブロックの更新
	for (int i = 0; i < mapWorld_.size(); ++i) {
		for (int j = 0; j < mapWorld_[i].size(); ++j) {
			auto& worldTransformBlock = mapWorld_[i][j];
			if (!worldTransformBlock)
				continue;

			// ウェーブ効果を計算
			float waveHeight = 0.0f;

			// ウェーブ範囲内にあるか判定
			if (i >= waveOriginX - waveSize && i <= waveOriginX + waveSize && j >= waveOriginZ - waveSize && j <= waveOriginZ + waveSize) {
				// ウェーブ効果を計算
				waveHeight = calculateWaveEffectSquare(j, i, time, waveOriginX, waveOriginZ, waveAmplitude, waveSize);
			}

			// マップチップのY座標にウェーブの高さを適用
			worldTransformBlock->worldTransformBlocks_->translation_.y = waveHeight;

			// AABBのmaxとminを設定
			worldTransformBlock->collAABB.max = Add(worldTransformBlock->worldTransformBlocks_->translation_, rad_);
			worldTransformBlock->collAABB.min = Subtract(worldTransformBlock->worldTransformBlocks_->translation_, rad_);

			// アフィン変換の更新
			worldTransformBlock->worldTransformBlocks_->UpdateMatrix();
		}
	}

	// ウェーブの時間を更新
	if (!hasWaveOccurred && time < waveDuration) {
		time += deltaTime;
	} else if (time >= waveDuration) {
		// ウェーブが発生したらフラグを立てる
		hasWaveOccurred = true;

		// 元の高さに戻す
		for (int i = 0; i < mapWorld_.size(); ++i) {
			for (int j = 0; j < mapWorld_[i].size(); ++j) {
				auto& worldTransformBlock = mapWorld_[i][j];
				if (!worldTransformBlock)
					continue;

				// マップチップのY座標を元に戻す
				worldTransformBlock->worldTransformBlocks_->translation_.y = 0.0f;

				// AABBのmaxとminを設定
				worldTransformBlock->collAABB.max = Add(worldTransformBlock->worldTransformBlocks_->translation_, rad_);
				worldTransformBlock->collAABB.min = Subtract(worldTransformBlock->worldTransformBlocks_->translation_, rad_);

				// アフィン変換の更新
				worldTransformBlock->worldTransformBlocks_->UpdateMatrix();
			}
		}
	}
}


// 四角形状のウェーブ効果の計算関数
float MapChipField::calculateWaveEffectSquare(int x, int z, float time, int waveOriginX, int waveOriginZ, float waveAmplitude, int waveSize) {
	// ウェーブの範囲内の座標に対して振幅効果を適用
	float distanceX = float(std::abs(x - waveOriginX));
	float distanceZ = float(std::abs(z - waveOriginZ));

	waveSize;

	// 四角形範囲内の座標に対してsin波を使って高さを計算
	float waveEffect = waveAmplitude * sin((distanceX + distanceZ) - time);

	return waveEffect;
}

// ウェーブ効果の計算関数（マンハッタン距離を使用し、四角形のウェーブを実現）
float MapChipField::calculateWaveEffect(int x, int z, float time, int waveOriginX, int waveOriginZ, float waveSpeed, float waveAmplitude, float waveRange) {
	// マンハッタン距離を計算
	float distance = fabsf(float(x - waveOriginX)) + fabsf(float(z - waveOriginZ));

	// ウェーブの範囲内か確認
	if (distance > waveRange) {
		return 0.0f; // ウェーブの範囲外なら影響なし
	}

	// ウェーブの高さを計算（sin波を使い、一定時間後に振幅を終わらせる）
	float waveEffect = waveAmplitude * sinf(time * waveSpeed - distance);

	// timeがwaveDurationを超えたら振幅を終わらせる
	if (time > waveDuration) {
		waveEffect = 0.0f;
	}

	return waveEffect;
}

void MapChipField::Draw(const ViewProjection& viewProjection) {
	// ブロック描画
	for (auto& worldTransformBlockLine : mapWorld_) {
		for (auto& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			blockModel_->Draw(*worldTransformBlock->worldTransformBlocks_, viewProjection);
		}
	}
}

void MapChipField::ResetMapChipData() {

	// マップチップデータをリセット
	mapChipData_.data.clear();
	mapChipData_.data.resize(kNumBlockVirtical);
	for (std::vector<MapChipType>& mapChipDataLine : mapChipData_.data) {
		mapChipDataLine.resize(kNumBlockHorizontal);
	}
}
// 読み込み
void MapChipField::LoadMapChipCsv(const std::string& filePath) {

	// マップチップデータをリセット
	ResetMapChipData();

	// ファイルを開く
	std::ifstream file;
	file.open(filePath);
	assert(file.is_open());

	// マップチップCSV
	std::stringstream mapChipCsv;
	// ファイルの内容を文字列にストリームにコピー
	mapChipCsv << file.rdbuf();
	// ファイルを閉じる
	file.close();

	// CSVからマップチップデータを読み込む
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {

		std::string line;
		getline(mapChipCsv, line);

		// 1行分の文字列をストリームに変換して解析しやすくする
		std::istringstream line_stream(line);

		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {

			std::string word;
			getline(line_stream, word, ',');

			if (mapChipTable.contains(word)) {
				mapChipData_.data[i][j] = mapChipTable[word];
			}
		}
	}
}

// マップチップ種別の取得
MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t zIndex) {

	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex)
	{
		return MapChipType::kBlank;
	}
	if (zIndex < 0 || kNumBlockVirtical - 1 < zIndex) 
	{
		return MapChipType::kBlank;
	}

	return mapChipData_.data[zIndex][xIndex];
}

// マップチップ座標の取得
//  縦 (z) と 横 (x) の指定で、その位置のマップチップのワールド座標を取得する
Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t zIndex, float posY) {
	return Vector3(kBlockWidth * xIndex, posY, kBlockHeight * zIndex); // z座標を使う
}

// マップチップ番号を計算
MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) {
	MapChipField::IndexSet indexSet = {};
	indexSet.xIndex = uint32_t((position.x + kBlockWidth / 2) / kBlockWidth);
	indexSet.zIndex = uint32_t((position.z + kBlockHeight / 2) / kBlockHeight); // z座標を使用

	return indexSet;
}

bool MapChipField::IsMapAABB(AABB& charcter, IndexSet& index) { return IsCollision(charcter, mapWorld_[index.zIndex][index.xIndex]->collAABB); }

bool MapChipField::IsMapAABB(AABB& charcter) {

	for (auto& worldTransformBlockLine : mapWorld_) {
		for (auto& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			if (IsCollision(charcter, worldTransformBlock->collAABB)) {
				return true;
			}
		}
	}

	return false;
}

void MapChipField::IsMapY(float& posY, float radY, IndexSet& index) {

	if (mapWorld_[index.zIndex][index.xIndex]->worldTransformBlocks_->translation_.y >= posY) {
		posY = mapWorld_[index.zIndex][index.xIndex]->worldTransformBlocks_->translation_.y + radY + rad_.y;
	}
}

void MapChipField::IsMapY(AABB& charcter, float& posY, float radY) {

	for (auto& worldTransformBlockLine : mapWorld_) {
		for (auto& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			if (IsCollision(charcter, worldTransformBlock->collAABB)) {
				if (worldTransformBlock->worldTransformBlocks_->translation_.y >= posY) {
					posY = worldTransformBlock->worldTransformBlocks_->translation_.y + radY + rad_.y;
				}
			}
		}
	}
}

void MapChipField::IsMapY(AABB& charcter, float& posY, Vector3 rad) {
	
	Vector3 pos = Add(charcter.min, rad);
	
	// キャラクターの位置からマップインデックスを取得
	IndexSet index = GetMapChipIndexSetByPosition(pos);

	// インデックス範囲内か確認
	if (index.xIndex < kNumBlockHorizontal && index.zIndex < kNumBlockVirtical) {
		// 近いマップ番号だけをチェック
		for (int i = -1; i <= 1; ++i) {
			for (int j = -1; j <= 1; ++j) {
				int x = index.xIndex + i;
				int z = index.zIndex + j;

				// 範囲外チェック
				if (x < 0 || x >= kNumBlockHorizontal || z < 0 || z >= kNumBlockVirtical)
					continue;

				auto& worldTransformBlock = mapWorld_[z][x];
				if (!worldTransformBlock)
					continue;

				if (IsCollision(charcter, worldTransformBlock->collAABB)) {
					if (worldTransformBlock->worldTransformBlocks_->translation_.y >= posY) {
						posY = worldTransformBlock->worldTransformBlocks_->translation_.y + rad.y + rad_.y;
					}
				}
			}
		}
	}
}

void MapChipField::IsMapY2(AABB& charcter, float& posY, float radY) {


	for (auto& worldTransformBlockLine : mapWorld_) {
		for (auto& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			if (IsCollision(charcter, worldTransformBlock->collAABB)) {

				
					posY = worldTransformBlock->worldTransformBlocks_->translation_.y + radY + rad_.y;
				
			}
		}
	}
}

uint32_t MapChipField::GetNumBlockVirtical() { return kNumBlockVirtical; }

uint32_t MapChipField::GetNumBlockHorizontal() { return kNumBlockHorizontal; }

