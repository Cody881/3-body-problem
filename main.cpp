#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <string>
#include "object.cpp"
#include "raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "raymath.h"

using namespace std;
//screen width / height
int screenWidth = 1000;
int screenHeight = 750;
//variables needed to create a panning feature
Vector2 offset = {0.0, 0.0};
bool dragging = false;
Vector2 lastMouse = {0.0, 0.0};
float zoom = 1.0;   // This is the scale of zoom
//gravitational constant
const double G = 6.6743 * pow(10, -11);
//scaling for time
int timeScaleOptions[4] = {86400, 604800, (int)(2.638 * pow(10, 6)), (int)(3.154 * pow(10, 7))}; // day/s, week/s, month/s, year/s,
int timeScale = timeScaleOptions[0]; //sim seconds per real life seconds
const int substeps = 100; //how many times we are running physics per frame
//Assigning astronomical units to pixels
const int AU = 200;
const double KM_to_AU = 6.6846 * pow(10, -9);
//scale to see planets (makes the simulation not perfectly to scale)
int planet_scale = 1000;
//number of objects on screen
int OBJECTNUM = 9;
//variables to store data from slider bar
float slideValue = 0.0;
int selection = 0;
float minValue = 0.0;
float maxValue = 100.0;

//used as a vector to hold intial data for object
struct planetData {
    string name;
    double mass; // in KG
    double rad; //in KM
    float distance; //in KM from sun, transformed to AU when drawn
    float vel;
};

//function to read each line of csv file
planetData parseLine(string line) {
    stringstream ss(line);
    //string variables to get the nums out of line
    string tempMass, tempRad, tempPos, tempVel;
    planetData p;

    getline(ss, p.name, ',');
    getline(ss, tempMass, ',');
    p.mass = stod(tempMass);
    getline(ss, tempRad, ',');
    p.rad = stod(tempRad);
    getline(ss, tempPos, ',');
    p.distance = stod(tempPos);
    getline(ss, tempVel, ',');
    p.vel = stod(tempVel);

    return p;
}

//converts data vector to data of the planet
void dataToObject(planetData p, Object &planet) {
    planet.setName(p.name);
    planet.setMass(p.mass);
    planet.setRad(p.rad);
    planet.setPos({p.distance / (float)KM_to_AU, 0});
    planet.setVel({0, p.vel});
}

