//*************************************************************************************

/*

Program rysujacy g�ry za pomoc� funkcji Weierstrassa.
Sterowanie:

+ / -		zwiekszenie/zmniejszenie ilosci punktow
* / /		zwiekszenie/zmniejszenie wspolczynnika a funkcji Weierstrassa
9 / 8		zwiekszenie/zmniejszenie wspolczynnika a funkcji Weierstrassa
] / [		zwiekszenie/zmniejszenie powiekszenia
X / x		obr�t wzgl�dem osi x
Y / y		obr�t wzgl�dem osi y
Z / z		obr�t wzgl�dem osi z
U / u		zwiekszenie/zmniejszenie wartosci offset_u
V / v		zwiekszenie/zmniejszenie wartosci offset_v
S / s		zwi�kszenie/zmniejszenie skalowania
R / r		regulacja czerwonej sk�adowej �wiat�a
G / g		regulacja zielonej sk�adowej �wiat�a
B / b		regulacja niebieskiej sk�adowej �wiat�a
  i			wy�wietlenie parametr�w w konsoli
  m			w��czenie/wy��czenie trybu przemieszczania �r�d�a �wiat�a
  1		    rysowanie modelu z punkt�w
  2			rysowanie modelu z wielok�t�w

*/

//*************************************************************************************/


#define M_PI 3.14159265358979323846
#include <windows.h>
#include <gl/gl.h>
#include <glut.h>
#include <math.h>
#include <cmath>
#include <iostream>
#include <thread>
#include <conio.h>
#include "Source.h"

int model = 1;  // 1- punkty, 2- siatka, 3 - wype�nione tr�jk�ty
int N = 50;		// ilo�� rysowanych punkt�w
bool flag = true;
bool flag2 = true;
float ****colors;		//macierz zawieraj�ca w kolory
float ***matrix;		//macierz zawieraj�ca punkty



typedef float point3[3];	//model punktu u�ywany przy rysowaniu osi
float a = 1;			//a i b, wsp�czynniki do r�wnania Weierstrassa
float b = 1;
float zoom = 25;		//parametr przybli�enia (alternatywne)
float offset_u = 0;		//przesuni�cie modelu po u
float offset_v = 0;		//przesuni�cie modelu po v
float scale = 1;		//parametr skalowania
float scalar = -0.0379907;			//poprawka do wektor�w normalnych u�ywana do poprawy cieniowania
bool movableLight = false;			//zmienna umo�liwiaj�ca w��czenie trybu przemieszczania �wiat�a


static GLint status = 0;       // stan klawiszy myszy
							   // 0 - nie naci�ni�to �adnego klawisza
							   // 1 - naci�ni�ty zosta� lewy klawisz


static GLfloat viewer[] = { 0.0, 0.0, 10.0 };		//po�o�enie obserwatora
static GLfloat theta[] = { 0.0, 0.0, 15.0 };		//po�o�enie obserwatora, u�ywane przy edycji po�o�enia
static GLfloat pix2angle;							//zmienne umo�liwaj�ce konwersje pixeli na k�t
static GLfloat pix2angley;
GLfloat light_position[] = { 0.0, 0.0, 2.0, 1.0 };	//pozycja �r�d�a �wiat�a
GLfloat RGB[] = { 1,0,0 };							//zmienne okre�laj�ce kolor �wiat�a




static int x_pos_old = 0;       // poprzednia pozycja kursora myszy

static int y_pos_old = 0;

static int delta_x = 0;        // r�nica pomi�dzy pozycj� bie��c�
									  // i poprzedni� kursora myszy
static int delta_y = 0;

static GLfloat R = 15;		


static GLfloat theta_x = 0.0;   // k�t obrotu obiektu
static GLfloat theta_y = 0.0;
static GLfloat theta_z = 1.0;

GLfloat ***vectors;				//macierz wektor�w normalnych






void showParameters()			//wy�wietla w konsoli parametry modelu
{
	using namespace std;
	system("cls");
	cout << "a = " << a << "b = " << b << "\nN = " << N << "\noffset_u = " << offset_u << "\noffset_v = " << offset_v << "\nscale = " << scale << endl;
	_getch();
	_getch();
}


GLfloat* avg_vector(GLfloat* v1, GLfloat* v2, GLfloat* v3, GLfloat* v4)	//oblicza �redni wektor
{
	GLfloat *new_vec = new GLfloat[3];
	new_vec[0] = (v1[0] + v2[0] + v3[0] + v4[0]) / 4;
	new_vec[1] = (v1[1] + v2[1] + v3[1] + v4[1]) / 4;
	new_vec[2] = (v1[2] + v2[2] + v3[2] + v4[2]) / 4;
	float length = sqrt(pow(new_vec[0], 2) + pow(new_vec[1], 2) + pow(new_vec[2], 2));
	new_vec[0] /= length * scalar;
	new_vec[1] /= length * scalar;
	new_vec[2] /= length * scalar;
	return new_vec;

}

