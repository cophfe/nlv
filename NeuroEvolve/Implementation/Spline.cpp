#include "Spline.h"
#include <iostream>

Spline::Spline(glm::vec2 firstPoint, bool loop, bool autoConstructIntermediates)
	: constructIntermediates(autoConstructIntermediates)
{
	controlPoints.push_back(firstPoint);
	this->loop = false;
	SetLooping(loop);
}

std::vector<glm::vec2> Spline::CalculateLine(float stepDistance)
{
	std::vector<glm::vec2> line;
	for (float i = 0; i < 1;)
	{
		glm::vec2 p, m;
		GetPointAndGradientOnSpline(i, p, m);

		line.push_back(p);
		i += stepDistance / glm::length(m);
	}
	line.push_back(GetPointOnSpline(1));

	return line;
}

float Spline::GetMinimumDistanceToPoint(glm::vec2 point, glm::vec2* closestPoint)
{
	int curveIndex;
	float tValue;
	GetClosestCurve(point, &curveIndex, &tValue);
	
	glm::vec2 finalPoint = EvaluateBezierCurve(controlPoints[curveIndex], controlPoints[curveIndex + 1]
		, controlPoints[curveIndex + 2], controlPoints[curveIndex + 3], tValue);
	if (closestPoint)
		*closestPoint = finalPoint;
	float dist = glm::length(finalPoint - point);
    return dist;
}

float Spline::GetStepSize(float t, float gradient, int curveIndex, glm::vec2 point)
{
	//get a step size that will not cause divergance using backtracking line search
	const int maxIterations = 20;
	float stepSize = 0.0005f;
	
	for (int i = 0; i < maxIterations; i++)
	{
		float distSq = glm::length2(EvaluateBezierCurve(controlPoints[curveIndex], controlPoints[curveIndex + 1]
			, controlPoints[curveIndex + 2], controlPoints[curveIndex + 3], t) - point);

		float stepDistSq = glm::length2(EvaluateBezierCurve(controlPoints[curveIndex], controlPoints[curveIndex + 1]
			, controlPoints[curveIndex + 2], controlPoints[curveIndex + 3], t - stepSize) - point);

		if (stepDistSq < distSq - stepSize * 0.5f * gradient)
			break;
		else
			stepSize *= 0.5f;
	}

	return stepSize;
}