//selects time scale based on the value of the slider
int selectingTime(float &slideValue) {
    int selection;
    if (slideValue < 25) {
        selection = 0;
        slideValue = 0;
    } else if (slideValue < 50){
        selection = 1;
        slideValue = 33;
    } else if (slideValue < 75) {
        selection = 2;
        slideValue = 66;
    } else {
        selection = 3;
        slideValue = 100;
    }
    return selection;
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "raylib window");
    SetWindowMinSize(screenWidth, screenHeight);
    SetTargetFPS(300);
    Object planets[OBJECTNUM];

    //intializing the planets, 0 being the sun and 1-8 being the planets in the solar system
    string line; // stores line of csv file
    vector<planetData> tempVec;
    //opening planet data file (stores all data needed to start simulation)
    ifstream fin("planetData.csv");
    if (!fin.is_open()) {
        cout << "error opening file" << endl;
        return 1;
    }
    //getting rid of header line
    getline(fin, line);
    //looping through rest of data
    int planet_counter = 0;
    while(getline(fin, line)) {
        planetData data = parseLine(line);
        dataToObject(data, planets[planet_counter]);
        planet_counter += 1;
    }

    //setting colors for different planets (Maybe put in csv file?)
    planets[0].setColor(YELLOW);
    planets[1].setColor(LIGHTGRAY);
    planets[2].setColor(WHITE);
    planets[3].setColor(BLUE);
    planets[4].setColor(RED);
    planets[5].setColor(ORANGE);
    planets[6].setColor(GOLD);
    planets[7].setColor(BLUE);
    planets[8].setColor(BLUE);
    
    //game loop
    float dt;
    while (!WindowShouldClose()) {
        DrawFPS(10, 10);
        dt = GetFrameTime() * timeScale;
        for (int i=0; i < substeps; i++) {
            //Getting gravitational influence
            for (int j=0; j < OBJECTNUM; j++) {
                Vector2 acc = {0, 0};
                Vector2 pos = planets[j].getPos();
                Vector2 changeInPos = {0, 0};
                for (int k=0; k < OBJECTNUM; k++) {
                    if (j != k) {
                        //This first part is to find the angle from object to object
                        changeInPos = planets[k].getPos() - pos;
                        float angle = atan2(changeInPos.y, changeInPos.x); //angle in rads
                        //calculates distance with pythagorean theorem 
                        float distance = sqrt(pow(changeInPos.x, 2) + pow(changeInPos.y, 2));
                        if (distance * 2000 <= planets[j].getRad()) continue;
                        //calculates the gravitational force with newtons law  
                        float force = G * planets[j].getMass() * planets[k].getMass() / pow(distance * 1000, 2);

                        acc.x += cos(angle)*force/planets[j].getMass() / float(1000);
                        acc.y += sin(angle)*force/planets[j].getMass() / float(1000);
                    }
                }
                planets[j].setAcc(acc);
                planets[j].getNextPos(dt / substeps);
            }
        }

        //logic regarding panning
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            if (mousePos.x > 225 || mousePos.y < GetScreenHeight() - 50){
                dragging = true;
                lastMouse = GetMousePosition();
            }
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            dragging = false;
        }
        if (dragging) {
            Vector2 m = GetMousePosition();
            Vector2 mDelta = m - lastMouse;
            offset = Vector2Add(offset, mDelta);
            lastMouse = m;
        }

        //logic regarding zooming in / out
        float wheel = GetMouseWheelMove();
        zoom += wheel/3;

        //calculating the total energy of the system
        //creating a bar graph to show total energy of the system.
        double KE = 0;
        double PE = 0;
        double total = 0;
        for (int i=0; i < OBJECTNUM; i++) {
            planets[i].calculateKE();
            KE += planets[i].getKE();
            for (int j=i+1; j < OBJECTNUM; j++) {  
                    //move the calculatePe out of the objects class or calculate individual PE
                    planets[i].calculatePE(G, planets[j].getMass(), planets[j].getPos());
                    PE += planets[i].getPE();
            }
        }
        total = KE + PE;

        screenHeight = GetScreenHeight();
        screenWidth = GetScreenWidth();
        BeginDrawing();
        ClearBackground((Color) {0, 0, 0, 0});
        
        //adding in a slider bar to change time
        DrawText(
            TextFormat("Value: %.2f", slideValue),
            55,
            GetScreenHeight() - 75,
            14,
            WHITE
        );
        GuiSliderBar(
            (Rectangle){55, (float)screenHeight - 55, 200, 50},
            "day/s",
            "year/s",
            &slideValue,
            minValue,
            maxValue
        );

        //updating time scale based on slider bar
        selection = selectingTime(slideValue);
        timeScale = timeScaleOptions[selection];

        //drawing Energy display   
        //might want to add this to different file, 
        //takes up a lot of space and makes main messy
        //drawing line / text to seperate + and -
        DrawLine(screenWidth - 70, 10, screenWidth - 70, 100, GRAY);
        //Kinetic
        DrawText(
            TextFormat("Kenetic Energy: %.3e", KE),
            GetScreenWidth() - 350,
            20,
            18,
            WHITE   
        );
        float keBarWidth = 70 * KE/(KE + abs(PE));
        DrawRectangle(
            screenWidth - 70 - keBarWidth,
            19,
            keBarWidth,
            20,
            WHITE
        );
        //Potential
        DrawText(
            TextFormat("Potential Energy: %.3e", PE),
            screenWidth - 370,
            45,
            18,
            WHITE   
        );
        float peBarWidth = 70 * abs(PE)/abs(KE + abs(PE));
        DrawRectangle(
            screenWidth - 70,
            45,
            peBarWidth,
            20,
            WHITE
        );
        //total energy
        DrawText(
            TextFormat("Total Energy: %.3e", total),
            screenWidth - 350,
            70,
            18,
            WHITE   
        );
        float totalBarWidth = 70 * abs(total)/(KE + abs(PE)); 
        DrawRectangle(
            screenWidth - 70,
            70,
            totalBarWidth,
            20,
            WHITE
        );

        //drawing planets
        for (int i=0; i < OBJECTNUM; i++) {
            planets[i].draw(AU * KM_to_AU, GetScreenWidth()/2 + offset.x, GetScreenHeight()/2 + offset.y, planet_scale, zoom);
        }
        EndDrawing();
    }

    CloseWindow();

    return 0;
}