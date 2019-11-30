//*************************************************************************************

/*

Program rysujacy góry za pomoc¹ funkcji Weierstrassa.
Sterowanie:

+ / -		zwiekszenie/zmniejszenie ilosci punktow
* / /		zwiekszenie/zmniejszenie wspolczynnika a funkcji Weierstrassa
9 / 8		zwiekszenie/zmniejszenie wspolczynnika a funkcji Weierstrassa
] / [		zwiekszenie/zmniejszenie powiekszenia
X / x		obrót wzglêdem osi x
Y / y		obrót wzglêdem osi y
Z / z		obrót wzglêdem osi z
U / u		zwiekszenie/zmniejszenie wartosci offset_u
V / v		zwiekszenie/zmniejszenie wartosci offset_v
S / s		zwiêkszenie/zmniejszenie skalowania
R / r		regulacja czerwonej sk³adowej œwiat³a
G / g		regulacja zielonej sk³adowej œwiat³a
B / b		regulacja niebieskiej sk³adowej œwiat³a
  i			wyœwietlenie parametrów w konsoli
  m			w³¹czenie/wy³¹czenie trybu przemieszczania Ÿród³a œwiat³a
  1		    rysowanie modelu z punktów
  2			rysowanie modelu z wielok¹tów

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

int model = 1;  // 1- punkty, 2- siatka, 3 - wype³nione trójk¹ty
int N = 50;		// iloœæ rysowanych punktów
bool flag = true;
bool flag2 = true;
float ****colors;		//macierz zawieraj¹ca w kolory
float ***matrix;		//macierz zawieraj¹ca punkty



typedef float point3[3];	//model punktu u¿ywany przy rysowaniu osi
float a = 1;			//a i b, wspó³czynniki do równania Weierstrassa
float b = 1;
float zoom = 25;		//parametr przybli¿enia (alternatywne)
float offset_u = 0;		//przesuniêcie modelu po u
float offset_v = 0;		//przesuniêcie modelu po v
float scale = 1;		//parametr skalowania
float scalar = -0.0379907;			//poprawka do wektorów normalnych u¿ywana do poprawy cieniowania
bool movableLight = false;			//zmienna umo¿liwiaj¹ca w³¹czenie trybu przemieszczania œwiat³a


static GLint status = 0;       // stan klawiszy myszy
							   // 0 - nie naciœniêto ¿adnego klawisza
							   // 1 - naciœniêty zostaæ lewy klawisz


static GLfloat viewer[] = { 0.0, 0.0, 10.0 };		//po³o¿enie obserwatora
static GLfloat theta[] = { 0.0, 0.0, 15.0 };		//po³o¿enie obserwatora, u¿ywane przy edycji po³o¿enia
static GLfloat pix2angle;							//zmienne umo¿liwaj¹ce konwersje pixeli na k¹t
static GLfloat pix2angley;
GLfloat light_position[] = { 0.0, 0.0, 2.0, 1.0 };	//pozycja Ÿród³a œwiat³a
GLfloat RGB[] = { 1,0,0 };							//zmienne okreœlaj¹ce kolor œwiat³a




static int x_pos_old = 0;       // poprzednia pozycja kursora myszy

static int y_pos_old = 0;

static int delta_x = 0;        // ró¿nica pomiêdzy pozycj¹ bie¿¹c¹
									  // i poprzedni¹ kursora myszy
static int delta_y = 0;

static GLfloat R = 15;		


static GLfloat theta_x = 0.0;   // k¹t obrotu obiektu
static GLfloat theta_y = 0.0;
static GLfloat theta_z = 1.0;

GLfloat ***vectors;				//macierz wektorów normalnych






void showParameters()			//wyœwietla w konsoli parametry modelu
{
	using namespace std;
	system("cls");
	cout << "a = " << a << "b = " << b << "\nN = " << N << "\noffset_u = " << offset_u << "\noffset_v = " << offset_v << "\nscale = " << scale << endl;
	_getch();
	_getch();
}


GLfloat* avg_vector(GLfloat* v1, GLfloat* v2, GLfloat* v3, GLfloat* v4)	//oblicza œredni wektor
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

GLfloat* cross_product(int u, int v, int u_2, int v_2, int u_3, int v_3)//oblicza wektor prostopad³y do poligonu
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

float x(float u, float v)								//funkcja zwracaj¹ca argument x
{
	return Weierstrass(u + offset_u, a, 50)*Weierstrass(v + offset_v, b, 50);
}


void Vector(int u, int v, float* vec)			//pomocnicza funkcja rysuj¹ca linie w kierunku dowolnego wektora
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
		matrix[u] = new float*[N];				//obliczanie wsp. x,y,z dla punktów u,v
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


	if (model == 1)			//rysowanie modelu z punktów
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

	else if (model == 2)		//rysowanie modelu z poligonów
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

	for (int u = 0; u < N; u++)			//zwalnianie zasobów pamiêci
	{
		for (int v = 0; v < N; v++)
		{
			delete[] matrix[u][v];

		}
		delete[] matrix[u];
	}
	delete[] matrix;

}

void Axes(void)			//funkcja rysuj¹ca osie
{
	//glRotated(30.0, 1.0, 1.0, 1.0);
	point3  x_min = { -5.0, 0.0, 0.0 };
	point3  x_max = { 5.0, 0.0, 0.0 };
	// pocz¹tek i koniec obrazu osi x

	point3  y_min = { 0.0, -5.0, 0.0 };
	point3  y_max = { 0.0,  5.0, 0.0 };
	// pocz¹tek i koniec obrazu osi y

	point3  z_min = { 0.0, 0.0, -5.0 };
	point3  z_max = { 0.0, 0.0,  5.0 };
	//  pocz¹tek i koniec obrazu osi y

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

// Funkcja okreœlaj¹ca co ma byæ rysowane (zawsze wywo³ywana gdy trzeba
// przerysowaæ scenê)


void RenderScene(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Czyszczenie okna aktualnym kolorem czyszcz¹cym

	glLoadIdentity();
	// Czyszczenie macierzy bie¿¹cej

	gluLookAt(viewer[0], viewer[1], viewer[2], 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	// Zdefiniowanie po³o¿enia obserwatora
	
	// Narysowanie osi przy pomocy funkcji zdefiniowanej wy¿ej

	if (status == 1 && movableLight == false)                     // jeœli lewy klawisz myszy wciêniêty
	{
		theta_x += delta_x * pix2angle * 0.1;  // modyfikacja k¹ta obrotu o kat proporcjonalny
		theta_y += delta_y * pix2angley * 0.1;

	}
	else if (status == 1 && movableLight == true)
	{
		light_position[0] += delta_x * pix2angle * 0.1;
		light_position[1] += delta_y * pix2angley * 0.1;
		MyInit();
	}
		
		// do ró¿nicy po³o¿eñ kursora myszy

	if (status == 2)
	{
		
		zoom += delta_y*0.1;		//przybli¿anie
		if (zoom < 1) zoom = 1;
		N += (int)delta_y*0.3;		//zmiana ilosci punktów w zale¿noœci od przyblizenia
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

// Funkcja ustalaj¹ca stan renderowania



void MyInit(void)
{

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Kolor czyszc¹cy (wype³nienia okna) ustawiono na czarny

	//  Definicja materia³u z jakiego zrobiony jest czajnik
//  i definicja Ÿród³a œwiat³a


// Definicja materia³u z jakiego zrobiony jest czajnik

	GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
	// wspó³czynniki ka =[kar,kag,kab] dla œwiat³a otoczenia

	GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
	// wspó³czynniki kd =[kdr,kdg,kdb] œwiat³a rozproszonego

	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
	// wspó³czynniki ks =[ksr,ksg,ksb] dla œwiat³a odbitego               

	GLfloat mat_shininess = { 20.0 };
	// wspó³czynnik n opisuj¹cy po³ysk powierzchni


// Definicja Ÿród³a œwiat³a


	GLfloat light_ambient[] = { RGB[0],RGB[1], RGB[2], 1.0 };
	// sk³adowe intensywnoœci œwiecenia Ÿród³a œwiat³a otoczenia
	// Ia = [Iar,Iag,Iab]

	GLfloat light_diffuse[] = { RGB[0],RGB[1], RGB[2], 1.0 };
	// sk³adowe intensywnoœci œwiecenia Ÿród³a œwiat³a powoduj¹cego
	// odbicie dyfuzyjne Id = [Idr,Idg,Idb]

	GLfloat light_specular[] = { RGB[0],RGB[1], RGB[2], 1.0 };
	// sk³adowe intensywnoœci œwiecenia Ÿród³a œwiat³a powoduj¹cego
	// odbicie kierunkowe Is = [Isr,Isg,Isb]
	

	GLfloat att_constant = { 1.0 };
	// sk³adowa sta³a ds dla modelu zmian oœwietlenia w funkcji
	// odleg³oœci od Ÿród³a

	GLfloat att_linear = { 0.05 };
	// sk³adowa liniowa dl dla modelu zmian oœwietlenia w funkcji
	// odleg³oœci od Ÿród³a

	GLfloat att_quadratic = { 0.001 };
	// sk³adowa kwadratowa dq dla modelu zmian oœwietlenia w funkcji
	// odleg³oœci od Ÿród³a


// Ustawienie parametrów materia³u i Ÿród³a œwiat³a


// Ustawienie patrametrów materia³u


	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);


	// Ustawienie parametrów Ÿród³a

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, att_constant);
	glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, att_linear);
	glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, att_quadratic);



	// Ustawienie opcji systemu oœwietlania sceny

	glShadeModel(GL_SMOOTH); // w³aczenie ³agodnego cieniowania
	glEnable(GL_LIGHTING);   // w³aczenie systemu oœwietlenia sceny
	glEnable(GL_LIGHT0);     // w³¹czenie Ÿród³a o numerze 0
	glEnable(GL_DEPTH_TEST); // w³¹czenie mechanizmu z-bufora

	
	// Kolor czyszcz¹cy (wype³nienia okna) ustawiono na czarny


}

/*************************************************************************************/

