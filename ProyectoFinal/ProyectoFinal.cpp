#include <iostream>
#include <cmath>

// Librerías de OpenGL y manejo de gráficos
#include <GL/glew.h>         
#include <GLFW/glfw3.h>     

// Librerías auxiliares
#include "stb_image.h"       
#include <glm/glm.hpp>       
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "SOIL2/SOIL2.h"     

// Clases personalizadas del proyecto
#include "Shader.h"          // Carga y uso de shaders
#include "Camera.h"          // Clase para controlar la cámara (FPS style)
#include "Model.h"           // Carga y dibujo de modelos .obj
#include "Texture.h"         // Manejo de texturas (no se usa mucho aquí)
#include "modelAnim.h"       // Modelos con shaders animados

// Callbacks y control de entrada
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void MouseCallback(GLFWwindow* window, double xPos, double yPos);
void DoMovement();  // Movimiento de cámara y objetos animados


// ------------------------------
// Variables para controlar animaciones por teclado
// ------------------------------
bool anim_Puerta_ent = false;   // Activación animación puerta de entrada
float rot_Puerta_ent = 0;       // Ángulo de rotación puerta entrada


bool anim_cajon = false;        // Activación animación del cajón
float tras_cajon = 0;           // Traslación del cajón (eje Z)

bool anim_mecedora = false;     // Activación animación mecedora

bool anim_radio = false;

float tiempo;                   // Variable para controlar el tiempo
float speed;                    // Velocidad de animaciones


// ------------------------------
// Tamaño de la ventana
// ------------------------------
const GLuint WIDTH = 1280, HEIGHT = 720;
int SCREEN_WIDTH, SCREEN_HEIGHT;


// ------------------------------
// Cámara tipo FPS
// ------------------------------
Camera camera(glm::vec3(10.0f, 3.0f, 0.0f),  // Nueva posición inicial (Y, Z, X)
	glm::vec3(0.0f, 1.0f, 0.0f),          // Vector up (no cambiar)
	180.0f, -10.0f);                      // Ángulos iniciales (yaw, pitch)

GLfloat lastX = WIDTH / 2.0;    // Posición X del mouse previa
GLfloat lastY = HEIGHT / 2.0;   // Posición Y del mouse previa
bool keys[1024];                // Array para saber qué teclas están presionadas
bool firstMouse = true;         // Flag para inicializar movimiento de mouse
float range = 0.0f;             // (No utilizado aquí, probablemente vestigio)
float rot = 0.0f;               // (No utilizado aquí)
float movCamera = 0.0f;         // (No utilizado aquí)

// ------------------------------
// Iluminación
// ------------------------------
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);  
bool active;                           // Se activa con barra espaciadora

// ------------------------------
// Variables para calcular deltaTime (tiempo entre frames)
// ------------------------------
GLfloat deltaTime = 0.0f;  // Tiempo entre frames actual y anterior
GLfloat lastFrame = 0.0f;  // Tiempo del último frame

// ------------------------------
// Posiciones de luces puntuales
// ------------------------------
// Se usan tres luces: una exterior, una interior, y una tipo "sol" (Y, Z, X)
glm::vec3 pointLightPositions[] = {
	glm::vec3(9.328f, 3.656f, -4.543f),   // Luz exterior (carpa)
	glm::vec3(-4.7f, 2.90f, -2.0f),    // Luz interior (recámara)
	glm::vec3(1.767f, 26.217f, 0.018f)    // Luz tipo sol (alta)
};

// ------------------------------
// Color de la luz que se activa con barra espaciadora
// ------------------------------
// Cambia entre amarillo encendido y negro apagado
glm::vec3 LightP1;