void Spline::GetClosestCurve(glm::vec2 point, int* controlPointIndex, float* curveT)
{
	//this aint optimised one bit lemme tell you

	//take 150 samples and find the lowest distance one
	const int sampleCount = 150;
	float t  = 0;
	float sqDist = glm::length2(point - GetPointOnSpline(0));

	for (size_t i = 1; i < sampleCount; i++)
	{
		float sqDist2 = glm::length2(point - GetPointOnSpline((float)i / sampleCount));
		if (sqDist2 < sqDist)
		{
			sqDist = sqDist2;
			t = (float)i / sampleCount;
		}
	}

	//get the t value local to the bezier curve, not the overall spline
	int curveCount = GetCurveCount();
	int curveIndex = (int)(t * curveCount);
	float localT = t * curveCount - curveIndex;
	if (t >= 1)
	{
		localT = 1;
		curveIndex = curveCount - 1;
	}
	else if (t < 0)
	{
		localT = 0;
		curveIndex = 0;
	}
	curveIndex *= 3;

	if (controlPointIndex)
		*controlPointIndex = curveIndex;

	if (!curveT)
		return;

	//now get bezier curve function in cubic standard form (for both x and y)
	glm::vec2 a = controlPoints[curveIndex + 3] - controlPoints[curveIndex]
		- 3.0f * controlPoints[curveIndex + 2] + 3.0f * controlPoints[curveIndex + 1];
	glm::vec2 b = 3.0f * controlPoints[curveIndex + 2] - 6.0f * controlPoints[curveIndex + 1] + 3.0f * controlPoints[curveIndex];
	glm::vec2 c = 3.0f * controlPoints[curveIndex + 1] - 3.0f * controlPoints[curveIndex];
	glm::vec2 d = controlPoints[curveIndex];

	

	//now use newtons method to get a more accurate result
	float tValue = localT;
	{
		//get distance squared function derivitive (in quintic standard form just because)
		float aA = 6.0f * (a.x * a.x + a.y * a.y);
		float bB = 10.0f * (a.x * b.x + a.y * b.y);
		float cC = 4.0f * (2.0f * (a.x * c.x + a.y * c.y) + b.x * b.x + b.y * b.y);
		float dD = 6.0f * (a.x * (d.x - point.x) + b.x * c.x + a.y * (d.y - point.y) + b.y * c.y);
		float eE = 2.0f * (2.0f * (b.x * (d.x - point.x) + b.y * (d.y - point.y)) + c.x * c.x + c.y * c.y);
		float fF = 2.0f * (c.x * (d.x - point.x) + c.y * (d.y - point.y));

		//iterate and get t
		float gradient = 10000;
		//it takes about three iterations in my use case and accuracy amount
		const int iterations = 6;
		for (int i = 0; i < iterations && glm::abs(gradient) > 0.001f; i++)
		{
			float t2 = tValue * tValue;
			float t3 = t2 * tValue;
			float t4 = t2 * t2;
			float t5 = t4 * tValue;

			gradient = aA * t5 + bB * t4 + cC * t3 + dD * t2 + eE * tValue + fF;

			//wait a second I forgot gradient descent sucked
			//gradient descent
			//tValue -= gradient * 0.00001f;
			
			//newtons method
			//this converges on a root
			//newT = (F'(t)*t - F(t))/F'(t)
			//the root we need to find is the root of the distance functions gradient, so we need to get the double derivitive
			float gradientGradient = 5.0f * aA * t4 + 4.0f * bB * t3 + 3.0f * cC * t2 + 2.0f * dD * tValue + eE;
			tValue = (gradientGradient * tValue - gradient) / gradientGradient;
		}
	}

	*curveT = tValue;
}

int Spline::GetClosestControlPointIndex(glm::vec2 point)
{
	int index = 0;
	float sqDist = glm::length2(point - controlPoints[0]);

	if (constructIntermediates)
	{
		int count = GetCurveCount();

		for (size_t i = 1; i < count; i++)
		{
			int ii = i * 3;
			float sqDist2 = glm::length2(point - controlPoints[ii]);
			if (sqDist2 < sqDist)
			{
				sqDist = sqDist2;
				index = ii;
			}
		}
	}
	else
	{
		int count = controlPoints.size();

		for (size_t i = 1; i < count; i++)
		{
			float sqDist2 = glm::length2(point - controlPoints[i]);
			if (sqDist2 < sqDist)
			{
				sqDist = sqDist2;
				index = i;
			}
		}
	}

	return index;
}

glm::vec2 Spline::GetPointOnSpline(float t)
{
	//t is not uniform
	int curveCount = GetCurveCount();
	int index = (int)(t * curveCount);
	float localT = t * curveCount - index;
	if (t >= 1)
	{
		localT = 1;
		index = curveCount - 1;
	}
	else if (t < 0)
	{
		localT = 0;
		index = 0;
	}

	index *= 3;
	return EvaluateBezierCurve(controlPoints[index], controlPoints[index + 1], controlPoints[index + 2], controlPoints[index + 3], localT);
}

glm::vec2 Spline::GetGradientOnSpline(float t)
{
	int curveCount = GetCurveCount();
	int index = (int)(t * curveCount);
	float localT = t * curveCount - index;
	if (t >= 1)
	{
		localT = 1;
		index = curveCount - 1;
	}
	else if (t < 0)
	{
		localT = 0;
		index = 0;
	}

	index *= 3;
	return EvaluateBezierCurveGradient(controlPoints[index], controlPoints[index + 1], controlPoints[index + 2], controlPoints[index + 3], localT);
}

