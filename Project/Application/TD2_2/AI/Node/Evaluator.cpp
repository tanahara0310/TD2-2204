#include "Evaluator.h"
#include "Engine/Utility/Random/RandomGenerator.h"
#include <cmath>

LambdaEvaluator::LambdaEvaluator(std::function<float()> func)
    : func_(std::move(func)) {}

float LambdaEvaluator::Evaluate() const {
   return func_ ? func_() : 0.0f;
}

DistanceEvaluator::DistanceEvaluator(float closeValue, float farValue, std::function<float()> getDistance, float minDistance, float maxDistance)
   : closeValue_(closeValue), farValue_(farValue),
   getDistance_(std::move(getDistance)),
   minDistance_(minDistance), maxDistance_(maxDistance) {}

// Vector3の座標を使用する便利なコンストラクタ
// pos1とpos2のポインタを受け取り、動的に距離を計算する
DistanceEvaluator::DistanceEvaluator(float closeValue, float farValue, const Vector3* pos1, const Vector3* pos2, float minDistance, float maxDistance)
   : closeValue_(closeValue), farValue_(farValue),
   minDistance_(minDistance), maxDistance_(maxDistance) {

   getDistance_ = [pos1, pos2]() -> float {
      if (!pos1 || !pos2) return 0.0f;
      Vector3 diff = *pos2 - *pos1;
      return MathCore::Vector::Length(diff);
      };
}

float DistanceEvaluator::Evaluate() const {
   if (!getDistance_) return closeValue_;

   float distance = getDistance_();

   // 距離が最小距離以下なら近い値を返す
   if (distance <= minDistance_) {
      return closeValue_;
   }

   // 距離が最大距離以上なら遠い値を返す
   if (distance >= maxDistance_) {
      return farValue_;
   }

   // 最小距離と最大距離の間で線形補間
   float t = (distance - minDistance_) / (maxDistance_ - minDistance_);
   return closeValue_ + t * (farValue_ - closeValue_);
}

HpRatioEvaluator::HpRatioEvaluator(float lowValue, float highValue, std::function<float()> getHpRatio, float minRatio, float maxRatio)
   : lowValue_(lowValue), highValue_(highValue),
   getHpRatio_(std::move(getHpRatio)),
   minRatio_(minRatio), maxRatio_(maxRatio) {}

float HpRatioEvaluator::Evaluate() const {
   if (!getHpRatio_) return lowValue_;

   float ratio = getHpRatio_();

   // 割合が最小割合以下なら低い値を返す
   if (ratio <= minRatio_) {
      return lowValue_;
   }

   // 割合が最大割合以上なら高い値を返す
   if (ratio >= maxRatio_) {
      return highValue_;
   }

   // 最小割合と最大割合の間で線形補間
   float t = (ratio - minRatio_) / (maxRatio_ - minRatio_);
   return lowValue_ + t * (highValue_ - lowValue_);
}

TimeBasedEvaluator::TimeBasedEvaluator(float startValue, float endValue,
   std::function<float()> getElapsedTime, float duration)
   : startValue_(startValue), endValue_(endValue),
   getElapsedTime_(std::move(getElapsedTime)), duration_(duration) {}

float TimeBasedEvaluator::Evaluate() const {
   if (!getElapsedTime_ || duration_ <= 0.0f) return startValue_;

   float elapsed = getElapsedTime_();
   float t = std::clamp(elapsed / duration_, 0.0f, 1.0f);
   
   return startValue_ + t * (endValue_ - startValue_);
}

AngleEvaluator::AngleEvaluator(float inRangeValue, float outRangeValue,
   std::function<float()> getAngle, float minAngle, float maxAngle)
   : inRangeValue_(inRangeValue), outRangeValue_(outRangeValue),
   getAngle_(std::move(getAngle)), minAngle_(minAngle), maxAngle_(maxAngle) {}

