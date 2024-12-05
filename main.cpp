

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <fstream>
#include <sstream>
#include <irrKlang.h>


#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
using namespace irrklang;

unsigned int compileShader(GLenum type, const char* source);
unsigned int createShader(const char* vsSource, const char* fsSource);
static unsigned loadImageToTexture(const char* filePath); 
void SetupTexture(GLuint textureID);
bool CheckCollision(float avatarX, float avatarY, float landX, float landY, float landWidth, float landHeight);
bool CheckAllCollisions(float avatarX, float avatarY, float xValues[], int numLands, float y, float landWidth, float landHeight);
void setupVAOandVBO(unsigned int& VAO, unsigned int& VBO, const float* vertices, size_t verticesSize, unsigned int stride);

int main(void)
{

    if (!glfwInit())
    {
        std::cout<<"GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    ISoundEngine* soundEngine = createIrrKlangDevice();
    if (!soundEngine) {
        std::cerr << "Greška: Zvucni engine nije mogao da se inicijalizuje!" << std::endl;
        return 1;
    }

    soundEngine->play2D("music/background.mp3", true);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //bez zastarelih funckija

    GLFWwindow* window;
    unsigned int wWidth = 800;
    unsigned int wHeight = 800;
    const char wTitle[] = "[My icy tower]";
    window = glfwCreateWindow(wWidth, wHeight, wTitle, glfwGetPrimaryMonitor(), NULL); 
    
    if (window == NULL)
    {
        std::cout << "Prozor nije napravljen! :(\n";
        glfwTerminate();
        return 2;
    }

    glfwMakeContextCurrent(window); 
    glfwSwapInterval(1); //sinhronizacija

    if (glewInit() != GLEW_OK)
    {
        std::cout << "GLEW nije mogao da se ucita! :'(\n";
        return 3;
    }

    //jako bitno nije radilo bez ovog
    int screenWidth, screenHeight; //prilagodjavanje
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);

    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");

    float background_vertices[] = {
        -1.0, -1.0,  0.0, 0.0, 
        1.0, -1.0,   1.0, 0.0, 
        1.0, 1.0,    1.0, 1.0, 

        -1.0, -1.0,  0.0, 0.0, 
        1.0, 1.0,    1.0, 1.0, 
        -1.0, 1.0,   0.0, 1.0  
    };

    float leftWall[] = {
        -1.0,  1.0,  0.0, 1.0,  
        -0.8,  1.0,  1.0, 1.0,  
        -1.0, -1.0,  0.0, 0.0,  

        -1.0, -1.0,  0.0, 0.0,  
        -0.8,  1.0,  1.0, 1.0,  
        -0.8, -1.0,  1.0, 0.0   
    };


    float rightWall[] = {
         0.8f,  1.0,  0.0, 1.0f,  
         1.0f,  1.0,  1.0, 1.0f,  
         0.8f, -1.0,  0.0, 0.0f,  

          0.8f, -1.0,  0.0, 0.0,  
          1.0f,  1.0,  1.0, 1.0,  
          1.0f, -1.0,  1.0, 0.0   
    };

    float letter[] = {
        -0.785,  0.965,  0.0, 1.0,  
        -0.515,  0.965,  1.0, 1.0,  
        -0.785,  0.89,  0.0, 0.0,  

        -0.785,  0.89,  0.0, 0.0,  
        -0.515,  0.965,  1.0, 1.0,  
        -0.515,  0.89,  1.0, 0.0   
    };

    float name[] = {
        0.455,  0.99,  0.0, 1.0,  
        0.785,  0.99,  1.0, 1.0,  
        0.455,  0.80,  0.0, 0.0,  

        0.455,  0.80,  0.0, 0.0,  
        0.785,  0.99,  1.0, 1.0,  
        0.785,  0.80,  1.0, 0.0   
    };

    float avatar[] = {
         -0.06,  -0.72,  0.0, 1.0,  
         0.06,  -0.72,  1.0, 1.0, 
         -0.06, -0.96,  0.0, 0.0,  

         -0.06, -0.96,  0.0, 0.0,  
         0.06,  -0.72,  1.0, 1.0,  
         0.06, -0.96,  1.0, 0.0   
    };

    float land[] = {
        -0.15, -0.03,
         0.15, -0.03,
        -0.15,  0.03,

        -0.15,  0.03,
         0.15, -0.03,
         0.15,  0.03,
    };

    float game_finished[] = {
        -0.7, -0.5,  0.0, 0.0, 
        0.7, -0.5,   1.0, 0.0, 
        0.7, 0.5,    1.0, 1.0, 

        -0.7, -0.5,  0.0, 0.0, 
        0.7, 0.5,    1.0, 1.0, 
        -0.7, 0.5,   0.0, 1.0  
    };


    unsigned int stride = (2 + 2) * sizeof(float);
    unsigned int stride_color = (2) * sizeof(float);
    
    //napravila funkciju da bude lakse, pretprocesiranje
    unsigned int backVAO, backVBO, leftWallVAO, leftWallVBO, rightWallVAO, rightWallVBO, letterVAO, letterVBO, avatarVAO, avatarVBO, finishedVAO, finishedVBO, nameVAO, nameVBO;
    setupVAOandVBO(backVAO, backVBO, background_vertices, sizeof(background_vertices), stride);
    setupVAOandVBO(leftWallVAO, leftWallVBO, leftWall, sizeof(leftWall), stride);
    setupVAOandVBO(rightWallVAO, rightWallVBO, rightWall, sizeof(rightWall), stride);
    setupVAOandVBO(letterVAO, letterVBO, letter, sizeof(letter), stride);
    setupVAOandVBO(avatarVAO, avatarVBO, avatar, sizeof(avatar), stride);
    setupVAOandVBO(finishedVAO, finishedVBO, game_finished, sizeof(game_finished), stride);
    setupVAOandVBO(nameVAO, nameVBO, name, sizeof(name), stride);


    // pretprocesiranje sa land
    unsigned int landVAO;
    glGenVertexArrays(1, &landVAO); 
    glBindVertexArray(landVAO); 
    unsigned int landVBO;
    glGenBuffers(1, &landVBO); 
    glBindBuffer(GL_ARRAY_BUFFER, landVBO); 
    glBufferData(GL_ARRAY_BUFFER, sizeof(land), land, GL_STATIC_DRAW); //Slanje podataka na memoriju graficke karte
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride_color, (void*)0);
    glEnableVertexAttribArray(0); 
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    
    //Tekstura
    unsigned backTexture = loadImageToTexture("res/brick2.jpg"); 
    unsigned wallTexture = loadImageToTexture("res/new.png");
    unsigned letterTexture1 = loadImageToTexture("res/level1.png");
    unsigned letterTexture2 = loadImageToTexture("res/level2.png");
    unsigned letterTexture3 = loadImageToTexture("res/level3.png");
    unsigned letterTexture4 = loadImageToTexture("res/level4.png");
    unsigned letterTexture5 = loadImageToTexture("res/level5.png");
    unsigned avatarLeft = loadImageToTexture("res/avatarLeft.png");
    unsigned avatarRight = loadImageToTexture("res/avatarRight.png");
    unsigned avatarMiddle = loadImageToTexture("res/avatarMiddle.png");
    unsigned gameOver = loadImageToTexture("res/game_over.png");
    unsigned gameWin = loadImageToTexture("res/game_win.png");
    unsigned nameTexture = loadImageToTexture("res/name.png");


    glBindTexture(GL_TEXTURE_2D, backTexture); //Podesavamo teksturu
    glGenerateMipmap(GL_TEXTURE_2D);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//   GL_REPEAT, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindTexture(GL_TEXTURE_2D, wallTexture); //Podesavamo teksturu za zid
    glGenerateMipmap(GL_TEXTURE_2D); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);   
    glBindTexture(GL_TEXTURE_2D, 0);

    GLuint textures[11] = { letterTexture1, letterTexture2, letterTexture3, letterTexture4, letterTexture5, avatarLeft, avatarRight, avatarMiddle, gameOver, gameWin, nameTexture};

    for (int i = 0; i < 11; ++i) {
        SetupTexture(textures[i]);
    }


    glUseProgram(unifiedShader);
    
    //prenos iz CPU u GPU, teksture
    unsigned uTexLoc = glGetUniformLocation(unifiedShader, "uTex");
    glUniform1i(uTexLoc, 0); 
    unsigned uWallTexLoc = glGetUniformLocation(unifiedShader, "uWallTex");
    glUniform1i(uWallTexLoc, 1);
    unsigned uLetterTexLoc = glGetUniformLocation(unifiedShader, "uLetterTex");
    glUniform1i(uLetterTexLoc, 2);
    unsigned uAvatarTexLoc = glGetUniformLocation(unifiedShader, "uAvatarTex");
    glUniform1i(uAvatarTexLoc, 3);
    unsigned uFinishedTexLoc = glGetUniformLocation(unifiedShader, "uFinishedTex");
    glUniform1i(uFinishedTexLoc, 4);
    unsigned uNameTexLoc = glGetUniformLocation(unifiedShader, "uNameTex");
    glUniform1i(uNameTexLoc, 5);
    
    glUseProgram(0);
    

    float rectangleColor[3] = {1.0, 1.0, 1.0}; // boja (bela)
    unsigned int uA = glGetUniformLocation(unifiedShader, "uA");
    unsigned int Loc = glGetUniformLocation(unifiedShader, "Loc");
    unsigned int isObject = glGetUniformLocation(unifiedShader, "isObject");


    //koordinate za land
    float x = 0;
    float y = 0;

    float color = 0.0; //boja
    double lastTimeColor  = 0.0; //vreme za boju
    int levelNum = 1; //koju teksturu za level
    float landWidth = 1.0; //koliko mnozimo sirinu
    double currentTimeColor;

    bool gameActive = false;
    double lastTimeGeneral = glfwGetTime(); //ovo je da se pomera generalno
    double loopRange = 10.0;
    bool isGameOver = false;
    bool isGameCompleted = false;

    //kontrola da se izvrsava 60 frejmova po sekundi
    const double TARGET_FPS = 60.0;
    const double FRAME_DURATION = 1.0 / TARGET_FPS;
    double previousTime = glfwGetTime();
    double elapsedTime; 
    
    //koristi se da space ne bi mogao da se drzi
    float previousSpaceTime = glfwGetTime();
    float currentSpaceTime;
    bool isSpacePressed = false;

    //avatar
    bool isLeft = false; //koju teksturu da uzme levu ili desnu
    bool isRight = false;
    float xAvatar = 0.0;
    float yAvatar = 0.0;
    float speedY = 0.0;  
    float speedX = 0.0;
    float acceleratorX = 0.000000004;
    float friction = 0.0000005;
    const double bounce_loss = 0.7;
    float gravity = -0.001; 



    bool isOnLand = true;
    bool isSpace = false;

    //land 
    float xValues[24] = {
            -0.5,  0.3,  -0.2,  0.6,  -0.4,
             0.1, -0.65,  0.5,  -0.3,  0.2,
            -0.6,  0.4,  -0.1,  0.0,  -0.55,
             0.35, -0.15, 0.65,  -0.45, 0.25,
             -0.5,  0.3,   -0.2, 0.6, // poslednje 4 su iste da se ne vidi prelaz
    };


    while (!glfwWindowShouldClose(window))
    {
        
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }
        

        double currentTimeGeneral = glfwGetTime();

        if (gameActive) {
            if (currentTimeGeneral - lastTimeGeneral >= 0.005) {
                if (y > -loopRange)
                {
                    lastTimeGeneral = currentTimeGeneral;
                    y -= 0.0015;
                }
                else {
                    y = fmod(y + loopRange, loopRange); 
                }

            }
        }

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (!isSpacePressed) {
                isSpacePressed = true; 
                previousSpaceTime = glfwGetTime();

                //start
                if (!gameActive) {
                    gameActive = true;
                    lastTimeColor = glfwGetTime(); // za boju
                }
                
                isSpace = true; //za skakanje avatara
                soundEngine->play2D("music/jump.mp3", false);

                

                //boja, level, sirina
                currentTimeColor = glfwGetTime();
                if (currentTimeColor - lastTimeColor >= 15.0) { 
                    lastTimeColor = currentTimeColor; 
                    if (color < 1.0) {
                        color += 0.25;
                        if (levelNum < 5) {
                            levelNum += 1;
                            landWidth *= 0.85;
                        }
                        
                    }
                    else {
                        color = 0.0;
                        isGameCompleted = true;
                        gameActive = false;
                    }
                }
            }
        }
        else {
            currentSpaceTime = glfwGetTime();
            if (currentSpaceTime - previousSpaceTime > 0.01)
            {
                isSpacePressed = true;
                previousSpaceTime = currentSpaceTime;
            }
            else {
                isSpacePressed = false; // Reset status kada taster nije pritisnut
            }
            
        }

        //kretanje levo desno sa lakom fizikom
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            speedX -= acceleratorX; //ubrzanje
        }
        else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            speedX += acceleratorX;
        }
        else {
            if (speedX > 0) {
                speedX -= friction; //usporenje
                if (speedX < 0) speedX = 0; // zaustavljanje
            }
            else if (speedX < 0) {
                speedX += friction;
                if (speedX > 0) speedX = 0;
            }
        }
             
            
        double currentTime = glfwGetTime();
        elapsedTime = currentTime - previousTime; 
        
        if (elapsedTime >= FRAME_DURATION) //ako je proslo dovoljno vremena app moze crtati novi frejm
        {
            //kraj igrice
            if (yAvatar < -1.05)
            {
                isGameOver = true;
                gameActive = false;
            }

            //koju tekturu avatara da uzme
            if (speedX > 0)
            {
                isLeft = false;
                isRight = true;
            }
            else if (speedX < 0)
            {
                isLeft = true;
                isRight = false;
            }
            else {
                isLeft = false;
                isRight = false;
            }

            //odbijanje od zida i kretanje
            xAvatar += speedX;

            if (xAvatar <= -0.8) {
                xAvatar = -0.8; 
                speedX = -speedX *bounce_loss; 
            }
            if (xAvatar >= 0.8) {
                xAvatar = 0.8; // zadrzi ga
                speedX = -speedX * bounce_loss; // obrni smer + loss
            }

            isOnLand = CheckAllCollisions(xAvatar, yAvatar, xValues, 24, y, landWidth, 0.06);
            //gravitacija
            if (gameActive)
            {
                if (!isOnLand) {
                    speedY += gravity; 
                    yAvatar += speedY; 
                }
                else {
                    speedY = 0.0;
                }

            }

            //skakanje
            if ((isOnLand && isSpace) || !gameActive) 
            {
                speedY = 0.035; 
                isOnLand = false; 
                isSpace = false;
            }

            //pozadina
            glClearColor(0.0, 0.0, 0.0, 1.0);
            glClear(GL_COLOR_BUFFER_BIT);

            glUseProgram(unifiedShader);
            glUniform3f(Loc, 0.0, 0.0, 1.0);
            glUniform1f(uA, color);
            glUniform1f(isObject, 0.0); 
        

            glActiveTexture(GL_TEXTURE0); 
            glBindTexture(GL_TEXTURE_2D, backTexture);

            glBindVertexArray(backVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindTexture(GL_TEXTURE_2D, 0); // jako bitno ugasiti teksturu pre aktivacije nove

            //wall
            glUniform1f(isObject, 4.0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, wallTexture);
            glBindVertexArray(leftWallVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glBindVertexArray(rightWallVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            glBindTexture(GL_TEXTURE_2D, 0);

            //startLand 

            if (!gameActive && !isGameOver && !isGameCompleted)
            {
                glUniform1f(isObject, 1.0); 
                glBindVertexArray(landVAO);
                glUniform3f(Loc, 0, -1.0, 5.3);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }


            //land
        
            glUniform1f(isObject, 1.0); 
            glBindVertexArray(landVAO);

            for (int i = -1; i < 24; i++)
            {
                glUniform3f(Loc, xValues[i + 1], y + i * 0.5, landWidth);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            //letter 
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUniform3f(Loc, 0.0, 0.0, 1.0);
            unsigned selectedTexture = 0;
            glActiveTexture(GL_TEXTURE2);
            if (levelNum == 1)
            {
                glBindTexture(GL_TEXTURE_2D, letterTexture1);
            }
            else if (levelNum == 2)
            {
                glBindTexture(GL_TEXTURE_2D, letterTexture2);
            }
            else if (levelNum == 3)
            {
                glBindTexture(GL_TEXTURE_2D, letterTexture3);
            }
            else if (levelNum == 4)
            {
                glBindTexture(GL_TEXTURE_2D, letterTexture4);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, letterTexture5);
            }
            glUniform1f(isObject, 2.0);
            glBindVertexArray(letterVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_BLEND);

            //name
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUniform3f(Loc, 0.0, 0.0, 1.0);
            glUniform1f(isObject, 6.0);
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D, nameTexture);
            glBindVertexArray(nameVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_BLEND);


            // avatar

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glUniform3f(Loc, xAvatar, yAvatar, 1.0);
            glUniform1f(isObject, 3.0);
            glActiveTexture(GL_TEXTURE3);
            if (!isLeft && !isRight)
            {
                glBindTexture(GL_TEXTURE_2D, avatarMiddle);
            }
            else if (isLeft)
            {
                glBindTexture(GL_TEXTURE_2D, avatarLeft);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, avatarRight);
            }
            
            glBindVertexArray(avatarVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindTexture(GL_TEXTURE_2D, 0);
            glDisable(GL_BLEND);

            // game over ili finished
            if (isGameOver || isGameCompleted)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glUniform3f(Loc, 0.0, 0.0, 1.0);
                glUniform1f(isObject, 5.0);
                glActiveTexture(GL_TEXTURE4);
                if (isGameCompleted)
                {
                    glBindTexture(GL_TEXTURE_2D, gameWin);
                }
                else {
                    
                    glBindTexture(GL_TEXTURE_2D, gameOver);
                }
                glBindVertexArray(finishedVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindTexture(GL_TEXTURE_2D, 0);
                glDisable(GL_BLEND);
            }
            
            



            glBindVertexArray(0);
            glUseProgram(0);
        

            glfwSwapBuffers(window); //glatko prikazivanje
            glfwPollEvents(); //plus obrada dogadjaja
            previousTime = currentTime;
        }

            
        
    }

    glDeleteTextures(1, &wallTexture);
    glDeleteTextures(1, &backTexture);
    glDeleteTextures(1, &letterTexture1);
    glDeleteTextures(1, &letterTexture2);
    glDeleteTextures(1, &letterTexture3);
    glDeleteTextures(1, &letterTexture4);
    glDeleteTextures(1, &letterTexture5);
    glDeleteTextures(1, &avatarLeft);
    glDeleteTextures(1, &avatarRight);
    glDeleteTextures(1, &avatarMiddle);
    glDeleteTextures(1, &gameOver);
    glDeleteTextures(1, &gameWin);
    glDeleteTextures(1, &nameTexture);

    
    glDeleteBuffers(1, &backVBO);
    glDeleteVertexArrays(1, &backVAO);
    glDeleteBuffers(1, &leftWallVBO);
    glDeleteVertexArrays(1, &leftWallVAO);
    glDeleteBuffers(1, &rightWallVBO);
    glDeleteVertexArrays(1, &rightWallVAO);
    glDeleteBuffers(1, &letterVBO);
    glDeleteVertexArrays(1, &letterVAO);
    glDeleteBuffers(1, &avatarVBO);
    glDeleteVertexArrays(1, &avatarVAO);
    glDeleteBuffers(1, &finishedVBO);
    glDeleteVertexArrays(1, &finishedVAO);
    glDeleteBuffers(1, &nameVBO);
    glDeleteVertexArrays(1, &nameVAO);

    soundEngine->drop();
    glDeleteProgram(unifiedShader);
    glfwTerminate();
    return 0;
}