void Spline::GetPointAndGradientOnSpline(float t, glm::vec2& point, glm::vec2& gradient)
{
	int curveCount = GetCurveCount();
	int index = (int)(t * curveCount);
	float localT = t * curveCount - index;
	if (t >= 1)
	{
		localT = 1;
		index = curveCount - 1;
	}
	else if (t < 0)
	{
		localT = 0;
		index = 0;
	}
	index *= 3;
	gradient = EvaluateBezierCurveGradient(controlPoints[index], controlPoints[index + 1], controlPoints[index + 2], controlPoints[index + 3], localT);
	point = EvaluateBezierCurve(controlPoints[index], controlPoints[index + 1], controlPoints[index + 2], controlPoints[index + 3], localT);
}

glm::vec2 Spline::EvaluateBezierCurve(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, float t)
{
	float opT = (1 - t);
	return opT * opT * opT * a + 3 * t * opT * opT * b + 3 * t * t * opT * c + t * t * t * d;
}

float Spline::EvaluateBezierCurve(float a, float b, float c, float d, float t)
{
	float opT = (1 - t);
	return opT * opT * opT * a + 3 * t * opT * opT * b + 3 * t * t * opT * c + t * t * t * d;
}

glm::vec2 Spline::EvaluateBezierCurveGradient(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, float t)
{
    return t * t * (-3.0f * a + 9.0f * b - 9.0f * c + 3.0f * d) + t * (6.0f * a - 12.0f * b + 6.0f * c) + (-3.0f * a + 3.0f * b);
}

float Spline::EvaluateBezierCurveGradient(float a, float b, float c, float d, float t)
{
    return t * t * (-3.0f * a + 9.0f * b - 9.0f * c + 3.0f * d) + t * (6.0f * a - 12.0f * b + 6.0f * c) + (-3.0f * a + 3.0f * b);
}

void Spline::SetLooping(bool value)
{
	if (value == loop)
		return;

	if (loop)
	{
		controlPoints.erase(controlPoints.end() - 2, controlPoints.end());
	}
	else
	{
		AddCurve(controlPoints[0]);
	}

	loop = value;
}

int Spline::GetCurveCount()
{
	return (controlPoints.size() - 1) / 3;
}

void Spline::AddCurve(glm::vec2 controlPoint)
{
	int size = loop ? controlPoints.size() - 3 : controlPoints.size();

	glm::vec2 intermediate1;
	glm::vec2 intermediate2;
	
	if (size < 4)
	{
		intermediate1 = (3.0f * controlPoints[size - 1] + controlPoint) * 0.25f;
		intermediate2 = (3.0f * controlPoint + controlPoints[size - 1]) * 0.25f;
	}
	else 
	{
		intermediate1 = 2.0f * controlPoints[size - 1] - controlPoints[size - 2];
		intermediate2 = (controlPoint + intermediate1) * 0.5f;
	}
	
	controlPoints.reserve(3);
	controlPoints.insert(controlPoints.begin() + size, {
		intermediate1,
		intermediate2,
		controlPoint
		});

	AutoCalculateIntermediates();
}

int Spline::InsertCurveFromPosition(glm::vec2 point)
{
	int curveIndex;
	GetClosestCurve(point, &curveIndex, nullptr);
	//insert after closest curve
	curveIndex += 3;

	int size = loop ? controlPoints.size() - 3 : controlPoints.size();

	glm::vec2 intermediate1;
	glm::vec2 intermediate2;

	if (size < 4)
	{
		intermediate1 = (3.0f * controlPoints[size - 1] + point) * 0.25f;
		intermediate2 = (3.0f * point + controlPoints[size - 1]) * 0.25f;
	}
	else
	{
		intermediate1 = 2.0f * controlPoints[size - 1] - controlPoints[size - 2];
		intermediate2 = (point + intermediate1) * 0.5f;
	}

	controlPoints.reserve(3);
	controlPoints.insert(controlPoints.begin() + curveIndex, {
		intermediate1,
		intermediate2,
		point
		});

	AutoCalculateIntermediates();
	return curveIndex;
}