int main()
{
	// Inicializar GLFW
	glfwInit();

	// Crear la ventana principal del proyecto 
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecto Final", nullptr, nullptr);
	if (nullptr == window)
	{
		// Si falla al crear la ventana, muestra error y termina el programa
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}
	glfwMakeContextCurrent(window); // Establece el contexto de OpenGL para esta ventana

	// Obtener las dimensiones reales del framebuffer 
	glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

	// Asignar funciones callback para teclado y mouse
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetCursorPosCallback(window, MouseCallback);

	// Oculta el cursor y lo bloquea en el centro de la pantalla para control de cámara estilo FPS
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Inicializar GLEW 
	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit()) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	// Definir el área de renderizado (viewport) y habilitar características gráficas
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glEnable(GL_DEPTH_TEST); // Para que OpenGL respete profundidad al dibujar
	glEnable(GL_BLEND);      // Activar blending (necesario para transparencias)

	// Cargar y compilar los distintos shaders usados en la escena
	Shader lightingShader("Shaders/lighting.vs", "Shaders/lighting.frag");  // Shader principal
	Shader lampShader("Shaders/lamp.vs", "Shaders/lamp.frag");              // Shader para dibujar la fuente de luz
	Shader SkyBoxshader("Shaders/SkyBox.vs", "Shaders/SkyBox.frag");        // Shader para el cielo (skybox)
	Shader animShader("Shaders/anim2.vs", "Shaders/anim2.frag");            // Shader animado 1 (pantalla, humo)
	Shader animShader2("Shaders/anim.vs", "Shaders/anim.frag");             // Shader animado 2 (pantalla secundaria)

	// Cargar todos los modelos 3D usados en el recorrido virtual
	Model Piso((char*)"Models/Piso/cuphead.obj");
	Model Cuphead((char*)"Models/casa-taza/cuphead.obj");
	Model Puerta((char*)"Models/Puerta/cuphead.obj");
	Model sillon((char*)"Models/sillon/sillon.obj");
	Model piano((char*)"Models/piano/piano.obj");
	Model estante((char*)"Models/estante/Estante.fbx");
	Model radio((char*)"Models/radio/radio.fbx");
	Model fonografo((char*)"Models/fonografo/fonografo.fbx");
	Model sillaMecedora((char*)"Models/sillaMecedora/mecedora.obj");
	Model espada((char*)"Models/espada/espada.obj");
	Model chimenea((char*)"Models/chimenea/chimenea/cuphead.obj");
	Model estante2((char*)"Models/Estante2/Estante/cuphead.obj");



	


	Model Buro((char*)"Models/Buro/Buro_base.obj");
	Model Buro_cajon((char*)"Models/Buro/Buro_cajon.obj");
	Model Humo((char*)"Models/Misc/humo/humo.obj");
	Model Lampara((char*)"Models/Lampara/lampara.obj");

	//Otros modelos
	
	

	
	

	// Vértices del cubo (con normales y coordenadas UV)
	// Este cubo es usado para pruebas, contenedores o geometría básica
	GLfloat vertices[] = {
		// Posición            // Normales           // Coordenadas de textura
		-0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,     0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f,  1.0f,
		0.5f,  0.5f, -0.5f,     0.0f,  0.0f, -1.0f,     1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,    0.0f,  0.0f, -1.0f,     0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,    0.0f,  0.0f, -1.0f,     0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,     0.0f,  0.0f,  1.0f,     1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,     1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  0.0f,  1.0f,  	1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,    0.0f,  0.0f,  1.0f,     0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,    1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,    1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,    0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,    -1.0f,  0.0f,  0.0f,    0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,    0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,    -1.0f,  0.0f,  0.0f,    1.0f,  0.0f,

		0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     1.0f,  1.0f,
		0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     0.0f,  1.0f,
		0.5f, -0.5f, -0.5f,     1.0f,  0.0f,  0.0f,     0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,     1.0f,  0.0f,  0.0f,     1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,     0.0f,  1.0f,
		0.5f, -0.5f, -0.5f,     0.0f, -1.0f,  0.0f,     1.0f,  1.0f,
		0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,     0.0f, -1.0f,  0.0f,     1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,    0.0f, -1.0f,  0.0f,     0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,    0.0f, -1.0f,  0.0f,     0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,     0.0f,  1.0f,
		0.5f,  0.5f, -0.5f,     0.0f,  1.0f,  0.0f,     1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,     0.0f,  1.0f,  0.0f,     1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,    0.0f,  1.0f,  0.0f,     0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,    0.0f,  1.0f,  0.0f,     0.0f,  1.0f
	};

	// Vértices del cubo que forma el skybox (sin textura ni normales)
	GLfloat skyboxVertices[] = {
		// Coordenadas de los 36 vértices del cubo cielo (skybox)
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		1.0f,  1.0f, -1.0f,
		1.0f,  1.0f,  1.0f,
		1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		1.0f, -1.0f,  1.0f
		
	};

	// Índices para dibujar triángulos a partir del arreglo de vértices
	// (usados con glDrawElements si fuera necesario)
	GLuint indices[] = {
		0,1,2,3,
		4,5,6,7,
		8,9,10,11,
		12,13,14,15,
		16,17,18,19,
		20,21,22,23,
		24,25,26,27,
		28,29,30,31,
		32,33,34,35
	};

	// Posiciones de prueba para cubos en diferentes ubicaciones
	// Nota: no se usan en la escena final, pero podrían servir para debug o prototipo
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};


