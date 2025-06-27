/* Jogo: Mochi's Journey
 *      Código adaptado de:
 *   - https://learnopengl.com/#!Getting-started/Hello-Triangle
 *   - https://antongerdelan.net/opengl/glcontext2.html
 *
 * Modificado por Gabriela Spanemberg Bado e Sadi Júnior
 * para a disciplina de Processamento Gráfico - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 23/06/2025
 *
 */

#include <iostream>
#include <string>
#include <assert.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>

// ---- HEADERS GLFW, GLAD, GLM, STB_IMAGE ----

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

// ---- ENUMS ----

enum TileType
{
	Sand = 0,
	Grass = 1,
	Dirt = 2,
	Lava = 3,
	Ice = 4,
	Water = 5,
	Player = 6,
};

enum TileMetadataType
{
	Walkable = 0,
	NonWalkable = 1,
	Lethal = 2,
	Coin = 3,
};

// ---- STRUCTS ----
struct Sprite
{
	GLuint VAO;
	GLuint textureID;
	vec3 position;
	vec3 dimensions;
	float ds, dt;
	int iAnimation, iFrame;
	int nAnimations, nFrames;
};

struct Mochi
{
	Sprite sprite;
	bool died;
	double diedAt;
	int lives;
	int coins;
};

struct Tile
{
	GLuint VAO;
	GLuint texID; // de qual tileset
	int iTile;	  // indice dele no tileset
	vec3 position;
	vec3 dimensions; // tamanho do losango 2:1
	float ds, dt;
};

struct TileMap
{
	int width;
	int height;
	vector<vector<TileType>> map;
	vector<vector<TileMetadataType>> mapMetadata;
	vector<Tile> tileset;
};

// ---- VARIÁVEIS GLOBAIS ----

const GLuint WINDOW_WIDTH = 1850, WINDOW_HEIGHT = 900;
const unsigned int MOCHI_LIVES = 3;
const unsigned int TOTAL_COINS = 5;
int player_i = 0;
int player_j = 0;
TileMap tilemap;
Mochi mochi;
Sprite coinSprite;
Sprite gameOverSprite;
bool gameOver;
bool gameWon;
double lastMoveTime = 0.0;
bool mochiIsIdle = true;
const int andandoFrente = 0;
const int andandoCostas = 1;
vector<vector<TileMetadataType>> originalMapMetadata;

// ---- DECLARAÇÃO DE FUNÇÕES ----

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);
int setupShader();
int setupSprite(int nAnimations, int nFrames, float &ds, float &dt);
int setupTile(int nTiles, float &ds, float &dt);
int loadTexture(string filePath, int &width, int &height);
void desenharMapa(GLuint shaderID, const TileMap &tilemap);
void desenharMochi(GLuint shaderID, const TileMap &tilemap);
bool carregarMapa(const string &mapPath, string &tileImagePath, int &nTiles, int &tileH, int &tileW, TileMap &tilemap);
void inicializarTilemap(const string &tileImagePath, int nTiles, int tileH, int tileW, TileMap &tilemap);
void moverJogador(int new_i, int new_j, int novaAnimacao);
void desenharGameOver(GLFWwindow *window, GLuint shaderID, const Sprite &gameOverSprite, double curr_s, bool &blinking, double &blinkStart, int &blinkCount);
void inicializarSprites();
bool inicializarTilemapEMapa(TileMap &tilemap, vector<vector<TileMetadataType>> &originalMapMetadata);
void atualizarTituloJanela(GLFWwindow *window, double elapsed_s);
void reiniciarJogo();
void processarMorteMochi(double curr_s, bool &gameOverMsgPrinted, bool &gameOverBlinking, double &gameOverBlinkStart, int &gameOverBlinkCount);

// ---- SHADERS ----

const GLchar *vertexShaderSource = R"(
 #version 400
 layout (location = 0) in vec3 position;
 layout (location = 1) in vec2 texc;
 out vec2 tex_coord;
 uniform mat4 model;
 uniform mat4 projection;
 void main()
 {
	tex_coord = texc;
	gl_Position = projection * model * vec4(position, 1.0);
 }
 )";

const GLchar *fragmentShaderSource = R"(
 #version 400
 in vec2 tex_coord;
 out vec4 color;
 uniform sampler2D tex_buff;
 uniform vec2 offsetTex;

 void main()
 {
	 color = texture(tex_buff,tex_coord + offsetTex);
 }
 )";