void Spline::AddCurve(glm::vec2 intermediatePoint1, glm::vec2 intermediatePoint2, glm::vec2 controlPoint)
{
	int size = loop ? controlPoints.size() : controlPoints.size() - 3;
	controlPoints.reserve(3);
	controlPoints.insert(controlPoints.begin() + size, {
		intermediatePoint1,
		intermediatePoint2,
		controlPoint
		});

	AutoCalculateIntermediates();
}

void Spline::RemoveCurve(int firstPointIndex)
{
	//no error checking because I have absolute faith
	if (firstPointIndex == 0)
	{
		controlPoints.erase(controlPoints.end() - 1, controlPoints.end());
		controlPoints.erase(controlPoints.begin() + firstPointIndex, controlPoints.begin() + firstPointIndex + 2);

	}
	else
		controlPoints.erase(controlPoints.begin() + firstPointIndex - 1, controlPoints.begin() + firstPointIndex + 2);

	AutoCalculateIntermediates();
}

void Spline::SetControlPoint(int pointIndex, glm::vec2 newValue)
{
	controlPoints[pointIndex] = newValue;
	AutoCalculateIntermediates();
}

void Spline::AutoCalculateIntermediates()
{
	int size = loop ? controlPoints.size() - 3: controlPoints.size();
	if (size < 4)
		return;

	if (constructIntermediates)
	{
		int count = GetCurveCount();
		for (size_t i = 0; i < count - 1; i++)
		{
			int last = i * 3;
			int current = last + 3;
			int next = last + 6;

			glm::vec2 lastDelta = controlPoints[last] - controlPoints[current];
			glm::vec2 nextDelta = controlPoints[next] - controlPoints[current];
			float distToLast = glm::length(lastDelta);
			float distToNext = glm::length(nextDelta);
			
			//FIX THIS, JUST MAKE THE DELTA AVERAGE 
			glm::vec2 delta = lastDelta / distToLast - nextDelta / distToNext;

			controlPoints[current - 1] = controlPoints[current] + delta * (0.25f * distToLast);
			controlPoints[current + 1] = controlPoints[current] - delta * (0.25f * distToNext);
		}

		//do start and end points
		if (loop)
		{
			glm::vec2 lastDelta = controlPoints[controlPoints.size() - 4] - controlPoints[0];
			glm::vec2 nextDelta = controlPoints[3] - controlPoints[0];
			float distToLast = glm::length(lastDelta);
			float distToNext = glm::length(nextDelta);
			glm::vec2 delta = lastDelta / distToLast - nextDelta / distToNext;
			controlPoints[1] = controlPoints[0] - delta * (0.25f * distToNext);
			controlPoints[controlPoints.size() - 2] = controlPoints[0] + delta * (0.25f * distToLast);
			controlPoints[controlPoints.size() - 1] = controlPoints[0];
		}
		else 
		{
			controlPoints[1] = (controlPoints[0] + controlPoints[2]) / 2.0f;
			controlPoints[controlPoints.size() - 2] = (controlPoints[controlPoints.size() - 1] + controlPoints[controlPoints.size() - 3]) / 2.0f;
		}
	}
	else if (loop)
	{
		glm::vec2 lastDelta = controlPoints[controlPoints.size() - 4] - controlPoints[0];
		glm::vec2 nextDelta = controlPoints[3] - controlPoints[0];
		float distToLast = glm::length(lastDelta);
		float distToNext = glm::length(nextDelta);
		glm::vec2 delta = lastDelta / distToLast - nextDelta / distToNext;
		controlPoints[1] = controlPoints[0] - delta * (0.25f * distToNext);
		controlPoints[controlPoints.size() - 2] = controlPoints[0] + delta * (0.25f * distToLast);
		controlPoints[controlPoints.size() - 1] = controlPoints[0];
	}
}

bool Spline::IsIntermediate(int index)
{
	int size = controlPoints.size();
	return index % 3 != 0;
}

int Spline::GetAttachedControlPointIndex(int index)
{
	if ((index - 1) % 3 == 1)
		return index + 1;
	else
		return index - 1;

}
