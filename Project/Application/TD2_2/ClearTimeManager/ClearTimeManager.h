#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

class ClearTimeManager {
public:
	ClearTimeManager(const std::string& path) : filePath(path) { LoadTimes(); }

	void LoadTimes();

	void SaveTimes();

	void RegisterTime(float newTime);

	const std::vector<float>& GetTimes() const { return times; }

private:
	std::string filePath;
	std::vector<float> times;
};
