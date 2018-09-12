#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <cmath>
#include <time.h>
#include <numeric>
#include <string>
#include <iostream>
#include <iomanip>

using namespace std;

//#define TOTAL_RECORD_NUM		20
#define EMIGRATIONS             1
#define FLAG_FINISH       		2
#define FLAG_DISPLAY_UNIT       3

//#define DISPLAY
//#define DIVERSITY

typedef double real;
struct Individual
{
	vector<double> elements;
	double fitness_value;
};

typedef vector<Individual> Population;


struct ProblemInfo
{
	int dim;
	int function_ID;
	int run_ID;
	int max_base_FEs;
	int seed;
	int running_time;
	double max_bound;
	double min_bound;

	vector<int> total_functions;
	vector<int> total_runs;
};

struct NodeInfo
{
    int task_ID;
	int node_ID;
	int node_num;
};

struct IslandInfo
{
	int island_size;
	int island_num;
    int interval;
	int migration_size;
    int buffer_capacity;
    string migration_topology;
	string buffer_manage;
};

struct DisplayUnit
{
	int FEs;
	double time;
	double communication_percentage;
	double fitness_value;
};

//#define DEBUG


#endif
