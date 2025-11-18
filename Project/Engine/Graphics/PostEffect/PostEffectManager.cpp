#include "PostEffectManager.h"

//各種ポストエフェクトのヘッダ
#include "Engine/Graphics/Common/DirectXCommon.h"
#include "Engine/Graphics/Render/Render.h"
#include "Effect/GrayScale.h"
#include "FullScreen.h"
#include "Effect/Blur.h"
#include "Effect/Shockwave.h"
#include "Effect/Vignette.h"
#include "Effect/RadialBlur.h"
#include "Effect/ColorGrading.h"
#include "Effect/ChromaticAberration.h"
#include "Effect/Sepia.h"
#include "Effect/Invert.h"
#include "Effect/RasterScroll.h"
#include "Effect/FadeEffect.h"
#include "PostEffectPresetManager.h"
#include "Engine/Utility/Debug/ImGui/ImguiManager.h"
#include <cassert>

// =============================================================================
// PingPongBuffer実装
// =============================================================================

PostEffectManager::PingPongBuffer::PingPongBuffer(DirectXCommon* dxCommon, Render* render)
	: dxCommon_(dxCommon)
	, render_(render)
	, currentInput_()
	, currentOutputIndex_(1) // 最初の出力は1番（0番に初期シーンがある前提）
{
}

void PostEffectManager::PingPongBuffer::Reset(D3D12_GPU_DESCRIPTOR_HANDLE input)
{
	currentInput_ = input;
	currentOutputIndex_ = 1;
}

bool PostEffectManager::PingPongBuffer::ApplyEffect(PostEffectBase* effect)
{
	if (!effect || !effect->IsEnabled()) {
		return false;
	}

	// エフェクトを現在の出力バッファに描画
	render_->OffscreenPreDraw(currentOutputIndex_);
	effect->Draw(currentInput_);
	render_->OffscreenPostDraw(currentOutputIndex_);

	// 今書き込んだバッファが次の入力になる
	currentInput_ = GetSrvHandle(currentOutputIndex_);

	// 次回の出力先を切り替え（ping-pong）
	currentOutputIndex_ = (currentOutputIndex_ == 0) ? 1 : 0;

	return true;
}

D3D12_GPU_DESCRIPTOR_HANDLE PostEffectManager::PingPongBuffer::GetCurrentOutput() const
{
	return currentInput_;
}

void PostEffectManager::PingPongBuffer::EnsureOutputInBuffer1(FullScreen* fullScreenEffect)
{
	// 最後に書き込まれたバッファを特定（currentOutputIndexは次回の出力先なので、その反対が最後の書き込み先）
	int lastWrittenIndex = (currentOutputIndex_ == 0) ? 1 : 0;

	// 既にバッファ1にある場合は何もしない
	if (lastWrittenIndex == 1) {
		return;
	}

	// バッファ0にある場合、バッファ1にコピー
	render_->OffscreenPreDraw(1);
	fullScreenEffect->Draw(currentInput_);
	render_->OffscreenPostDraw(1);

	// 出力を更新
	currentInput_ = GetSrvHandle(1);
	currentOutputIndex_ = 0; // 次回は0に書き込む
}

D3D12_GPU_DESCRIPTOR_HANDLE PostEffectManager::PingPongBuffer::GetSrvHandle(int index) const
{
	return (index == 0)
		? dxCommon_->GetOffScreenSrvHandle()
		: dxCommon_->GetOffScreen2SrvHandle();
}

// =============================================================================
// PostEffectManager実装
// =============================================================================