// ---- FUNÇÃO MAIN ----
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 8);

	GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Mochi's Journey --Gabriela e Sadi", nullptr, nullptr);
	if (!window)
	{
		std::cerr << "Falha ao criar a janela GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Falha ao inicializar GLAD" << std::endl;
		return -1;
	}

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	GLuint shaderID = setupShader();

	inicializarSprites();

	if (!inicializarTilemapEMapa(tilemap, originalMapMetadata))
		return -1;

	glUseProgram(shaderID);

	double prev_s = glfwGetTime();
	double title_countdown_s = 0.1;
	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(shaderID, "tex_buff"), 0);

	mat4 projection = ortho(0.0, (double)WINDOW_WIDTH, (double)WINDOW_HEIGHT, 0.0, -1.0, 1.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	double FPS = 12.0;
	std::cout << "\n==============================\n";
	std::cout << " Iniciando Mochi's Journey!\n";
	std::cout << " Vidas do Mochi: " << mochi.lives << "\n";
	std::cout << " Encontre as " << TOTAL_COINS << " moedas douradas para restaurar o equilíbrio entre os elementos e VENCER O JOGO!" << "\n";
	std::cout << "==============================\n" << std::endl;

	gameOver = false;
	gameWon = false;
	mochi.lives = MOCHI_LIVES;
	mochi.died = false;
	mochi.coins = 0;
	player_i = 0;
	player_j = 0;
	mochi.sprite.iAnimation = andandoFrente;
	mochi.sprite.iFrame = 0;
	mochiIsIdle = true;

	bool gameOverMsgPrinted = false;
	bool gameOverBlinking = false;
	double gameOverBlinkStart = 0.0;
	int gameOverBlinkCount = 0;

	while (!glfwWindowShouldClose(window))
	{
		double curr_s = glfwGetTime();
		double elapsed_s = curr_s - prev_s;
		prev_s = curr_s;

		title_countdown_s -= elapsed_s;
		if (title_countdown_s <= 0.0 && elapsed_s > 0.0)
		{
			atualizarTituloJanela(window, elapsed_s);
			title_countdown_s = 0.1;
		}

		glfwPollEvents();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glLineWidth(10);
		glPointSize(20);

		if (mochi.died && curr_s - mochi.diedAt > 1.0)
		{
			if (mochi.lives > 0) {
				processarMorteMochi(curr_s, gameOverMsgPrinted, gameOverBlinking, gameOverBlinkStart, gameOverBlinkCount);
			} else {
				gameOver = true;
				if (!gameOverMsgPrinted) {
					std::cout << "\n==============================\n";
					std::cout << " GAME OVER!\n";
					std::cout << " Mochi morreu e acabaram as suas vidas!\n";
					std::cout << " Pressione R para reiniciar\n";
					std::cout << "==============================\n" << std::endl;
					gameOverMsgPrinted = true;
				}
				if (!gameOverBlinking) {
					gameOverBlinking = true;
					gameOverBlinkStart = curr_s;
					gameOverBlinkCount = 0;
				}
			}
		}

		gameWon = mochi.coins >= TOTAL_COINS;
		if (gameWon)
		{
			std::cout << "\n==============================\n";
			std::cout << " CONGRATULATIONS! Mochi encontrou todas as moedas e restaurou o equilíbrio entre os elementos!\n";
			std::cout << " Vidas do Mochi: " << mochi.lives << "\n";
			std::cout << " Moedas do Mochi: " << mochi.coins << "\n";
			std::cout << " Pressione R para reiniciar\n";
			std::cout << "==============================\n" << std::endl;
		}

		if (!gameOver)
		{
			desenharMapa(shaderID, tilemap);
			desenharMochi(shaderID, tilemap);
			gameOverMsgPrinted = false;
			gameOverBlinking = false;
		}
		else
		{
			desenharGameOver(window, shaderID, gameOverSprite, curr_s, gameOverBlinking, gameOverBlinkStart, gameOverBlinkCount);
		}

		static double lastFrameTime = glfwGetTime();
		double now = glfwGetTime();

		if ((now - lastFrameTime) > (1.0 / FPS))
		{
			lastFrameTime = now;

			if (mochiIsIdle)
			{
				mochi.sprite.iAnimation = 0;
			}

			mochi.sprite.iFrame = (mochi.sprite.iFrame + 1) % mochi.sprite.nFrames;
		}
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

// ---- IMPLEMENTAÇÃO DAS FUNÇÕES ----

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		gameOver = false;

		mochi.lives = MOCHI_LIVES;
		mochi.died = false;
		mochi.coins = 0;

		tilemap.mapMetadata = originalMapMetadata;

		player_i = 0;
		player_j = 0;

		mochi.sprite.iAnimation = andandoFrente;
		mochi.sprite.iFrame = 0;
		mochiIsIdle = true;

		std::cout << "\n==============================\n";
		std::cout << " Reiniciando Mochi's Journey!\n";
		std::cout << " Vidas do Mochi: " << mochi.lives << "\n";
		std::cout << " Moedas do Mochi: " << mochi.coins << "\n";
		std::cout << "==============================\n"
				  << std::endl;
	}

	if (gameOver || gameWon)
	{
		return;
	}
	if (mochi.died)
	{
		return;
	}

	if (action == GLFW_PRESS || action == GLFW_REPEAT)
	{
		int new_i = player_i;
		int new_j = player_j;

		if (key == GLFW_KEY_UP || key == GLFW_KEY_W) // norte
			moverJogador(player_i - 1, player_j, andandoCostas);
		else if (key == GLFW_KEY_LEFT || key == GLFW_KEY_A) // oeste
			moverJogador(player_i, player_j - 1, andandoFrente);
		else if (key == GLFW_KEY_DOWN || key == GLFW_KEY_S) // sul
			moverJogador(player_i + 1, player_j, andandoFrente);
		else if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_D) // leste
			moverJogador(player_i, player_j + 1, andandoFrente);
		else if (key == GLFW_KEY_C) // sudeste
			moverJogador(player_i + 1, player_j + 1, andandoFrente);
		else if (key == GLFW_KEY_Z) // sudoeste
			moverJogador(player_i + 1, player_j - 1, andandoFrente);
		else if (key == GLFW_KEY_Q) // noroeste
			moverJogador(player_i - 1, player_j - 1, andandoCostas);
		else if (key == GLFW_KEY_E) // nordeste
			moverJogador(player_i - 1, player_j + 1, andandoCostas);
	}
}

