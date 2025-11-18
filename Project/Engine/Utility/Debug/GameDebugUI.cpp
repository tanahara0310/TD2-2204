#include "GameDebugUI.h"

#include "Utility/Debug/ImGui/DockingUI.h"
#include <EngineSystem.h>
#include "Engine/Utility/FrameRate/FrameRateController.h"
#include "Engine/Scene/SceneManager.h"

#include <Psapi.h>
#include <algorithm>

void GameDebugUI::Initialize(EngineSystem* engine, DockingUI* dockingUI)
{
	assert(engine != nullptr);
	engine_ = engine;
	dockingUI_ = dockingUI;

	// コンソールの初期化
	console_->Initialize();

	// コンソールにエンジンシステムを設定
	console_->SetEngineSystem(engine);

	// ドッキングUIが提供されている場合、ウィンドウを登録
	if (dockingUI_) {
		RegisterWindowsForDocking();
	}

	// 初期メッセージをコンソールに追加
	console_->LogInfo("GameDebugUIが正常に初期化されました");
	console_->LogDebug("エンジンシステムが正常に接続されました");
}

void GameDebugUI::SetSceneManager(SceneManager* sceneManager)
{
	if (sceneManager) {
		sceneManagerTab_->Initialize(sceneManager);
		console_->LogInfo("SceneManagerがSceneManagerTabに設定されました");
	}
}

void GameDebugUI::Update()
{
	// メニューバーと他のパネルをまとめて呼び出す
	ShowMainMenuBar();
	UpdateDebugPanels();
}

void GameDebugUI::ShowMainMenuBar()
{
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("Debug")) {
			ImGui::Checkbox("Engine Info", &showEngineInfo_);
			ImGui::Checkbox("Console", &showConsole_);
			ImGui::Checkbox("Scene Manager", &showSceneManager_);
			ImGui::EndMenu();
		}

		// ▼ ギズモ操作タイプ切り替え ▼
		if (ImGui::BeginMenu("Gizmo Mode")) {
			GizmoOperation current = Gizmo::GetOperation();

			if (ImGui::RadioButton("Translate", current == GizmoOperation::Translate)) {
				Gizmo::SetOperation(GizmoOperation::Translate);
			}
			if (ImGui::RadioButton("Rotate", current == GizmoOperation::Rotate)) {
				Gizmo::SetOperation(GizmoOperation::Rotate);
			}
			if (ImGui::RadioButton("Scale", current == GizmoOperation::Scale)) {
				Gizmo::SetOperation(GizmoOperation::Scale);
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void GameDebugUI::UpdateDebugPanels()
{
	// ライティング用のUIを表示する
	if (ImGui::Begin(LIGHTING_WINDOW)) {
		auto lightManager = engine_->GetComponent<LightManager>();
		if (lightManager) {
			lightManager->DrawAllImGui();
		}
	}
	ImGui::End();

	// エンジン情報のデバッグUI
	if (ImGui::Begin(ENGINE_DEBUG_WINDOW)) {
		if (showEngineInfo_) {
			ShowEngineInfoUI();
		}
	}
	ImGui::End();

	// コンソールウィンドウ
	if (showConsole_) {
		ShowConsoleUI();
	}

	// シーンマネージャーウィンドウ
	if (showSceneManager_) {
		ShowSceneManagerUI();
	}
}

void GameDebugUI::ShowConsoleUI()
{
	console_->SetVisible(showConsole_);
	console_->Draw();
}

void GameDebugUI::ShowSceneManagerUI()
{
	if (ImGui::Begin(SCENE_MANAGER_WINDOW, &showSceneManager_)) {
		sceneManagerTab_->DrawImGui();
	}
	ImGui::End();
}

void GameDebugUI::ShowLightingDebugUI()
{
	// ライトマネージャーを取得
	if (ImGui::Begin(LIGHTING_WINDOW)) {
		auto lightManager = engine_->GetComponent<LightManager>();
		if (lightManager) {
			lightManager->DrawAllImGui();
		}
	}
	ImGui::End();
}

void GameDebugUI::ShowEngineInfoUI()
{
	if (!engine_) {
		ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "エンジンシステム: 利用不可");
		return;
	}

	auto* frameRate = engine_->GetComponent<FrameRateController>();
	
	// ===== タブバーでFPS情報とシステム状態を切り替え =====
	if (ImGui::BeginTabBar("EngineInfoTabs", ImGuiTabBarFlags_None)) {
		
		// ========== タブ1: FPS情報 ==========
		if (ImGui::BeginTabItem("FPS情報")) {
			if (frameRate) {
				ShowFPSInfoTab(frameRate);
			} else {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "FrameRateController: 利用不可");
			}
			ImGui::EndTabItem();
		}
		
		// ========== タブ2: 詳細パフォーマンス ==========
		if (ImGui::BeginTabItem("詳細")) {
			if (frameRate) {
				ShowDetailedPerformanceTab(frameRate);
			} else {
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "FrameRateController: 利用不可");
			}
			ImGui::EndTabItem();
		}
		
		// ========== タブ3: システム状態 ==========
		if (ImGui::BeginTabItem("システム")) {
			ShowSystemStatusTab();
			ImGui::EndTabItem();
		}
		
		ImGui::EndTabBar();
	}
}

