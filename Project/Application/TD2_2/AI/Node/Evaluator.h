#pragma once
#include <functional>
#include <memory>
#include <vector>
#include <numeric>
#include <MathCore.h>
#include <algorithm>

struct EvalModifier {
   float scale = 1.0f;
   float bias = 0.0f;
   bool clamp01 = true;
};

class IEvaluator {
public:
   virtual ~IEvaluator() = default;
   virtual float Evaluate() const = 0;
};

class LambdaEvaluator : public IEvaluator {
public:
   explicit LambdaEvaluator(std::function<float()> func);

   float Evaluate() const override;

private:
   std::function<float()> func_;
};

// 距離に基づく評価
class DistanceEvaluator : public IEvaluator {
public:
   // closeValue: 近い時の値, farValue: 遠い時の値
   // minDistance: 最小距離（この距離以下では closeValue を返す）
   // maxDistance: 最大距離（この距離以上では farValue を返す）
   DistanceEvaluator(float closeValue, float farValue,
      std::function<float()> getDistance,
      float minDistance = 0.0f, float maxDistance = 10.0f);

   // Vector3の座標を使用する便利なコンストラクタ
   // pos1とpos2のポインタを受け取り、動的に距離を計算する
   DistanceEvaluator(float closeValue, float farValue,
      const Vector3* pos1, const Vector3* pos2,
      float minDistance = 0.0f, float maxDistance = 10.0f);

   float Evaluate() const override;

private:
   float closeValue_;    // 近い時の値
   float farValue_;      // 遠い時の値
   std::function<float()> getDistance_;  // 距離を取得する関数
   float minDistance_;   // 最小距離
   float maxDistance_;   // 最大距離
};

// HP割合に基づく評価
class HpRatioEvaluator : public IEvaluator {
public:
   // lowValue: HP割合が低い時の値, highValue: HP割合が高い時の値
   // minRatio: 最小割合（この割合以下では lowValue を返す）
   // maxRatio: 最大割合（この割合以上では highValue を返す）
   HpRatioEvaluator(float lowValue, float highValue,
      std::function<float()> getHpRatio,
      float minRatio = 0.0f, float maxRatio = 1.0f);

   float Evaluate() const override;

private:
   float lowValue_;     // HP割合が低い時の値
   float highValue_;    // HP割合が高い時の値
   std::function<float()> getHpRatio_;  // HP割合を取得する関数
   float minRatio_;     // 最小割合
   float maxRatio_;     // 最大割合
};

// 時間経過に基づく評価
class TimeBasedEvaluator : public IEvaluator {
public:
   TimeBasedEvaluator(float startValue, float endValue,
      std::function<float()> getElapsedTime,
      float duration);

   float Evaluate() const override;

private:
   float startValue_;
   float endValue_;
   std::function<float()> getElapsedTime_;
   float duration_;
};

// 角度に基づく評価（視界内判定など）
class AngleEvaluator : public IEvaluator {
public:
   // inRangeValue: 角度範囲内の値, outRangeValue: 範囲外の値
   AngleEvaluator(float inRangeValue, float outRangeValue,
      std::function<float()> getAngle,
      float minAngle = -45.0f, float maxAngle = 45.0f);

   float Evaluate() const override;

private:
   float inRangeValue_;
   float outRangeValue_;
   std::function<float()> getAngle_;
   float minAngle_;
   float maxAngle_;
};

// カウンターに基づく評価（攻撃回数、連続ヒット数など）
class CounterEvaluator : public IEvaluator {
public:
   CounterEvaluator(float minValue, float maxValue,
      std::function<int()> getCounter,
      int minCount = 0, int maxCount = 10);

   float Evaluate() const override;

private:
   float minValue_;
   float maxValue_;
   std::function<int()> getCounter_;
   int minCount_;
   int maxCount_;
};

// 確率的評価（ランダム性を持つ）
class RandomEvaluator : public IEvaluator {
public:
   RandomEvaluator(float minValue, float maxValue);

   float Evaluate() const override;

private:
   float minValue_;
   float maxValue_;
};

// カーブベース評価（イージング関数を使用）
class CurveEvaluator : public IEvaluator {
public:
   enum class CurveType {
      Linear,
      EaseIn,
      EaseOut,
      EaseInOut
   };

   CurveEvaluator(float startValue, float endValue,
      std::function<float()> getProgress,
      CurveType curveType = CurveType::Linear);

   float Evaluate() const override;

private:
   float startValue_;
   float endValue_;
   std::function<float()> getProgress_;
   CurveType curveType_;
   
   float ApplyCurve(float t) const;
};

// 複数の評価を合成する
class CompositeEvaluator : public IEvaluator {
public:
   enum class CombineMode { Sum, Product, Average, WeightedSum, Max, Min };

   explicit CompositeEvaluator(CombineMode mode = CombineMode::Sum);

   void AddEvaluator(std::unique_ptr<IEvaluator> eval,
      float weight = 1.0f,
      EvalModifier mod = {});

   float Evaluate() const override;

private:
   struct Entry {
      std::unique_ptr<IEvaluator> eval;
      float weight;
      EvalModifier mod;
   };
   CombineMode mode_;
   std::vector<Entry> evaluators_;
};

// ======================================================================
// ヘルパー関数群
// ======================================================================

// 距離に基づくEvaluatorを作成するヘルパー関数
inline std::unique_ptr<DistanceEvaluator> MakeDistanceEvaluator(
   float closeValue, float farValue,
   const Vector3* pos1, const Vector3* pos2,
   float minDistance = 0.0f, float maxDistance = 10.0f) {

   return std::make_unique<DistanceEvaluator>(
      closeValue, farValue, pos1, pos2, minDistance, maxDistance);
}

