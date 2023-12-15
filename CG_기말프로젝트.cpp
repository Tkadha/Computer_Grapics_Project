#define  _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/ext.hpp>
#include <gl/glm.hpp>
#include <gl/ext/matrix_transform.hpp>
#include <stdlib.h>
#include <random>
#include <fstream>
#include <algorithm>
#include "stb_image.h"

#define Width 1200
#define Height 800
#define Wall_count 13
#define Clear_Wall_count 4
#define Floor_count 10
#define Button_count 4
#define Glass_count 18

void make_vertexShaders();
void make_fragmentShaders();
void make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
char* filetobuf(const char* file);
void InitBuffer();
GLvoid Keyboard(unsigned char key, int x, int y);
GLvoid UpKeyboard(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void pos_change(int win_x, int win_y, float* gl_x, float* gl_y);
glm::vec3 set_dir(float yaw, float pitch);
void move(int);
void move_floor(int);
void button_collision(int);
void drop(int);
void bullet_move(int);
void portal_update(int);
bool collision(glm::vec3 camera_positon);
void elevator(int value);
void cube_collision(float yaw, float pitch);
bool is_in_cube(glm::vec3 pos);
bool collision_check(glm::vec3, glm::vec3, glm::vec3, glm::vec3);
void render();
bool in_endzone();


GLchar* vertexSource, * fragmentSource; //--- 소스코드 저장 변수
GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint shaderProgramID;
GLuint vao;


std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> random_color(0.1, 1);
std::uniform_real_distribution<float> distribution(0.0025f, 0.005f);

const float tall = 0.75f;
glm::vec3 cha_size = { .3f,.75f,.3f };

class Shape {
public:
	GLuint vbo[4];
	int vertex_count;
	unsigned int texturemap;
	int width_image, height_image, number_of_channel;
	unsigned char* data;

	void Load_Obj(const char* path)
	{
		int v_count = 0;
		int n_count = 0;
		int f_count = 0;
		int vt_count = 0;
		std::string lineHeader;
		std::ifstream in(path);
		if (!in) {
			std::cerr << path << "파일 못찾음";
			exit(1);
		}
		while (in >> lineHeader) {
			if (lineHeader == "v")	++v_count;
			else if (lineHeader == "f")	++f_count;
			else if (lineHeader == "vt") ++vt_count;
		}
		in.close();
		in.open(path);

		glm::vec3* vertex = new glm::vec3[v_count];
		glm::vec3* face = new glm::vec3[f_count];
		glm::vec3* face_text = new glm::vec3[f_count];
		glm::vec2* texture = new glm::vec2[vt_count];
		glm::vec3* vertexdata = new glm::vec3[f_count * 3];
		glm::vec3* normaldata = new glm::vec3[f_count * 3];
		glm::vec4* colordata = new glm::vec4[f_count * 3];
		glm::vec2* texdata = new glm::vec2[f_count * 3];
		vertex_count = f_count * 3;
		int v_incount = 0;
		int vt_incount = 0;
		int f_incount = 0;
		int color_count = 0;
		while (in >> lineHeader) {
			if (lineHeader == "v") {
				in >> vertex[v_incount].x >> vertex[v_incount].y >> vertex[v_incount].z;
				++v_incount;
			}
			else if (lineHeader == "vt") {
				in >> texture[vt_incount].x >> texture[vt_incount].y;
				++vt_incount;
			}
			else if (lineHeader == "f") {
				in >> face[f_incount].x >> face[f_incount].y >> face[f_incount].z >> face_text[f_incount].x >> face_text[f_incount].y >> face_text[f_incount].z;
				vertexdata[f_incount * 3 + 0] = vertex[static_cast<int>(face[f_incount].x - 1)];
				vertexdata[f_incount * 3 + 1] = vertex[static_cast<int>(face[f_incount].y - 1)];
				vertexdata[f_incount * 3 + 2] = vertex[static_cast<int>(face[f_incount].z - 1)];

				texdata[f_incount * 3 + 0] = texture[static_cast<int>(face_text[f_incount].x - 1)];
				texdata[f_incount * 3 + 1] = texture[static_cast<int>(face_text[f_incount].y - 1)];
				texdata[f_incount * 3 + 2] = texture[static_cast<int>(face_text[f_incount].z - 1)];
				++f_incount;
			}
		}
		for (int i = 0; i < vertex_count / 3; i++)
		{
			glm::vec3 normal = glm::cross(vertexdata[i * 3 + 1] - vertexdata[i * 3 + 0], vertexdata[i * 3 + 2] - vertexdata[i * 3 + 0]);
			normaldata[i * 3 + 0] = normal;
			normaldata[i * 3 + 1] = normal;
			normaldata[i * 3 + 2] = normal;

		}
		for (int i = 0; i < vertex_count; ++i) {
			colordata[color_count].x = random_color(gen);
			colordata[color_count].y = random_color(gen);
			colordata[color_count].z = random_color(gen);
			colordata[color_count].a = 1.0f;
			color_count++;
		}

		glGenBuffers(4, vbo);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), vertexdata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec3), normaldata, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec2), texdata, GL_STATIC_DRAW);
		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(3);

		delete[] vertex;
		vertex = nullptr;
		delete[] face;
		face = nullptr;
		delete[] face_text;
		face_text = nullptr;
		delete[] vertexdata;
		vertexdata = nullptr;
		delete[] normaldata;
		normaldata = nullptr;
		delete[] colordata;
		colordata = nullptr;
		delete[] texdata;
		texdata = nullptr;
	}
	void Set_color(float r, float g, float b) {
		glm::vec4* colordata = new glm::vec4[vertex_count];
		for (int i = 0; i < vertex_count; ++i) {
			colordata[i] = { r,g,b,1.0f };
		}
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
		delete[] colordata;
		colordata = nullptr;
	}
	void Set_alpha(float r, float g, float b, float alp) {
		glm::vec4* colordata = new glm::vec4[vertex_count];
		for (int i = 0; i < vertex_count; ++i) {
			colordata[i] = { r,g,b,alp };
		}
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(glm::vec4), colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(1);
		delete[] colordata;
		colordata = nullptr;
	}
	void Create_texture(const char* path) {
		glGenTextures(1, &texturemap);
		glBindTexture(GL_TEXTURE_2D, texturemap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		data = stbi_load(path, &width_image, &height_image, &number_of_channel, 0);
		if (data)
		{
			if (number_of_channel == 4) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_image, height_image, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			}
			else {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width_image, height_image, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

			}
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
	}
	void Draw_shape() {
		int PosLocation = glGetAttribLocation(shaderProgramID, "in_Position"); //	: 0
		int ColorLocation = glGetAttribLocation(shaderProgramID, "in_Color"); //	: 1
		int NormalLocation = glGetAttribLocation(shaderProgramID, "in_Normal"); //	: 2
		int TextureLocation = glGetAttribLocation(shaderProgramID, "vTexCoord"); //	: 3
		glEnableVertexAttribArray(PosLocation);
		glEnableVertexAttribArray(ColorLocation);
		glEnableVertexAttribArray(NormalLocation);
		glEnableVertexAttribArray(TextureLocation);

		glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
		glVertexAttribPointer(PosLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
		glVertexAttribPointer(ColorLocation, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
		glVertexAttribPointer(NormalLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
		glVertexAttribPointer(TextureLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
		glBindTexture(GL_TEXTURE_2D, texturemap);
		glDrawArrays(GL_TRIANGLES, 0, vertex_count);

		glDisableVertexAttribArray(ColorLocation);
		glDisableVertexAttribArray(PosLocation);
		glDisableVertexAttribArray(NormalLocation);
		glDisableVertexAttribArray(TextureLocation);
	}
};

class Floor :public Shape {
public:
	glm::vec3 pos;
	glm::vec3 scale;
	float addy;
	void translate() {
		glm::mat4 trans = glm::mat4(1.0f);
		unsigned int shape_location = glGetUniformLocation(shaderProgramID, "transform");
		trans = glm::translate(trans, glm::vec3(pos));
		trans = glm::scale(trans, glm::vec3(scale));
		glUniformMatrix4fv(shape_location, 1, GL_FALSE, glm::value_ptr(trans));
	}
};
class Wall : public Shape {
public:
	glm::vec3 pos;
	glm::vec3 scale;
	bool see;
	void translate() {
		glm::mat4 trans = glm::mat4(1.0f);
		unsigned int shape_location = glGetUniformLocation(shaderProgramID, "transform");
		trans = glm::translate(trans, glm::vec3(pos));
		trans = glm::scale(trans, glm::vec3(scale));
		glUniformMatrix4fv(shape_location, 1, GL_FALSE, glm::value_ptr(trans));
	}
};
class Cube : public Shape {
public:
	glm::vec3 pos;
	glm::vec3 scale;
	float dropspeed = -0.003f;

	bool grab;
	void translate() {
		glm::mat4 trans = glm::mat4(1.0f);
		unsigned int shape_location = glGetUniformLocation(shaderProgramID, "transform");
		trans = glm::translate(trans, glm::vec3(pos));
		trans = glm::scale(trans, glm::vec3(scale));
		glUniformMatrix4fv(shape_location, 1, GL_FALSE, glm::value_ptr(trans));
	}

	void update(glm::vec3 p, glm::vec3 dir) {
		if (grab) {
			pos = p + dir * 1.25f;
		}
	}
};
class Button : public Shape {
public:
	glm::vec3 pos;
	glm::vec3 scale;
	bool push;

	void translate() {
		glm::mat4 trans = glm::mat4(1.0f);
		unsigned int shape_location = glGetUniformLocation(shaderProgramID, "transform");
		trans = glm::translate(trans, glm::vec3(pos));
		trans = glm::scale(trans, glm::vec3(scale));
		glUniformMatrix4fv(shape_location, 1, GL_FALSE, glm::value_ptr(trans));
	}

};
class Glass :public Shape {
public:
	glm::vec3 pos;
	glm::vec3 scale;
	void translate() {
		glm::mat4 trans = glm::mat4(1.0f);
		unsigned int shape_location = glGetUniformLocation(shaderProgramID, "transform");
		trans = glm::translate(trans, glm::vec3(pos));
		trans = glm::scale(trans, glm::vec3(scale));
		glUniformMatrix4fv(shape_location, 1, GL_FALSE, glm::value_ptr(trans));
	}
};
class Cake :public Shape {
public:
	glm::vec3 pos;
	glm::vec3 scale;
	void translate() {
		glm::mat4 trans = glm::mat4(1.0f);
		unsigned int shape_location = glGetUniformLocation(shaderProgramID, "transform");
		trans = glm::translate(trans, glm::vec3(pos));
		trans = glm::scale(trans, glm::vec3(scale));
		glUniformMatrix4fv(shape_location, 1, GL_FALSE, glm::value_ptr(trans));
	}
};
class Bullet : public Shape {
public:
	glm::vec3 pos;
	glm::vec3 scale;
	glm::vec3 dir;
	bool render = false;
	const float speed = 0.1f;

	void update() {
		pos += dir * speed;
	}

	bool collision_check(glm::vec3 o_pos, glm::vec3 o_scale) {
		bool xCollision = std::abs(pos.x - o_pos.x) * 2 < (scale.x + o_scale.x);
		bool yCollision = std::abs(pos.y - o_pos.y) * 2 < (scale.y + o_scale.y);
		bool zCollision = std::abs(pos.z - o_pos.z) * 2 < (scale.z + o_scale.z);

		return xCollision && yCollision && zCollision;
	}

	glm::vec3 collision_dir(glm::vec3 o_pos, glm::vec3 o_scale) {
		glm::vec3 dir{ 0,0,0 };
		float x = (scale.x + o_scale.x) - std::abs(pos.x - o_pos.x) * 2;
		float y = (scale.y + o_scale.y) - std::abs(pos.y - o_pos.y) * 2;
		float z = (scale.z + o_scale.z) - std::abs(pos.z - o_pos.z) * 2;
		//if (std::min(std::min(x, y),z) == x) {
		if (std::min(x, z) == x) {
			if (pos.x > o_pos.x)
				dir.x = 0.5f;
			else
				dir.x = -0.5f;
		}/*
		else if (std::min(std::min(x, y), z) == y) {
			if (pos.y > o_pos.y)
				dir.y = 1;
			else
				dir.y = -1;
		}*/
		//else if (std::min(std::min(x, y), z) == z) {
		else if (std::min(x, z) == z) {
			if (pos.z > o_pos.z)
				dir.z = 0.5f;
			else
				dir.z = -0.5f;
		}

		return dir;
	}

	void translate() {
		glm::mat4 trans = glm::mat4(1.0f);
		unsigned int shape_location = glGetUniformLocation(shaderProgramID, "transform");
		trans = glm::translate(trans, glm::vec3(pos));
		trans = glm::scale(trans, glm::vec3(scale));
		glUniformMatrix4fv(shape_location, 1, GL_FALSE, glm::value_ptr(trans));
	}
};

class Portal : public Shape {
public:
	glm::vec3 pos;
	glm::vec3 scale;
	glm::vec3 dir;
	bool render = false;

	bool pair = false;
	glm::vec3 pair_portal_pos, pair_portal_dir;

	void teleport(glm::vec3* pos, float* yaw) {
		if (pair) {
			*pos = pair_portal_pos + pair_portal_dir * 1.5f;
			(*pos).y += tall / 2;
			if ((dir.x == pair_portal_dir.x) && (dir.z == pair_portal_dir.z))
				*yaw += 180;
			else if (dir.x == pair_portal_dir.z)
				*yaw += 270;
			else if (-dir.x == pair_portal_dir.z)
				*yaw -= 270;
		}

	}

	bool collision_check(glm::vec3 p) {
		bool xCollision = std::abs(pos.x - p.x) * 2 < (scale.x + 0.25f);
		bool yCollision = std::abs(pos.y - p.y) * 2 < 1.0f;
		bool zCollision = std::abs(pos.z - p.z) * 2 < (scale.z + 0.25f);

		return xCollision && yCollision && zCollision;
	}

	void translate() {
		glm::mat4 trans = glm::mat4(1.0f);
		unsigned int shape_location = glGetUniformLocation(shaderProgramID, "transform");
		trans = glm::translate(trans, glm::vec3(pos));
		trans = glm::scale(trans, glm::vec3(scale));
		glUniformMatrix4fv(shape_location, 1, GL_FALSE, glm::value_ptr(trans));
	}

};

glm::vec3 light_pos;
glm::vec3 light_color;
glm::vec3 camera_pos;

// 기본 맵
Floor ground[10][10];
Floor ceiling[10][10];        // 천장
Wall map_wall[4][20][10];
// 벽들
Wall clear_wall[Clear_Wall_count];
Wall walls[Wall_count];

// 바닥
Floor floors[Floor_count];

// 들고 다닐수 있는 큐브
Cube cube;

// 버튼
Button button[Button_count];

// 유리
Glass glass[Glass_count];

// 케이크
Cake cake;

// 포탈건
//PortalGun portalgun;

// 준하

int UD;
int LR;


float dropspeed = -0.003f;
bool ghost = false;



// 태경

// 포탈에서 나가는 구체(임시)
Bullet blue_bullet, red_bullet;
Portal blue, red;

GLfloat window_w = Width, window_h = Height;	// 마우스 입력 좌표 변환할 때 창 크기에 따라 하기 위해서 쓰임 
float speed = 0.05f;		// 주인공 이동속도

	//gpt는 신이야
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);	// 일인칭 카메라 방향 바꾸려고 추가

float cameraSpeed = 0.05f; // 조절 가능한 카메라 이동 속도
float sensitivity = 0.05f; // 조절 가능한 마우스 감도

float yaw = 10.0f; // 카메라의 초기 yaw 각도
float pitch = 10.0f;  // 카메라의 초기 pitch 각도

bool firstMouse = true;
float lastX = 400.0f;
float lastY = 300.0f;

// 선을 그릴 때 사용할 점의 좌표


void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(window_w, window_h);
	glutCreateWindow("Project_Portal");
	glewExperimental = GL_TRUE;
	glewInit();
	make_shaderProgram();

	{	//설명		
		std::cout << "q/Q: 프로그램 종료" << std::endl;
	}

	{
		light_color = { 0.4f,0.4f,0.4f };
		light_pos = { 0.f,12.f,0.f };
		camera_pos = { -4.5f, 1.f, -4.5f };
	}
	blue_bullet.render = false;
	red_bullet.render = false;
	blue.render = false;
	red.render = false;

	InitBuffer();

	glutTimerFunc(10, move, 0);
	glutTimerFunc(10, move_floor, 1);
	glutTimerFunc(10, button_collision, 0);
	glutTimerFunc(10, drop, 0);
	glutTimerFunc(10, bullet_move, 0);
	glutTimerFunc(10, portal_update, 0);
	glutTimerFunc(10, elevator, 0);

	//--- 세이더 읽어와서 세이더 프로그램 만들기
	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutKeyboardUpFunc(UpKeyboard);
	glutMouseFunc(Mouse);
	glutPassiveMotionFunc(Motion);	// 마우스 이동 항상 입력받기
	glutMainLoop();
}
GLvoid drawScene() //--- 콜백 함수: 그리기 콜백 함수
{
	GLfloat rColor, gColor, bColor;
	rColor = gColor = bColor = 0.7f;
	glClearColor(rColor, gColor, bColor, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);
	glBindVertexArray(vao);

	ShowCursor(false);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	unsigned int proj_location = glGetUniformLocation(shaderProgramID, "projection");
	unsigned int view_location = glGetUniformLocation(shaderProgramID, "view");

	unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos");
	glUniform3f(lightPosLocation, light_pos.x, light_pos.y, light_pos.z);
	unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor");
	glUniform3f(lightColorLocation, light_color.x, light_color.y, light_color.z);
	unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos");
	glUniform3f(viewPosLocation, light_pos.x, light_pos.y, light_pos.z);

	{
		//glm::mat4 ortho = glm::ortho(0.0f, window_w, 0.0f, window_h, -1.0f, 1.0f);
		//glUniformMatrix4fv(proj_location, 1, GL_FALSE, &ortho[0][0]);

		//// 뷰 행렬 (2D 오버레이)
		//glm::mat4 overlay_view = glm::mat4(1.0f); // 2D 이미지는 카메라 이동이 필요 없으므로 항등 행렬을 사용합니다.
		//glUniformMatrix4fv(view_location, 1, GL_FALSE, &overlay_view[0][0]);

		//glDisable(GL_CULL_FACE);
		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//portalgun.translate();
		//portalgun.Draw_shape();

		//glDisable(GL_BLEND);
	}


	glViewport(0, 0, window_w, window_h);
	//투영행렬
	glm::mat4 proj = glm::mat4(1.0f);
	proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);
	proj = glm::translate(proj, glm::vec3(0.f, 0.f, 0.f));
	glUniformMatrix4fv(proj_location, 1, GL_FALSE, &proj[0][0]);

	//뷰 행렬
	glm::vec3 cameraPos = glm::vec3(camera_pos); //--- 카메라 위치
	//glm::vec3 cameraDirection = glm::vec3(camera_pos.x - 2.0f, camera_pos.y, 0.f); //--- 카메라 바라보는 방향
	glm::vec3 cameraDirection = camera_pos + cameraFront; // fps만들려고 바꿨음 (gpt님님님이 이렇게 하라해서)
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향
	glm::mat4 view = glm::mat4(1.0f);
	view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	glUniformMatrix4fv(view_location, 1, GL_FALSE, &view[0][0]);

	render();

	// blue portal
	if (blue.render) {
		glViewport(window_w * 7 / 8, window_h * 7 / 8, window_w / 8, window_h / 8);
		{
			//투영행렬
			glm::mat4 proj = glm::mat4(1.0f);
			proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);
			proj = glm::translate(proj, glm::vec3(0.f, 0.f, 0.f));
			glUniformMatrix4fv(proj_location, 1, GL_FALSE, &proj[0][0]);

			//뷰 행렬
			glm::vec3 cameraPos = glm::vec3(red.pos + red.dir); //--- 카메라 위치
			glm::vec3 cameraDirection = red.dir; // fps만들려고 바꿨음 (gpt님님님이 이렇게 하라해서)
			glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향
			glm::mat4 view = glm::mat4(1.0f);
			view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
			glUniformMatrix4fv(view_location, 1, GL_FALSE, &view[0][0]);

			render();
		}
	}

	// red portal
	if (blue.render) {
		glViewport(0, window_h * 7 / 8, window_w / 8, window_h / 8);
		{
			//투영행렬
			glm::mat4 proj = glm::mat4(1.0f);
			proj = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 50.0f);
			proj = glm::translate(proj, glm::vec3(0.f, 0.f, 0.f));
			glUniformMatrix4fv(proj_location, 1, GL_FALSE, &proj[0][0]);

			//뷰 행렬
			glm::vec3 cameraPos = glm::vec3(blue.pos + blue.dir); //--- 카메라 위치
			glm::vec3 cameraDirection = blue.dir; // fps만들려고 바꿨음 (gpt님님님이 이렇게 하라해서)
			glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향
			glm::mat4 view = glm::mat4(1.0f);
			view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
			glUniformMatrix4fv(view_location, 1, GL_FALSE, &view[0][0]);

			render();
		}
	}

	glutSwapBuffers(); // 화면에 출력하기
}
GLvoid Reshape(int w, int h) //--- 콜백 함수: 다시 그리기 콜백 함수
{
	glViewport(0, 0, w, h);
	window_w = w;
	window_h = h;
}
void make_vertexShaders()
{
	vertexSource = filetobuf("vertex_project.glsl");
	//--- 버텍스 세이더 객체 만들기
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(vertexShader, 1, (const GLchar**)&vertexSource, 0);
	//--- 버텍스 세이더 컴파일하기
	glCompileShader(vertexShader);
	//--- 컴파일이 제대로 되지 않은 경우: 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cout << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}
void make_fragmentShaders()
{
	fragmentSource = filetobuf("fragment_project.glsl");
	//--- 프래그먼트 세이더 객체 만들기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	//--- 세이더 코드를 세이더 객체에 넣기
	glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentSource, 0);
	//--- 프래그먼트 세이더 컴파일
	glCompileShader(fragmentShader);
	//--- 컴파일이 제대로 되지 않은 경우: 컴파일 에러 체크
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: fragment shader 컴파일 실패\n" << errorLog << std::endl;
		return;
	}
}
void make_shaderProgram()
{
	make_vertexShaders(); //--- 버텍스 세이더 만들기
	make_fragmentShaders(); //--- 프래그먼트 세이더 만들기
	//-- shader Program
	shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertexShader);
	glAttachShader(shaderProgramID, fragmentShader);
	glLinkProgram(shaderProgramID);
	//--- 세이더 삭제하기
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	//--- Shader Program 사용하기
	glUseProgram(shaderProgramID);
}
char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading
	if (!fptr) // Return NULL on failure
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	length = ftell(fptr); // Find out how many bytes into the file we are
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer
	fclose(fptr); // Close the file
	buf[length] = 0; // Null terminator
	return buf; // Return the buffer
}
void InitBuffer() {

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	{
		for (int i = 0; i < 10; ++i) {
			for (int j = 0; j < 10; ++j) {
				ceiling[i][j].Load_Obj("cube_floor.obj");
				ceiling[i][j].Set_color(1.f, 1.f, 1.f);
				ceiling[i][j].Create_texture("./resource/floor_1.jpg");
				ceiling[i][j].pos = { 0.5f + 1.f * (j - 5),10.f,0.5f + 1.f * (i - 5) };
				ceiling[i][j].scale = { 1.f,1.f,1.f };
				ground[i][j].Load_Obj("cube_floor.obj");
				ground[i][j].Set_color(1.f, 1.f, 1.f);
				ground[i][j].Create_texture("./resource/floor_1.jpg");
				ground[i][j].pos = { 0.5f + 1.f * (j - 5),-1.f,0.5f + 1.f * (i - 5) };
				ground[i][j].scale = { 1.f,1.f,1.f };
			}
		}
		for (int i = 0; i < 20; ++i) {
			for (int j = 0; j < 10; ++j) {
				map_wall[0][i][j].Load_Obj("cube_floor.obj");
				map_wall[0][i][j].Set_color(1.f, 1.f, 1.f);
				map_wall[0][i][j].Create_texture("./resource/wall_2.jpg");
				map_wall[0][i][j].pos = { -5.5f, i * 1.f, 0.5f + 1.f * (j - 5) };
				map_wall[0][i][j].scale = { 1.f,1.f,1.f };
				map_wall[1][i][j].Load_Obj("cube_floor.obj");
				map_wall[1][i][j].Set_color(1.f, 1.f, 1.f);
				map_wall[1][i][j].Create_texture("./resource/wall_2.jpg");
				map_wall[1][i][j].pos = { 5.5f, i * 1.f, 0.5f + 1.f * (j - 5) };
				map_wall[1][i][j].scale = { 1.f,1.f,1.f };
				map_wall[2][i][j].Load_Obj("cube_floor.obj");
				map_wall[2][i][j].Set_color(1.f, 1.f, 1.f);
				map_wall[2][i][j].Create_texture("./resource/wall_2.jpg");
				map_wall[2][i][j].pos = { 0.5f + 1.f * (j - 5), i * 1.f, -5.5f };
				map_wall[2][i][j].scale = { 1.f,1.f,1.f };
				map_wall[3][i][j].Load_Obj("cube_floor.obj");
				map_wall[3][i][j].Set_color(1.f, 1.f, 1.f);
				map_wall[3][i][j].Create_texture("./resource/wall_2.jpg");
				map_wall[3][i][j].pos = { 0.5f + 1.f * (j - 5), i * 1.f, 5.5f };
				map_wall[3][i][j].scale = { 1.f,1.f,1.f };
			}
		}
	}


	for (int i = 0; i < Clear_Wall_count; ++i) {
		clear_wall[i].Load_Obj("cube_floor.obj");
		clear_wall[i].Set_alpha(0.44f, 0.77f, 0.92f, 0.4f);
		clear_wall[i].Create_texture("./resource/white.png");
		clear_wall[i].see = true;
	}
	{
		clear_wall[0].pos = { -2.5f - (0.125f / 2),0.f - 0.125f - 0.00001f,-3.75f - (0.125f / 2) };
		clear_wall[0].scale = { 0.125f, 2.5f, 2.5f };

		clear_wall[1].pos = { -2.5f - 0.00001f , 8.f, -2.5f + 0.001f };
		clear_wall[1].scale = { 5.f, 2.f, 0.125f };

		clear_wall[2].pos = { -3.75f - 0.000001f,8.f + 0.000001f, 0.f };
		clear_wall[2].scale = { 2.5f, 2.f, 0.125f };

	}


	for (int i = 0; i < Wall_count; ++i) {
		walls[i].Load_Obj("cube_floor.obj");
		walls[i].Set_color(0.9f, 0.9f, 0.9f);
		walls[i].Create_texture("./resource/metal_wall.jpg");
	}
	{
		walls[0].pos = { -2.5f,0.f,-2.5f };
		walls[0].scale = { 5.f, 8.f, 0.125f };

		walls[1].pos = { 0.f,0.f, -3.75f };
		walls[1].scale = { 0.125f, 10.f, 2.5f };	// 세로

		walls[2].pos = { 0.f,0.f, -1.25f };
		walls[2].scale = { 0.125f, 10.f, 2.5f };

		walls[3].pos = { -1.25f,0.f, 0.f };
		walls[3].scale = { 2.5f, 10.f, 0.125f };	// 가로

		walls[4].pos = { -3.75f,0.f, 0.f };
		walls[4].scale = { 2.5f, 8.f, 0.125f };

		walls[5].pos = { -2.5f,0.f, 3.75f };
		walls[5].scale = { 0.125f, 10.f, 2.5f };

		walls[6].pos = { -2.5f,8.f, 1.25f };
		walls[6].scale = { 0.125f, 2.f, 2.5f };

		walls[7].pos = { -3.75f - 0.125f / 2,0.f, 2.5f + 0.125f / 2 };
		walls[7].scale = { 2.5f, 6.f, 0.125f };

		walls[8].pos = { 0.f,0.f, 3.75f };
		walls[8].scale = { 0.125f, 4.f, 2.5f };

		walls[9].pos = { 0.f,0.f, 1.25f };
		walls[9].scale = { 0.125f, 6.f, 2.5f };

		walls[10].pos = { -1.25f,4.f, 2.5f };
		walls[10].scale = { 2.5f, 2.f, 0.125f };

		walls[11].pos = { 3.75f,0.f, -3.75f - 0.625f };
		walls[11].scale = { 0.125f, 10.f, 1.25f };

		walls[12].pos = { 3.75f + 0.625f ,2.f, -3.75f };
		walls[12].scale = { 1.25f, 8.f, 0.125f };
	}


	for (int i = 0; i < Floor_count; ++i) {
		floors[i].Load_Obj("cube_floor.obj");
		floors[i].Set_color(0.9f, 0.9f, 0.9f);
		floors[i].Create_texture("./resource/metal_wall.jpg");
	}
	{
		floors[0].pos = { -3.75f,2.5f - 0.125f,-3.75f - (0.125f / 2) };
		floors[0].scale = { 2.5f, 0.125f, 2.5f };

		//움직이는 발판
		floors[1].pos = { -3.75f, 8.f - 0.125f, 3.75f };
		floors[1].scale = { 2.5f, 0.125f, 2.5f };
		floors[1].addy = -0.01f;

		floors[2].pos = { -1.25f, 5.f - 0.125f, -3.75f - (0.125f / 2) };
		floors[2].scale = { 2.5f, 0.125f, 2.5f };

		floors[3].pos = { -3.75f, 8.f - 0.125f, -3.75f - (0.125f / 2) };
		floors[3].scale = { 2.5f, 0.125f, 2.5f };

		floors[4].pos = { -3.75f, 8.f - 0.125f, 1.25f };
		floors[4].scale = { 2.5f, 0.125f, 2.5f };

		floors[5].pos = { -3.25f, 5.f, -1.25f };
		floors[5].scale = { 3.5f, 0.125f, 2.5f };

		floors[6].pos = { -2.5f, 6.f, 1.25f };
		floors[6].scale = { 5.f, 0.125f, 2.5f };

		floors[7].pos = { -1.25f, 4.f, 3.75f };
		floors[7].scale = { 2.5f, 0.125f, 2.5f };

		floors[8].pos = { -3.75f, 6.f, 3.75f };
		floors[8].scale = { 2.5f, 0.125f, 2.5f };

		floors[9].pos = { 4.5f, 0.f - 0.125f + 0.001f, -4.5f };
		floors[9].scale = { 1.25f, 0.125f, 1.25f };
		floors[9].addy = 0.01f;
	}

	for (int i = 0; i < Button_count; ++i) {
		button[i].Load_Obj("cube_floor.obj");
		button[i].Set_color(1.0f, 0.2f, 0.2f);
		button[i].Create_texture("./resource/white.png");
	}
	{
		button[0].pos = { -3.75f - 0.625f ,0.f, -3.75f + 0.625f };
		button[0].scale = { 0.75f, 0.125f, 0.75f };
		button[0].push = false;

		button[1].pos = { -3.75f - 0.625f ,8.f, -3.75f + 0.625f };
		button[1].scale = { 0.75f, 0.125f, 0.75f };
		button[1].push = false;

		button[2].pos = { -2.0f, 0.f, -1.25f };
		button[2].scale = { 0.75f, 0.125f, 0.75f };
		button[2].push = false;

		button[3].pos = { 0.625f, 8.f + 0.125f, -3.75f - 0.625f };
		button[3].scale = { 0.5f, 0.125f, 0.5f };
		button[3].push = false;
	}

	for (int i = 0; i < Glass_count; ++i) {
		glass[i].Load_Obj("cube_floor.obj");
		glass[i].Set_alpha(1.f, 1.f, 1.f, 0.5f);
		glass[i].Create_texture("./resource/white.png");
	}
	{
		glass[0].pos = { 0.f,6.f, 2.5f };
		glass[0].scale = { 0.125f, 4.f, 5.f };

		glass[1].pos = { 0.625f, 8.f, -3.75f - 0.625f };
		glass[1].scale = { 1.f, 0.125f, 1.f };

		glass[2].pos = { 1.25f, 0.f, -3.75f };
		glass[2].scale = { 2.5f, 10.f, 0.125f };

		glass[3].pos = { 2.5f + 0.625f, 2.f, -3.75f };
		glass[3].scale = { 1.25f, 6.f, 0.125f };

		glass[4].pos = { 3.75f, 0.f, -2.5f };
		glass[4].scale = { 2.5f, 2.f, 0.125f };

		glass[5].pos = { 2.5f, 0.f, -2.5f - 0.625f };
		glass[5].scale = { 0.125f, 2.f, 1.25f };

		glass[6].pos = { 3.75f, 2.f, -2.5f - 0.625f };
		glass[6].scale = { 2.5f, 0.125f, 1.25f };

		glass[7].pos = { 2.5f + 0.625f, 8.f, -2.5f - 0.125f };
		glass[7].scale = { 1.25f, 2.f, 0.125f };

		glass[8].pos = { 2.5f , 8.f, -3.75f + 0.625f };
		glass[8].scale = { 0.125f, 2.f, 1.25f };

		glass[9].pos = { 2.5f + 0.625f, 8.f, -3.75f - 0.625f };
		glass[9].scale = { 1.25f, 0.125f, 1.25f };

		glass[10].pos = { 3.75f + 0.625f, 7.f, 0.f };
		glass[10].scale = { 1.25f, 0.125f, 1.25f };

		glass[11].pos = { 0.625f, 6.5f, -1.25f };
		glass[11].scale = { 1.25f, 0.125f, 1.25f };

		glass[12].pos = { 3.75f + 0.625f, 5.f, 3.75f };
		glass[12].scale = { 1.25f, 0.125f, 1.25f };

		glass[13].pos = { 2.5f, 5.f, -2.5f };
		glass[13].scale = { 5.f, 0.125f, 1.25f };

		glass[14].pos = { 2.5f, 5.f, -2.5f + 0.625f };
		glass[14].scale = { 5.f, 3.f, 0.125f };

		glass[15].pos = { 2.5f + 0.625f , 8.f, -3.75f + 0.625f };
		glass[15].scale = { 1.25f, 0.125f, 1.25f };

		glass[16].pos = { 3.75f + 0.625f , 7.f, 0.f + 0.625f };
		glass[16].scale = { 1.25f, 3.f, 0.125f };

		glass[17].pos = { 3.75f , 7.f, 0.f + 0.625f / 2 };
		glass[17].scale = { 0.125f, 3.f, 0.625f };

	}


	cube.Load_Obj("cube_floor.obj");
	cube.Set_color(1.f, 1.f, 1.f);
	cube.Create_texture("./resource/cube_face.png");
	cube.pos = { -4.f,0.f,-4.f };
	cube.scale = { 0.5f, 0.5f, 0.5f };

	cake.Load_Obj("cake.obj");
	cake.Set_color(1.f, 1.f, 1.f);
	cake.Create_texture("./resource/cake.png");
	cake.pos = { 0.f,11.f,0.f };
	cake.scale = { 2.f, 0.5f, 2.f };

	//portalgun.Load_Obj("plane.obj");
	//portalgun.Set_color(1.f, 1.f, 1.f);
	//portalgun.Create_texture("./resource/portal_gun.png");
	//portalgun.pos = { -4.f,0.f,-4.f };
	//portalgun.scale = { 1.f, 1.f, 1.f };


	blue_bullet.Load_Obj("cube_floor.obj");
	blue_bullet.Set_alpha(0.f, 0.f, 1.f, 0.5f);
	blue_bullet.Create_texture("./resource/white.png");
	blue_bullet.scale = { 0.05f, 0.05f, 0.05f };

	red_bullet.Load_Obj("cube_floor.obj");
	red_bullet.Set_alpha(1.f, 0.f, 0.f, 0.5f);
	red_bullet.Create_texture("./resource/white.png");
	red_bullet.scale = { 0.05f, 0.05f, 0.05f };

	blue.Load_Obj("cube_floor.obj");
	blue.Set_alpha(0.f, 0.f, 1.f, 0.5f);
	blue.Create_texture("./resource/white.png");
	blue.scale = { 0.5f, 0.5f, 0.5f };
	blue.render = false;

	red.Load_Obj("cube_floor.obj");
	red.Set_alpha(1.f, 0.f, 0.f, 0.5f);
	red.Create_texture("./resource/white.png");
	red.scale = { 0.5f, 0.5f, 0.5f };
	red.render = false;

}
GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'q':
	case 'Q':
		exit(0);
		break;
	case 's':
		if (UD == 0)	UD = -1;
		break;
	case 'w':
		if (UD == 0)	UD = 1;
		break;
	case 'a':
		if (LR == 0)	LR = -1;
		break;
	case 'd':
		if (LR == 0)	LR = 1;
		break;

	case 'f':
		if (cube.grab)
			cube.grab = false;
		else
			cube_collision(yaw, pitch);
		break;
	case 'r':
		cube.pos = camera_pos + cameraFront * 1.25f;
		break;
	case 'g':
		if (ghost)
			ghost = false;
		else
			ghost = true;
		break;
	case 'l':
		camera_pos = { -4.5f, 1.f, -4.5f };
		cube.pos = { -4.f,0.f,-4.f };
		blue_bullet.render = false;
		red_bullet.render = false;
		blue.render = false;
		red.render = false;
	}
	glutPostRedisplay();
}
GLvoid UpKeyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'w':
		if (UD == 1)	UD = 0;
		break;
	case 's':
		if (UD == -1)	UD = 0;
		break;
	case 'a':
		if (LR == -1)	LR = 0;
		break;
	case 'd':
		if (LR == 1)	LR = 0;
		break;
	}
	glutPostRedisplay();
}

void Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (!in_endzone()) {
			blue_bullet.dir = cameraFront;
			blue_bullet.pos = camera_pos;
			blue_bullet.render = true;
		}
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		if (!in_endzone()) {
			red_bullet.dir = cameraFront;
			red_bullet.pos = camera_pos;
			red_bullet.render = true;
		}
	}
	glutPostRedisplay();
}

void Motion(int x, int y)
{
	if (firstMouse) {
		lastX = static_cast<float>(x);
		lastY = static_cast<float>(y);
		firstMouse = false;
	}

	float xOffset = static_cast<float>(x - window_w / 2);
	float yOffset = static_cast<float>(window_h / 2 - y);

	xOffset *= sensitivity;
	yOffset *= sensitivity;

	yaw += xOffset;
	pitch += yOffset;

	// pitch의 범위를 제한합니다.
	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	glm::vec3 newFront;
	newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	newFront.y = sin(glm::radians(pitch));
	newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(newFront);

	lastX = static_cast<float>(x);
	lastY = static_cast<float>(y);

	glutWarpPointer(window_w / 2, window_h / 2);	// 마우스를 중앙에 고정 (gpt님님님이 알려줌)

	glutPostRedisplay();
}

glm::vec3 set_dir(float yaw, float pitch) {
	glm::vec3 dir;
	dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	dir.y = sin(glm::radians(pitch));
	dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	return dir;
}


