#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/array.hpp>
#include <iostream>

using namespace boost::asio;

static bool isRunning(true);
const char *PORT = "/dev/ttyUSB0";
io_service io;
serial_port port( io, PORT );
boost::array<char, 64> rbuf;

static void waitKeyPressed(void)
{
	getchar();
	isRunning = false;
}

void read_callback(const boost::system::error_code& e, std::size_t size)
{
	//std::cout << "read :" << size << "byte[s] " << std::endl;
	std::cout.write(rbuf.data(), size);

	port.async_read_some( buffer(rbuf), boost::bind(&read_callback, _1, _2 ));
}

void write_callback(const boost::system::error_code& e, std::size_t size )
{
	std::cout << "write :" << size << "byte[s]" << std::endl;
}

int main(int argc, char *argv[])
{
	const boost::thread thr_wait(&waitKeyPressed);

	//std::string wbuf = argv[1];

	port.set_option(serial_port_base::baud_rate(115200));
	port.set_option(serial_port_base::character_size(8));
	port.set_option(serial_port_base::flow_control(serial_port_base::flow_control::none));
	port.set_option(serial_port_base::parity(serial_port_base::parity::none));
	port.set_option(serial_port_base::stop_bits(serial_port_base::stop_bits::one));

	boost::thread thr_io(boost::bind(&io_service::run, &io));

	port.async_read_some( buffer(rbuf), boost::bind(&read_callback, _1, _2 ));

	//port.async_write_some( buffer(wbuf), boost::bind(&write_callback, _1, _2));

	while(isRunning){
		sleep(2);
	}

	return 0;
}