// -----------------------------
// Configuración de buffers para renderizado de objetos normales
// -----------------------------
	GLuint VBO, VAO, EBO;  // Buffers: Vertex Array Object, Vertex Buffer Object, Element Buffer Object

	glGenVertexArrays(1, &VAO);   // Genera 1 VAO (almacena el estado de los atributos de vértices)
	glGenBuffers(1, &VBO);        // Genera 1 VBO (almacena los vértices)
	glGenBuffers(1, &EBO);        // Genera 1 EBO (almacena los índices de triángulos)
	
	glBindVertexArray(VAO);       // Activa el VAO para configurarlo
	glBindBuffer(GL_ARRAY_BUFFER, VBO);  // Enlaza el VBO como buffer de vértices
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);  // Carga los vértices al buffer

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);  // Enlaza el EBO como buffer de índices
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);  // Carga los índices al buffer

	// Atributo 0: posición (3 floats), inicio en 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	// Atributo 1: normales (3 floats), inicio en posición 3
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Atributo 2: coordenadas de textura (2 floats), inicio en posición 6
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);  // Desenlaza el VAO

	// -----------------------------
	// Configuración del VAO para representar la lámpara (pequeño cubo iluminado)
	// -----------------------------
	GLuint lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);

	// Se reutiliza el mismo VBO anterior, ya contiene los vértices del cubo
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	// Solo se necesita la posición para la lámpara (no normales ni texturas)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);  // Desenlaza el VAO de la lámpara

	// -----------------------------
	// Configuración del Skybox (fondo de entorno en forma de cubo)
	// -----------------------------
	GLuint skyboxVBO, skyboxVAO;
	glGenVertexArrays(1, &skyboxVAO);     // Genera VAO del skybox
	glGenBuffers(1, &skyboxVBO);          // Genera VBO del skybox
	glBindVertexArray(skyboxVAO);         // Enlaza el VAO
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);  // Enlaza el VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);  // Carga los vértices

	// Atributo 0: solo posición (skybox no usa normales ni texturas)
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);

	// -----------------------------
	// Cargar el cubemap (texturas del skybox)
	// -----------------------------
	// Se espera que 'faces' contenga las rutas a las 6 imágenes del cubemap (aquí aún está vacío)
	vector<const GLchar*> faces;
	GLuint cubemapTexture = TextureLoading::LoadCubemap(faces);  // Devuelve ID de textura del cubemap

	// -----------------------------
	// Matriz de proyección en perspectiva (para toda la escena)
	// -----------------------------
	// Calcula el campo visual en base al zoom de la cámara y el tamaño de la pantalla
	glm::mat4 projection = glm::perspective(camera.GetZoom(),
		(GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT,
		0.1f, 1000.0f);
	glEnable(GL_DEPTH_TEST);

	// -----------------------------
	// Bucle principal del juego/render
	// -----------------------------
	while (!glfwWindowShouldClose(window)) // Se repite hasta que el usuario cierre la ventana
	{
		// -----------------------------
		// Cálculo de deltaTime (tiempo entre frames)
		// -----------------------------
		GLfloat currentFrame = glfwGetTime();                // Tiempo actual (segundos desde inicio)
		deltaTime = currentFrame - lastFrame;                // Delta = diferencia de tiempo
		lastFrame = currentFrame;                            // Actualizar el último tiempo

		// -----------------------------
		// Procesamiento de entradas (teclado, mouse, etc.)
		// -----------------------------
		glfwPollEvents();    // Captura eventos de entrada
		DoMovement();        // Aplica los movimientos (cámara y animaciones activas)

		// -----------------------------
		// Limpieza de buffers de color y profundidad
		// -----------------------------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);                // Color de fondo (gris oscuro)
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  // Limpia buffers antes de dibujar

		// -----------------------------
		// Activar shader principal para iluminación
		// -----------------------------
		lightingShader.Use();  // Activa el shader lightingShader (programa GLSL)

		// -----------------------------
		// Pasar posición de la cámara al shader (para cálculos de iluminación especular)
		// -----------------------------
		GLint viewPosLoc = glGetUniformLocation(lightingShader.Program, "viewPos");
		glUniform3f(viewPosLoc, camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

		// -----------------------------
		// Propiedad del material general (brillo especular)
		// -----------------------------
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

		// -----------------------------
		// Luz direccional (como el sol o luz general ambiental)
		// -----------------------------
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.ambient"), 0.704f, 0.57f, 0.475f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.diffuse"), 0.1f, 0.1f, 0.1f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "dirLight.specular"), 0.1f, 0.1f, 0.1f);

		// -----------------------------
		// Luz puntual 0 - Exterior (carpa)
		// -----------------------------
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].position"), pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].ambient"), 0.01f, 0.01f, 0.01f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].diffuse"), 0.10f, 0.10f, 0.01f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[0].specular"), 1.0f, 1.0f, 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].linear"), 0.9917f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[0].quadratic"), 3.16f);

		// -----------------------------
		// Luz puntual 1 - Interior (recámara)  
		// -----------------------------
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].position"), pointLightPositions[1].x, pointLightPositions[1].y - 1.5f, pointLightPositions[1].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].ambient"), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].diffuse"), LightP1.x, LightP1.y, LightP1.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[1].specular"), LightP1.x, LightP1.y, LightP1.z);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].linear"), 0.50f);    // Subido
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[1].quadratic"), 0.50f); // Subidísimo para caída fuerte



		// -----------------------------
		// Luz puntual 2 - Luz solar (blanca, lejana)
		// -----------------------------
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].position"), pointLightPositions[2].x, pointLightPositions[2].y, pointLightPositions[2].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].ambient"), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].diffuse"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "pointLights[2].specular"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].linear"), 0.14f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "pointLights[2].quadratic"), 0.07f);

		// -----------------------------
		// Luz tipo spotlight (foco hacia abajo desde la lámpara interior)
		// -----------------------------
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.position"), pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.direction"), 0.0f, -1.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.ambient"), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.diffuse"), LightP1.x, LightP1.y, LightP1.z);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "spotLight.specular"), LightP1.x, LightP1.y, LightP1.z);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.constant"), 1.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.linear"), 0.09f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.quadratic"), 0.032f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.cutOff"), glm::cos(glm::radians(30.5f)));     // Ángulo de corte interior
		glUniform1f(glGetUniformLocation(lightingShader.Program, "spotLight.outerCutOff"), glm::cos(glm::radians(45.0f))); // Ángulo de corte exterior

		// (Ya estaba puesta antes pero se repite) - brillo general del material
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 32.0f);

		// -----------------------------
		// Transformaciones de cámara (vista)
		// -----------------------------
		glm::mat4 view;
		view = camera.GetViewMatrix(); // Calcula la matriz de vista (posición y orientación)

		// -----------------------------
		// Obtener ubicaciones de uniformes para model, view, projection
		// -----------------------------
		GLint modelLoc = glGetUniformLocation(lightingShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(lightingShader.Program, "view");
		GLint projLoc = glGetUniformLocation(lightingShader.Program, "projection");

		// -----------------------------
		// Enviar matrices al shader
		// -----------------------------
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// -----------------------------
		// Preparar para dibujar objetos
		// -----------------------------
		glBindVertexArray(VAO);             // Activa el VAO con configuración de atributos
		glm::mat4 tmp = glm::mat4(1.0f);    // Matriz temporal (no se usa en esta sección)


		//Carga de modelo 

		view = camera.GetViewMatrix(); // Obtiene la matriz de vista desde la cámara



		// --- Piso ---
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0.01f, 0.0f, 0.01f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));  
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Piso.Draw(lightingShader);

		// --- Casa  ---
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f);
		model = glm::mat4(1);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Cuphead.Draw(lightingShader);

		// --- Puerta ---
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.0f, 0.0f, 0.0f);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f);
		model = glm::mat4(1);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Puerta.Draw(lightingShader);

		

		// --- Configuración de materiales para la siguiente parte (buró animado, puertas, etc.) ---
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.ambient"), 0.1f, 0.1f, 0.1f);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.diffuse"), 0.619, 0.313, 0);
		glUniform3f(glGetUniformLocation(lightingShader.Program, "material.specular"), 0.1, 0.1, 0.1);
		glUniform1f(glGetUniformLocation(lightingShader.Program, "material.shininess"), 1.0f);

		// --- Base del buró ---
		model = glm::mat4(1); // Matriz de modelo inicial
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Buro.Draw(lightingShader); // Dibuja el modelo base del buró

		// --- Cajón del buró (CORREGIDO) ---
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-tras_cajon, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Buro_cajon.Draw(lightingShader);

		// --- Lampara (con transparencia) ---
		glEnable(GL_BLEND); // Activa mezcla para transparencia
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // Configura la fórmula de mezcla
		glUniform4f(glGetUniformLocation(lightingShader.Program, "colorAlpha"), 1.0f, 1.0f, 0.0f, 0.95f); // Color amarillo semitransparente
		model = glm::mat4(1);
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		Lampara.Draw(lightingShader);


		// --- Restaurar configuración sin transparencia ---
		glDisable(GL_BLEND);
		glUniform4f(glGetUniformLocation(lightingShader.Program, "colorAlpha"), 1.0f, 1.0f, 1.0f, 1.0f);
		glUniform1i(glGetUniformLocation(lightingShader.Program, "trans"), 1);

		// --- Sillon ---
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(0.2f, 0.0f, -0.9f));
		model = glm::scale(model, glm::vec3(1.3f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		sillon.Draw(lightingShader);

		// --- Piano ---
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-3.5f, 0.0f, 3.0f));
		model = glm::scale(model, glm::vec3(0.9f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		piano.Draw(lightingShader);

		//--- Estante ---
		model = glm::mat4(1.0f);
		// 1. Mover
		model = glm::translate(model, glm::vec3(-3.5f, 0.0f, -1.0f));
		model = glm::scale(model, glm::vec3(0.2f));

		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		estante.Draw(lightingShader);

		//--- Radio ---
		model = glm::mat4(1);

		// Posición original de la radio según tu código
		glm::vec3 posRadio = glm::vec3(-3.5f, 0.0f, -2.5f);

		// --- INICIO ANIMACIÓN VIBRACIÓN ---
		if (anim_radio) {
			float velocidadMusica = 12.0f;
			float fuerzaBajos = 0.01f;
			float vibracion = std::abs(glm::sin(glfwGetTime() * velocidadMusica)) * fuerzaBajos;
			posRadio.x += vibracion;
		}
		// --- FIN ANIMACIÓN ---

		model = glm::translate(model, posRadio); // Usamos la posición con vibración
		model = glm::scale(model, glm::vec3(0.5f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		radio.Draw(lightingShader);

		//--- Fonografo ---
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-4.6f, 0.5f, 3.0f));
		model = glm::scale(model, glm::vec3(0.7f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		fonografo.Draw(lightingShader);

		//--- Silla Mecedora ---
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-7.0f, 0.0f, 0.8f));
		model = glm::scale(model, glm::vec3(0.02f));

		// --- LÓGICA DE ANIMACIÓN CONDICIONAL ---
		float anguloMecedora = 0.0f;

		if (anim_mecedora) {

			anguloMecedora = glm::sin(glfwGetTime() * 2.0f) * 10.0f;
		}

		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

		model = glm::rotate(model, glm::radians(180.0f + anguloMecedora), glm::vec3(0.0f, 0.0f, 1.0f));

		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		sillaMecedora.Draw(lightingShader);

		//--- Espada ---
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-4.11f, 1.5f, 0.1f));
		model = glm::scale(model, glm::vec3(0.3f));
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		espada.Draw(lightingShader);

		//-- Chimenea ---
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-4.0f, 0.0f, 1.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		chimenea.Draw(lightingShader);

		//-- Estante ---
		model = glm::mat4(1);
		model = glm::translate(model, glm::vec3(-6.0f, 0.0f, -0.5f));
		model = glm::scale(model, glm::vec3(1.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		estante2.Draw(lightingShader);

		// --- Animación de tiempo ---
		speed = 0.5f;
		tiempo = speed * glfwGetTime();
		glUniform1f(glGetUniformLocation(lightingShader.Program, "time"), tiempo);

		// --- Preparar objeto transparente (comentado) ---
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::mat4(1);
		model = glm::scale(model, glm::vec3(1.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1f(glGetUniformLocation(lightingShader.Program, "transparencia"), 0.0);
		//objTras.Draw(lightingShader); // Línea comentada, probablemente objeto transparente aún no definido
		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(0);

		// --- Shader para pantalla animada ---
		animShader2.Use();
		speed = 0.001f;
		tiempo = speed * glfwGetTime();

		modelLoc = glGetUniformLocation(animShader2.Program, "model");
		viewLoc = glGetUniformLocation(animShader2.Program, "view");
		projLoc = glGetUniformLocation(animShader2.Program, "projection");
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform1f(glGetUniformLocation(animShader2.Program, "time"), tiempo);


		glBindVertexArray(0); // Desvincula VAO


		// --- Shader para animación de partículas (humo) ---
		animShader.Use();

		// 1. CONTROL DE VELOCIDAD:
		// Para que vaya más despacio, reduce este número.
		// Antes tenías 3.8f (muy rápido). Prueba con 1.0f o 0.5f.
		speed = 4.0f;

		tiempo = speed * glfwGetTime();

		// Ubicamos las variables uniformes para el shader animado
		modelLoc = glGetUniformLocation(animShader.Program, "model");
		viewLoc = glGetUniformLocation(animShader.Program, "view");
		projLoc = glGetUniformLocation(animShader.Program, "projection");

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Enviamos las matrices view y projection
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glUniform1f(glGetUniformLocation(animShader.Program, "time"), tiempo);
		glUniform4f(glGetUniformLocation(animShader.Program, "colorAlpha"), 1.0f, 1.0f, 1.0f, 0.01f);

		glm::vec3 cameraPos = camera.GetPosition();

		// 2. CONTROL DE POSICIÓN:
		// Cambia estos 3 números para mover el humo de lugar:
		// X (Izquierda/Derecha), Y (Arriba/Abajo), Z (Fondo/Frente)
		glm::vec3 humoPosition = glm::vec3(-2.90f, 3.0f, -2.0f);

		model = glm::mat4(1.0f);
		model = glm::translate(model, humoPosition);

		// Aplica rotación para que el humo siempre mire hacia la cámara (billboarding)
		// Esto hace que el humo parezca 3D aunque sea una imagen plana
		model[0][0] = view[0][0];
		model[0][1] = view[1][0];
		model[0][2] = view[2][0];
		model[1][0] = view[0][1];
		model[1][1] = view[1][1];
		model[1][2] = view[2][1];
		model[2][0] = view[0][2];
		model[2][1] = view[1][2];
		model[2][2] = view[2][2];

		model = glm::scale(model, glm::vec3(0.534f, 0.70f, 0.333f));

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		Humo.Draw(animShader);

		glDisable(GL_BLEND);
		


		glBindVertexArray(0); // Desvincula cualquier VAO activo


		// --- Renderizado del objeto de luz (lámpara pequeña) ---
		lampShader.Use(); // Activamos el shader de lámpara

		// Obtenemos las ubicaciones de las matrices
		modelLoc = glGetUniformLocation(lampShader.Program, "model");
		viewLoc = glGetUniformLocation(lampShader.Program, "view");
		projLoc = glGetUniformLocation(lampShader.Program, "projection");

		// Enviamos las matrices view y projection
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// Posiciona la lámpara principal (no se usa directamente aquí)
		model = glm::mat4(1);
		model = glm::translate(model, lightPos);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		// Dibuja la luz puntual como un pequeño cubo
		glBindVertexArray(lightVAO);
		model = glm::mat4(1);
		model = glm::translate(model, pointLightPositions[1]); // Posición de la luz interior
		model = glm::scale(model, glm::vec3(0.1f)); // Escala el cubo para que sea pequeño (representa fuente de luz)
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36); // Dibuja el cubo de la lámpara
		glBindVertexArray(0);


		// --- Renderizado del Skybox (último en dibujarse) ---
		glDepthFunc(GL_LEQUAL); // Permite pasar la prueba de profundidad cuando los valores sean iguales (para skybox)

		SkyBoxshader.Use(); // Activa el shader del skybox
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // Elimina la traslación de la matriz de vista (mantiene la rotación)
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(glGetUniformLocation(SkyBoxshader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(skyboxVAO); // Vincula el VAO del cubo del skybox
		glActiveTexture(GL_TEXTURE1); // Activa la textura para el skybox
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture); // Enlaza la textura tipo cubemap
		glDrawArrays(GL_TRIANGLES, 0, 36); // Dibuja el cubo del skybox
		glBindVertexArray(0);

		glDepthFunc(GL_LESS); // Restaura el valor por defecto del test de profundidad


		// --- Intercambio de buffers ---
		glfwSwapBuffers(window); // Muestra en pantalla el frame renderizado
	}



	// Libera los recursos asociados al VAO principal
	glDeleteVertexArrays(1, &VAO);
	// Libera los recursos asociados al VAO utilizado para la lámpara
	glDeleteVertexArrays(1, &lightVAO);
	// Libera el VBO (Vertex Buffer Object) que contiene los vértices
	glDeleteBuffers(1, &VBO);
	// Libera el EBO (Element Buffer Object) que contiene los índices
	glDeleteBuffers(1, &EBO);
	// Libera el VAO usado para el skybox
	glDeleteVertexArrays(1, &skyboxVAO);
	// Libera el VBO del skybox
	glDeleteBuffers(1, &skyboxVBO);
	// Termina el contexto de GLFW y libera todos los recursos reservados por GLFW
	glfwTerminate();


	return 0;
}


// Is called whenever a key is pressed/released via GLFW
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{



	if (GLFW_KEY_ESCAPE == key && GLFW_PRESS == action)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
		{
			keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
		}
	}

	if (keys[GLFW_KEY_1]) {
		if (rot_Puerta_ent >= 90.0f || rot_Puerta_ent <= 0.001f) {
			anim_Puerta_ent = !anim_Puerta_ent;
		}
	}


	if (keys[GLFW_KEY_2]) {
		if (tras_cajon >= 0.0f || tras_cajon <= -0.487f) {
			anim_cajon = !anim_cajon;
		}
	}

	if (keys[GLFW_KEY_3])
	{
		active = !active;
		if (active)
			LightP1 = glm::vec3(1.0f, 1.0f, 0.0f);
		else
			LightP1 = glm::vec3(0.0f, 0.0f, 0.0f);
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		anim_mecedora = !anim_mecedora; 
	}
	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
	{
		anim_radio = !anim_radio;
	}
}

// Callback que se ejecuta cada vez que se mueve el mouse dentro de la ventana
void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
	// Si es la primera vez que se mueve el mouse, se inicializan las coordenadas previas
	if (firstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		firstMouse = false;
	}

	// Calcula la diferencia de movimiento del mouse desde el último cuadro
	GLfloat xOffset = xPos - lastX;
	GLfloat yOffset = lastY - yPos;  // Invertido porque en la pantalla, Y va de abajo hacia arriba
	lastX = xPos;
	lastY = yPos;

	// Pasa las diferencias de movimiento al sistema de cámara para girar la vista
	camera.ProcessMouseMovement(xOffset, yOffset);
}

