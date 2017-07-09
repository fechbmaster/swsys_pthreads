#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <time.h> 
#include <sys/time.h>
#include <iomanip>
#include <thread>
#include <vector>
#include <algorithm>
#include <string>

using namespace std;

bool output = false;
bool single_threaded = false;

double getTimeElapsed(struct timeval ended, struct timeval started) {
	return (ended.tv_sec - started.tv_sec) + (ended.tv_usec - started.tv_usec) / 1000000.00;
}

void writeToFile(double **laplace, int rowcol) {
	ofstream myfile;
	myfile.open("/tmp/result.txt");

	for (int i = 0; i < rowcol; i++) {
		for (int j = 0; j < rowcol; j++) {
			myfile << " " << std::fixed << std::setw(11) << std::setprecision(3) << laplace[i][j] << " ";
		}
		myfile << "\n";
	}
	myfile.close();
}


void calculateLaplace(int start, int end, int width, double **laplace_in, double **laplace_out) {
	for (int i = start; i < end; i++) {
		for (int j = 1; j <= width - 1; j++) {
			laplace_out[i][j] = 0.25*(laplace_in[i + 1][j] + laplace_in[i - 1][j] + laplace_in[i][j + 1] + laplace_in[i][j - 1]);
		}
	}
}


int main(int argc, char *argv[])
{
	struct timeval t0, t1;
	double htime;
	double **laplace0, **laplace1, prev;
	int i, j, rowcol, iterations, toggle = 0;
	if (argc != 3) cout << "Usage: hw4 [points at row] [iterations]" << endl;
	else
	{
		rowcol = atoi(argv[1]);
		iterations = atoi(argv[2]);
		laplace0 = new double *[rowcol + 1];
		laplace1 = new double *[rowcol + 1];
		for (i = 0; i <= rowcol; i++) {
			laplace0[i] = new double[rowcol + 1];
			laplace1[i] = new double[rowcol + 1];
		}
		for (i = 0; i <= rowcol; i++)
			for (j = 0; j <= rowcol; j++)
			{
				if (j == 0) {
					laplace0[i][j] = 1.0;
					laplace1[i][j] = 1.0;
				}
				else laplace0[i][j] = 0.0;
			}

		//start-time
		gettimeofday(&t0, NULL);


		for(int n = 0; n < iterations; n++)
		{
			unsigned threadNumber = std::thread::hardware_concurrency();
			if (single_threaded) threadNumber = 1;
			int step_size = rowcol / threadNumber;

			vector<std::thread> threads(threadNumber + 1); //+1 for "rest-task"
			// split the array in subtasks that can be executed by a thread
			for (int k = 0; k < threadNumber; k += step_size) {
				int start = k + 1;
				int end = k + step_size - 1;

				threads.push_back(std::thread(calculateLaplace, start, end, rowcol, (toggle == 0 ? laplace0 : laplace1), (toggle == 0 ? laplace1 : laplace0)));
			}
			for (auto &cur_thread : threads) {
				if (cur_thread.joinable()) {
					cur_thread.join();
				}
			}

			toggle = 1 - toggle;
			
		}
		//endtime
		gettimeofday(&t1, NULL);
		htime = getTimeElapsed(t1, t0);
		cout << "Done!" << endl << "Time used: " << htime << endl;

		if (output) writeToFile((toggle == 1 ? laplace0 : laplace1), rowcol);
		for (i = 0; i <= rowcol; i++) delete[] laplace0[i];
		for (i = 0; i <= rowcol; i++) delete[] laplace1[i];
		delete[] laplace0;
		delete[] laplace1;
		exit(0);
	}
	return 0;
}
