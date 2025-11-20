#include <EngineSystem.h>


#include "TestScene.h"
#include "WinApp/WinApp.h"
#include "Scene/SceneManager.h"
#include "Engine/Graphics/Model/Skeleton/SkeletonDebugRenderer.h"

#include <iostream>

using namespace MathCore;
using namespace CollisionUtils;

// アプリケーションの初期化
void TestScene::Initialize(EngineSystem* engine)
{
   BaseScene::Initialize(engine);


   // ===== コンソールにシーン初期化ログを出力 =====
#ifdef _DEBUG
   auto console = engine_->GetConsole();
   if (console) {
	  console->LogInfo("TestScene: 初期化を開始しました");
	  console->LogInfo("TestScene: ComponentManager のテストが成功しました");
   }
#endif

   ///========================================================
   // モデルの読み込みと初期化
   ///========================================================

   // コンポーネントを直接取得
   auto dxCommon = engine_->GetComponent<DirectXCommon>();
   auto modelManager = engine_->GetComponent<ModelManager>();

   if (!dxCommon || !modelManager) {
	  return; // 必須コンポーネントがない場合は終了
   }

   // ===== 3Dゲームオブジェクトの生成と初期化=====
   // Sphereオブジェクト
   auto sphere = std::make_unique<SphereObject>();
   sphere->Initialize(engine_);
   sphere->SetActive(false);  // Skeletonテスト中は非表示
   gameObjects_.push_back(std::move(sphere));

   // Fenceオブジェクト
   auto fence = std::make_unique<FenceObject>();
   fence->Initialize(engine_);
   fence->SetActive(false);  // Skeletonテスト中は非表示
   gameObjects_.push_back(std::move(fence));

   // Terrainオブジェクト
   auto terrain = std::make_unique<TerrainObject>();
   terrain->Initialize(engine_);
   terrain->SetActive(false);  // Skeletonテスト中は非表示
   gameObjects_.push_back(std::move(terrain));

   // AnimatedCubeオブジェクト
   auto animatedCube = std::make_unique<AnimatedCubeObject>();
   animatedCube->Initialize(engine_);
   animatedCube->SetActive(false);  // Skeletonテスト中は非表示
   gameObjects_.push_back(std::move(animatedCube));

   // SkeletonModelオブジェクト
   auto skeletonModel = std::make_unique<SkeletonModelObject>();
   skeletonModel->Initialize(engine_);
   skeletonModel->SetActive(true);  // デフォルトは非表示
   gameObjects_.push_back(std::move(skeletonModel));

   // WalkModelオブジェクト
   auto walkModel = std::make_unique<WalkModelObject>();
   walkModel->Initialize(engine_);
   walkModel->SetActive(true);
   gameObjects_.push_back(std::move(walkModel));

   // SneakWalkModelオブジェクト
   auto sneakWalkModel = std::make_unique<SneakWalkModelObject>();
   sneakWalkModel->Initialize(engine_);
   sneakWalkModel->SetActive(true);
   gameObjects_.push_back(std::move(sneakWalkModel));

   // SkyBoxの初期化（gameObjects_に追加）
   auto skyBox = std::make_unique<SkyBoxObject>();
   skyBox->Initialize(engine_);
   gameObjects_.push_back(std::move(skyBox));

   // スプライトオブジェクトの初期化（複数作成）
   // スプライト1: uvChecker
   auto sprite1 = std::make_unique<SpriteObject>();
   sprite1->Initialize(engine_, "Resources/SampleResources/uvChecker.png");
   sprite1->GetSprite()->SetPosition({ 100.0f, 100.0f, 0.0f });
   sprite1->GetSprite()->SetScale({ 0.5f, 0.5f, 1.0f });
   spriteObjects_.push_back(std::move(sprite1));

   // スプライト2: circle
   auto sprite2 = std::make_unique<SpriteObject>();
   sprite2->Initialize(engine_, "Resources/SampleResources/circle.png");
   sprite2->GetSprite()->SetPosition({ 400.0f, 200.0f, 0.0f });
   sprite2->GetSprite()->SetScale({ 1.0f, 1.0f, 1.0f });
   spriteObjects_.push_back(std::move(sprite2));

   // スプライト3: 別のuvChecker（異なる位置）
   auto sprite3 = std::make_unique<SpriteObject>();
   sprite3->Initialize(engine_, "Resources/SampleResources/uvChecker.png");
   sprite3->GetSprite()->SetPosition({ 700.0f, 400.0f, 0.0f });
   sprite3->GetSprite()->SetScale({ 0.8f, 0.8f, 1.0f });
   spriteObjects_.push_back(std::move(sprite3));

   // ===== パーティクルシステムの初期化 =====
   particleSystem_ = std::make_unique<ParticleSystem>();
   particleSystem_->Initialize(dxCommon, engine_->GetComponent<ResourceFactory>());

   // パーティクルシステムの設定
   particleSystem_->SetTexture("Resources/SampleResources/circle.png");
   particleSystem_->SetEmitterPosition({ 0.0f, 2.0f, 0.0f });
   particleSystem_->SetBlendMode(BlendMode::kBlendModeAdd);  // 加算合成
   particleSystem_->SetBillboardType(BillboardType::ViewFacing);

   // エミッションモジュールの設定
   {
	  auto& emissionModule = particleSystem_->GetEmissionModule();
	  auto emissionData = emissionModule.GetEmissionData();
	  emissionData.rateOverTime = 20;  // 1秒に20個のパーティクルを放出
	  emissionData.shapeType = EmissionModule::ShapeType::Sphere;
	  emissionData.radius = 0.5f;
	  emissionData.emitFromSurface = false;
	  emissionModule.SetEmissionData(emissionData);
   }

   // 速度モジュールの設定
   {
	  auto& velocityModule = particleSystem_->GetVelocityModule();
	  auto velocityData = velocityModule.GetVelocityData();
	  velocityData.startSpeed = { 0.0f, 1.0f, 0.0f };
	  velocityData.randomSpeedRange = { 1.0f, 1.0f, 1.0f };
	  velocityData.useRandomDirection = true;
	  velocityModule.SetVelocityData(velocityData);
   }

   // 色モジュールの設定
   {
	  auto& colorModule = particleSystem_->GetColorModule();
	  auto colorData = colorModule.GetColorData();
	  colorData.useGradient = true;
	  colorData.startColor = { 1.0f, 0.8f, 0.2f, 1.0f };  // 黄色
	  colorData.endColor = { 1.0f, 0.2f, 0.0f, 0.0f };    // 赤でフェードアウト
	  colorModule.SetColorData(colorData);
   }

   // ライフタイムモジュールの設定
   {
	  auto& lifetimeModule = particleSystem_->GetLifetimeModule();
	  auto lifetimeData = lifetimeModule.GetLifetimeData();
	  lifetimeData.startLifetime = 2.0f;
	  lifetimeData.lifetimeRandomness = 0.25f;
	  lifetimeModule.SetLifetimeData(lifetimeData);
   }

   // サイズモジュールの設定
   {
	  auto& sizeModule = particleSystem_->GetSizeModule();
	  auto sizeData = sizeModule.GetSizeData();
	  sizeData.startSize = 0.3f;
	  sizeData.endSize = 0.05f;
	  sizeData.sizeOverLifetime = true;
	  sizeModule.SetSizeData(sizeData);
   }

   // パーティクルシステムを再生開始
   particleSystem_->Play();

#ifdef _DEBUG
   if (console) {
	  console->LogInfo("TestScene: 全てのゲームオブジェクトを初期化しました");
   }
#endif

   // テクスチャの読み込み
   auto& textureManager = TextureManager::GetInstance();
   textureChecker_ = textureManager.Load("Resources/SampleResources/uvChecker.png");
   textureCircle_ = textureManager.Load("Resources/SampleResources/circle.png");

   // サウンドリソースを取得
   auto soundManager = engine_->GetComponent<SoundManager>();
   if (soundManager) {
	  mp3Resource_ = soundManager->CreateSoundResource("Resources/Audio/BGM/test.mp3");
   }

}