void PostEffectManager::Initialize(DirectXCommon* dxCommon, Render* render)
{
	assert(dxCommon);
	assert(render);
	directXCommon_ = dxCommon;
	render_ = render;

	// プリセットマネージャーの初期化
	presetManager_ = std::make_unique<PostEffectPresetManager>();

	// 基本的なポストエフェクトを登録
	auto grayScale = std::make_unique<GrayScale>();
	grayScale->Initialize(dxCommon);
	grayScale_ = grayScale.get();
	RegisterEffect("GrayScale", std::move(grayScale));

	auto fullScreen = std::make_unique<FullScreen>();
	fullScreen->Initialize(dxCommon);
	fullScreen_ = fullScreen.get();
	RegisterEffect("FullScreen", std::move(fullScreen));

	// ブラーエフェクトを登録
	auto blur = std::make_unique<Blur>();
	blur->Initialize(dxCommon);
	blur_ = blur.get();
	RegisterEffect("Blur", std::move(blur));

	// ラジアルブラーエフェクトを登録
	auto radialBlur = std::make_unique<RadialBlur>();
	radialBlur->Initialize(dxCommon);
	radialBlur_ = radialBlur.get();
	RegisterEffect("RadialBlur", std::move(radialBlur));

	// ショックウェーブエフェクトを登録
	auto shockwave = std::make_unique<Shockwave>();
	shockwave->Initialize(dxCommon);
	shockwave_ = shockwave.get();
	RegisterEffect("Shockwave", std::move(shockwave));

	// ヴィネットエフェクトを登録
	auto vignette = std::make_unique<Vignette>();
	vignette->Initialize(dxCommon);
	vignette_ = vignette.get();
	RegisterEffect("Vignette", std::move(vignette));

	// カラーグレーディングエフェクトを登録
	auto colorGrading = std::make_unique<ColorGrading>();
	colorGrading->Initialize(dxCommon);
	colorGrading_ = colorGrading.get();
	RegisterEffect("ColorGrading", std::move(colorGrading));

	// 色収差エフェクトを登録
	auto chromaticAberration = std::make_unique<ChromaticAberration>();
	chromaticAberration->Initialize(dxCommon);
	chromaticAberration_ = chromaticAberration.get();
	RegisterEffect("ChromaticAberration", std::move(chromaticAberration));

	// セピアエフェクトを登録
	auto sepia = std::make_unique<Sepia>();
	sepia->Initialize(dxCommon);
	sepia_ = sepia.get();
	RegisterEffect("Sepia", std::move(sepia));

	// インバートエフェクトを登録
	auto invert = std::make_unique<Invert>();
	invert->Initialize(dxCommon);
	invert_ = invert.get();
	RegisterEffect("Invert", std::move(invert));

	// ラスタースクロールエフェクトを登録
	auto rasterScroll = std::make_unique<RasterScroll>();
	rasterScroll->Initialize(dxCommon);
	rasterScroll_ = rasterScroll.get();
	RegisterEffect("RasterScroll", std::move(rasterScroll));

	// フェードエフェクトを登録
	auto fadeEffect = std::make_unique<FadeEffect>();
	fadeEffect->Initialize(dxCommon);
	fadeEffect_ = fadeEffect.get();
	RegisterEffect("FadeEffect", std::move(fadeEffect));

	// 初期設定：FullScreenは常に有効、他はデフォルトで無効
	SetEffectEnabled("FullScreen", true);
	SetEffectEnabled("GrayScale", false);
	SetEffectEnabled("Blur", false);
	SetEffectEnabled("RadialBlur", false);
	SetEffectEnabled("Shockwave", false);
	SetEffectEnabled("Vignette", false);
	SetEffectEnabled("ColorGrading", false);
	SetEffectEnabled("ChromaticAberration", false);
	SetEffectEnabled("Sepia", false);
	SetEffectEnabled("Invert", false);
	SetEffectEnabled("RasterScroll", false);
	SetEffectEnabled("FadeEffect", true);

	// 最終テクスチャハンドルの初期化
	finalDisplayHandle_ = directXCommon_->GetOffScreenSrvHandle();
}

void PostEffectManager::RegisterEffect(const std::string& name, std::unique_ptr<PostEffectBase> effect)
{
	effects_[name] = std::move(effect);
}

PostEffectBase* PostEffectManager::GetEffect(const std::string& name)
{
	auto it = effects_.find(name);
	if (it != effects_.end()) {
		return it->second.get();
	}
	return nullptr;
}

const PostEffectBase* PostEffectManager::GetEffect(const std::string& name) const
{
	auto it = effects_.find(name);
	if (it != effects_.end()) {
		return it->second.get();
	}
	return nullptr;
}

std::vector<std::string> PostEffectManager::CollectEnabledEffectNames(
	const std::vector<std::string>& effectNames) const
{
	std::vector<std::string> enabledNames;
	enabledNames.reserve(effectNames.size());

	for (const auto& name : effectNames) {
		if (auto* effect = GetEffect(name); effect && effect->IsEnabled()) {
			enabledNames.push_back(name);
		}
	}

	return enabledNames;
}

D3D12_GPU_DESCRIPTOR_HANDLE PostEffectManager::ExecuteDefaultEffectChain(
	D3D12_GPU_DESCRIPTOR_HANDLE inputSrvHandle)
{
	// 有効なエフェクト名を収集
	auto enabledNames = CollectEnabledEffectNames(defaultEffectChain_);

	// 有効なエフェクトがない場合は入力をそのまま返す
	if (enabledNames.empty()) {
		finalDisplayHandle_ = inputSrvHandle;
		return inputSrvHandle;
	}

	// Ping-Pongバッファで順次エフェクトを適用
	PingPongBuffer pingPong(directXCommon_, render_);
	pingPong.Reset(inputSrvHandle);

	for (const auto& name : enabledNames) {
		if (auto* effect = GetEffect(name)) {
			pingPong.ApplyEffect(effect);
		}
	}

	// 最終結果をオフスクリーン#1に保証
	pingPong.EnsureOutputInBuffer1(fullScreen_);

	// 最終結果を保存して返す
	finalDisplayHandle_ = pingPong.GetCurrentOutput();
	return finalDisplayHandle_;
}

