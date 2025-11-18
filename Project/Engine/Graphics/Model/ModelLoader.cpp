#include "ModelLoader.h"

#include <cassert>
#include "Engine/Graphics/Structs/VertexData.h"
#include "Engine/Math/MathCore.h"

ModelData ModelLoader::LoadModelFile(const std::string& directoryPath, const std::string& filename)
{
	const aiScene* scene = LoadAssimpFile(directoryPath + "/" + filename);
	assert(scene && scene->HasMeshes());

	ModelData result;

	// 全メッシュを統合して頂点データとインデックスデータを作成
	for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex) {
		aiMesh* mesh = scene->mMeshes[meshIndex];
		assert(mesh->HasNormals());
		assert(mesh->HasTextureCoords(0));

		// 頂点データの変換
		uint32_t baseVertexIndex = static_cast<uint32_t>(result.vertices.size());
		for (uint32_t vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex) {
			aiVector3D position = mesh->mVertices[vertexIndex];
			aiVector3D normal = mesh->mNormals[vertexIndex];
			aiVector3D texCoord = mesh->mTextureCoords[0][vertexIndex];

			VertexData vertex{};
			vertex.position = { position.x, position.y, position.z, 1.0f };
			vertex.normal = { normal.x, normal.y, normal.z };
			vertex.texcoord = { texCoord.x, texCoord.y };

			result.vertices.push_back(vertex);
		}

		// 面からIndexの情報を取得する
		for (uint32_t faceIndex = 0; faceIndex < mesh->mNumFaces; ++faceIndex) {
			const aiFace& face = mesh->mFaces[faceIndex];
			assert(face.mNumIndices == 3); // 三角形のみ対応

			for (uint32_t element = 0; element < face.mNumIndices; ++element) {
				uint32_t vertexIndex = face.mIndices[element];
				result.indices.push_back(baseVertexIndex + vertexIndex);
			}
		}

		// SkinCluster情報の読み込み
		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
			aiBone* bone = mesh->mBones[boneIndex];
			std::string jointName = bone->mName.C_Str();
			JointWeightData& jointWeightData = result.skinClusterData[jointName];

			aiMatrix4x4 bindPoseMatrixAssimo = bone->mOffsetMatrix.Inverse();
			aiVector3D scale, translate;
			aiQuaternion rotate;
			bindPoseMatrixAssimo.Decompose(scale, rotate, translate);
			Matrix4x4 bindPoseMatrix = MathCore::Matrix::MakeAffine(
				{ scale.x, scale.y, scale.z },
				{ rotate.x, rotate.y, rotate.z, rotate.w },
				{ translate.x, translate.y, translate.z }
			);
			jointWeightData.inverseBindPoseMatrix = MathCore::Matrix::Inverse(bindPoseMatrix);

			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex) {
				jointWeightData.vertexWeights.push_back({ bone->mWeights[weightIndex].mWeight, bone->mWeights[weightIndex].mVertexId });
			}
		}

		// 最初のメッシュのマテリアルデータを取得
		if (meshIndex == 0 && mesh->mMaterialIndex < scene->mNumMaterials) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
				aiString texPath;
				if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS) {
					result.material.textureFilePath = directoryPath + "/" + texPath.C_Str();
				}
			}
		}
	}

	// Node階層構造の読み込み
	result.rootNode = ReadNode(scene->mRootNode);

	return result;
}

const aiScene* ModelLoader::LoadAssimpFile(const std::string& filepath)
{
	static Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(
		filepath.c_str(),
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_ConvertToLeftHanded |
		aiProcess_FlipUVs
	);

	assert(scene && "Failed to load model file");

	return scene;
}

Node ModelLoader::ReadNode(aiNode* node) {
	Node result;

	// aiNodeからSRTを抽出
	aiVector3D scale, translate;
	aiQuaternion rotate;
	node->mTransformation.Decompose(scale, rotate, translate);

	// QuaternionTransformに変換（座標変換はaiProcess_ConvertToLeftHandedで処理済み）
	result.transform.scale = { scale.x, scale.y, scale.z };
	result.transform.rotate = { rotate.x, rotate.y, rotate.z, rotate.w };
	result.transform.translate = { translate.x, translate.y, translate.z };

	// localMatrixを再構築
	result.localMatrix = MathCore::Matrix::MakeAffine(
		result.transform.scale,
		result.transform.rotate,
		result.transform.translate
	);

	// Node名を格納
	result.name = node->mName.C_Str();

	// 子Nodeを再帰的に読み込み
	result.children.resize(node->mNumChildren);
	for (uint32_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex) {
		result.children[childIndex] = ReadNode(node->mChildren[childIndex]);
	}

	return result;
}