// Funkcja ma za zadanie utrzymanie sta³ych proporcji rysowanych
// w przypadku zmiany rozmiarów okna.
// Parametry vertical i horizontal (wysokoœæ i szerokoœæ okna) s¹
// przekazywane do funkcji za ka¿dym razem gdy zmieni siê rozmiar okna.



void ChangeSize(GLsizei horizontal, GLsizei vertical)
{

	pix2angle = 360.0 / (float)horizontal;  // przeliczenie pikseli na stopnie
	pix2angley = 360.0 / (float)vertical;
	glMatrixMode(GL_PROJECTION);
	// Prze³¹czenie macierzy bie¿¹cej na macierz projekcji

	glLoadIdentity();
	// Czyszcznie macierzy bie¿¹cej

	gluPerspective(70, 1.0, 1, 70.0);
	// Ustawienie parametrów dla rzutu perspektywicznego


	if (horizontal <= vertical)
		glViewport(0, (vertical - horizontal) / 2, horizontal, horizontal);

	else
		glViewport((horizontal - vertical) / 2, 0, vertical, vertical);
	// Ustawienie wielkoœci okna okna widoku (viewport) w zale¿noœci
	// relacji pomiêdzy wysokoœci¹ i szerokoœci¹ okna

	glMatrixMode(GL_MODELVIEW);
	// Prze³¹czenie macierzy bie¿¹cej na macierz widoku modelu  

	glLoadIdentity();
	// Czyszczenie macierzy bie¿¹cej

}

