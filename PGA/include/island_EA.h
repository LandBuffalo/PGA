#ifndef __ISLANDEA_H__
#define __ISLANDEA_H__
#pragma once
#include "config.h"
#include "random.h"
#include "GA.h"
#include "migrate.h"

class IslandEA
{
private:
	GA	*					GA_;
	Random                  random_;
	NodeInfo				node_info_;
	Migrate 				migrate_;

	ProblemInfo				problem_info_;
	IslandInfo				island_info_;

	Individual 				best_individuals_;
    Population 				sub_population_;

	vector<DisplayUnit>		v_display_unit_;
	double 					start_time_;

	int						RunEA();
    int                     RecordDisplayUnit(int current_FEs, double current_time, double communication_time);
	int 					PrintResult();
	int 					SendFlagFinish();
	int 					RecvResultFromOtherIsland(vector <vector<DisplayUnit> > &total_display_unit);
	int 					MergeResults(vector <vector<DisplayUnit> > &total_display_unit);
	int 					CheckAndCreatRecordFile();
	int 					SendResultToIsland0();
    int                    	Finish();
    int                     RunMigration(int generation);
public:
							IslandEA(const NodeInfo node_info);
							~IslandEA();
	int 					Initilize(IslandInfo island_info, ProblemInfo problem_info);
	int 					Unitilize();
	int						Execute();
};

#endif