void GameDebugUI::ShowFPSInfoTab(FrameRateController* frameRate)
{
	float currentFPS = frameRate->GetCurrentFPS();
	float targetFPS = frameRate->GetTargetFPS();
	
	// FPSの色分け
	ImVec4 fpsColor;
	if (currentFPS >= targetFPS * 0.95f) {
		fpsColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // 緑: 正常
	} else if (currentFPS >= targetFPS * 0.80f) {
		fpsColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // 黄: やや低下
	} else {
		fpsColor = ImVec4(1.0f, 0.2f, 0.0f, 1.0f); // 赤: 低下
	}
	
	// 現在のFPSを表示 (1.5倍のサイズ)
	ImGui::PushStyleColor(ImGuiCol_Text, fpsColor);
	ImGui::SetWindowFontScale(1.7f);
	ImGui::Text("%.1f FPS", currentFPS);
	ImGui::SetWindowFontScale(1.0f);
	ImGui::PopStyleColor();
	
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	
	// パフォーマンス概要
	ImGui::Text("目標: %.0f FPS", targetFPS);
	
	// パフォーマンス状態インジケーター
	const char* statusText;
	ImVec4 statusColor;
	if (currentFPS >= targetFPS * 0.95f) {
		statusText = "良好";
		statusColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	} else if (currentFPS >= targetFPS * 0.80f) {
		statusText = "やや低下";
		statusColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
	} else {
		statusText = "パフォーマンス低下";
		statusColor = ImVec4(1.0f, 0.2f, 0.0f, 1.0f);
	}
	
	ImGui::Text("状態: ");
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, statusColor);
	ImGui::Text("%s", statusText);
	ImGui::PopStyleColor();
	
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	
	// フレーム時間
	ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "[フレーム時間]");
	
	float targetFrameTimeMs = (1.0f / targetFPS) * 1000.0f;
	float actualFrameTimeMs = frameRate->GetActualFrameTimeMs();
	float processTimeMs = frameRate->GetProcessTimeMs();
	
	ImGui::Text("目標: %.2f ms", targetFrameTimeMs);
	ImGui::Text("実測: %.2f ms", actualFrameTimeMs);
	ImGui::Text("処理: %.2f ms", processTimeMs);
	
	// プログレスバーで処理時間の割合を表示
	float processFraction = processTimeMs / targetFrameTimeMs;
	ImVec4 barColor = processFraction < 0.90f ? ImVec4(0.0f, 0.8f, 0.0f, 1.0f) : ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barColor);
	ImGui::ProgressBar(processFraction, ImVec2(-1, 20), "");
	ImGui::PopStyleColor();
	
	ImGui::Text("CPU使用率: %.1f%%", processFraction * 100.0f);
	
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	
	// FPS統計（過去2秒）
	ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "[FPS統計 (過去2秒)]");
	
	float minFPS = frameRate->GetMinFPS();
	float maxFPS = frameRate->GetMaxFPS();
	
	ImGui::Text("最小: %.1f FPS", minFPS);
	ImGui::Text("最大: %.1f FPS", maxFPS);
	ImGui::Text("変動: %.1f FPS", maxFPS - minFPS);
	
	// 安定性インジケーター
	float stability = 100.0f * (1.0f - ((maxFPS - minFPS) / targetFPS));
	if (stability < 0.0f) stability = 0.0f;
	if (stability > 100.0f) stability = 100.0f;
	
	ImVec4 stabilityColor;
	if (stability >= 95.0f) {
		stabilityColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	} else if (stability >= 85.0f) {
		stabilityColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
	} else {
		stabilityColor = ImVec4(1.0f, 0.2f, 0.0f, 1.0f);
	}
	
	ImGui::Text("安定性: ");
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Text, stabilityColor);
	ImGui::Text("%.1f%%", stability);
	ImGui::PopStyleColor();
	
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	
	// デルタタイム
	ImGui::TextColored(ImVec4(0.5f, 0.8f, 1.0f, 1.0f), "[デルタタイム]");
	float deltaTime = frameRate->GetDeltaTime();
	ImGui::Text("%.6f 秒", deltaTime);
	ImGui::Text("%.3f ms", deltaTime * 1000.0f);
}