void pos_change(int win_x, int win_y, float* gl_x, float* gl_y)
{
	*gl_x = (win_x - window_w / 2) / (window_w / 2);
	*gl_y = -(win_y - window_h / 2) / (window_h / 2);
}

void bullet_move(int value) {
	blue_bullet.update();
	red_bullet.update();

	bool blue_collide = false, red_collide = false;

	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 10; ++j) {
			if (blue_bullet.collision_check(ground[i][j].pos, ground[i][j].scale)) {
				blue.dir = blue_bullet.collision_dir(ground[i][j].pos, ground[i][j].scale);
				blue_collide = true;
			}
			else if (red_bullet.collision_check(ground[i][j].pos, ground[i][j].scale)) {
				red.dir = red_bullet.collision_dir(ground[i][j].pos, ground[i][j].scale);
				red_collide = true;
			}
		}
	}
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 10; ++j) {
			//if (i == 0 && j == 9) continue;
			if (blue_bullet.collision_check(ceiling[i][j].pos, ceiling[i][j].scale)) {
				blue.dir = blue_bullet.collision_dir(ceiling[i][j].pos, ceiling[i][j].scale);
				blue_collide = true;
			}
			if (red_bullet.collision_check(ceiling[i][j].pos, ceiling[i][j].scale)) {
				red.dir = red_bullet.collision_dir(ceiling[i][j].pos, ceiling[i][j].scale);
				red_collide = true;
			}
		}
	}

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 20; ++j) {
			for (int k = 0; k < 10; ++k) {
				if (blue_bullet.collision_check(map_wall[i][j][k].pos, map_wall[i][j][k].scale)) {
					blue.dir = blue_bullet.collision_dir(map_wall[i][j][k].pos, map_wall[i][j][k].scale);
					blue_collide = true;
				}
				if (red_bullet.collision_check(map_wall[i][j][k].pos, map_wall[i][j][k].scale)) {
					red.dir = red_bullet.collision_dir(map_wall[i][j][k].pos, map_wall[i][j][k].scale);
					red_collide = true;
				}
			}
		}
	}

	for (int i = 0; i < Wall_count; ++i) {
		if (blue_bullet.collision_check(walls[i].pos, walls[i].scale)) {
			blue.dir = blue_bullet.collision_dir(walls[i].pos, walls[i].scale);
			blue_collide = true;
		}
		if (red_bullet.collision_check(walls[i].pos, walls[i].scale)) {
			red.dir = red_bullet.collision_dir(walls[i].pos, walls[i].scale);
			red_collide = true;
		}
	}
	for (int i = 0; i < Floor_count; ++i) {
		if (blue_bullet.collision_check(floors[i].pos, floors[i].scale)) {
			blue.dir = blue_bullet.collision_dir(floors[i].pos, floors[i].scale);
			blue_collide = true;
		}
		if (red_bullet.collision_check(floors[i].pos, floors[i].scale)) {
			red.dir = red_bullet.collision_dir(floors[i].pos, floors[i].scale);
			red_collide = true;
		}
	}

	for (int i = 0; i < Clear_Wall_count; ++i) {
		if (blue_bullet.collision_check(clear_wall[i].pos, clear_wall[i].scale)) {
			if (clear_wall[i].see) {
				blue_bullet.render = false;
				blue_bullet.dir = { 0,0,0 };
			}
		}
		if (red_bullet.collision_check(clear_wall[i].pos, clear_wall[i].scale)) {
			if (clear_wall[i].see) {
				red_bullet.render = false;
				red_bullet.dir = { 0,0,0 };
			}
		}
	}


	//std::cout << collide << std::endl;
	if (blue_collide) {
		blue_bullet.render = false;
		blue_bullet.dir = { 0,0,0 };

		blue.pos = blue_bullet.pos;
		blue.render = true;
	}
	if (red_collide) {
		red_bullet.render = false;
		red_bullet.dir = { 0,0,0 };

		red.pos = red_bullet.pos;
		red.render = true;
	}

	glutTimerFunc(10, bullet_move, value);
	glutPostRedisplay();
}

