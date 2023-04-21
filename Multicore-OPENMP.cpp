#include "DS_timer.h"
#include "DS_definitions.h"
#include "genetic_findway.h"
#include <iostream>

int main(int argc, char** argv)
{

	DS_timer timer(4);

	timer.setTimerName(0, (char*)"All Serial");
	timer.setTimerName(1, (char*)"Evaluation Serial, Breeding Parallel");
	timer.setTimerName(2, (char*)"Evaluation Parallel, Breeding Serial");
	timer.setTimerName(3, (char*)"All Parallel");


	cout << "All Serial\n" << endl;
	// Mode 0.
	timer.onTimer(0);
	App app1 = App(0);
	timer.offTimer(0);

	cout << "Evaluation Serial, Breeding Parallel\n" << endl;
	// Mode 1.
	timer.onTimer(1);
	App app2 = App(1);
	timer.offTimer(1);


	cout << "Evaluation Parallel, Breeding Serial\n" << endl;
	// Mode 2.
	timer.onTimer(2);
	App app3 = App(2);
	timer.offTimer(2);

	cout << "All Parallel\n" << endl;
	// Mode 3.
	timer.onTimer(3);
	App app4 = App(3);
	timer.offTimer(3);

	timer.printTimer();

	cout << "All tasks ended. Program shutting down...\n" << endl;
	return 0;
}