// HP割合に基づくEvaluatorを作成するヘルパー関数
inline std::unique_ptr<HpRatioEvaluator> MakeHpRatioEvaluator(
   float lowValue, float highValue,
   std::function<float()> getHpRatio,
   float minRatio = 0.0f, float maxRatio = 1.0f) {

   return std::make_unique<HpRatioEvaluator>(
      lowValue, highValue, std::move(getHpRatio), minRatio, maxRatio);
}

// 複合エバリュエーターを作成するヘルパー関数
inline std::unique_ptr<CompositeEvaluator> MakeCompositeEvaluator(
   CompositeEvaluator::CombineMode mode = CompositeEvaluator::CombineMode::Product) {
   return std::make_unique<CompositeEvaluator>(mode);
}

// 時間ベースEvaluatorを作成
inline std::unique_ptr<TimeBasedEvaluator> MakeTimeBasedEvaluator(
   float startValue, float endValue,
   std::function<float()> getElapsedTime,
   float duration) {
   return std::make_unique<TimeBasedEvaluator>(startValue, endValue, std::move(getElapsedTime), duration);
}

// 角度ベースEvaluatorを作成
inline std::unique_ptr<AngleEvaluator> MakeAngleEvaluator(
   float inRangeValue, float outRangeValue,
   std::function<float()> getAngle,
   float minAngle = -45.0f, float maxAngle = 45.0f) {
   return std::make_unique<AngleEvaluator>(inRangeValue, outRangeValue, std::move(getAngle), minAngle, maxAngle);
}

// カウンターベースEvaluatorを作成
inline std::unique_ptr<CounterEvaluator> MakeCounterEvaluator(
   float minValue, float maxValue,
   std::function<int()> getCounter,
   int minCount = 0, int maxCount = 10) {
   return std::make_unique<CounterEvaluator>(minValue, maxValue, std::move(getCounter), minCount, maxCount);
}

// ランダムEvaluatorを作成
inline std::unique_ptr<RandomEvaluator> MakeRandomEvaluator(
   float minValue = 0.0f, float maxValue = 1.0f) {
   return std::make_unique<RandomEvaluator>(minValue, maxValue);
}

// カーブベースEvaluatorを作成
inline std::unique_ptr<CurveEvaluator> MakeCurveEvaluator(
   float startValue, float endValue,
   std::function<float()> getProgress,
   CurveEvaluator::CurveType curveType = CurveEvaluator::CurveType::Linear) {
   return std::make_unique<CurveEvaluator>(startValue, endValue, std::move(getProgress), curveType);
}

// ======================================================================
// 複合Evaluatorヘルパー関数
// ======================================================================

// 距離とHP割合を組み合わせた複合エバリュエーター（近接攻撃判定用）
inline std::unique_ptr<CompositeEvaluator> MakeDistanceHpCompositeEvaluator(
   const Vector3* pos1, const Vector3* pos2,
   std::function<float()> getHpRatio,
   float closeDistance = 5.0f,
   float farDistance = 15.0f,
   CompositeEvaluator::CombineMode mode = CompositeEvaluator::CombineMode::Product) {
   
   auto composite = std::make_unique<CompositeEvaluator>(mode);
   
   // 距離評価：近いほど高評価
   composite->AddEvaluator(
      MakeDistanceEvaluator(1.0f, 0.0f, pos1, pos2, closeDistance, farDistance),
      1.0f
   );
   
   // HP評価：HPが低いほど防御的に（近接を避ける）
   composite->AddEvaluator(
      MakeHpRatioEvaluator(0.3f, 1.0f, getHpRatio, 0.0f, 1.0f),
      1.0f
   );
   
   return composite;
}

// 距離、HP、時間を組み合わせた複合エバリュエーター（段階的な難易度調整用）
inline std::unique_ptr<CompositeEvaluator> MakePhaseBasedEvaluator(
   const Vector3* pos1, const Vector3* pos2,
   std::function<float()> getHpRatio,
   std::function<float()> getBattleTime,
   float maxBattleTime = 180.0f) {
   
   auto composite = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::WeightedSum);
   
   // 距離評価（30%）
   composite->AddEvaluator(
      MakeDistanceEvaluator(1.0f, 0.0f, pos1, pos2, 3.0f, 20.0f),
      0.3f
   );
   
   // HP評価（40%）- HPが低いほど積極的に
   composite->AddEvaluator(
      MakeHpRatioEvaluator(1.0f, 0.2f, getHpRatio, 0.0f, 1.0f),
      0.4f
   );
   
   // 時間評価（30%）- 時間経過で強化
   composite->AddEvaluator(
      MakeTimeBasedEvaluator(0.5f, 1.0f, getBattleTime, maxBattleTime),
      0.3f
   );
   
   return composite;
}

// 視界内かつ適切な距離にいるかを評価
inline std::unique_ptr<CompositeEvaluator> MakeVisibilityAndRangeEvaluator(
   const Vector3* pos1, const Vector3* pos2,
   std::function<float()> getAngle,
   float optimalDistance = 10.0f,
   float viewAngle = 60.0f) {
   
   auto composite = std::make_unique<CompositeEvaluator>(CompositeEvaluator::CombineMode::Product);
   
   // 視界内判定
   composite->AddEvaluator(
      MakeAngleEvaluator(1.0f, 0.0f, getAngle, -viewAngle, viewAngle),
      1.0f
   );
   
   // 距離評価
   composite->AddEvaluator(
      MakeDistanceEvaluator(1.0f, 0.0f, pos1, pos2, optimalDistance * 0.5f, optimalDistance * 1.5f),
      1.0f
   );
   
   return composite;
}