void portal_update(int value) {
	if (blue.render && red.render) {
		blue.pair = true;
		blue.pair_portal_pos = red.pos;
		blue.pair_portal_dir = red.dir;

		red.pair = true;
		red.pair_portal_pos = blue.pos;
		red.pair_portal_dir = blue.dir;
	}
	else {
		blue.pair = false;
		red.pair = false;
	}
	glutTimerFunc(10, portal_update, value);
	glutPostRedisplay();
}

void move(int value) {
	if (!collision(camera_pos) || ghost) {
		camera_pos.x += cameraFront.x * speed * UD;
		camera_pos.z += cameraFront.z * speed * UD;
		if (LR == -1) {
			glm::vec3 dir = set_dir(yaw - 90, pitch);
			camera_pos.x += dir.x * speed * 0.7f;
			camera_pos.z += dir.z * speed * 0.7f;
		}
		else if (LR == 1) {
			glm::vec3 dir = set_dir(yaw + 90, pitch);
			camera_pos.x += dir.x * speed * 0.7f;
			camera_pos.z += dir.z * speed * 0.7f;
		}
	}

	if (blue.collision_check(camera_pos)) {
		blue.teleport(&camera_pos, &yaw);
	}
	else if (red.collision_check(camera_pos)) {
		red.teleport(&camera_pos, &yaw);
	}

	camera_pos.x = std::clamp(camera_pos.x, -4.8f, 4.8f);
	camera_pos.z = std::clamp(camera_pos.z, -4.8f, 4.8f);

	cube.update(camera_pos, cameraFront);

	glutTimerFunc(10, move, value);
	glutPostRedisplay();
}

