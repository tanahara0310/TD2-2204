#include "ClearTimeManager.h"

void ClearTimeManager::LoadTimes() {
	std::ifstream in(filePath);
	times.clear();
	if (in.is_open()) {
		float t;
		while (in >> t) {
			times.push_back(t);
		}
	}
	if (times.size() < 3) {
		times.resize(3, 9999.0f); // 初期値
	}
}

void ClearTimeManager::SaveTimes() {
	std::ofstream out(filePath);
	for (auto t : times) {
		out << t << "\n";
	}
}

void ClearTimeManager::RegisterTime(float newTime) {
	times.push_back(newTime);
	std::sort(times.begin(), times.end());
	if (times.size() > 3) {
		times.resize(3);
	}
	SaveTimes();
}