unsigned int compileShader(GLenum type, const char* source)
{
    std::string content = "";
    std::ifstream file(source);
    std::stringstream ss;
    if (file.is_open())
    {
        ss << file.rdbuf();
        file.close();
        std::cout << "Uspjesno procitao fajl sa putanje \"" << source << "\"!" << std::endl;
    }
    else {
        ss << "";
        std::cout << "Greska pri citanju fajla sa putanje \"" << source << "\"!" << std::endl;
    }
     std::string temp = ss.str();
     const char* sourceCode = temp.c_str(); //u niz karaktera

    int shader = glCreateShader(type);
    
    int success;
    char infoLog[512];
    glShaderSource(shader, 1, &sourceCode, NULL); //postavlja izvorni kod za šejder objekat
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); //uspesnost kompajliranja
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        if (type == GL_VERTEX_SHADER)
            printf("VERTEX");
        else if (type == GL_FRAGMENT_SHADER)
            printf("FRAGMENT");
        printf(" sejder ima gresku! Greska: \n");
        printf(infoLog);
    }
    return shader;
}
unsigned int createShader(const char* vsSource, const char* fsSource)
{
    
    unsigned int program;
    unsigned int vertexShader;
    unsigned int fragmentShader;

    program = glCreateProgram(); //prazan

    vertexShader = compileShader(GL_VERTEX_SHADER, vsSource);
    fragmentShader = compileShader(GL_FRAGMENT_SHADER, fsSource);

    
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program); //povezivanje
    glValidateProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
    if (success == GL_FALSE)
    {
        glGetShaderInfoLog(program, 512, NULL, infoLog);
        std::cout << "Objedinjeni sejder ima gresku! Greska: \n";
        std::cout << infoLog << std::endl;
    }

    glDetachShader(program, vertexShader); //ne trebaju nam kao zasebni
    glDeleteShader(vertexShader);
    glDetachShader(program, fragmentShader);
    glDeleteShader(fragmentShader);

    return program;
}
static unsigned loadImageToTexture(const char* filePath) {
    int TextureWidth;
    int TextureHeight;
    int TextureChannels;
    unsigned char* ImageData = stbi_load(filePath, &TextureWidth, &TextureHeight, &TextureChannels, 0);
    if (ImageData != NULL)
    {
        //Slike se osnovno ucitavaju naopako pa se moraju ispraviti da budu uspravne
        stbi__vertical_flip(ImageData, TextureWidth, TextureHeight, TextureChannels);

        // Provjerava format 
        GLint InternalFormat = -1;
        switch (TextureChannels) {
        case 1: InternalFormat = GL_RED; break;
        case 2: InternalFormat = GL_RG; break;
        case 3: InternalFormat = GL_RGB; break;
        case 4: InternalFormat = GL_RGBA; break;
        default: InternalFormat = GL_RGB; break;
        }

        unsigned int Texture;
        glGenTextures(1, &Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, TextureWidth, TextureHeight, 0, InternalFormat, GL_UNSIGNED_BYTE, ImageData); //alociranje memorije u GPU za tekturu sa zadatim dimenzijama i formatom
        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(ImageData);
        return Texture;
    }
    else
    {
        std::cout << "Textura nije ucitana! Putanja texture: " << filePath << std::endl;
        stbi_image_free(ImageData);
        return 0;
    }


}

