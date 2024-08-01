#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <strstream>
#include <sstream>
#include <algorithm>
//3d coordinate
struct vec3d {
	float x, y, z;
};

//triangle is a set of 3 3d coordinates
struct triangle {
	vec3d p[3];
	vec3d normal;
};

//a mesh is a collection of triangles
struct mesh {
	std::vector<triangle> tris;

	bool LoadFromObjectFile(std::string sFilename) {
		std::ifstream f(sFilename);

		if (!f.is_open()) {
			return false;
		}

		std::vector<vec3d> verts;

		while (!f.eof()) {
			char line[128];
			f.getline(line, 128);

			std::strstream s;

			s << line;

			char junk;
			
			if (line[0] == 'v') {
				vec3d v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}

			if (line[0] == 'f') {
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}
			
		}

		return true;
	}
};

vec3d vCamera;

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
	mesh meshObj;
	if (!meshObj.LoadFromObjectFile("spaceship_sample_triangles.obj")) {
		return -1;
	}

	//initialize the cube mesh with coordinates
	/*meshCube.tris = {
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


	};*/

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


		std::vector<triangle> vecTrianglesToRaster;

		//add a ligh_direction that points to the player
		vec3d light_direction = { 0.0f,0.0f,-1.0f };
		
		//normalize all triangles using projection matrix
		for (auto& tri : meshObj.tris) {
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
				triTranslated.p[i].z = triZXRotated.p[i].z + 15.0f;
			}
			
			//calculating normal
			vec3d line1, line2, normal;
			line1.x = triTranslated.p[1].x - triTranslated.p[0].x;
			line1.y = triTranslated.p[1].y - triTranslated.p[0].y;
			line1.z = triTranslated.p[1].z - triTranslated.p[0].z;

			line2.x = triTranslated.p[2].x - triTranslated.p[0].x;
			line2.y = triTranslated.p[2].y - triTranslated.p[0].y;
			line2.z = triTranslated.p[2].z - triTranslated.p[0].z;

			normal.x = line1.y * line2.z - line1.z * line2.y;
			normal.y = line1.z * line2.x - line1.x * line2.z;
			normal.z = line1.x * line2.y - line1.y * line2.x;

			//normalize the normal
			float mag = sqrtf(normal.x * normal.x + normal.y * normal.y + normal.z * normal.z);
			normal.x /= mag; normal.y /= mag; normal.z /= mag;


			if (((normal.x) * (triTranslated.p[0].x - vCamera.x)) +
				((normal.y) * (triTranslated.p[0].y - vCamera.y)) +
				((normal.z) * (triTranslated.p[0].z - vCamera.z)) < 0.0f) {

				//project to 2D space
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
				triProjected.normal.x = normal.x;
				triProjected.normal.y = normal.y;
				triProjected.normal.z = normal.z;
				vecTrianglesToRaster.push_back(triProjected);

			}

		}

		//sort the triangles based on mid point z component of each triangle(painters algorithm)
		sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](triangle& t1, triangle& t2) {
			float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
			float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
			return z1 > z2;
		});


		for (auto& triProjected : vecTrianglesToRaster) {
			//Define and draw the triangle
			sf::Vertex trianglePoints[] = {
				sf::Vector2f(triProjected.p[0].x, triProjected.p[0].y),
				sf::Vector2f(triProjected.p[1].x, triProjected.p[1].y),
				sf::Vector2f(triProjected.p[2].x, triProjected.p[2].y),
			};

			//normalize light_direction
			float light_mag = sqrtf(light_direction.x * light_direction.x + light_direction.y * light_direction.y + light_direction.z * light_direction.z);

			light_direction.x /= light_mag; light_direction.y /= light_mag; light_direction.z /= light_mag;

			//get similarity b/w normal and light_direction
			float dp = (triProjected.normal.x * light_direction.x) + (triProjected.normal.y * light_direction.y) + (triProjected.normal.z * light_direction.z);

			sf::Color color = sf::Color(255 * dp, 255 * dp, 255 * dp);

			trianglePoints[0].color = color;
			trianglePoints[1].color = color;
			trianglePoints[2].color = color;

			//draw the cube	
			window.draw(trianglePoints, 3, sf::Triangles);

		}

		//display window after drawing
		window.display();

	}
	return 0;
}