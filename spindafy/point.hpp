#pragma once

class Point{
public:
	int x = 0, y = 0;

	Point() = default;
	Point(int x, int y) : x(x), y(y){}
	Point &operator+=(const Point &other){
		this->x += other.x;
		this->y += other.y;
		return *this;
	}
	Point operator+(const Point &other) const{
		auto ret = *this;
		ret += other;
		return ret;
	}
	Point &operator-=(const Point &other){
		this->x -= other.x;
		this->y -= other.y;
		return *this;
	}
	Point operator-(const Point &other) const{
		auto ret = *this;
		ret -= other;
		return ret;
	}
	Point operator-() const{
		auto ret = *this;
		ret.x = -ret.x;
		ret.y = -ret.y;
		return ret;
	}
};
