#ifndef SEARCHANALYSIS_H
#define SEARCHANALYSIS_H
#include <chrono>
#include <nlohmann/json.hpp>

class SearchAnalysis {
private:
    static std::chrono::high_resolution_clock::time_point clock;
    static std::chrono::high_resolution_clock::time_point timePaused;
    static nlohmann::json data;
    static nlohmann::basic_json<>* current;
    static const nlohmann::json emptyGraph;
public:
    static void StartClock();
    static void PauseClock();
    static void ResumeClock();
    static void EnterGraph(const std::string& key);
    static void LabelGraph(const std::string& title);
    static void LabelAxes(const std::string& xLabel, const std::string& yLabel);
    static void SetInterpolationOrder(int order);
    static void InsertPoint(unsigned long x,unsigned long y);
    static void InsertTimePoint(unsigned long y);
    static void ExportData(const std::string& filename);
    static void ClearData();
};

#endif //SEARCHANALYSIS_H
