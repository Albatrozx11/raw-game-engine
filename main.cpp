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

	vec3d() : x(0), y(0), z(0) {};
	vec3d(float x, float y, float z) : x(x), y(y), z(z) {};
	
	vec3d operator+=(float w) {
		x += w;
		y += w;
		z += 0;
		return *this;
	}

	vec3d operator*=(float w) {
		x *= w;
		y *= w;
		z *= 1;
		return *this;
	}

	vec3d operator/=(float w) {
		x /= w;
		y /= w;
		z /= w;
		return *this;
	}

	float length_squared() const {
		return x * x + y * y + z * z;
	}

	float length() const{
		return std::sqrtf(length_squared());
	}
};

inline vec3d operator+(const vec3d& u, const vec3d& v) {
	return vec3d(u.x + v.x, u.y + v.y, u.z + v.z);
}

inline vec3d operator-(const vec3d& u, const vec3d& v) {
	return vec3d(u.x - v.x, u.y - v.y, u.z - v.z);
}

inline vec3d operator*(const vec3d& u, const vec3d& v) {
	return vec3d(u.x * v.x, u.y * v.y, u.z * v.z);
}

inline vec3d operator/(const vec3d& u, const vec3d& v) {
	return vec3d(u.x / v.x, u.y / v.y, u.z / v.z);
}

inline vec3d cross(const vec3d& u, const vec3d& v) {
	return vec3d(u.y * v.z - u.z * v.y,
		u.z * v.x - u.x * v.z,
		u.x * v.y - u.y * v.x);
}

inline float dot(const vec3d& u, const vec3d& v) {
	return u.x * v.x + u.y * v.y + u.z * v.z;
}

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
		j /= w;
	}
}

void MultiplyMatrixTriangle(triangle& t1, triangle& t2, float m[4][4]) {
	MultiplyMatrixVector(t1.p[0], t2.p[0], m);
	MultiplyMatrixVector(t1.p[1], t2.p[1], m);
	MultiplyMatrixVector(t1.p[2], t2.p[2], m);
}



int main() {
	sf::ContextSettings settings;
	settings.antialiasingLevel = 8;//set anti aliasing level

	//initialize sfml window
	sf::RenderWindow window(sf::VideoMode(1500, 800), "RAW", sf::Style::Default, settings);

	window.setVerticalSyncEnabled(true);

	//declare clock variable of sfml
	sf::Clock Clock;

	//create a cube mesh
	mesh meshObj;
	if (!meshObj.LoadFromObjectFile("car_dirty.obj")) {
		return -1;
	}

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
	float fAspectRatio = screenHeight / screenWidth;
	float fFov = 90.0f;
	float fFovRad = 1.0f / tanf(fFov * 0.5f * (3.14159265359f / 180.0f));
	float fNear = 0.1f;
	float fFar = 1000.0f;
	float zScaleFactor = fFar / (fFar - fNear);

	projMatrix[0][0] = fAspectRatio * fFovRad;
	projMatrix[1][1] = fFovRad;
	projMatrix[2][2] = zScaleFactor;
	projMatrix[3][2] = -fNear * zScaleFactor;
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
			triangle triProjected, triTranslated, triZRotated, triZXRotated;

			//Rotate in Z axis
			MultiplyMatrixTriangle(tri, triZRotated, rotZMat);

			//Rotate in X axis
			MultiplyMatrixTriangle(triZRotated,triZXRotated, rotXMat);


			//Translate the triangle in the z axis
			triTranslated = triZXRotated;
			triTranslated.p[0].z = triZXRotated.p[0].z + 8.0f;
			triTranslated.p[1].z = triZXRotated.p[1].z + 8.0f;
			triTranslated.p[2].z = triZXRotated.p[2].z + 8.0f;

			//calculating normal
			vec3d line1, line2, normal;
			line1 = triTranslated.p[1] - triTranslated.p[0];
			line2 = triTranslated.p[2] - triTranslated.p[0];
			normal = cross(line1, line2);

			//normalize the normal
			normal /= normal.length();

			float dp = dot(normal, (triTranslated.p[0] - vCamera));

			if (dp < 0.0f) {

				//project to 2D space
				MultiplyMatrixTriangle(triTranslated, triProjected, projMatrix);

				//Translate to screen space
				triProjected.p[0] += 1.0f;
				triProjected.p[1] += 1.0f;
				triProjected.p[2] += 1.0f;

				//scale to screen space
				triProjected.p[0].x *= 0.5f * screenWidth;
				triProjected.p[0].y *= 0.5f * screenHeight;
				triProjected.p[1].x *= 0.5f * screenWidth;
				triProjected.p[1].y *= 0.5f * screenHeight;
				triProjected.p[2].x *= 0.5f * screenWidth;
				triProjected.p[2].y *= 0.5f * screenHeight;

				//store normal data in the projected triangles
				triProjected.normal = normal;


				//add the projected triangles to a new vector
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
			light_direction /= light_direction.length();

			//get similarity b/w normal and light_direction
			float dp = dot(triProjected.normal, light_direction);

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
