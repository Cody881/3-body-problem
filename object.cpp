#include <iostream>
#include <cmath>
#include "raylib.h"
#include "raymath.h"

using namespace std;

class Object
{
    private:
        Vector2 pos;
        Vector2 vel;
        Vector2 acc;
        double rad;
        double mass;
        string name;
        Color color;
        double KE;
        double PE;
        double totalEnergy;

    public:
        Object(Vector2 pos_val = { 0, 0}, Vector2 vel_val = { 0, 0}, 
               Vector2 acc_val = { 100, 100}, int rad_val = 30, int mass = 1000 ) {
            pos = pos_val; 
            vel = vel_val;
            acc = acc_val;
            rad = rad_val;
        }

        Vector2 getNextPos(float timePassed) {
            vel.x += acc.x * timePassed;
            vel.y += acc.y * timePassed;
            pos.x += vel.x * timePassed;
            pos.y += vel.y * timePassed;

            return pos;
        }

        void draw(double KMToPixels, float cx, float cy, int planet_scale, float zoom) {
            float draw_X = float(cx + pos.x * KMToPixels * zoom);
            float draw_Y = float(cy + pos.y * KMToPixels * zoom);
            if (name == "Sun")
                planet_scale = 50;
            float draw_rad = min(max((double)3, rad*KMToPixels*planet_scale), (double)40);
            DrawCircle(draw_X, draw_Y, draw_rad*zoom, color);
        }
        
        void calculateEnergy () {
            //magnitude of velocity in KM/s, converted to meters
            double magV = sqrt(pow(vel.y, 2) + pow(vel.x, 2))*1000;
            KE = .5*mass*pow(magV, 2);
            PE = ;
            totalEnergy = ;
        }

        //Adding setter and getter functions
        void setRad(double val) { rad = val; }
        double getRad() { return rad; }

        void setPos(Vector2 val) { pos = val; }
        Vector2 getPos() { return pos; }

        void setVel(Vector2 val) { vel = val; }
        Vector2 getVel() { return vel; }

        void setAcc(Vector2 val) { acc = val; }
        Vector2 getAcc() { return acc; }

        void setMass(double val) { mass = val; }
        double getMass() { return mass; }
        
        void setName(string val) { name = val; }
        string getName() { return name; }

        void setColor(Color val) { color = val; }
        Color getColor() { return color; }
};