void move_floor(int value) {
	floors[value].pos.y += floors[value].addy;
	if ((floors[value].pos.y > 8.f - 0.125f) || (floors[value].pos.y < 6.f - 0.125f)) {
		floors[value].addy *= -1;
	}
	glutTimerFunc(10, move_floor, value);
	glutPostRedisplay();
}

void button_collision(int value) {
	for (int i = 0; i < Button_count; ++i) {
		if (!button[i].push) {
			if ((camera_pos.x >= button[i].pos.x - button[i].scale.x) && (camera_pos.x <= button[i].pos.x + button[i].scale.x)) {
				if ((camera_pos.z >= button[i].pos.z - button[i].scale.z) && (camera_pos.z <= button[i].pos.z + button[i].scale.z)) {
					if ((camera_pos.y - tall <= button[i].pos.y + button[i].scale.y) && (camera_pos.y - tall >= button[i].pos.y)) {
						button[i].push = true;
						clear_wall[i].see = false;
						button[i].pos.y -= button[i].scale.y / 2;
					}
				}
			}
			if (collision_check(cube.pos, cube.scale, button[i].pos, button[i].scale)) {
				button[i].push = true;
				clear_wall[i].see = false;
				button[i].pos.y -= button[i].scale.y / 2;
			}
		}
		else if (button[i].push) {
			/*if ((cube.pos.x <= button[i].pos.x - button[i].scale.x) || (cube.pos.x >= button[i].pos.x + button[i].scale.x) || (camera_pos.z <= button[i].pos.z - button[i].scale.z) || (camera_pos.z >= button[i].pos.z + button[i].scale.z)) {

				button[i].push = false;
				clear_wall[i].see = true;
				button[i].pos.y += button[i].scale.y / 2;
			}*/
			if (!collision_check(cube.pos, cube.scale, button[i].pos, button[i].scale) &&
				((cube.pos.x <= button[i].pos.x - button[i].scale.x) || (cube.pos.x >= button[i].pos.x + button[i].scale.x) || (camera_pos.z <= button[i].pos.z - button[i].scale.z) || (camera_pos.z >= button[i].pos.z + button[i].scale.z))) {
				button[i].push = false;
				clear_wall[i].see = true;
				button[i].pos.y += button[i].scale.y / 2;
			}
		}
	}
	glutTimerFunc(10, button_collision, value);
	glutPostRedisplay();
}
void drop(int value) {
	bool contact = false;
	for (int i = 0; i < Floor_count; ++i) {
		if ((camera_pos.x + 0.2f >= floors[i].pos.x - floors[i].scale.x / 2) && (camera_pos.x - 0.2f <= floors[i].pos.x + floors[i].scale.x / 2)) {
			if ((camera_pos.z + 0.2f >= floors[i].pos.z - floors[i].scale.z / 2) && (camera_pos.z - 0.2f <= floors[i].pos.z + floors[i].scale.z / 2)) {
				if ((camera_pos.y - tall <= floors[i].pos.y + floors[i].scale.y) && (camera_pos.y - tall >= floors[i].pos.y)) {
					camera_pos.y = floors[i].pos.y + floors[i].scale.y + tall;
					contact = true;
					dropspeed = -0.003f;
				}
			}
		}
	}
	if (!contact) {
		for (int i = 0; i < Wall_count; ++i) {
			if ((camera_pos.x + 0.2f >= walls[i].pos.x - walls[i].scale.x / 2) && (camera_pos.x - 0.2f <= walls[i].pos.x + walls[i].scale.x / 2)) {
				if ((camera_pos.z + 0.2f >= walls[i].pos.z - walls[i].scale.z / 2) && (camera_pos.z - 0.2f <= walls[i].pos.z + walls[i].scale.z / 2)) {
					if ((camera_pos.y - tall <= walls[i].pos.y + walls[i].scale.y) && (camera_pos.y - tall >= walls[i].pos.y)) {
						camera_pos.y = walls[i].pos.y + walls[i].scale.y + 1.f;
						contact = true;
						dropspeed = -0.003f;
					}
				}
			}
		}
	}
	if (!contact) {
		for (int i = 0; i < Clear_Wall_count; ++i) {
			if (clear_wall[i].see)
				if ((camera_pos.x + 0.2f >= clear_wall[i].pos.x - clear_wall[i].scale.x / 2) && (camera_pos.x - 0.2f <= clear_wall[i].pos.x + clear_wall[i].scale.x / 2)) {
					if ((camera_pos.z + 0.2f >= clear_wall[i].pos.z - clear_wall[i].scale.z / 2) && (camera_pos.z - 0.2f <= clear_wall[i].pos.z + clear_wall[i].scale.z / 2)) {
						if ((camera_pos.y - 1.f <= clear_wall[i].pos.y + clear_wall[i].scale.y) && (camera_pos.y - 1.f >= clear_wall[i].pos.y)) {
							camera_pos.y = clear_wall[i].pos.y + clear_wall[i].scale.y + 1.f;
							contact = true;
							dropspeed = -0.003f;
							break;
						}
					}
				}
		}
	}
	if (!contact) {
		for (int i = 0; i < Glass_count; ++i) {
			if ((camera_pos.x + 0.2f >= glass[i].pos.x - glass[i].scale.x / 2) && (camera_pos.x - 0.2f <= glass[i].pos.x + glass[i].scale.x / 2)) {
				if ((camera_pos.z + 0.2f >= glass[i].pos.z - glass[i].scale.z / 2) && (camera_pos.z - 0.2f <= glass[i].pos.z + glass[i].scale.z / 2)) {
					if ((camera_pos.y - 1.f <= glass[i].pos.y + glass[i].scale.y) && (camera_pos.y - 1.f >= glass[i].pos.y)) {
						camera_pos.y = glass[i].pos.y + glass[i].scale.y + 1.f;
						contact = true;
						dropspeed = -0.003f;
						break;
					}
				}
			}
		}
	}
	if (!contact) {
		for (int i = 0; i < 10; ++i) {
			for (int j = 0; j < 10; ++j) {
				if ((camera_pos.x + 0.2f >= ceiling[i][j].pos.x - ceiling[i][j].scale.x / 2) && (camera_pos.x - 0.2f <= ceiling[i][j].pos.x + ceiling[i][j].scale.x / 2)) {
					if ((camera_pos.z + 0.2f >= ceiling[i][j].pos.z - ceiling[i][j].scale.z / 2) && (camera_pos.z - 0.2f <= ceiling[i][j].pos.z + ceiling[i][j].scale.z / 2)) {
						if ((camera_pos.y - 1.f <= ceiling[i][j].pos.y + ceiling[i][j].scale.y) && (camera_pos.y - 1.f >= ceiling[i][j].pos.y)) {
							if (i == 0 && j == 9) continue;
							camera_pos.y = ceiling[i][j].pos.y + ceiling[i][j].scale.y + 1.f;
							contact = true;
							dropspeed = -0.003f;
							break;
						}
					}
				}
			}
		}
	}
	if (!contact) {
		if (ghost) {
			camera_pos.y = tall;
		}
		else if (camera_pos.y > tall) {
			camera_pos.y += dropspeed;
			dropspeed += -0.003f;
		}
		else {
			camera_pos.y = tall;
			dropspeed = -0.003f;
		}
	}

	if (cube.grab) {
		glutTimerFunc(10, drop, value);
		glutPostRedisplay();
		return;
	}

	bool cube_contact = false;
	for (int i = 0; i < Floor_count; ++i) {
		if (collision_check(cube.pos, cube.scale, floors[i].pos, floors[i].scale)) {
			cube.pos.y = floors[i].pos.y + floors[i].scale.y;
			cube_contact = true;
			cube.dropspeed = -0.03f;
		}
	}
	if (!cube_contact) {
		for (int i = 0; i < Wall_count; ++i) {
			if ((cube.pos.x + 0.5f >= walls[i].pos.x - walls[i].scale.x / 2) && (cube.pos.x - 0.5f <= walls[i].pos.x + walls[i].scale.x / 2)) {
				if ((cube.pos.z + 0.5f >= walls[i].pos.z - walls[i].scale.z / 2) && (cube.pos.z - 0.5f <= walls[i].pos.z + walls[i].scale.z / 2)) {
					if ((cube.pos.y - 0.5f <= walls[i].pos.y + walls[i].scale.y) && (cube.pos.y - 0.5f >= walls[i].pos.y)) {
						cube.pos.y = walls[i].pos.y + walls[i].scale.y;
						cube_contact = true;
						cube.dropspeed = -0.03f;
					}
				}
			}
		}
	}
	if (!cube_contact) {
		for (int i = 0; i < Clear_Wall_count; ++i) {
			if (clear_wall[i].see)
				if ((cube.pos.x + 0.5f >= clear_wall[i].pos.x - clear_wall[i].scale.x / 2) && (cube.pos.x - 0.5f <= clear_wall[i].pos.x + clear_wall[i].scale.x / 2)) {
					if ((cube.pos.z + 0.5f >= clear_wall[i].pos.z - clear_wall[i].scale.z / 2) && (cube.pos.z - 0.5f <= clear_wall[i].pos.z + clear_wall[i].scale.z / 2)) {
						if ((cube.pos.y - .5f <= clear_wall[i].pos.y + clear_wall[i].scale.y) && (cube.pos.y - 0.5f >= clear_wall[i].pos.y)) {
							cube.pos.y = clear_wall[i].pos.y + clear_wall[i].scale.y;
							cube_contact = true;
							cube.dropspeed = -0.03f;
							break;
						}
					}
				}
		}
	}
	if (!cube_contact) {
		for (int i = 0; i < Glass_count; ++i) {
			if ((cube.pos.x + 0.5f >= glass[i].pos.x - glass[i].scale.x / 2) && (cube.pos.x - 0.5f <= glass[i].pos.x + glass[i].scale.x / 2)) {
				if ((cube.pos.z + 0.5f >= glass[i].pos.z - glass[i].scale.z / 2) && (cube.pos.z - 0.5f <= glass[i].pos.z + glass[i].scale.z / 2)) {
					if ((cube.pos.y - 0.5f <= glass[i].pos.y + glass[i].scale.y) && (cube.pos.y - 0.5f >= glass[i].pos.y)) {
						cube.pos.y = glass[i].pos.y + glass[i].scale.y;
						cube_contact = true;
						cube.dropspeed = -0.03f;
						break;
					}
				}
			}
		}
	}
	if (!cube_contact) {
		for (int i = 0; i < 10; ++i) {
			for (int j = 0; j < 10; ++j) {
				if ((cube.pos.x + 0.5f >= ceiling[i][j].pos.x - ceiling[i][j].scale.x / 2) && (cube.pos.x - 0.5f <= ceiling[i][j].pos.x + ceiling[i][j].scale.x / 2)) {
					if ((cube.pos.z + 0.5f >= ceiling[i][j].pos.z - ceiling[i][j].scale.z / 2) && (cube.pos.z - 0.5f <= ceiling[i][j].pos.z + ceiling[i][j].scale.z / 2)) {
						if ((cube.pos.y - 0.5f <= ceiling[i][j].pos.y + ceiling[i][j].scale.y) && (cube.pos.y - 0.5f >= ceiling[i][j].pos.y)) {
							if (i == 0 && j == 9) continue;
							cube.pos.y = ceiling[i][j].pos.y + ceiling[i][j].scale.y;
							cube_contact = true;
							cube.dropspeed = -0.03f;
							break;
						}
					}
				}
			}
		}
	}
	if (!cube_contact) {
		if (cube.pos.y > 0) {
			cube.pos.y += cube.dropspeed;
		}
		else {
			cube.pos.y = 0;
			cube.dropspeed = -0.03f;
		}
	}
	glutTimerFunc(10, drop, value);
	glutPostRedisplay();
}