void SetupTexture(GLuint textureID) {
    glBindTexture(GL_TEXTURE_2D, textureID); 
    glGenerateMipmap(GL_TEXTURE_2D); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

    glBindTexture(GL_TEXTURE_2D, 0); 
}


bool CheckCollision(float avatarX, float avatarY, float landX, float landY, float landWidth, float landHeight) {
    bool collisionX = avatarX + 0.06 >= landX - landWidth*0.3 / 2.0 && avatarX - 0.06 <= landX + landWidth*0.3 / 2.0; // Širina avatara je 0.12
    bool collisionY = avatarY -0.84  - 0.12 <= landY + landHeight / 2.0 && avatarY - 0.84 - 0.12 >= landY - landHeight / 2.0;    
    return collisionX && collisionY;
}

bool CheckAllCollisions(float avatarX, float avatarY, float xValues[], int numLands, float y, float landWidth, float landHeight) {
    for (int i = -1; i < numLands; i++) {
        float landX = xValues[i + 1];
        float landY = y + i * 0.5; // Racunamo Y koordinatu
        if (CheckCollision(avatarX, avatarY, landX, landY, landWidth, landHeight)) {
            return true;
        }
    }
    return false;
}

void setupVAOandVBO(unsigned int& VAO, unsigned int& VBO, const float* vertices, size_t verticesSize, unsigned int stride) {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verticesSize, vertices, GL_STATIC_DRAW);

    // Podesila prvi atribut (pozicija)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0); //location 0
    glEnableVertexAttribArray(0);

    // Podesila drugi atribut (boja/tekstura)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}













