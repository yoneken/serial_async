#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <iostream>
#include <fstream>

#include <GLUT/glut.h>
#include <stdlib.h>

//#define FILE_SAVE

using namespace boost::asio;
using namespace boost::posix_time;

const char *PORT = "/dev/tty.usbserial";
io_service io;
serial_port port( io, PORT );
boost::array<char, 64> rbuf;
int fp = 0;
char fbuf[128] = {0};
std::ofstream ofs;
boost::mutex mtx;

#define KEY_ESCAPE 27
#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480
int window;
void **font=GLUT_BITMAP_HELVETICA_10;		// For printing bitmap fonts
char s[30];									// Tmp variable for storing the display strings

int val[640];
int val_pointer = 0;
volatile unsigned long sample_num = 0;
int param_divx = 100;
int param_divy = 100;
int state_num = 0;
int result;
double amp = 1000.0;

void read_callback(const boost::system::error_code& e, std::size_t size)
{
	boost::mutex::scoped_lock lk(mtx);

	for(unsigned int i=0;i<size;i++){
		unsigned char c = rbuf.at(i);
/*		fbuf[fp++] = c;
		if(c == '\n'){
			fbuf[fp] = 0x0;
			printf("%s", fbuf);
			fp = 0;
		}
*/
//		printf("%d: %x \r\n", state_num, c);
		switch(state_num){
		  case 0:
		  case 1:
			if(c == 0xaa) state_num++;
			else state_num = 0;
			break;
		  case 2:
			if(c == 0x02) state_num++;
			else state_num = 0;
			break;
		  case 3:
			result = c<<8;
			state_num++;
			break;
		  case 4:
			result |= c;
			state_num++;
			break;
		  default:
			state_num = 0;
		}
		
		if(state_num == 5){
			//printf("%d\r\n", result);
			val[val_pointer++] = result;
			if(val_pointer == 640) val_pointer = 0;
			sample_num++;
#ifdef FILE_SAVE
			ptime now = second_clock::local_time();
			ofs << to_iso_string(now) << "," << result << std::endl;
#endif /* FILE_SAVE */
		}
	}

	port.async_read_some( buffer(rbuf), boost::bind(&read_callback, _1, _2 ));
}

void write_callback(const boost::system::error_code& e, std::size_t size )
{
	std::cout << "write :" << size << "byte[s]" << std::endl;
}

/*
 * @brief A general OpenGL initialization function.  Sets all of the initial parameters.
 */
void InitGL(int _width, int _height)
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, _width, _height);
	glShadeModel(GL_SMOOTH);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, _width, 0.0, _height);
	glMatrixMode(GL_MODELVIEW);
	glShadeModel(GL_FLAT);
}

/*
 * @brief Any User DeInitialization Goes Here
 */
void Deinitialize (void)
{
}

/*
 * @brief Function for displaying bitmap fonts on the screen
 */
void glPrint(float x, float y, void *font,char *string) {

	char *c;
	glRasterPos2f(x, y);
	for (c=string; *c != '\0'; c++) {
		glutBitmapCharacter(font, *c);
	}
}

/*
 * @brief The function called when our window is resized (which shouldn't happen, because we're fullscreen)
 */
void ReSizeGLScene(int _width, int _height)
{
	if (_height==0)
		_height=1;

	glViewport(0, 0, _width, _height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, _width, 0.0, _height);
	glMatrixMode(GL_MODELVIEW);
}

/*
 * @brief The main drawing function.
 */