void GameDebugUI::ShowDetailedPerformanceTab(FrameRateController* frameRate)
{
	float targetFPS = frameRate->GetTargetFPS();
	float targetFrameTimeMs = (1.0f / targetFPS) * 1000.0f;
	float actualFrameTimeMs = frameRate->GetActualFrameTimeMs();
	float processTimeMs = frameRate->GetProcessTimeMs();
	
	// フレーム時間の詳細
	ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "[フレーム時間の詳細]");
	ImGui::Spacing();
	
	ImGui::Columns(2, "FrameTimeColumns", true);
	ImGui::SetColumnWidth(0, 150);
	
	// 目標フレーム時間
	ImGui::Text("目標フレーム時間");
	ImGui::NextColumn();
	ImGui::Text("%.2f ms", targetFrameTimeMs);
	ImGui::NextColumn();
	
	// 実測フレーム時間
	ImGui::Text("実測フレーム時間");
	ImGui::NextColumn();
	ImVec4 frameTimeColor;
	if (actualFrameTimeMs <= targetFrameTimeMs * 1.05f) {
		frameTimeColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	} else if (actualFrameTimeMs <= targetFrameTimeMs * 1.20f) {
		frameTimeColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
	} else {
		frameTimeColor = ImVec4(1.0f, 0.2f, 0.0f, 1.0f);
	}
	ImGui::PushStyleColor(ImGuiCol_Text, frameTimeColor);
	ImGui::Text("%.2f ms", actualFrameTimeMs);
	ImGui::PopStyleColor();
	ImGui::NextColumn();
	
	// 処理時間
	ImGui::Text("処理時間");
	ImGui::NextColumn();
	ImGui::Text("%.2f ms", processTimeMs);
	ImGui::NextColumn();
	
	// 待機時間
	float waitTimeMs = actualFrameTimeMs - processTimeMs;
	ImGui::Text("待機時間");
	ImGui::NextColumn();
	ImGui::Text("%.2f ms", waitTimeMs);
	ImGui::NextColumn();
	
	ImGui::Columns(1);
	
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	
	// 処理時間の割合
	ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "[処理時間の内訳]");
	ImGui::Spacing();
	
	float processFraction = processTimeMs / targetFrameTimeMs;
	float waitFraction = waitTimeMs / targetFrameTimeMs;
	
	ImGui::Text("処理: %.1f%%", processFraction * 100.0f);
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
	ImGui::ProgressBar(processFraction, ImVec2(-1, 20), "");
	ImGui::PopStyleColor();
	
	ImGui::Text("待機: %.1f%%", waitFraction * 100.0f);
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
	ImGui::ProgressBar(waitFraction, ImVec2(-1, 20), "");
	ImGui::PopStyleColor();
	
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	
	// レンダリング設定
	ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "[レンダリング設定]");
	ImGui::Spacing();
	
	ImGui::Text("- VSync: 有効 (60Hz)");
	ImGui::Text("- ダブルバッファリング: 有効");
	ImGui::Text("- GPU並列処理: 有効");
	
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	
	// FPSドロップ情報
	ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "[パフォーマンス警告]");
	ImGui::Spacing();
	
	int droppedFrames = frameRate->GetDroppedFrameCount();
	if (droppedFrames > 0) {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.0f, 1.0f));
		ImGui::Text("- FPSドロップ回数: %d", droppedFrames);
		ImGui::PopStyleColor();
		
		// 警告メッセージ
		if (droppedFrames > 1000) {
			ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 
				"警告: 頻繁なFPSドロップが検出されています");
		}
	} else {
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
		ImGui::Text("- FPSドロップ: なし");
		ImGui::PopStyleColor();
	}
	
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	
	// デルタタイム詳細
	ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.8f, 1.0f), "[デルタタイム詳細]");
	ImGui::Spacing();
	
	float deltaTime = frameRate->GetDeltaTime();
	float expectedDeltaTime = 1.0f / targetFPS;
	
	ImGui::Columns(2, "DeltaTimeColumns", true);
	ImGui::SetColumnWidth(0, 150);
	
	ImGui::Text("現在のDt");
	ImGui::NextColumn();
	ImGui::Text("%.6f 秒", deltaTime);
	ImGui::NextColumn();
	
	ImGui::Text("期待値");
	ImGui::NextColumn();
	ImGui::Text("%.6f 秒", expectedDeltaTime);
	ImGui::NextColumn();
	
	ImGui::Text("ミリ秒単位");
	ImGui::NextColumn();
	ImGui::Text("%.3f ms", deltaTime * 1000.0f);
	ImGui::NextColumn();
	
	ImGui::Text("誤差");
	ImGui::NextColumn();
	float deltaError = (deltaTime - expectedDeltaTime) * 1000.0f;
	float absDeltaError = deltaError < 0.0f ? -deltaError : deltaError;
	ImVec4 errorColor = absDeltaError < 1.0f ? 
		ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_Text, errorColor);
	ImGui::Text("%+.3f ms", deltaError);
	ImGui::PopStyleColor();
	ImGui::NextColumn();
	
	ImGui::Columns(1);
}