int setupShader()
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

int setupSprite(int nAnimations, int nFrames, float &ds, float &dt)
{

	ds = 1.0 / (float)nFrames;
	dt = 1.0 / (float)nAnimations;
	// Aqui setamos as coordenadas x, y e z do triângulo e as armazenamos de forma
	// sequencial, já visando mandar para o VBO (Vertex Buffer Objects)
	// Cada atributo do vértice (coordenada, cores, coordenadas de textura, normal, etc)
	// Pode ser arazenado em um VBO único ou em VBOs separados
	GLfloat vertices[] = {
		// x   y    z    s     t
		-0.5, 0.5, 0.0, 0.0, dt,   // V0
		-0.5, -0.5, 0.0, 0.0, 0.0, // V1
		0.5, 0.5, 0.0, ds, dt,	   // V2
		0.5, -0.5, 0.0, ds, 0.0	   // V3
	};

	GLuint VBO, VAO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	return VAO;
}

int setupTile(int nTiles, float &ds, float &dt)
{
	ds = 1.0 / (float)nTiles;
	dt = 1.0;

	float th = 1.0, tw = 1.0;

	GLfloat vertices[] = {
		// x   y    z    s     t
		0.0, th / 2.0f, 0.0, 0.0, dt / 2.0f, // A
		tw / 2.0f, th, 0.0, ds / 2.0f, dt,	 // B
		tw / 2.0f, 0.0, 0.0, ds / 2.0f, 0.0, // D
		tw, th / 2.0f, 0.0, ds, dt / 2.0f	 // C
	};

	GLuint VBO, VAO;

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return VAO;
}

int loadTexture(string filePath, int &width, int &height)
{
	GLuint texID;

	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	int nrChannels;

	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	glBindTexture(GL_TEXTURE_2D, 0);

	return texID;
}