// Función que actualiza el estado de animaciones y la cámara en cada cuadro
void DoMovement()
{
	// Animación de la puerta de entrada: abre si está activada y aún no llega a 90°
	if (anim_Puerta_ent) {
		if (rot_Puerta_ent < 90.0f) {
			rot_Puerta_ent += 100.0f * deltaTime;
		}
	}
	// Cierra si está desactivada y aún no regresa a 0°
	if (!anim_Puerta_ent) {
		if (rot_Puerta_ent > 0.0f) {
			rot_Puerta_ent -= 100.0f * deltaTime;
		}
	}


	// Animación del cajón: se desliza hacia afuera si está activado
	if (anim_cajon) {
		if (tras_cajon > -0.487f) {
			tras_cajon -= 4.0f * deltaTime;
		}
	}
	// Se regresa a su lugar si se desactiva
	if (!anim_cajon) {
		if (tras_cajon < 0.0f) {
			tras_cajon += 4.0f * deltaTime;
		}
	}


	// Controles de movimiento de cámara (WASD o flechas)
	if (keys[GLFW_KEY_W] || keys[GLFW_KEY_UP])
	{
		camera.ProcessKeyboard(FORWARD, deltaTime); // Avanza
	}

	if (keys[GLFW_KEY_S] || keys[GLFW_KEY_DOWN])
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime); // Retrocede
	}

	if (keys[GLFW_KEY_A] || keys[GLFW_KEY_LEFT])
	{
		camera.ProcessKeyboard(LEFT, deltaTime); // Izquierda
	}

	if (keys[GLFW_KEY_D] || keys[GLFW_KEY_RIGHT])
	{
		camera.ProcessKeyboard(RIGHT, deltaTime); // Derecha
	}
}