/*************************************************************************************/

// G³ówny punkt wejœcia programu. Program dzia³a w trybie konsoli



void Mouse(int btn, int state, int x, int y)
{


	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		x_pos_old = x;        // przypisanie aktualnie odczytanej pozycji kursora
		y_pos_old = y;                 // jako pozycji poprzedniej
		status = 1;          // wciêniêty zosta³ lewy klawisz myszy
	}
	else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		y_pos_old = y;
		status = 2;
	}
	else

		status = 0;          // nie zosta³ wciêniêty ¿aden klawisz
}

/*************************************************************************************/
// Funkcja "monitoruje" po³o¿enie kursora myszy i ustawia wartoœci odpowiednich
// zmiennych globalnych

void Motion(GLsizei x, GLsizei y)
{

	delta_x = x - x_pos_old;     // obliczenie ró¿nicy po³o¿enia kursora myszy
	delta_y = y - y_pos_old;

	x_pos_old = x;            // podstawienie bie¿¹cego po³o¿enia jako poprzednie
	y_pos_old = y;
	glutPostRedisplay();     // przerysowanie obrazu sceny
}

void keys(unsigned char key, int x, int y)		//funkcja obs³uguj¹ca klawiaturê
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

	glutCreateWindow("Góry");
	//glutIdleFunc(moveLight);
	glutKeyboardFunc(keys);
	glutDisplayFunc(RenderScene);
	// Okreœlenie, ¿e funkcja RenderScene bêdzie funkcj¹ zwrotn¹
	// (callback function).  Bedzie ona wywo³ywana za ka¿dym razem
	// gdy zajdzie potrzba przeryswania okna

	glutReshapeFunc(ChangeSize);
	// Dla aktualnego okna ustala funkcjê zwrotn¹ odpowiedzialn¹
	// zazmiany rozmiaru okna      
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
	MyInit();
	// Funkcja MyInit() (zdefiniowana powy¿ej) wykonuje wszelkie
	// inicjalizacje konieczne  przed przyst¹pieniem do renderowania

	glEnable(GL_DEPTH_TEST);
	// W³¹czenie mechanizmu usuwania powierzchni niewidocznych

	glutMainLoop();
	// Funkcja uruchamia szkielet biblioteki GLUT
	

}
