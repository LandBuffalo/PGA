#ifndef __MIGRATE_HH__
#define __MIGRATE_HH__
#include <mpi.h>
#include "random.h"
#include "config.h"

class Migrate
{
private:
	Random					random_;
	IslandInfo              island_info_;
	ProblemInfo             problem_info_;
	NodeInfo				node_info_;
    vector<int>             destinations_;
    double  *               msg_send_;
    MPI_Request             mpi_request_;
    double                  previous_best_value_;
    Population              recv_buffer_;
    int                     success_sent_flag_;
	int						SerialIndividualToMsg(double *msg_of_node_EA, vector<Individual> &individual);
    int                     DeserialMsgToIndividual(vector<Individual> &individual, double *msg_of_node_EA, int length);

    int                     CheckAndRecvEmigrations();
    int                     UpdateDestination();
    int                     SendEmigrations(Population &population, int best_ID);
    int                     FindBestIndividual(Population &population);
public:
							Migrate(const NodeInfo node_info);
							~Migrate();

	int 					Initilize(IslandInfo island_info, ProblemInfo problem_info);
	int 					Unitilize();
    int                     Finish();
    int                     ExportAndImport(Population &population);
    Population * recv_buffer();
};
#endif
