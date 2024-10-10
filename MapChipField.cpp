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
				mapObject->collAABB.max = Add(mapObject->worldTransformBlocks_->translation_, rad);
				mapObject->collAABB.min = Subtract(mapObject->worldTransformBlocks_->translation_, rad);

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
	// ブロックの更新
	int i = 0;
	for (auto& worldTransformBlockLine : mapWorld_) {
		int j = 0;
		for (auto& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			// AABBのmaxとminを設定
			worldTransformBlock->collAABB.max = Add(worldTransformBlock->worldTransformBlocks_->translation_, rad);
			worldTransformBlock->collAABB.min = Subtract(worldTransformBlock->worldTransformBlocks_->translation_, rad);
			MapChipType o =  GetMapChipTypeByIndex(i,j);
			int a;
			a = int(o);
			ImGui::InputInt("num", &a);
			// maxの表示
			ImGui::InputFloat3(("max" + std::to_string(i) + "," + std::to_string(j)).c_str(), &worldTransformBlock->collAABB.max.x);
			// minの表示
			ImGui::InputFloat3(("min" + std::to_string(i) + "," + std::to_string(j)).c_str(), &worldTransformBlock->collAABB.min.x);

			// アフィン変換
			worldTransformBlock->worldTransformBlocks_->matWorld_ = MakeAffineMatrixMatrix(
			    worldTransformBlock->worldTransformBlocks_->scale_, 
				worldTransformBlock->worldTransformBlocks_->rotation_, 
				worldTransformBlock->worldTransformBlocks_->translation_);

			// 定数バッファに転送する
			worldTransformBlock->worldTransformBlocks_->TransferMatrix();
			j++;
		}
		i++;
	}
	ImGui::End();
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

bool MapChipField::IsMapAABB(AABB& charcter, IndexSet& index) { return IsCollision(charcter, mapWorld_[index.xIndex][index.zIndex]->collAABB); }

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

	if (mapWorld_[index.xIndex][index.zIndex]->worldTransformBlocks_->translation_.y >= posY) {
		posY = mapWorld_[index.xIndex][index.zIndex]->worldTransformBlocks_->translation_.y + radY + rad.y;
	}
}

void MapChipField::IsMapY(AABB& charcter, float& posY, float radY) {

	for (auto& worldTransformBlockLine : mapWorld_) {
		for (auto& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			if (IsCollision(charcter, worldTransformBlock->collAABB)) {
				if (worldTransformBlock->worldTransformBlocks_->translation_.y >= posY) {
					posY = worldTransformBlock->worldTransformBlocks_->translation_.y + radY + rad.y;
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

				
					posY = worldTransformBlock->worldTransformBlocks_->translation_.y + radY + rad.y;
				
			}
		}
	}
}

uint32_t MapChipField::GetNumBlockVirtical() { return kNumBlockVirtical; }

uint32_t MapChipField::GetNumBlockHorizontal() { return kNumBlockHorizontal; }
