#pragma once
#include "Engine/Graphics/Model/Skeleton/Skeleton.h"
#include "Engine/Graphics/LineRenderer.h"
#include "Engine/Math/Matrix/Matrix4x4.h"
#include <vector>

/// @brief Skeletonのデバッグ描画クラス
class SkeletonDebugRenderer {
public:
	/// @brief Skeletonのライン配列を生成
	/// @param skeleton Skeleton
	/// @param worldMatrix ワールド行列
	/// @param jointRadius Jointの球の半径
	/// @param outLines 出力先のライン配列
	static void GenerateSkeletonLines(
		const Skeleton& skeleton,
		const Matrix4x4& worldMatrix,
		float jointRadius,
		std::vector<LineRenderer::Line>& outLines
	);

	/// @brief SkeletonのImGuiデバッグUI（共通実装）
	/// @param skeleton Skeleton
	/// @param drawSkeleton スケルトン描画フラグ（参照）
	/// @param jointRadius Joint半径（参照）
	/// @param objectName オブジェクト名（ID生成用）
	/// @return ImGuiで変更があった場合true
	static bool DrawSkeletonImGui(
		const Skeleton* skeleton,
		bool& drawSkeleton,
		float& jointRadius,
		const char* objectName
	);
};