void TestScene::Update()
{
   BaseScene::Update();

   // KeyboardInput を直接取得
   auto keyboard = engine_->GetComponent<KeyboardInput>();
   if (!keyboard) {
	  return; // キーボードは必須
   }

   // Tabキーでテストシーンをリスタート
   if (keyboard && keyboard->IsKeyTriggered(DIK_TAB)) {

	  if (sceneManager_) {
		 sceneManager_->ChangeScene("TestScene");

#ifdef _DEBUG
		 auto console = engine_->GetConsole();
		 if (console) {
			console->LogInfo("TestScene: シーンを再起動しました");
		 }
#endif
	  }
	  return;
   }


   // ===== スペースキーでMP3再生　====
   if (keyboard && keyboard->IsKeyTriggered(DIK_SPACE)) {
	  if (mp3Resource_ && mp3Resource_->IsValid()) {
		 // 現在再生中かチェック
		 bool isPlaying = mp3Resource_->IsPlaying();
		 if (isPlaying) {
			// 再生中の場合は停止
			mp3Resource_->Stop();

			// ===== コンソールにサウンド停止ログを出力 =====
#ifdef _DEBUG
			auto console = engine_->GetConsole();
			if (console) {
			   console->LogDebug("オーディオ: MP3サウンドを停止しました");
			}
#endif
		 } else {
			// 停止中の場合は再生開始（ワンショット）
			mp3Resource_->Play(false);

			// ===== コンソールにサウンド再生ログを出力 =====
#ifdef _DEBUG
			auto console = engine_->GetConsole();
			if (console) {
			   console->LogDebug("オーディオ: MP3サウンドの再生を開始しました");
			}
#endif
		 }
	  }
   }

   ICamera* activeCamera = cameraManager_->GetActiveCamera();

#ifdef _DEBUG
   // F1キーでデバッグカメラに切り替え
   if (keyboard->IsKeyTriggered(DIK_F1)) {
	  cameraManager_->SetActiveCamera("Debug");
	  auto console = engine_->GetConsole();
	  if (console) {
		 console->LogInfo("TestScene: デバッグカメラに切り替えました");
	  }
   }

   // F2キーでリリースカメラに切り替え
   if (keyboard->IsKeyTriggered(DIK_F2)) {
	  cameraManager_->SetActiveCamera("Release");
	  auto console = engine_->GetConsole();
	  if (console) {
		 console->LogInfo("TestScene: リリースカメラに切り替えました");
	  }
   }

   //if (ImGui::Begin("クォータニオンデバッグ表示")) {

   //	// クォータニオンの回転を行列に変換して表示
   //	Vector3 axis = MathCore::Vector::Normalize({ 1.0f,1.0f,1.0f });
   //	float angle = 0.44f;
   //	Matrix4x4 rotateMatrix = Matrix::MakeRotateAxisAngle(axis, angle);
   //	ImGui::Text("rotateMatrix");
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix.m[0][0], rotateMatrix.m[0][1], rotateMatrix.m[0][2], rotateMatrix.m[0][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix.m[1][0], rotateMatrix.m[1][1], rotateMatrix.m[1][2], rotateMatrix.m[1][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix.m[2][0], rotateMatrix.m[2][1], rotateMatrix.m[2][2], rotateMatrix.m[2][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix.m[3][0], rotateMatrix.m[3][1], rotateMatrix.m[3][2], rotateMatrix.m[3][3]);

   //	//ある方向からある方向への回転を行列に変換して表示
   //	Vector3 from0 = Vector::Normalize({ 1.0f,0.7f,0.5f });
   //	Vector3 to0 = -from0;
   //	Vector3 from1 = Vector::Normalize({ -0.6f,0.9f, 0.2f });
   //	Vector3 to1 = Vector::Normalize({ 0.4f,0.7f,-0.5f });

   //	Matrix4x4 rotateMatrix0 = Matrix::DirectionToDirection(Vector::Normalize({ 1.0f,0.0f,0.0f }),
   //		Vector::Normalize({ -1.0f, 0.0f, 0.0f }));
   //	Matrix4x4 rotateMatrix1 = Matrix::DirectionToDirection(from0, to0);
   //	Matrix4x4 rotateMatrix2 = Matrix::DirectionToDirection(from1, to1);

   //	ImGui::Text("DirectionToDirection");
   //	ImGui::Text("rotateMatrix0");
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix0.m[0][0], rotateMatrix0.m[0][1], rotateMatrix0.m[0][2], rotateMatrix0.m[0][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix0.m[1][0], rotateMatrix0.m[1][1], rotateMatrix0.m[1][2], rotateMatrix0.m[1][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix0.m[2][0], rotateMatrix0.m[2][1], rotateMatrix0.m[2][2], rotateMatrix0.m[2][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix0.m[3][0], rotateMatrix0.m[3][1], rotateMatrix0.m[3][2], rotateMatrix0.m[3][3]);
   //	ImGui::Text("rotateMatrix1");
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix1.m[0][0], rotateMatrix1.m[0][1], rotateMatrix1.m[0][2], rotateMatrix1.m[0][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix1.m[1][0], rotateMatrix1.m[1][1], rotateMatrix1.m[1][2], rotateMatrix1.m[1][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix1.m[2][0], rotateMatrix1.m[2][1], rotateMatrix1.m[2][2], rotateMatrix1.m[2][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix1.m[3][0], rotateMatrix1.m[3][1], rotateMatrix1.m[3][2], rotateMatrix1.m[3][3]);
   //	ImGui::Text("rotateMatrix2");
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix2.m[0][0], rotateMatrix2.m[0][1], rotateMatrix2.m[0][2], rotateMatrix2.m[0][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix2.m[1][0], rotateMatrix2.m[1][1], rotateMatrix2.m[1][2], rotateMatrix2.m[1][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix2.m[2][0], rotateMatrix2.m[2][1], rotateMatrix2.m[2][2], rotateMatrix2.m[2][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix2.m[3][0], rotateMatrix2.m[3][1], rotateMatrix2.m[3][2], rotateMatrix2.m[3][3]);
   //
   //	//クォータニオンの計算
   //	Quaternion q1 = { 2.0f, 3.0f, 4.0f, 1.0f };
   //	Quaternion q2 = { 1.0f, 3.0f, 5.0f, 2.0f };
   //	Quaternion identity = QuaternionMath::Identity();
   //	Quaternion conj = QuaternionMath::Conjugate(q1);
   //	Quaternion inv = QuaternionMath::Inverse(q1);
   //	Quaternion normal = QuaternionMath::Normalize(q1);
   //	Quaternion mult1 = QuaternionMath::Multiply(q1, q2);
   //	Quaternion mult2 = QuaternionMath::Multiply(q2, q1);
   //	float norm = QuaternionMath::Norm(q1);

   //	ImGui::Text("Quaternion");
   //	ImGui::Text("Identity");
   //	ImGui::Text("x:%.2f y:%.2f z:%.2f w:%.2f", identity.x, identity.y, identity.z, identity.w);
   //	ImGui::Text("Conjugate");
   //	ImGui::Text("x:%.2f y:%.2f z:%.2f w:%.2f", conj.x, conj.y, conj.z, conj.w);
   //	ImGui::Text("Inverse");
   //	ImGui::Text("x:%.2f y:%.2f z:%.2f w:%.2f", inv.x, inv.y, inv.z, inv.w);
   //	ImGui::Text("Normalize");
   //	ImGui::Text("x:%.2f y:%.2f z:%.2f w:%.2f", normal.x, normal.y, normal.z, normal.w);
   //	ImGui::Text("Multiply(q1,q2)");
   //	ImGui::Text("x:%.2f y:%.2f z:%.2f w:%.2f", mult1.x, mult1.y, mult1.z, mult1.w);
   //	ImGui::Text("Multiply(q2,q1)");
   //	ImGui::Text("x:%.2f y:%.2f z:%.2f w:%.2f", mult2.x, mult2.y, mult2.w);
   //	ImGui::Text("Norm");
   //	ImGui::Text("%.2f", norm);

   //	//クォータニオンによるベクトルの回転
   //	Quaternion rotation = QuaternionMath::MakeRotateAxisAngle(Vector::Normalize({ 1.0f, 0.4f, -0.2f }),
   //		0.45f);

   //	Vector3 pointY = { 2.1f, -0.9f, 1.3f };
   //	Matrix4x4 rotateMatrix = QuaternionMath::MakeRotateMatrix(rotation);
   //	Vector3 rotateByQuaternion = QuaternionMath::RotateVector(pointY, rotation);
   //	Vector3 rotateByMatrix = CoordinateTransform::TransformCoord(pointY, rotateMatrix);

   //	ImGui::Text("Quaternion Rotate Vector");
   //	ImGui::Text("rotation");
   //	ImGui::Text("x:%.2f y:%.2f z:%.2f w:%.2f", rotation.x, rotation.y, rotation.z, rotation.w);
   //	ImGui::Text("rotateMatrix");
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix.m[0][0], rotateMatrix.m[0][1], rotateMatrix.m[0][2], rotateMatrix.m[0][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix.m[1][0], rotateMatrix.m[1][1], rotateMatrix.m[1][2], rotateMatrix.m[1][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix.m[2][0], rotateMatrix.m[2][1], rotateMatrix.m[2][2], rotateMatrix.m[2][3]);
   //	ImGui::Text("%.3f %.3f %.3f %.3f", rotateMatrix.m[3][0], rotateMatrix.m[3][1], rotateMatrix.m[3][2], rotateMatrix.m[3][3]);
   //	ImGui::Text("rotateByQuaternion");
   //	ImGui::Text("x:%.2f y:%.2f z:%.2f", rotateByQuaternion.x, rotateByQuaternion.y, rotateByQuaternion.z);
   //	ImGui::Text("rotateByMatrix");
   //	ImGui::Text("x:%.2f y:%.2f z:%.2f", rotateByMatrix.x, rotateByMatrix.y, rotateByMatrix.z);


   //	//球面線形補間
   //	Quaternion rotation0 = QuaternionMath::MakeRotateAxisAngle({ 0.71f, 0.71f, 0.0f }, 0.3f);
   //	Quaternion rotation1 = QuaternionMath::MakeRotateAxisAngle({ 0.71f, 0.0f, 0.71f }, std::numbers::pi_v<float>);

   //	Quaternion interpolated0 = QuaternionMath::Slerp(rotation0, rotation1, 0.0f);
   //	Quaternion interpolated1 = QuaternionMath::Slerp(rotation0, rotation1, 0.3f);
   //	Quaternion interpolated2 = QuaternionMath::Slerp(rotation0, rotation1, 0.5f);
   //	Quaternion interpolated3 = QuaternionMath::Slerp(rotation0, rotation1, 0.7f);
   //	Quaternion interpolated4 = QuaternionMath::Slerp(rotation0, rotation1, 1.0f);

   //	ImGui::Text("Quaternion Slerp");
   //	ImGui::Text("interpolated0 (t=0.0)");
   //	ImGui::Text("x:%.2f y:%.2f z:%.2f w:%.2f", interpolated0.x, interpolated0.y, interpolated0.z, interpolated0.w);
   //	ImGui::Text("interpolated1 (t=0.3)");
   //	ImGui::Text("x:%.2f y:%.2f z:%.2f w:%.2f", interpolated1.x, interpolated1.y, interpolated1.z, interpolated1.w);
   //	ImGui::Text("interpolated2 (t=0.5)");
   //	ImGui::Text("x:%.2f y:%.2f z:%.2f w:%.2f", interpolated2.x, interpolated2.y, interpolated2.z, interpolated2.w);
   //	ImGui::Text("interpolated3 (t=0.7)");
   //	ImGui::Text("x:%.2f y:%.2f z:%.2f w:%.2f", interpolated3.x, interpolated3.y, interpolated3.z, interpolated3.w);
   //	ImGui::Text("interpolated4 (t=1.0)");
   //	ImGui::Text("x:%.2f y:%.2f z:%.2f w:%.2f", interpolated4.x, interpolated4.y, interpolated4.z, interpolated4.w);


   //}
   //ImGui::End();


   // カメラマネージャーのImGui（統一されたウィンドウ）
   cameraManager_->DrawImGui();

   if (ImGui::Begin("オブジェクト制御")) {

	  // ===== 全オブジェクトのImGuiデバッグUI =====
	  for (auto& obj : gameObjects_) {
		 if (obj && obj->IsActive()) {
			obj->DrawImGui();
		 }
	  }

	  // スプライトオブジェクトのImGuiデバッグUI（全て表示）
	  for (auto& spriteObj : spriteObjects_) {
		 if (spriteObj) {
			spriteObj->DrawImGui();
		 }
	  }

	  ImGui::Separator();

   }
   ImGui::End();

#endif // _DEBUG

   // ===== 3Dゲームオブジェクトの更新=====
   for (auto& obj : gameObjects_) {
	  if (obj && obj->IsActive()) {
		 obj->Update();
	  }
   }

   // ライトマネージャーの更新
   auto lightManager = engine_->GetComponent<LightManager>();
   if (lightManager) {
	  lightManager->UpdateAll();
   }

   // パーティクルシステムの更新
   if (particleSystem_ && activeCamera) {
	  particleSystem_->Update(activeCamera);
   }


}

void TestScene::Draw()
{
   // コンポーネント取得
   auto renderManager = engine_->GetComponent<RenderManager>();
   auto dxCommon = engine_->GetComponent<DirectXCommon>();
   auto lineRenderer = engine_->GetComponent<LineRenderer>();
   ICamera* activeCamera = cameraManager_->GetActiveCamera();

   // 必須コンポーネントのチェック
   if (!renderManager || !dxCommon || !activeCamera) {
	  return;
   }

   ID3D12GraphicsCommandList* cmdList = dxCommon->GetCommandList();

   // ===== RenderManagerによる統一描画システム =====
   // フレーム開始時に描画コンテキストを設定（1回のみ）
   renderManager->SetCamera(activeCamera);
   renderManager->SetCommandList(cmdList);

   // 全てのゲームオブジェクトを描画キューに追加
   for (const auto& obj : gameObjects_) {
	  if (obj && obj->IsActive()) {
		 renderManager->AddDrawable(obj.get());
	  }
   }

   // パーティクルシステムを描画キューに追加（統一された方法）
   if (particleSystem_ && particleSystem_->IsActive()) {
	  renderManager->AddDrawable(particleSystem_.get());
   }

   // 全てのスプライトオブジェクトを描画キューに追加
   for (const auto& spriteObj : spriteObjects_) {
	  if (spriteObj && spriteObj->IsActive()) {
		 renderManager->AddDrawable(spriteObj.get());
	  }
   }

   // 一括描画（自動的にパスごとにソート・グループ化）
   renderManager->DrawAll();

   // フレーム終了時にキューをクリア
   renderManager->ClearQueue();

   // ===== デバッグ描画（スケルトンなど）=====
   if (lineRenderer) {
	  std::vector<LineRenderer::Line> debugLines;

	  for (const auto& obj : gameObjects_) {
		 if (obj && obj->IsActive()) {
			obj->DrawDebug(debugLines);
		 }
	  }

	  if (!debugLines.empty()) {
		 lineRenderer->Draw(
			cmdList,
			activeCamera->GetViewMatrix(),
			activeCamera->GetProjectionMatrix(),
			debugLines
		 );
	  }
   }
}


void TestScene::Finalize()
{
   // このシーン専用のライトを削除
   directionalLight_ = nullptr;
}