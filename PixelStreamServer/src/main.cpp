/*
 * main.cpp
 *
 *  Created on: Jul 18, 2015
 *      Author: Peter
 */


#include <vector>
#include<iostream>
#include <stdio.h>
#include "../header/VivoTechConnection.hpp"


using namespace std;


int main(int argc, char **argv)
{

	signal(SIGPIPE, SIG_IGN);

	VivoTechConnection *con;
	con = new VivoTechConnection();
	con->StreamInit();
	while(1){};

	return 0;
}
