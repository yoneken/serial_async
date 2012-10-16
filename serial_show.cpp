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
double val[640];
int val_pointer = 0;
double amp = 1000.0;

void read_callback(const boost::system::error_code& e, std::size_t size)
{
	boost::mutex::scoped_lock lk(mtx);

	for(unsigned int i=0;i<size;i++){
		char c = rbuf.at(i);
		fbuf[fp++] = c;
		if(c == '\n'){
#ifdef FILE_SAVE
			ofs.write(fbuf, fp);
#endif /* FILE_SAVE */
			int time;
			double value;
			sscanf(fbuf, "%d,%lf\r\n",
				&time, &value);
			//printf("%6d, %3.5f\r\n", time, value);
			val[val_pointer++] = value;
			if(val_pointer == 640) val_pointer = 0;
			fp = 0;
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
	
	int p = val_pointer;
	glColor3ub(255, 255, 0);	// Draw In Yellow
	glBegin(GL_LINES);
	for(int i=0;i<640-1;i++){
		int p2;
		if(p == 640) p = 0;
		if(p != 640-1) p2 = p+1;
		else p2 = 0;
		glVertex2i( i, (int)(val[p]*amp)+20);
		glVertex2i( i+1, (int)(val[p2]*amp)+20);
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
  if (keys == KEY_ESCAPE) {
		exit(0);
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