void DrawGLScene()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisableClientState(GL_COLOR_ARRAY);
	
	// Draw Axis
	glColor3ub(100, 100, 100);	// Draw In Gray
	// X-axis
	glBegin(GL_LINES);
		glVertex2i(  0, 20);
		glVertex2i(DEFAULT_WIDTH-1, 20);
	glEnd();
	// Y-axis
	glBegin(GL_LINES);
		glVertex2i( 20,  0);
		glVertex2i( 20,DEFAULT_HEIGHT-1);
	glEnd();
	// X divider
	int dx0 = ((double)(param_divx - (sample_num % param_divx)))/((double)param_divx/100);
	int dx1 = sample_num / param_divx + 1;
	for(int i=0;i<6;i++){
		char divider[8];
		sprintf(divider, "%d", (dx1+i)*param_divx);
		glPrint( 20+dx0-8+100*i, 5, (void *)font, divider);
		glBegin(GL_LINE_STRIP);
			glVertex2i( 20+dx0+100*i, 20);
			glVertex2i( 20+dx0+100*i, DEFAULT_HEIGHT-1);
		glEnd();
	}
	// Y divider
	for(int i=0;i<4;i++){
		char divider[8];
		sprintf(divider, "%d", param_divy*(i+1));
		glPrint( 2, 20-5+100*(i+1), (void *)font, divider);
		glBegin(GL_LINE_STRIP);
			glVertex2i( 20, 20+100*(i+1));
			glVertex2i(DEFAULT_WIDTH-1, 20+100*(i+1));
		glEnd();
	}
	
	// Draw values
	glColor3ub(255, 255, 0);	// Draw In Yellow
	int p = val_pointer;
	glBegin(GL_LINES);
	for(int i=0;i<(int)(DEFAULT_WIDTH*((double)param_divx/100.0))-1;i++){
		int p2;
		if(p == DEFAULT_WIDTH) p = 0;
		if(p != DEFAULT_WIDTH-1) p2 = p+1;
		else p2 = 0;
		glVertex2i( 20+(int)(i/((double)param_divx/100.0)), (int)(val[p]/((double)param_divy/100.0))+20);
		glVertex2i( 20+(int)((i+1)/((double)param_divx/100.0)), (int)(val[p2]/((double)param_divy/100.0))+20);
		p++;
	}
	glEnd();
	
	glFlush ();
	glutSwapBuffers();
}

/*
 * @brief The function called whenever a normal key is pressed.
 */
void NormalKeyPressed(unsigned char keys, int x, int y)
{
	switch(keys){
	  case KEY_ESCAPE:
		exit(0);
		break;
	  case 'z':
		param_divy /= 2;
		if(param_divy < 2) param_divy = 2;
		break;
	  case 'Z':
		param_divy *= 2;
		break;
	  case 't':
		param_divx /= 2;
		if(param_divx < 2) param_divx = 2;
		break;
	  case 'T':
		param_divx *= 2;
		break;
	}
}

/*
 * @brief The function called whenever a special key is pressed.
 */
void SpecialKeyPressed(int key, int x, int y)
{
	switch (key) {
	}
}

int main(int argc, char *argv[])
{
	if(argc==2){
		double d = atof(argv[1]);
		if(d!=0) amp = d;
		else amp = 1000.0;
	}
#ifdef FILE_SAVE
	ptime now = second_clock::local_time();
	std::string logname = to_iso_string(now) + std::string(".csv");
	ofs.open(logname.c_str());
#endif /* FILE_SAVE */
	port.set_option(serial_port_base::baud_rate(57600));
	port.set_option(serial_port_base::character_size(8));
	port.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));
	port.set_option(serial_port_base::parity(serial_port_base::parity::none));
	port.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));

	boost::thread thr_io(boost::bind(&io_service::run, &io));
	port.async_read_some( buffer(rbuf), boost::bind(&read_callback, _1, _2 ));

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA);
	glutInitWindowSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
	glutInitWindowPosition(0, 0);
	window = glutCreateWindow("Value window");
	glutDisplayFunc(&DrawGLScene);
	glutIdleFunc(&DrawGLScene);
	glutReshapeFunc(&ReSizeGLScene);
	glutKeyboardFunc(&NormalKeyPressed);
	glutSpecialFunc(&SpecialKeyPressed);
	InitGL(DEFAULT_WIDTH, DEFAULT_HEIGHT);
	glutMainLoop();
	
	return 0;
}