GLfloat* cross_product(int u, int v, int u_2, int v_2, int u_3, int v_3)//oblicza wektor prostopad�y do poligonu
{
	float AB[3];
	float AC[3];
	
	AB[0] = (matrix[(u_2%N)][v_2%N][0] - matrix[u%N][v%N][0]);
	AB[1] = matrix[u_2%N][v_2%N][1] - matrix[u%N][v%N][1];
	AB[2] = matrix[(u_2)%N][v_2%N][2] - matrix[u%N][v%N][2];

	AC[0] = matrix[u_3%N][v_3%N][0] - matrix[u%N][v%N][0];
	AC[1] = matrix[u_3%N][v_3%N][1] - matrix[u%N][v%N][1];
	AC[2] = matrix[(u_3)%N][v_3%N][2] - matrix[u%N][v%N][2];
	
	GLfloat ABxAC[] = { AB[1] * AC[2] - AB[2] * AC[1], AB[2] * AC[0] - AB[0] * AC[2], AB[0] * AC[1] - AB[1] * AC[0] };
	float length = sqrt(pow(ABxAC[0],2) + pow(ABxAC[1],2) + pow(ABxAC[2],2));
	GLfloat ABxAC2[] = { ABxAC[0] / length, ABxAC[1] / length, ABxAC[2] / length };
	return ABxAC2;
 }




float Weierstrass(float x, float a, int iterations)		//funkcja Weierstrassa
{
	float sum = 0;
	for (int k = 1; k < iterations; k++)
	{
		sum += float((sin(M_PI*pow(k, a)*x)) / (M_PI*pow(k, a)));
	}
	return sum;
}

float x(float u, float v)								//funkcja zwracaj�ca argument x
{
	return Weierstrass(u + offset_u, a, 50)*Weierstrass(v + offset_v, b, 50);
}


void Vector(int u, int v, float* vec)			//pomocnicza funkcja rysuj�ca linie w kierunku dowolnego wektora
{												//(nieuzywana)
	for (int i = 0; i < 100; i++)
	{
		glBegin(GL_LINES);
		glColor3f(1, 0, 0);
		glVertex3f(matrix[u][v][0], matrix[u][v][1], matrix[u][v][2]);
		glVertex3f(matrix[u][v][0] + vec[0] * 1/i, matrix[u][v][1] + vec[1] * 1/i, matrix[u][v][2] + vec[2] * 1/i);
		glEnd();
		glFlush();
	}
	
}


void Model()											//funkcja rysujaca model
{
	matrix = new float**[N];

	for (int u = 0; u < N; u++)
	{
		matrix[u] = new float*[N];				//obliczanie wsp. x,y,z dla punkt�w u,v
		for (int v = 0; v < N; v++)
		{
			float *p = new float[4];
			matrix[u][v] = p;
			float nu = (float)(u) / N;
			float nv = (float)(v) / N;


		
			p[0] = x(nu, nv)*scale;
			p[1] = nv - 0.3;
			p[2] = nu - 0.5;

			
		}
	}


	if (model == 1)			//rysowanie modelu z punkt�w
	{

		for (int u = 0; u < N; u++)
		{
			for (int v = 0; v < N; v++)
			{
				glBegin(GL_POINTS);

				GLfloat *avg = avg_vector(cross_product(u, v, u + 1, v, u, v + 1),		//obliczanie wektora normalnego
					cross_product(u, v, u, v + 1, abs(u - 1), v),
					cross_product(u, v, abs(u - 1), v, u, abs(v - 1)),
					cross_product(u, v, u, abs(v - 1), u + 1, v));

	

				glNormal3fv(avg);


				glColor3f(1, 1, 1);
				glVertex3f(matrix[u][v][0], matrix[u][v][1], matrix[u][v][2]);
				glEnd();

			}
		}

		
	}

	else if (model == 2)		//rysowanie modelu z poligon�w
	{
		for (int u = 0; u < N-1; u++)
		{
			float nu = (float)(u) / N;
			for (int v = 0; v < N-1; v++)
			{
				float nv = (float)(v) / N;
				glBegin(GL_POLYGON);
				
				GLfloat *avg = avg_vector(cross_product(u, v, u + 1, v, u, v + 1),
					cross_product(u, v, u, v + 1, abs(u - 1), v),
					cross_product(u, v, abs(u - 1), v, u, abs(v - 1)),
					cross_product(u, v, u, abs(v - 1), u + 1, v));

				

				glNormal3fv(avg);

									   
				glVertex3f(matrix[u%N][v%N][0], matrix[u%N][v%N][1], matrix[u%N][v%N][2]);

				glVertex3f(matrix[(u + 1) % N][v%N][0], matrix[(u + 1) % N][v%N][1], matrix[(u + 1) % N][v%N][2]);

				glVertex3f(matrix[(u + 1) % N][(v + 1) % N][0], matrix[(u + 1) % N][(v + 1) % N][1], matrix[(u + 1) % N][(v + 1) % N][2]);

				glVertex3f(matrix[(u) % N][(v + 1) % N][0], matrix[(u) % N][(v + 1) % N][1], matrix[(u) % N][(v + 1) % N][2]);
				glEnd();
				glFlush();
			}
		}
		
	}

	for (int u = 0; u < N; u++)			//zwalnianie zasob�w pami�ci
	{
		for (int v = 0; v < N; v++)
		{
			delete[] matrix[u][v];

		}
		delete[] matrix[u];
	}
	delete[] matrix;

}