void desenharMapa(GLuint shaderID, const TileMap &tilemap)
{
	float tileW = tilemap.tileset[0].dimensions.x;
	float tileH = tilemap.tileset[0].dimensions.y;

	float centerMapX = (tilemap.width - tilemap.height) * (tileW / 2.0f);
	float centerMapY = (tilemap.width + tilemap.height) * (tileH / 2.0f);

	float x0 = (WINDOW_WIDTH / 2.0f) - (centerMapX / 2.0f);
	float y0 = (WINDOW_HEIGHT / 2.0f) - (centerMapY / 2.0f);

	for (int i = 0; i < tilemap.height; i++)
	{
		for (int j = 0; j < tilemap.width; j++)
		{
			mat4 model = mat4(1);
			const Tile &curr_tile = tilemap.tileset[tilemap.map[i][j]];

			float x = x0 + (j - i) * (curr_tile.dimensions.x / 2.0f);
			float y = y0 + (j + i) * (curr_tile.dimensions.y / 2.0f);

			model = translate(model, vec3(x, y, 0.0));
			model = scale(model, curr_tile.dimensions);
			glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

			vec2 offsetTex;
			offsetTex.s = curr_tile.iTile * curr_tile.ds;
			offsetTex.t = 0.0;
			glUniform2f(glGetUniformLocation(shaderID, "offsetTex"), offsetTex.s, offsetTex.t);

			glBindVertexArray(curr_tile.VAO);
			glBindTexture(GL_TEXTURE_2D, curr_tile.texID);
			glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

			if (tilemap.mapMetadata[i][j] == TileMetadataType::Coin)
			{
				mat4 model = mat4(1.0f);

				float coinX = x + (coinSprite.dimensions.x * 1.2f);
				float coinY = y + (coinSprite.dimensions.y * 0.5f);
				model = translate(model, vec3(coinX, coinY, 0.1f));
				model = scale(model, coinSprite.dimensions);

				glUniformMatrix4fv(
					glGetUniformLocation(shaderID, "model"), 1, GL_FALSE,
					value_ptr(model));
				glUniform2f(
					glGetUniformLocation(shaderID, "offsetTex"),
					0.0f, 0.0f);

				glBindVertexArray(coinSprite.VAO);
				glBindTexture(GL_TEXTURE_2D, coinSprite.textureID);
				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}
	}
	const Tile &marker = tilemap.tileset[TileType::Player];

	float px = x0 + (player_j - player_i) * (tileW / 2.0f);
	float py = y0 + (player_j + player_i) * (tileH / 2.0f);

	mat4 model = mat4(1);
	model = translate(model, vec3(px, py, 0.1f));
	model = scale(model, marker.dimensions);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

	vec2 offsetTex;
	offsetTex.s = marker.iTile * marker.ds;
	offsetTex.t = 0.0;
	glUniform2f(glGetUniformLocation(shaderID, "offsetTex"), offsetTex.s, offsetTex.t);

	glBindVertexArray(marker.VAO);
	glBindTexture(GL_TEXTURE_2D, marker.texID);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void desenharMochi(GLuint shaderID, const TileMap &tilemap)
{
	float tileW = tilemap.tileset[0].dimensions.x;
	float tileH = tilemap.tileset[0].dimensions.y;

	float map_display_offset_x = (WINDOW_WIDTH / 2.0f) - ((tilemap.width - tilemap.height) * (tileW / 2.0f) / 2.0f);
	float map_display_offset_y = (WINDOW_HEIGHT / 2.0f) - ((tilemap.width + tilemap.height) * (tileH / 2.0f) / 2.0f);

	float px = map_display_offset_x + (player_j - player_i) * (tileW / 2.0f);
	float py = map_display_offset_y + (player_j + player_i) * (tileH / 2.0f);

	mat4 model = mat4(1);

	float mochi_x_position = px + (tileW / 2.0f);
	float mochi_y_position = py + tileH - (mochi.sprite.dimensions.y / 2.0f) + 40.0f;

	model = translate(model, vec3(mochi_x_position, mochi_y_position, 0.2f));
	model = scale(model, mochi.sprite.dimensions);

	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

	vec2 offsetTex;
	offsetTex.s = mochi.sprite.iFrame * mochi.sprite.ds;
	offsetTex.t = mochi.sprite.iAnimation * mochi.sprite.dt;

	glUniform2f(glGetUniformLocation(shaderID, "offsetTex"), offsetTex.s, offsetTex.t);

	glBindVertexArray(mochi.sprite.VAO);
	glBindTexture(GL_TEXTURE_2D, mochi.sprite.textureID);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

bool carregarMapa(const string &mapPath, string &tileImagePath, int &nTiles, int &tileH, int &tileW, TileMap &tilemap)
{
	std::ifstream file(mapPath);
	if (!file.is_open())
	{
		cerr << "Erro ao abrir o arquivo de mapa: " << mapPath << endl;
		return false;
	}

	file >> tileImagePath >> nTiles >> tileH >> tileW;

	file >> tilemap.width >> tilemap.height;

	originalMapMetadata.resize(tilemap.height, vector<TileMetadataType>(tilemap.width));

	tilemap.map.resize(tilemap.height, vector<TileType>(tilemap.width));
	tilemap.mapMetadata.resize(tilemap.height, vector<TileMetadataType>(tilemap.width));

	for (int i = 0; i < tilemap.height; ++i)
	{
		for (int j = 0; j < tilemap.width; ++j)
		{
			int tileNum;
			file >> tileNum;
			tilemap.map[i][j] = static_cast<TileType>(tileNum);
		}
	}

	for (int i = 0; i < tilemap.height; ++i)
	{
		for (int j = 0; j < tilemap.width; ++j)
		{
			int metadataNum;
			file >> metadataNum;
			originalMapMetadata[i][j] = static_cast<TileMetadataType>(metadataNum);
		}
	}

	tilemap.mapMetadata = originalMapMetadata;

	return true;
}

void inicializarTilemap(const string &tileImagePath, int nTiles, int tileH, int tileW, TileMap &tilemap)
{
	int imgWidth, imgHeight;
	GLuint texID = loadTexture(tileImagePath, imgWidth, imgHeight);

	tilemap.tileset.clear();
	for (int i = 0; i < nTiles; ++i)
	{
		Tile tile;
		tile.dimensions = vec3(tileW, tileH, 1.0);
		tile.iTile = i;
		tile.texID = texID;
		tile.VAO = setupTile(nTiles, tile.ds, tile.dt);
		tilemap.tileset.push_back(tile);
	}
}

void moverJogador(int novo_i, int novo_j, int novaAnim)
{
	if (novo_i < 0 || novo_i >= tilemap.height || novo_j < 0 || novo_j >= tilemap.width)
		return;

	switch (tilemap.mapMetadata[novo_i][novo_j])
	{
	case TileMetadataType::NonWalkable:
		return;
	case TileMetadataType::Lethal:
		mochi.died = true;
		mochi.diedAt = glfwGetTime();
		mochi.lives--;
		break;
	case TileMetadataType::Coin:
		mochi.coins++;

		tilemap.mapMetadata[novo_i][novo_j] = TileMetadataType::Walkable;

		std::cout << "\n==============================\n";
		std::cout << " Mochi coletou uma moeda!" << "\n";
		std::cout << " Total de moedas: " << mochi.coins << "\n";
		std::cout << "==============================\n"
				  << std::endl;
		break;
	default:
		break;
	}

	if (player_i != novo_i || player_j != novo_j)
	{
		player_i = novo_i;
		player_j = novo_j;
		mochi.sprite.iAnimation = novaAnim;
		mochi.sprite.iFrame = 0;
		mochiIsIdle = false;
		lastMoveTime = glfwGetTime();
	}
}

void desenharGameOver(GLFWwindow *window, GLuint shaderID, const Sprite &gameOverSprite, double curr_s, bool &blinking, double &blinkStart, int &blinkCount)
{
	const int GAME_OVER_BLINKS = 5;
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int winWidth, winHeight;
	glfwGetFramebufferSize(window, &winWidth, &winHeight);
	float go_x = winWidth / 2.0f;
	float go_y = winHeight / 2.0f - (0.1f * winHeight);

	bool drawGameOver = true;
	if (blinking && blinkCount < GAME_OVER_BLINKS * 2)
	{
		double blinkElapsed = curr_s - blinkStart;
		int blinkPhase = (int)(blinkElapsed / 0.2);
		if (blinkPhase > blinkCount)
		{
			blinkCount = blinkPhase;
		}
		drawGameOver = (blinkPhase % 2 == 0);
		if (blinkCount >= GAME_OVER_BLINKS * 2)
		{
			blinking = false;
		}
	}

	if (drawGameOver)
	{
		mat4 model = mat4(1.0f);
		model = translate(model, vec3(go_x, go_y, 0.5f));
		model = scale(model, gameOverSprite.dimensions);

		glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));
		glUniform2f(glGetUniformLocation(shaderID, "offsetTex"), 0.0f, 0.0f);

		glBindVertexArray(gameOverSprite.VAO);
		glBindTexture(GL_TEXTURE_2D, gameOverSprite.textureID);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

	void inicializarSprites() {
	int imgWidth, imgHeight;
	GLuint mochiTexID = loadTexture("../assets/sprites/Mochi/Walk/Slime1_Walk_full.png", imgWidth, imgHeight);
	mochi.sprite.nAnimations = 4;
	mochi.sprite.nFrames = 8;
	mochi.sprite.VAO = setupSprite(mochi.sprite.nAnimations, mochi.sprite.nFrames, mochi.sprite.ds, mochi.sprite.dt);
	mochi.sprite.position = vec3(0.0, 0.0, 0.0);
	mochi.sprite.dimensions = vec3((imgWidth / mochi.sprite.nFrames) * 3.0f, (imgHeight / mochi.sprite.nAnimations) * 3.0f, 1.0f);
	mochi.sprite.textureID = mochiTexID;
	mochi.sprite.iAnimation = 3;
	mochi.sprite.iFrame = 0;

	int coinWidth, coinHeight;
	GLuint coinTextureID = loadTexture("../assets/sprites/coin/coin.png", coinWidth, coinHeight);
	coinSprite.textureID = coinTextureID;
	coinSprite.VAO = setupSprite(1, 1, coinSprite.ds, coinSprite.dt);
	coinSprite.dimensions = vec3((float)coinWidth, (float)coinHeight, 1.0f);

	int goWidth, goHeight;
	GLuint goTexID = loadTexture("../assets/sprites/game-over.png", goWidth, goHeight);
	gameOverSprite.textureID = goTexID;
	gameOverSprite.VAO = setupSprite(1, 1, gameOverSprite.ds, gameOverSprite.dt);
	gameOverSprite.dimensions = vec3((float)goWidth, (float)goHeight, 1.0f);
}

bool inicializarTilemapEMapa(TileMap &tilemap, vector<vector<TileMetadataType>> &originalMapMetadata) {
	string tileImagePath;
	int nTiles, tileH, tileW;
	if (!carregarMapa("../src/GS_ProvaGrauB/map.txt", tileImagePath, nTiles, tileH, tileW, tilemap))
		return false;
	inicializarTilemap(tileImagePath, nTiles, tileH, tileW, tilemap);
	return true;
}

void atualizarTituloJanela(GLFWwindow *window, double elapsed_s) {
	double fps = 1.0 / elapsed_s;
	char tmp[256];
	sprintf(tmp, "Mochi's Journey -- Gabriela e Sadi\tFPS %.2lf", fps);
	glfwSetWindowTitle(window, tmp);
}

void reiniciarJogo() {
	gameOver = false;
	mochi.lives = MOCHI_LIVES;
	mochi.died = false;
	mochi.coins = 0;
	tilemap.mapMetadata = originalMapMetadata;
	player_i = 0;
	player_j = 0;
	mochi.sprite.iAnimation = andandoFrente;
	mochi.sprite.iFrame = 0;
	mochiIsIdle = true;
	std::cout << "\n==============================\n";
	std::cout << " Reiniciando Mochi's Journey!\n";
	std::cout << " Vidas do Mochi: " << mochi.lives << "\n";
	std::cout << " Moedas do Mochi: " << mochi.coins << "\n";
	std::cout << "==============================\n" << std::endl;
}

void processarMorteMochi(double curr_s, bool &gameOverMsgPrinted, bool &gameOverBlinking, double &gameOverBlinkStart, int &gameOverBlinkCount) {
	gameOver = mochi.lives <= 0;
	if (!gameOver) {
		player_i = 0;
		player_j = 0;
		mochi.died = false;
		mochi.sprite.iAnimation = andandoFrente;
		mochi.sprite.iFrame = 0;
		mochiIsIdle = true;
		std::cout << "\n==============================\n";
		std::cout << " Mochi morreu!\n";
		std::cout << " Vidas do Mochi: " << mochi.lives << "\n";
		std::cout << " Reiniciando a vida de Mochi\n";
		std::cout << "==============================\n" << std::endl;
	} else {
		if (!gameOverMsgPrinted) {
			std::cout << "\n==============================\n";
			std::cout << " GAME OVER!\n";
			std::cout << " Mochi morreu e acabaram as suas vidas!\n";
			std::cout << " Pressione R para reiniciar\n";
			std::cout << "==============================\n" << std::endl;
			gameOverMsgPrinted = true;
		}
		if (!gameOverBlinking) {
			gameOverBlinking = true;
			gameOverBlinkStart = curr_s;
			gameOverBlinkCount = 0;
		}
	}
}