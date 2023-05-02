#pragma once
#include <glm/glm.hpp>
#include <vector>

class Entity {
public:
	virtual void render()=0;
	virtual void update()=0;
};

//-----------FRUIT---------------//
class Fruit : Entity {
private:
public:
	glm::vec2 position;

	void render() {

	}

	void update() {

	}
};

enum Direction {
	UP,
	DOWN,
	LEFT,
	RIGHT
};

//-----------SNAKE---------------//
class Snake : Entity {
private:
	Direction dir;
	std::vector<glm::vec2> body;
public:
	void render() {

	}

	void update() {
		// move body
		for (auto i = body.end(); i != body.begin(); i--) {
			i->x = (i - 1)->x;
			i->y = (i - 1)->y;
			//i = (i - 1);
		}

		// move head
		if (dir == UP)
			body.at(0).y += 1;
		else if (dir == DOWN)
			body.at(0).y -= 1;
		else if (dir == LEFT)
			body.at(0).x -= 1;
		else if (dir == RIGHT)
			body.at(0).x += 1;
	}

	bool isColliding(glm::vec2 position) {
		for (auto i = body.begin(); i != body.end(); i++) {
			if (i->x == position.x && i->y == position.y)
				return true;
		}
		return false;
	}

	void setDirection(Direction dir) {
		this->dir = dir;
	}
};