void Axes(void)			//funkcja rysuj�ca osie
{
	//glRotated(30.0, 1.0, 1.0, 1.0);
	point3  x_min = { -5.0, 0.0, 0.0 };
	point3  x_max = { 5.0, 0.0, 0.0 };
	// pocz�tek i koniec obrazu osi x

	point3  y_min = { 0.0, -5.0, 0.0 };
	point3  y_max = { 0.0,  5.0, 0.0 };
	// pocz�tek i koniec obrazu osi y

	point3  z_min = { 0.0, 0.0, -5.0 };
	point3  z_max = { 0.0, 0.0,  5.0 };
	//  pocz�tek i koniec obrazu osi y

	glColor3f(1.0f, 0.0f, 0.0f);  // kolor rysowania osi - czerwony
	glBegin(GL_LINES); // rysowanie osi x

	glVertex3fv(x_min);
	glVertex3fv(x_max);

	glEnd();

	glColor3f(0.0f, 1.0f, 0.0f);  // kolor rysowania - zielony
	glBegin(GL_LINES);  // rysowanie osi y

	glVertex3fv(y_min);
	glVertex3fv(y_max);

	glEnd();

	glColor3f(0.0f, 0.0f, 1.0f);  // kolor rysowania - niebieski
	glBegin(GL_LINES); // rysowanie osi z

	glVertex3fv(z_min);
	glVertex3fv(z_max);

	glEnd();
}

/*************************************************************************************/

// Funkcja okre�laj�ca co ma by� rysowane (zawsze wywo�ywana gdy trzeba
// przerysowa� scen�)