bool collision(glm::vec3 camera_positon) {
	camera_positon.x += cameraFront.x * speed * UD * 2;
	camera_positon.z += cameraFront.z * speed * UD * 2;
	if (LR == -1) {
		glm::vec3 dir = set_dir(yaw - 90, pitch);
		camera_positon.x += dir.x * speed * 0.7f * 2;
		camera_positon.z += dir.z * speed * 0.7f * 2;
	}
	else if (LR == 1) {
		glm::vec3 dir = set_dir(yaw + 90, pitch);
		camera_positon.x += dir.x * speed * 0.7f * 2;
		camera_positon.z += dir.z * speed * 0.7f * 2;
	}
	// 충돌검사 충돌 하면 true

	for (int k = 0; k < 4; ++k) {
		for (int i = 0; i < 20; ++i) {
			for (int j = 0; j < 10; ++j) {
				if ((camera_positon.x + cha_size.x >= map_wall[k][i][j].pos.x) && (camera_positon.x - cha_size.x <= map_wall[k][i][j].pos.x)) {
					if ((camera_positon.z + 0.3f >= map_wall[k][i][j].pos.z) && (camera_positon.z - 0.3f <= map_wall[k][i][j].pos.z)) {
						if ((camera_positon.y <= map_wall[k][i][j].pos.y) && (camera_positon.y >= map_wall[k][i][j].pos.y)) {
							return true;
						}
					}
				}
			}
		}
	}
	for (int i = 0; i < Floor_count; ++i) {
		if ((camera_positon.x + cha_size.x >= floors[i].pos.x - floors[i].scale.x / 2) && (camera_positon.x - cha_size.x <= floors[i].pos.x + floors[i].scale.x / 2)) {
			if ((camera_positon.z + cha_size.z >= floors[i].pos.z - floors[i].scale.z / 2) && (camera_positon.z - cha_size.z <= floors[i].pos.z + floors[i].scale.z / 2)) {
				if ((camera_positon.y <= floors[i].pos.y + floors[i].scale.y) && (camera_positon.y >= floors[i].pos.y)) {
					return true;
				}
			}
		}
	}
	for (int i = 0; i < Wall_count; ++i) {
		if ((camera_positon.x + cha_size.x >= walls[i].pos.x - walls[i].scale.x / 2) && (camera_positon.x - cha_size.x <= walls[i].pos.x + walls[i].scale.x / 2)) {
			if ((camera_positon.z + cha_size.z >= walls[i].pos.z - walls[i].scale.z / 2) && (camera_positon.z - cha_size.z <= walls[i].pos.z + walls[i].scale.z / 2)) {
				if ((camera_positon.y <= walls[i].pos.y + walls[i].scale.y) && (camera_positon.y >= walls[i].pos.y)) {
					return true;
				}
			}
		}
	}
	for (int i = 0; i < Clear_Wall_count; ++i) {
		if (clear_wall[i].see) {
			if ((camera_positon.x + cha_size.x >= clear_wall[i].pos.x - clear_wall[i].scale.x / 2) && (camera_positon.x - cha_size.x <= clear_wall[i].pos.x + clear_wall[i].scale.x / 2)) {
				if ((camera_positon.z + cha_size.z >= clear_wall[i].pos.z - clear_wall[i].scale.z / 2) && (camera_positon.z - cha_size.z <= clear_wall[i].pos.z + clear_wall[i].scale.z / 2)) {
					if ((camera_positon.y + 0.001f <= clear_wall[i].pos.y + clear_wall[i].scale.y) && (camera_positon.y + 0.001f >= clear_wall[i].pos.y)) {
						return true;
					}
				}
			}
		}
	}
	for (int i = 0; i < Glass_count; ++i) {
		if ((camera_positon.x + cha_size.x >= glass[i].pos.x - glass[i].scale.x / 2) && (camera_positon.x - cha_size.x <= glass[i].pos.x + glass[i].scale.x / 2)) {
			if ((camera_positon.z + cha_size.z >= glass[i].pos.z - glass[i].scale.z / 2) && (camera_positon.z - cha_size.z <= glass[i].pos.z + glass[i].scale.z / 2)) {
				if ((camera_positon.y <= glass[i].pos.y + glass[i].scale.y) && (camera_positon.y >= glass[i].pos.y)) {
					return true;
				}
			}
		}
	}

	return false;
}

