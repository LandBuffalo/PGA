#ifndef __GA_H__
#define __GA_H__
#include <stdio.h>
#include "config.h"
#include "CEC2014.h"
#include "random.h"
#include <sstream>
typedef double real;

struct GAInfo
{
    double CR;
	double MR;
	int selection_strategy;
	int pool_size;
};

class GA
{
private:
    ProblemInfo             problem_info_;
    int                     pop_size_;
    CEC2014   				cec2014_;
    Random          		random_;

	GAInfo          		GA_info_;
    vector<int>             SelectParents(Population & population, vector<int> parents_index_pool);
    Population              candidate_;
    int                     Reproduce(Population & population, Population * recv_buffer);
int                     Reproduce(Population & population);
public:
							GA();
							~GA();
	int    		 			Initilize(ProblemInfo problem_info, int pop_size);
    int             		InitilizePopulation(Population & population);
	int            			Unitilize();
	int    		 			Run(Population & population, Population * recv_buffer);
 int    		 			Run(Population & population);
	string          		GetParameters();

	Individual     	        FindBestIndividual(Population & population);
    double                  CheckBound(real to_check_elements, real min_bound, real max_bound);
};


#endif
