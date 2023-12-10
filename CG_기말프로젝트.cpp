#define  _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <gl/glm/ext.hpp>
#include <gl/glm/glm.hpp>
#include <gl/glm/ext/matrix_transform.hpp>
#include <stdlib.h>
#include <random>
#include <fstream>
#include "stb_image.h"

#define Width 1200
#define Height 800

void make_vertexShaders();
void make_fragmentShaders();
void make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
char* filetobuf(const char* file);
void InitBuffer();
GLvoid Keyboard(unsigned char key, int x, int y);
void Mouse(int button, int state, int x, int y);
void Motion(int x, int y);
void pos_change(int win_x, int win_y, float* gl_x, float* gl_y);

GLchar* vertexSource, * fragmentSource; //--- 소스코드 저장 변수
GLuint vertexShader, fragmentShader; //--- 세이더 객체
GLuint shaderProgramID;
GLuint vao;


std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> random_color(0.1, 1);

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
};
class Wall : public Shape {
public:
	glm::vec3 pos;
};

glm::vec3 light_pos;
glm::vec3 light_color;
glm::vec3 camera_pos;

Floor floors[10][10];
Wall walls[4][10][10];


glm::vec3 head_angle;

// 태경
glm::vec3 mouse_before{};
const float dpi = 5.0f;		// 사실 dpi 아니고 mouse_speed
GLfloat window_w = Width, window_h = Height;	// 마우스 입력 좌표 변환할 때 창 크기에 따라 하기 위해서 쓰임 
	
	//gpt는 신이야
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);	// 일인칭 카메라 방향 바꾸려고 추가

float cameraSpeed = 0.05f; // 조절 가능한 카메라 이동 속도
float sensitivity = 0.05f; // 조절 가능한 마우스 감도

float yaw = -90.0f; // 카메라의 초기 yaw 각도
float pitch = 0.0f;  // 카메라의 초기 pitch 각도

bool firstMouse = true;
float lastX = 400.0f;
float lastY = 300.0f;




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
		light_color = { 0.7f,0.7f,0.7f };
		light_pos = { 1.f,10.f,1.f };
		camera_pos = { 0.f, 1.f, 0.f };
	}

	InitBuffer();
	//--- 세이더 읽어와서 세이더 프로그램 만들기
	glutDisplayFunc(drawScene); //--- 출력 콜백 함수
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
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

	glEnable(GL_DEPTH_TEST);

	unsigned int proj_location = glGetUniformLocation(shaderProgramID, "projection");
	unsigned int view_location = glGetUniformLocation(shaderProgramID, "view");

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
	//std::cout << cameraPos.x << '\n';
	view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	view = glm::rotate(view, glm::radians(head_angle.y), glm::vec3(0.f, 1.f, 0.f));
	glUniformMatrix4fv(view_location, 1, GL_FALSE, &view[0][0]);


	unsigned int lightPosLocation = glGetUniformLocation(shaderProgramID, "lightPos");
	glUniform3f(lightPosLocation, light_pos.x, light_pos.y, light_pos.z);
	unsigned int lightColorLocation = glGetUniformLocation(shaderProgramID, "lightColor");
	glUniform3f(lightColorLocation, light_color.x, light_color.y, light_color.z);
	unsigned int viewPosLocation = glGetUniformLocation(shaderProgramID, "viewPos");
	glUniform3f(viewPosLocation, light_pos.x, light_pos.y, light_pos.z);
	unsigned int AmbientPosLocation = glGetUniformLocation(shaderProgramID, "amb_light");

	glm::mat4 trans = glm::mat4(1.0f);
	unsigned int shape_location = glGetUniformLocation(shaderProgramID, "transform");
	glUniformMatrix4fv(shape_location, 1, GL_FALSE, glm::value_ptr(trans));

	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 10; ++j) {
			trans = glm::mat4(1.0f);
			trans = glm::translate(trans, glm::vec3(floors[i][j].pos));
			glUniformMatrix4fv(shape_location, 1, GL_FALSE, glm::value_ptr(trans));
			floors[i][j].Draw_shape();
		}
	}
	for (int k = 0; k < 4; ++k) {
		for (int i = 0; i < 10; ++i) {
			for (int j = 0; j < 10; ++j) {
				trans = glm::mat4(1.0f);
				trans = glm::translate(trans, glm::vec3(walls[k][i][j].pos));
				glUniformMatrix4fv(shape_location, 1, GL_FALSE, glm::value_ptr(trans));
				walls[k][i][j].Draw_shape();
			}
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

	for (int i = 0; i < 10; ++i) {
		for (int j = 0; j < 10; ++j) {
			floors[i][j].Load_Obj("cube_floor.obj");
			floors[i][j].Set_color(1.f, 1.f, 1.f);
			floors[i][j].Create_texture("./resource/floor_1.jpg");
			floors[i][j].pos = { 0.5f + 1.f * (j - 5),-1.f,0.5f + 1.f * (i - 5) };
			walls[0][i][j].Load_Obj("cube_floor.obj");
			walls[0][i][j].Set_color(1.f, 1.f, 1.f);
			walls[0][i][j].Create_texture("./resource/wall_2.jpg");
			walls[0][i][j].pos = { -5.5f, i * 1.f, 0.5f + 1.f * (j - 5) };
			walls[1][i][j].Load_Obj("cube_floor.obj");
			walls[1][i][j].Set_color(1.f, 1.f, 1.f);
			walls[1][i][j].Create_texture("./resource/wall_2.jpg");
			walls[1][i][j].pos = { 5.5f, i * 1.f, 0.5f + 1.f * (j - 5) };
			walls[2][i][j].Load_Obj("cube_floor.obj");
			walls[2][i][j].Set_color(1.f, 1.f, 1.f);
			walls[2][i][j].Create_texture("./resource/wall_2.jpg");
			walls[2][i][j].pos = { 0.5f + 1.f * (j - 5), i * 1.f, -5.5f };
			walls[3][i][j].Load_Obj("cube_floor.obj");
			walls[3][i][j].Set_color(1.f, 1.f, 1.f);
			walls[3][i][j].Create_texture("./resource/wall_2.jpg");
			walls[3][i][j].pos = { 0.5f + 1.f * (j - 5), i * 1.f, 5.5f };
		}
	}
}
GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'q':
	case 'Q':
		exit(0);
		break;
	case 's':
		camera_pos.x += 0.1f;
		break;
	case 'w':
		camera_pos.x -= 0.1f;
		break;
	case 'y':
		head_angle.y += 1.f;
		break;
	}
	glutPostRedisplay();
}


void Mouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		//leftclick = true;

	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		//leftclick = false;
	}
	glutPostRedisplay();
}

void Motion(int x, int y)
{
	/*float mouse_x, mouse_y;
	pos_change(x, y, &mouse_x, &mouse_y);

	head_angle.y += (mouse_x - mouse_before.x) * dpi;

	mouse_before.x = mouse_x;

	std::cout << mouse_x << std::endl;*/
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


void pos_change(int win_x, int win_y, float* gl_x, float* gl_y)
{
	*gl_x = (win_x - window_w / 2) / (window_w / 2);
	*gl_y = -(win_y - window_h / 2) / (window_h / 2);
}
