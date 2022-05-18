#pragma once
#include "raylib.h"
#include <string>

struct Coord
{
	Coord() = default;
	Coord(short x, short y) : x(x), y(y) {}
	Coord&& operator* (const int val) {
		return { (short)(x * val), (short)(y * val) };
	}
	Coord&& operator/ (const int val) {
		return { (short)(x / val), (short)(y / val) };
	}
	Coord&& operator* (const float val) {
		return { (short)(x * val), (short)(y * val)};
	}
	Coord&& operator/ (const float val) {
		return { (short)(x / val), (short)(y / val) };
	}
	Coord&& operator+ (const Coord& other) {
		return { x + other.x, y + other.y };
	}
	Coord&& operator- (const Coord& other) {
		return { x - other.x, y - other.y };

	}
	bool operator== (const Coord& other) {
		return x == other.x && y == other.y;
	}
	short x, y;
};

class Button
{
private:
	enum class State
	{
		PRESSED,
		HOVER,
		DEFAULT
	};
	State state;
	bool pressed = false;
public:
	Coord pos;
	Coord size;
	Color colour;
	Color hoverColour;
	Color textColour;
	int fontSize;
	float xPadding;
	std::string text;
	void(*onClick)();

	Button(Coord pos, Coord size, Color colour, Color hoverColour, Color textColour, int fontSize, float xPadding, std::string text, void(*onClick)())
		: pos(pos), size(size), colour(colour), hoverColour(hoverColour), textColour(textColour), fontSize(fontSize), xPadding(xPadding), text(text), onClick(onClick) {}

	void Draw() {
		switch (state)
		{
		case Button::State::DEFAULT:
			DrawRectangle(pos.x, pos.y, size.x, size.y, colour);
			break;
		case Button::State::HOVER:
			DrawRectangle(pos.x, pos.y, size.x, size.y, hoverColour);
			break;
		case Button::State::PRESSED:
			DrawRectangle(pos.x, pos.y, size.x, size.y, 
				Color{ (unsigned char)(hoverColour.r / 2), (unsigned char)(hoverColour.g / 2), (unsigned char)(hoverColour.b / 2), hoverColour.a});
			break;
		}
		DrawText(text.c_str(), pos.x + xPadding, pos.y + size.y / 2 - fontSize / 2, fontSize, textColour);

	}

	void Update()
	{
		auto mousePosition = GetMousePosition();

		if (mousePosition.x > pos.x && mousePosition.x < pos.x + size.x &&
			mousePosition.y > pos.y && mousePosition.y < pos.y + size.y)
		{
			if (state == State::DEFAULT)
			{
				state = State::HOVER;
			}

			if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
			{
				state = State::PRESSED;
			}
			else if (IsMouseButtonUp(MOUSE_LEFT_BUTTON) && state == State::PRESSED)
			{
				state = State::HOVER;
				pressed = true;
				if (onClick)
					onClick();
			}
		}
		else if (IsMouseButtonUp(MOUSE_LEFT_BUTTON))
		{
			state = State::DEFAULT;
		}
	}

	bool Pressed() { bool p = pressed; pressed = false; return p; }
};