void RenderScene(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Czyszczenie okna aktualnym kolorem czyszcz�cym

	glLoadIdentity();
	// Czyszczenie macierzy bie��cej

	gluLookAt(viewer[0], viewer[1], viewer[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	// Zdefiniowanie po�o�enia obserwatora
	
	// Narysowanie osi przy pomocy funkcji zdefiniowanej wy�ej

	if (status == 1 && movableLight == false)                     // je�li lewy klawisz myszy wci�ni�ty
	{
		theta_x += delta_x * pix2angle * 0.1;  // modyfikacja k�ta obrotu o kat proporcjonalny
		theta_y += delta_y * pix2angley * 0.1;

	}
	else if (status == 1 && movableLight == true)
	{
		light_position[0] += delta_x * pix2angle * 0.1;
		light_position[1] += delta_y * pix2angley * 0.1;
		MyInit();
	}
		
		// do r�nicy po�o�e� kursora myszy

	if (status == 2)
	{
		
		zoom += delta_y*0.1;		//przybli�anie
		if (zoom < 1) zoom = 1;
		N += (int)delta_y*0.3;		//zmiana ilosci punkt�w w zale�no�ci od przyblizenia
		if (N < 10) N = 10;
	}

	
	theta[0] = R * cos(theta_x) * cos(theta_y);
	theta[1] = R * sin(theta_y);
	theta[2] = R * sin(theta_x) * cos(theta_y);
	gluLookAt(theta[0], theta[1], theta[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glScaled(zoom, zoom, zoom);		//operacja przyblizania obrazu 
	

	Model();
	Axes();

	

	glutSwapBuffers();
	//

}

/*************************************************************************************/

// Funkcja ustalaj�ca stan renderowania



void MyInit(void)
{

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Kolor czyszc�cy (wype�nienia okna) ustawiono na czarny

	//  Definicja materia�u z jakiego zrobiony jest czajnik
//  i definicja �r�d�a �wiat�a


// Definicja materia�u z jakiego zrobiony jest czajnik

	GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
	// wsp�czynniki ka =[kar,kag,kab] dla �wiat�a otoczenia

	GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	// wsp�czynniki kd =[kdr,kdg,kdb] �wiat�a rozproszonego

	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	// wsp�czynniki ks =[ksr,ksg,ksb] dla �wiat�a odbitego               

	GLfloat mat_shininess = { 20.0 };
	// wsp�czynnik n opisuj�cy po�ysk powierzchni


// Definicja �r�d�a �wiat�a


	GLfloat light_ambient[] = { RGB[0],RGB[1], RGB[2], 1.0 };
	// sk�adowe intensywno�ci �wiecenia �r�d�a �wiat�a otoczenia
	// Ia = [Iar,Iag,Iab]

	GLfloat light_diffuse[] = { RGB[0],RGB[1], RGB[2], 1.0 };
	// sk�adowe intensywno�ci �wiecenia �r�d�a �wiat�a powoduj�cego
	// odbicie dyfuzyjne Id = [Idr,Idg,Idb]

	GLfloat light_specular[] = { RGB[0],RGB[1], RGB[2], 1.0 };
	// sk�adowe intensywno�ci �wiecenia �r�d�a �wiat�a powoduj�cego
	// odbicie kierunkowe Is = [Isr,Isg,Isb]
	

	GLfloat att_constant = { 1.0 };
	// sk�adowa sta�a ds dla modelu zmian o�wietlenia w funkcji
	// odleg�o�ci od �r�d�a

	GLfloat att_linear = { 0.05 };
	// sk�adowa liniowa dl dla modelu zmian o�wietlenia w funkcji
	// odleg�o�ci od �r�d�a

	GLfloat att_quadratic = { 0.001 };
	// sk�adowa kwadratowa dq dla modelu zmian o�wietlenia w funkcji
	// odleg�o�ci od �r�d�a


// Ustawienie parametr�w materia�u i �r�d�a �wiat�a


// Ustawienie patrametr�w materia�u


	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);


	// Ustawienie parametr�w �r�d�a

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, att_constant);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, att_linear);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, att_quadratic);



	// Ustawienie opcji systemu o�wietlania sceny

	glShadeModel(GL_SMOOTH); // w�aczenie �agodnego cieniowania
	glEnable(GL_LIGHTING);   // w�aczenie systemu o�wietlenia sceny
	glEnable(GL_LIGHT0);     // w��czenie �r�d�a o numerze 0
	glEnable(GL_DEPTH_TEST); // w��czenie mechanizmu z-bufora

	
	// Kolor czyszcz�cy (wype�nienia okna) ustawiono na czarny


}

/*************************************************************************************/

// Funkcja ma za zadanie utrzymanie sta�ych proporcji rysowanych
// w przypadku zmiany rozmiar�w okna.
// Parametry vertical i horizontal (wysoko�� i szeroko�� okna) s�
// przekazywane do funkcji za ka�dym razem gdy zmieni si� rozmiar okna.



void ChangeSize(GLsizei horizontal, GLsizei vertical)
{

	pix2angle = 360.0 / (float)horizontal;  // przeliczenie pikseli na stopnie
	pix2angley = 360.0 / (float)vertical;
	glMatrixMode(GL_PROJECTION);
	// Prze��czenie macierzy bie��cej na macierz projekcji

	glLoadIdentity();
	// Czyszcznie macierzy bie��cej

	gluPerspective(70, 1.0, 1, 70.0);
	// Ustawienie parametr�w dla rzutu perspektywicznego


	if (horizontal <= vertical)
		glViewport(0, (vertical - horizontal) / 2, horizontal, horizontal);

	else
		glViewport((horizontal - vertical) / 2, 0, vertical, vertical);
	// Ustawienie wielko�ci okna okna widoku (viewport) w zale�no�ci
	// relacji pomi�dzy wysoko�ci� i szeroko�ci� okna

	glMatrixMode(GL_MODELVIEW);
	// Prze��czenie macierzy bie��cej na macierz widoku modelu  

	glLoadIdentity();
	// Czyszczenie macierzy bie��cej

}

/*************************************************************************************/

// G��wny punkt wej�cia programu. Program dzia�a w trybie konsoli



void Mouse(int btn, int state, int x, int y)
{


	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		x_pos_old = x;        // przypisanie aktualnie odczytanej pozycji kursora
		y_pos_old = y;                 // jako pozycji poprzedniej
		status = 1;          // wci�ni�ty zosta� lewy klawisz myszy
	}
	else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		y_pos_old = y;
		status = 2;
	}
	else

		status = 0;          // nie zosta� wci�ni�ty �aden klawisz
}

