#pragma once

void change_color_menu();

void menu();

GLfloat * avg_vector(GLfloat * v1, GLfloat * v2, GLfloat * v3, GLfloat * v4);

GLfloat * cross_product(int u, int v, int u_2, int v_2, int u_3, int v_3);

float Weierstrass(float x, float a, int iterations);

float x(float u, float v);

void Vector(int u, int v, float * vec);

void Model();

void Axes(void);

void MyInit(void);
