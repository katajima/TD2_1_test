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
	// 列数を設定（縦方向のブロック数）
	worldTransformBlocks_.resize(kNumBlockVirtical);
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		// 1列の要素数を設定（横方向のブロック数）
		worldTransformBlocks_[i].resize(kNumBlockHorizontal);
	}

	// ブロックの生成
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				// unique_ptrでWorldTransformのインスタンスを生成
				auto worldTransform = std::make_unique<WorldTransform>();
				worldTransform->Initialize();
				worldTransform->translation_ = GetMapChipPositionByIndex(j, i);
				worldTransformBlocks_[i][j] = std::move(worldTransform);
				collAABB[i][j].max = Add(worldTransformBlocks_[i][j]->translation_, rad);
				collAABB[i][j].min = Subtract(worldTransformBlocks_[i][j]->translation_, rad);
			}
		}
	}

	blockModel_ = Model::CreateFromOBJ("block", true);
}

void MapChipField::Update() {

	ImGui::Begin("MapAABBMove");
	ImGui::DragFloat("pos", &worldTransformBlocks_[3][3]->translation_.y, 0.01f);
	ImGui::End();

	ImGui::Begin("MapAABB");
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			// AABBのmaxとminを設定
			collAABB[i][j].max = Add(worldTransformBlocks_[i][j]->translation_, rad);
			collAABB[i][j].min = Subtract(worldTransformBlocks_[i][j]->translation_, rad);

			// maxの表示
			ImGui::InputFloat3(("max" + std::to_string(i) + "," + std::to_string(j)).c_str(), &collAABB[i][j].max.x);
			// minの表示
			ImGui::InputFloat3(("min" + std::to_string(i) + "," + std::to_string(j)).c_str(), &collAABB[i][j].min.x);
		}
	}
	ImGui::End();
	// ブロックの更新
	for (auto& worldTransformBlockLine : worldTransformBlocks_) {
		for (auto& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			// アフィン変換
			worldTransformBlock->matWorld_ = MakeAffineMatrixMatrix(worldTransformBlock->scale_, worldTransformBlock->rotation_, worldTransformBlock->translation_);

			// 定数バッファに転送する
			worldTransformBlock->TransferMatrix();
		}
	}
}

void MapChipField::Draw(const ViewProjection& viewProjection) {
	// ブロック描画
	for (auto& worldTransformBlockLine : worldTransformBlocks_) {
		for (auto& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			blockModel_->Draw(*worldTransformBlock, viewProjection);
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
MapChipType MapChipField::GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex) {

	if (xIndex < 0 || kNumBlockHorizontal - 1 < xIndex) {
		return MapChipType::kBlank;
	}
	if (yIndex < 0 || kNumBlockVirtical - 1 < yIndex) {
		return MapChipType::kBlank;
	}

	return mapChipData_.data[yIndex][xIndex];
}

// マップチップ座標の取得
//  縦 (z) と 横 (x) の指定で、その位置のマップチップのワールド座標を取得する
Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t zIndex) {
	return Vector3(kBlockWidth * xIndex, 0, kBlockHeight * zIndex); // z座標を使う
}

// マップチップ番号を計算
MapChipField::IndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) {
	MapChipField::IndexSet indexSet = {};
	indexSet.xIndex = uint32_t((position.x + kBlockWidth / 2) / kBlockWidth);
	indexSet.zIndex = uint32_t((position.z + kBlockHeight / 2) / kBlockHeight); // z座標を使用

	return indexSet;
}

bool MapChipField::IsMapAABB(AABB& charcter, IndexSet& index) { return IsCollision(charcter, collAABB[index.xIndex][index.zIndex]); }

bool MapChipField::IsMapAABB(AABB& charcter) {
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (IsCollision(charcter, collAABB[i][j])) {
				return true;
			}
		}
	}
	return false;
}

void MapChipField::IsMapY(float& posY, float radY, IndexSet& index) {

	if (worldTransformBlocks_[index.xIndex][index.zIndex]->translation_.y >= posY) {
		posY = worldTransformBlocks_[index.xIndex][index.zIndex]->translation_.y + radY + rad.y;
	}
}

void MapChipField::IsMapY(AABB& charcter, float& posY, float radY) {
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (IsCollision(charcter, collAABB[i][j])) {

				if (worldTransformBlocks_[i][j]->translation_.y >= posY) {
					posY = worldTransformBlocks_[i][j]->translation_.y + radY + rad.y;
				}
			}
		}
	}
}

void MapChipField::IsMapY2(AABB& charcter, float& posY, float radY) {
	for (uint32_t i = 0; i < kNumBlockVirtical; ++i) {
		for (uint32_t j = 0; j < kNumBlockHorizontal; ++j) {
			if (IsCollision(charcter, collAABB[i][j])) {

				posY = worldTransformBlocks_[i][j]->translation_.y + radY + rad.y;
			}
		}
	}
}

uint32_t MapChipField::GetNumBlockVirtical() { return kNumBlockVirtical; }

uint32_t MapChipField::GetNumBlockHorizontal() { return kNumBlockHorizontal; }
