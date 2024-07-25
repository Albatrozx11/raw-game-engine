#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
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
	sf::RenderWindow window(sf::VideoMode(1500,800), "RAW",sf::Style::Default,settings);

	window.setVerticalSyncEnabled(true);

	//declare clock variable of sfml
	sf::Clock Clock;

	//create a cube mesh
	mesh meshCube;

	//initialize the cube mesh with coordinates
	meshCube.tris = {
		//SOUTH
		{0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f, 0.0f},
		{0.0f, 0.0f, 0.0f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f},


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

	//define projection matrix
	float projMatrix[4][4] = { 0.0f };
	//define rotation matrices
	float rotZMat[4][4] = { 0.0f };
	float rotXMat[4][4] = { 0.0f };
	
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

	float ftheta = 0.0f;
	
	//GAME LOOP
	while (window.isOpen()) {
		sf::Event event;

		//accumulate the time elapsed and restart the clock
		ftheta += 1.0f * Clock.getElapsedTime().asSeconds();
		Clock.restart();


		//check for window close event
		while (window.pollEvent(event)) {

			if (event.type == sf::Event::Closed)
				window.close();
		}

		//clear the window before drawing
		window.clear(sf::Color::Black);

		//initialize X rotation matrix
		rotXMat[0][0] = 1.0f;
		rotXMat[1][1] = cosf(ftheta * 0.5f);
		rotXMat[1][2] = sinf(ftheta * 0.5f);
		rotXMat[2][1] = -sinf(ftheta * 0.5f);
		rotXMat[2][2] = cosf(ftheta * 0.5f);
		rotXMat[3][3] = 1.0f;

		//initialize Z rotation matrix
		rotZMat[0][0] = cosf(ftheta);
		rotZMat[0][1] = -sinf(ftheta);
		rotZMat[1][0] = sinf(ftheta);
		rotZMat[1][1] = cosf(ftheta);
		rotZMat[2][2] = 1.0f;
		rotZMat[3][3] = 1.0f;

		//normalize all triangles using projection matrix
		for (auto& tri : meshCube.tris) {
			triangle triProjected,triTranslated,triZRotated,triZXRotated;

			//Rotate in Z axis
			MultiplyMatrixVector(tri.p[0], triZRotated.p[0], rotZMat);
			MultiplyMatrixVector(tri.p[1], triZRotated.p[1], rotZMat);
			MultiplyMatrixVector(tri.p[2], triZRotated.p[2], rotZMat);

			//Rotate in X axis
			MultiplyMatrixVector(triZRotated.p[0], triZXRotated.p[0], rotXMat);
			MultiplyMatrixVector(triZRotated.p[1], triZXRotated.p[1], rotXMat);
			MultiplyMatrixVector(triZRotated.p[2], triZXRotated.p[2], rotXMat);


			//Translate the triangle in the z axis
			triTranslated = triZXRotated;
			for (int i = 0; i < 3; i++) {
				triTranslated.p[i].z = triZXRotated.p[i].z + 3.0f;
			}

			MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], projMatrix);
			MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], projMatrix);
			MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], projMatrix);

			for (int i = 0; i < 3; i++) {
				//Translate to screen space
				triProjected.p[i].x += 1.0f;
				triProjected.p[i].y += 1.0f;

				//scale to screen space
				triProjected.p[i].x *= 0.5f * screenWidth;
				triProjected.p[i].y *= 0.5f * screenHeight;
			}

			//Define and draw the triangle
			sf::Vertex trianglePoints[] = {
				sf::Vector2f(triProjected.p[0].x, triProjected.p[0].y),
				sf::Vector2f(triProjected.p[1].x, triProjected.p[1].y),
				sf::Vector2f(triProjected.p[2].x, triProjected.p[2].y),
			};
			trianglePoints[0].color = sf::Color::Transparent;
			trianglePoints[1].color = sf::Color::Transparent;
			window.draw(trianglePoints, 3, sf::Triangles);

		}

		//display window after drawing
		window.display();

	}
	return 0;
}