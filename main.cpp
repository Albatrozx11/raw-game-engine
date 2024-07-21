#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>

//3d coordinate
struct vec3d {
	float x, y, z;
};

//triangle is a set of 3 3d coordinates
struct triangle {
	vec3d p[3];
};

//a mesh is a collection of triangles
struct mesh {
	std::vector<triangle> tris;
};


void MultiplyMatrixVector(vec3d& i, vec3d& j, float m[4][4]) {
	j.x = i.x * m[0][0] + i.y * m[1][0] + i.z * m[2][0] + m[3][0];
	j.y = i.x * m[0][1] + i.y * m[1][1] + i.z * m[2][1] + m[3][1];
	j.z = i.x * m[0][2] + i.y * m[1][2] + i.z * m[2][2] + m[3][2];
	float w = i.x * m[0][3] + i.y * m[1][3] + i.z * m[2][3] + m[3][3];

	if (w != 0.0f) {
		j.x /= w; j.y /= w; j.z /= w;
	}
}

int main() {
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;//set anti aliasing level

	//initialize sfml window
	sf::RenderWindow window(sf::VideoMode(800, 600), "RAW",sf::Style::Default,settings);

	window.setVerticalSyncEnabled(true);

	//create a cube mesh
	mesh meshCube;

	//initialize the cube mesh with coordinates
	meshCube.tris = {
		//SOUTH
		{0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f,},
		{0.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,},


		//EAST
		{1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, },
		{1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, },


		//NORTH
		{1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, },
		{1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, },


		//WEST
		{0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, },
		{0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, },


		//TOP
		{0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, },
		{0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, },

		//BOTTOM
		{1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, },
		{1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, },


	};

	//get width and height of the window
	sf::Vector2u size = window.getSize();
	float screenWidth = (float)window.getSize().x;
	float screenHeight = (float)window.getSize().y;

	//initialize projection matrix
	float projMatrix[4][4] = { 0.0f };
	
	//projection matrix
	float fAspectRatio =  screenHeight / screenWidth;
	float fFov = 90.0f;
	float fFovRad = 1.0f / tanf(fFov * 0.5f * (3.14159265359f / 180.0f));
	float fNear = 0.1f;
	float fFar = 1000.0f;
	float zScaleFactor = fFar / (fFar - fNear);

	projMatrix[0][0] = fAspectRatio * fFovRad;
	projMatrix[1][1] = fFovRad;
	projMatrix[2][2] = zScaleFactor;
	projMatrix[3][2] = -fNear * zScaleFactor ;
	projMatrix[2][3] = 1.0f;
	projMatrix[3][3] = 0.0f;

	//normalize all triangles using projection matrix
	for (auto tri : meshCube.tris) {
		triangle triProjected;
		MultiplyMatrixVector(tri.p[0], triProjected.p[0], projMatrix);
		MultiplyMatrixVector(tri.p[1], triProjected.p[1], projMatrix);
		MultiplyMatrixVector(tri.p[2], triProjected.p[2], projMatrix);
	}


	//GAME LOOP
	while (window.isOpen()) {
		sf::Event event;

		//check for window close event
		while (window.pollEvent(event)) {

			if (event.type == sf::Event::Closed)
				window.close();
		}

		//clear the window before drawing
		window.clear(sf::Color::Black);
		
		//window.draw(circle);

		//display window after drawing
		window.display();

	}
	return 0;
}