glm::vec3 ending_eff = { distribution(gen),distribution(gen) ,distribution(gen) };

void ending(int value) {
	light_color -= ending_eff;
	if (light_color.r < 0)
		light_color.r = 0.4f;
	if (light_color.g < 0)
		light_color.g = 0.4f;
	if (light_color.b < 0)
		light_color.b = 0.4f;

	glutTimerFunc(10, ending, value);
	glutPostRedisplay();
}

bool in_endzone() {
	if ((camera_pos.x + 0.2f >= floors[9].pos.x - floors[9].scale.x / 2) && (camera_pos.x - 0.2f <= floors[9].pos.x + floors[9].scale.x / 2)) {
		if ((camera_pos.z + 0.2f >= floors[9].pos.z - floors[9].scale.z / 2) && (camera_pos.z - 0.2f <= floors[9].pos.z + floors[9].scale.z / 2)) {
			if ((camera_pos.y - tall <= floors[9].pos.y + floors[9].scale.y) && (camera_pos.y - tall >= floors[9].pos.y)) {
				return true;
			}
		}
	}
	return false;
}

void elevator(int value) {
	if (in_endzone()) {
		if (floors[9].pos.y <= 10.8f) {
			floors[9].pos.y += floors[9].addy;
			camera_pos.y = floors[9].pos.y + floors[9].scale.y + tall;

			if (light_color.r > 0)
				light_color += glm::vec3(-.0005f, -.0005f, -.0005f);

			if (blue.render)
				blue.render = false;
			if (red.render)
				red.render = false;
		}
	}
	if (camera_pos.y > 11.5f) {
		light_color = { 0.4f,0.4f,0.4f };
		glutTimerFunc(10, ending, value);
	}
	else
		glutTimerFunc(10, elevator, value);
	glutPostRedisplay();
}