/*************************************************************************************/
// Funkcja "monitoruje" po�o�enie kursora myszy i ustawia warto�ci odpowiednich
// zmiennych globalnych

void Motion(GLsizei x, GLsizei y)
{

	delta_x = x - x_pos_old;     // obliczenie r�nicy po�o�enia kursora myszy
	delta_y = y - y_pos_old;

	x_pos_old = x;            // podstawienie bie��cego po�o�enia jako poprzednie
	y_pos_old = y;
	glutPostRedisplay();     // przerysowanie obrazu sceny
}

void keys(unsigned char key, int x, int y)		//funkcja obs�uguj�ca klawiatur�
{

if (key == '+') N++;
if (key == '-') N--;
if (N < 0) N = 0;

if (key == '/') a -= 0.01;
if (key == '*') a += 0.01;
if (a < 1) a = 1;

if (key == '8') b-=0.01;
if (key == '9') b+=0.01;
if (b < 1) b = 1;

if (key == ']') zoom++;
if (key == '[') zoom--;
if (zoom < 1) zoom = 1;
if (key == 'A') scalar += 0.001;
if (key == 'a') scalar -= 0.001;

if (key == 'x')
{
		theta[0] -= 5;
		if (theta[0] > 360.0) theta[0] -= 360.0;
}
if (key == 'X')
{
		theta[0] += 5;
		if (theta[0] > 360.0) theta[0] -= 360.0;
}
if (key == 'y')
{
		theta[1] -= 5;
		if (theta[1] > 360.0) theta[1] -= 360.0;
}
if (key == 'Y')
{
		theta[1] += 5;
		if (theta[1] > 360.0) theta[1] -= 360.0;
}
if (key == 'z')
{
		theta[2] -= 5;
		if (theta[2] > 360.0) theta[2] -= 360.0;
}
if (key == 'Z')
{
		theta[2] += 5;
		if (theta[2] > 360.0) theta[2] -= 360.0;
}
if (key == 'R')
{
	RGB[0] += 0.1;
	if (RGB[0] > 1) RGB[0] = 0;
	MyInit();
}
if (key == 'r')
{
	RGB[0] -= 0.1;
	if (RGB[0] < 0) RGB[0] = 1;
	MyInit();
}
if (key == 'G')
{
	RGB[1] += 0.1;
	if (RGB[1] > 1) RGB[1] = 0;
	MyInit();
}
if (key == 'g')
{
	RGB[1] -= 0.1;
	if (RGB[1] < 0) RGB[1] = 1;
	MyInit();
}
if (key == 'B')
{
	RGB[2] += 0.1;
	if (RGB[2] > 1) RGB[2] = 0;
	MyInit();
}
if (key == 'b')
{
	RGB[2] -= 0.1;
	if (RGB[2] < 0) RGB[2] = 1;
	MyInit();
}

if (key == 'U') offset_u += 0.01;
if (key == 'u') offset_u -= 0.01;
if (key == 'V') offset_v += 0.01;
if (key == 'v') offset_v -= 0.01;
if (key == 'S') scale += 0.1;
if (key == 's') scale -= 0.1;
if (scale < 1) scale = 1;
if (key == '1') model = 1;
if (key == '2') model = 2;
if (key == 'm')
{
	if (movableLight) movableLight = false;
	else movableLight = true;
}
if (key == 'i') showParameters();

RenderScene();
}


void main(void)
{
	//glutIdleFunc(spinEgg);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(300, 300);
	//glutIdleFunc(menu);

	glutCreateWindow("G�ry");
	//glutIdleFunc(moveLight);
	glutKeyboardFunc(keys);
	glutDisplayFunc(RenderScene);
	// Okre�lenie, �e funkcja RenderScene b�dzie funkcj� zwrotn�
	// (callback function).  Bedzie ona wywo�ywana za ka�dym razem
	// gdy zajdzie potrzba przeryswania okna

	glutReshapeFunc(ChangeSize);
	// Dla aktualnego okna ustala funkcj� zwrotn� odpowiedzialn�
	// zazmiany rozmiaru okna      
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	MyInit();
	// Funkcja MyInit() (zdefiniowana powy�ej) wykonuje wszelkie
	// inicjalizacje konieczne  przed przyst�pieniem do renderowania

	glEnable(GL_DEPTH_TEST);
	// W��czenie mechanizmu usuwania powierzchni niewidocznych

	glutMainLoop();
	// Funkcja uruchamia szkielet biblioteki GLUT
	

}
