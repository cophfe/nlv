#pragma once
#include <vector>
#include "glm.hpp"
#include "gtx/norm.hpp"

//this is a small thing used for the racer so it will be tailered for that specific use and won't be optimised
class Spline
{
public:
	Spline() = default;
	Spline(glm::vec2 firstPoint, bool loop = false, bool autoConstructIntermediates = false);

	int InsertCurveFromPosition(glm::vec2 point);
	void AddCurve(glm::vec2 controlPoint);
	void AddCurve(glm::vec2 intermediatePoint1, glm::vec2 intermediatePoint2, glm::vec2 controlPoint);
	void RemoveCurve(int firstPointIndex);
	void SetControlPoint(int pointIndex, glm::vec2 newValue);
	glm::vec2 GetControlPoint(int pointIndex) { return controlPoints[pointIndex]; };
	const std::vector<glm::vec2>& GetControlPoints() const { return controlPoints; }

	std::vector<glm::vec2> CalculateLine(float stepDistance);

	float GetMinimumDistanceToPoint(glm::vec2 point, glm::vec2* closestPoint);
	int GetClosestControlPointIndex(glm::vec2 point);

	glm::vec2 GetPointOnSpline(float t);
	glm::vec2 GetGradientOnSpline(float t);
	void GetPointAndGradientOnSpline(float t, glm::vec2& point, glm::vec2& gradient);

	static glm::vec2 EvaluateBezierCurve(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, float t);
	static float EvaluateBezierCurve(float a, float b, float c, float d, float t);
	static glm::vec2 EvaluateBezierCurveGradient(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, float t);
	static float EvaluateBezierCurveGradient(float a, float b, float c, float d, float t);

	bool GetAutoConstructIntermediates() { return constructIntermediates; }
	bool IsLooping() { return loop; }
	void SetLooping(bool value);
	void SetConstructIntermediates(bool value) { constructIntermediates = value; }
	void AutoCalculateIntermediates();

	int GetCurveCount();

	//i know 'intermediates' isn't the right nomenclature
	bool IsIntermediate(int index);
	static int GetAttachedControlPointIndex(int index);

private:
	void GetClosestCurve(glm::vec2 point, int* controlPointIndex, float* curveT);
	float GetStepSize(float t, float gradient, int curveIndex, glm::vec2 point);
	

	std::vector<glm::vec2> controlPoints;
	bool loop;
	bool constructIntermediates;
};

