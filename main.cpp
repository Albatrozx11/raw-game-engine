#include <SFML/Graphics.hpp>
#include <vector>
struct vec3d {
	float x, y, z;
};

struct triangle {
	vec3d p[3];
};

struct mesh {
	std::vector<triangle> tris;
};


int main() {
	sf::RenderWindow window(sf::VideoMode(256, 240), "RAW");

	while (window.isOpen()) {
		sf::Event event;

	}
	return 0;
}