float AngleEvaluator::Evaluate() const {
   if (!getAngle_) return outRangeValue_;

   float angle = getAngle_();

   // 角度が範囲内なら inRangeValue
   if (angle >= minAngle_ && angle <= maxAngle_) {
      return inRangeValue_;
   }

   // 範囲外なら outRangeValue
   return outRangeValue_;
}

CounterEvaluator::CounterEvaluator(float minValue, float maxValue,
   std::function<int()> getCounter, int minCount, int maxCount)
   : minValue_(minValue), maxValue_(maxValue),
   getCounter_(std::move(getCounter)), minCount_(minCount), maxCount_(maxCount) {}

float CounterEvaluator::Evaluate() const {
   if (!getCounter_) return minValue_;

   int counter = getCounter_();

   // カウントが最小値以下
   if (counter <= minCount_) {
      return minValue_;
   }

   // カウントが最大値以上
   if (counter >= maxCount_) {
      return maxValue_;
   }

   // 線形補間
   float t = static_cast<float>(counter - minCount_) / static_cast<float>(maxCount_ - minCount_);
   return minValue_ + t * (maxValue_ - minValue_);
}

RandomEvaluator::RandomEvaluator(float minValue, float maxValue)
   : minValue_(minValue), maxValue_(maxValue) {}

float RandomEvaluator::Evaluate() const {
   return RandomGenerator::GetInstance().GetFloat(minValue_, maxValue_);
}

CurveEvaluator::CurveEvaluator(float startValue, float endValue,
   std::function<float()> getProgress, CurveType curveType)
   : startValue_(startValue), endValue_(endValue),
   getProgress_(std::move(getProgress)), curveType_(curveType) {}

float CurveEvaluator::Evaluate() const {
   if (!getProgress_) return startValue_;

   float progress = std::clamp(getProgress_(), 0.0f, 1.0f);
   float t = ApplyCurve(progress);

   return startValue_ + t * (endValue_ - startValue_);
}

float CurveEvaluator::ApplyCurve(float t) const {
   switch (curveType_) {
      case CurveType::EaseIn:
         return t * t;
      
      case CurveType::EaseOut:
         return 1.0f - (1.0f - t) * (1.0f - t);
      
      case CurveType::EaseInOut:
         if (t < 0.5f) {
            return 2.0f * t * t;
         } else {
            return 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f;
         }
      
      case CurveType::Linear:
      default:
         return t;
   }
}

CompositeEvaluator::CompositeEvaluator(CombineMode mode)
   : mode_(mode) {}

void CompositeEvaluator::AddEvaluator(std::unique_ptr<IEvaluator> eval, float weight, EvalModifier mod) {
   evaluators_.push_back({ std::move(eval), weight, mod });
}

float CompositeEvaluator::Evaluate() const {
   if (evaluators_.empty()) return 0.0f;

   std::vector<float> values;
   values.reserve(evaluators_.size());

   for (auto& e : evaluators_) {
      float v = e.eval->Evaluate();
      v = v * e.mod.scale + e.mod.bias;
      if (e.mod.clamp01) v = std::clamp(v, 0.0f, 1.0f);
      values.push_back(v * e.weight);
   }

   switch (mode_) {
      case CombineMode::Sum: return std::accumulate(values.begin(), values.end(), 0.0f);
      case CombineMode::Product: return std::accumulate(values.begin(), values.end(), 1.0f, std::multiplies<>());
      case CombineMode::Average: return std::accumulate(values.begin(), values.end(), 0.0f) / values.size();
      case CombineMode::WeightedSum: {
         float sum = 0, wsum = 0;
         for (size_t i = 0; i < values.size(); ++i) { sum += values[i]; wsum += evaluators_[i].weight; }
         return (wsum > 0) ? sum / wsum : 0.0f;
      }
      case CombineMode::Max: return *std::max_element(values.begin(), values.end());
      case CombineMode::Min: return *std::min_element(values.begin(), values.end());
   }
   return 0.0f;
}
