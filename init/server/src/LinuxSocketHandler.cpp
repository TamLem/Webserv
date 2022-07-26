#include "LinuxSocketHandler.hpp"


// Constructors
LinuxSocketHandler::LinuxSocketHandler()
{
	std::cout << "LinuxSocketHandler Default Constructor called" << std::endl;
	/*CODE*/
}

LinuxSocketHandler::LinuxSocketHandler(const LinuxSocketHandler &src)
{
	std::cout << "LinuxSocketHandler Copy Constructor called" << std::endl;
	*this = src;
}

// Deconstructors
LinuxSocketHandler::~LinuxSocketHandler()
{
	/*CODE*/
	std::cout << "LinuxSocketHandler Deconstructor called" << std::endl;
}

// Overloaded Operators
LinuxSocketHandler &LinuxSocketHandler::operator=(const LinuxSocketHandler &src)
{
	std::cout << "LinuxSocketHandler Assignation operator called" << std::endl;
	if (this == &src)
		return *this;

	/*CODE*/
	return *this;
}

// Public Methods

// Getter

// Setter