void PostEffectManager::ExecuteEffect(const std::string& name, D3D12_GPU_DESCRIPTOR_HANDLE inputSrvHandle)
{
	auto* effect = GetEffect(name);
	if (effect) {
		effect->Draw(inputSrvHandle);
	}
}

void PostEffectManager::SetEffectEnabled(const std::string& effectName, bool enabled)
{
	auto* effect = GetEffect(effectName);
	if (effect) {
		effect->SetEnabled(enabled);
	}
}

bool PostEffectManager::IsEffectEnabled(const std::string& effectName) const
{
	auto it = effects_.find(effectName);
	if (it != effects_.end()) {
		return it->second->IsEnabled();
	}
	return false;
}

void PostEffectManager::Update(float deltaTime)
{
	// ショックウェーブエフェクトの更新
	if (shockwave_) {
		shockwave_->Update(deltaTime);
	}

	// ラスタースクロールエフェクトの更新
	if (rasterScroll_) {
		rasterScroll_->Update(deltaTime);
	}

	// フェードエフェクトの更新
	if (fadeEffect_) {
		fadeEffect_->Update(deltaTime);
	}
}

void PostEffectManager::DrawImGui()
{
	if (ImGui::Begin("Post Effects")) {
		// プリセット管理タブ
		presetManager_->ShowImGui(this);

		ImGui::Separator();

		// エフェクトチェーン状態の表示
		if (ImGui::CollapsingHeader("エフェクトチェーン状態", ImGuiTreeNodeFlags_DefaultOpen)) {
			auto enabledNames = CollectEnabledEffectNames(defaultEffectChain_);

			ImGui::Text("エフェクトチェーン: %s",
				enabledNames.empty() ? "非アクティブ (パススルー)" : "アクティブ");

			ImGui::Text("登録済みエフェクト数: %zu", effects_.size());
			ImGui::Text("有効なエフェクト数: %zu", enabledNames.size());
			ImGui::Text("デフォルトエフェクトチェーン:");

			for (const auto& name : defaultEffectChain_) {
				auto* effect = GetEffect(name);
				if (effect && effect->IsEnabled()) {
					ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "  - %s", name.c_str());
				} else {
					ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "  - %s (無効)", name.c_str());
				}
			}

			if (enabledNames.empty()) {
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
					"エフェクトが無効 - 元の画像を描画中");
			}

			ImGui::Separator();
		}

		// 各エフェクトのパラメータ調整
		for (auto& [name, effect] : effects_) {
			ImGui::PushID(name.c_str());

			if (ImGui::CollapsingHeader(name.c_str())) {
				// FullScreenエフェクト以外はenable/disableチェックボックスを表示
				if (name != "FullScreen") {
					bool enabled = effect->IsEnabled();
					if (ImGui::Checkbox("有効", &enabled)) {
						effect->SetEnabled(enabled);
					}
					ImGui::Separator();
				}

				// エフェクトのパラメータ調整
				effect->DrawImGui();
			}

			ImGui::PopID();
		}
	}
	ImGui::End();
}

// エフェクトゲッター
GrayScale* PostEffectManager::GetGrayScale() { return grayScale_; }
FullScreen* PostEffectManager::GetFullScreen() { return fullScreen_; }
Blur* PostEffectManager::GetBlur() { return blur_; }
Shockwave* PostEffectManager::GetShockwave() { return shockwave_; }
Vignette* PostEffectManager::GetVignette() { return vignette_; }
RadialBlur* PostEffectManager::GetRadialBlur() { return radialBlur_; }
ColorGrading* PostEffectManager::GetColorGrading() { return colorGrading_; }
ChromaticAberration* PostEffectManager::GetChromaticAberration() { return chromaticAberration_; }
Sepia* PostEffectManager::GetSepia() { return sepia_; }
Invert* PostEffectManager::GetInvert() { return invert_; }
RasterScroll* PostEffectManager::GetRasterScroll() { return rasterScroll_; }
FadeEffect* PostEffectManager::GetFadeEffect() { return fadeEffect_; }

D3D12_GPU_DESCRIPTOR_HANDLE PostEffectManager::GetFinalDisplayTextureHandle() const
{
	return finalDisplayHandle_;
}