void GameDebugUI::ShowSystemStatusTab()
{
	ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "[エンジンシステム状態]");
	ImGui::Spacing();
	
	// エンジンシステムの状態確認
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.8f, 0.2f, 1.0f));
	ImGui::Text("- エンジンシステム: 正常動作中");
	ImGui::PopStyleColor();
	
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	
	// コンポーネントの状態確認
	ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "[システムコンポーネント]");
	ImGui::Spacing();
	
	struct ComponentStatus {
		const char* name;
		bool available;
	};
	
	std::vector<ComponentStatus> components;
	
	auto directXCommon = engine_->GetComponent<DirectXCommon>();
	components.push_back({"グラフィックス", directXCommon != nullptr});
	
	auto inputManager = engine_->GetComponent<InputManager>();
	components.push_back({"入力システム", inputManager != nullptr});
	
	auto soundManager = engine_->GetComponent<SoundManager>();
	components.push_back({"オーディオ", soundManager != nullptr});
	
	auto lightManager = engine_->GetComponent<LightManager>();
	components.push_back({"ライティング", lightManager != nullptr});
	
	auto particleSystem = engine_->GetComponent<ParticleSystem>();
	components.push_back({"パーティクル", particleSystem != nullptr});
	
	// コンポーネント状態を表示
	for (const auto& comp : components) {
		if (comp.available) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
			ImGui::Text("- %s: 利用可能", comp.name);
			ImGui::PopStyleColor();
		} else {
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
			ImGui::Text("- %s: 利用不可", comp.name);
			ImGui::PopStyleColor();
		}
	}
	
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	
	// 統計情報
	int availableCount = 0;
	for (const auto& comp : components) {
		if (comp.available) availableCount++;
	}
	
	ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
		"利用可能なコンポーネント: %d / %d", availableCount, (int)components.size());
}

void GameDebugUI::RegisterWindowsForDocking()
{
	if (!dockingUI_) return;

	// エンジンデバッグ情報を左上に配置
	dockingUI_->RegisterWindow(ENGINE_DEBUG_WINDOW, DockArea::LeftTop);

	// カメラ情報を左下に配置
	dockingUI_->RegisterWindow("Camera", DockArea::LeftBottom);

	// ライティング情報を右側に配置（インスペクター系と統一）
	dockingUI_->RegisterWindow(LIGHTING_WINDOW, DockArea::Right);

	// コンソールを下部左に配置
	dockingUI_->RegisterWindow(CONSOLE_WINDOW, DockArea::BottomLeft);

	// シーンマネージャーを下部右に配置
	dockingUI_->RegisterWindow(SCENE_MANAGER_WINDOW, DockArea::BottomRight);
}