void cube_collision(float yaw, float pitch) {
	glm::vec3 line = camera_pos;

	for (int t = 0; t < 2; ++t) {
		line += cameraFront;
		if (is_in_cube(line)) {
			cube.grab = true;
		}
	}
}

bool is_in_cube(glm::vec3 pos) {
	return(pos.x <= cube.pos.x + cube.scale.x && pos.x >= cube.pos.x - cube.scale.x &&
		pos.y <= cube.pos.y + cube.scale.y && pos.y >= cube.pos.y - cube.scale.y &&
		pos.z <= cube.pos.z + cube.scale.z && pos.z >= cube.pos.z - cube.scale.z);
}

bool collision_check(glm::vec3 pos1, glm::vec3 scale1, glm::vec3 pos2, glm::vec3 scale2) {
	bool xCollision = std::abs(pos1.x - pos2.x) * 2 < (scale1.x + scale2.x);
	bool yCollision = std::abs(pos1.y - pos2.y) * 2 < (scale1.y + scale2.y);
	bool zCollision = std::abs(pos1.z - pos2.z) * 2 < (scale1.z + scale2.z);

	return xCollision && yCollision && zCollision;
}

void render() {
	glm::mat4 trans = glm::mat4(1.0f);
	unsigned int shape_location = glGetUniformLocation(shaderProgramID, "transform");
	glUniformMatrix4fv(shape_location, 1, GL_FALSE, glm::value_ptr(trans));
	//----------------------------기본 맵-----------------------------------
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 10; ++j) {
			ceiling[i][j].translate();
			ceiling[i][j].Draw_shape();
		}
	}
	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 10; ++j) {
			trans = glm::mat4(1.0f);
			trans = glm::translate(trans, glm::vec3(ground[i][j].pos));
			glUniformMatrix4fv(shape_location, 1, GL_FALSE, glm::value_ptr(trans));
			ground[i][j].Draw_shape();
		}
	}
	for (int k = 0; k < 4; ++k) {
		for (int i = 0; i < 20; ++i) {
			for (int j = 0; j < 10; ++j) {
				trans = glm::mat4(1.0f);
				trans = glm::translate(trans, glm::vec3(map_wall[k][i][j].pos));
				glUniformMatrix4fv(shape_location, 1, GL_FALSE, glm::value_ptr(trans));
				map_wall[k][i][j].Draw_shape();
			}
		}
	}
	//----------------------------------------------------------------------

	//-------------------------------벽-------------------------------------
	for (int i = 0; i < Wall_count; ++i) {
		walls[i].translate();
		walls[i].Draw_shape();
	}
	//----------------------------------------------------------------------



	//-------------------------------바닥------------------------------------
	for (int i = 0; i < Floor_count; ++i) {
		floors[i].translate();
		floors[i].Draw_shape();
	}
	//----------------------------------------------------------------------




	//-------------------------------큐브-----------------------------------
	cube.translate();
	cube.Draw_shape();
	//----------------------------------------------------------------------

	//-------------------------------케이크---------------------------------
	cake.translate();
	cake.Draw_shape();
	//----------------------------------------------------------------------

	//-------------------------------버튼-----------------------------------
	for (int i = 0; i < Button_count; ++i) {
		button[i].translate();
		button[i].Draw_shape();
	}
	//----------------------------------------------------------------------

	//-------------------------------총알?-----------------------------------
	if (blue_bullet.render) {
		blue_bullet.translate();
		blue_bullet.Draw_shape();
	}
	if (red_bullet.render) {
		red_bullet.translate();
		red_bullet.Draw_shape();
	}
	//----------------------------------------------------------------------

	//-------------------------------포탈-----------------------------------
	if (blue.render) {
		blue.translate();
		blue.Draw_shape();
	}

	if (red.render) {
		red.translate();
		red.Draw_shape();
	}
	//----------------------------------------------------------------------

	//-------------------------------반투명벽-------------------------------
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	for (int i = 0; i < Clear_Wall_count; ++i) {
		if (clear_wall[i].see) {
			clear_wall[i].translate();
			clear_wall[i].Draw_shape();
		}
	}
	//----------------------------------------------------------------------

	//-------------------------------유리-----------------------------------
	for (int i = 0; i < Glass_count; ++i) {
		glass[i].translate();
		glass[i].Draw_shape();
	}
	//----------------------------------------------------------------------

	glDisable(GL_BLEND);
}
