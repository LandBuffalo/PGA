#include "migrate.h"

Migrate::Migrate(NodeInfo node_info)
{
    node_info_ = node_info;
}

Migrate::~Migrate()
{

}

int Migrate::Initilize(IslandInfo island_info, ProblemInfo problem_info)
{
    island_info_ = island_info;
    problem_info_ = problem_info;
    success_sent_flag_ = 1;

    msg_send_ = new double[island_info.migration_size * (problem_info_.dim + 1)];
    return 0;
}
int Migrate::Unitilize()
{
    delete []msg_send_;
    destinations_.clear();
    return 0;
}

int Migrate::CheckAndRecvEmigrations()
{
    MPI_Status mpi_status;
    int flag = 0;
    int tag = problem_info_.function_ID * 100 +  problem_info_.run_ID;
    MPI_Iprobe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &flag, &mpi_status);
    while(flag == 1)
    {
	    int message_length = 0;//problem_info_.dim + 1;
        MPI_Get_count(&mpi_status, MPI_DOUBLE, &message_length);

        double * msg_recv = new double[message_length];
        MPI_Recv(msg_recv, message_length, MPI_DOUBLE, mpi_status.MPI_SOURCE, tag, MPI_COMM_WORLD, &mpi_status);

        Population emigration_import;
        DeserialMsgToIndividual(emigration_import, msg_recv, message_length / (problem_info_.dim + 1));
        delete [] msg_recv;
        for (int i = 0; i < emigration_import.size(); i++)
        	recv_buffer_.push_back(emigration_import[i]);
        flag = 0;
        MPI_Iprobe(MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &flag, &mpi_status);
    }

    return 0;
}

int Migrate::UpdateDestination()
{

        destinations_.push_back((island_info_.island_num + 1 + island_info_.island_num) % island_info_.island_num);
        destinations_.push_back((island_info_.island_num + 2 + island_info_.island_num) % island_info_.island_num);
        destinations_.push_back((island_info_.island_num - 1 + island_info_.island_num) % island_info_.island_num);
        destinations_.push_back((island_info_.island_num - 2 + island_info_.island_num) % island_info_.island_num);
    return 0;
}


int Migrate::FindBestIndividual(Population &population)
{
	int best_ID = 0;
	double best_value = population[0].fitness_value;
	for(int i = 0; i < population.size(); i++)
	{
		if(population[i].fitness_value < best_value)
		{
			best_ID = i;
			best_value = population[i].fitness_value;
		}
	}
	if(best_value >= previous_best_value_)
	{
		previous_best_value_ = best_value;
		return -1;
	}
	else
		return best_ID;
}


int Migrate::SendEmigrations(Population &population, int best_ID)
{
    MPI_Status mpi_status;

    if(success_sent_flag_ == 0)
    {
        MPI_Test(&mpi_request_, &success_sent_flag_, &mpi_status);
        CheckAndRecvEmigrations();
    }
    if(success_sent_flag_ == 1)
    {
		Population emigration_export;
        vector<int> selected_individual_ID = random_.Permutate(population.size(), island_info_.migration_size - 1);
        int message_length = island_info_.migration_size * (problem_info_.dim + 1);
        int tag = problem_info_.function_ID * 100 +  problem_info_.run_ID;

        Individual best_individual = population[best_ID];
        best_individual.fitness_value = -population[best_ID].fitness_value;
        emigration_export.push_back(best_individual);

        for (int i = 0; i < island_info_.migration_size - 1; i++)
            emigration_export.push_back(population[selected_individual_ID[i]]);

        SerialIndividualToMsg(msg_send_, emigration_export);

        MPI_Isend(msg_send_, message_length, MPI_DOUBLE, destinations_[0], tag, MPI_COMM_WORLD, &mpi_request_);

        destinations_.erase(destinations_.begin());
        success_sent_flag_ = 0;      
    }

    return 0;
}
int Migrate::ExportAndImport(Population &population)
{
	int best_ID = FindBestIndividual(population);
	if(best_ID >= 0)
	{
		UpdateDestination();
		while(destinations_.size() > 0)
			SendEmigrations(population, best_ID);
	}
	CheckAndRecvEmigrations();

    return 0;
}

int Migrate::DeserialMsgToIndividual(vector<Individual> &individual, double *msg, int length)
{
    int count = 0;

    for (int i = 0; i < length; i++)
    {
        Individual local_individual;
        for(int j = 0; j < problem_info_.dim; j++)
        {
            local_individual.elements.push_back(msg[count]);
            count++;
        }
        local_individual.fitness_value = msg[count];
        count++;
        individual.push_back(local_individual);
    }
    return 0;
}


int Migrate::SerialIndividualToMsg(double *msg, vector<Individual> &individual)
{
    int count = 0;
    for (int i = 0; i < individual.size(); i++)
    {
        for (int j = 0; j < problem_info_.dim; j++)
        {
            msg[count] = individual[i].elements[j];
            count++;
        }
        msg[count] = individual[i].fitness_value;
        count++;
    }
    return 0;
}

int Migrate::Finish()
{
    MPI_Status mpi_status;

    while(success_sent_flag_ == 0)
    {
        MPI_Test(&mpi_request_, &success_sent_flag_, &mpi_status);
        CheckAndRecvEmigrations();
    }
    int flag_finish = 1;
    for(int i = 0; i < node_info_.node_num; i++)
        if(i != node_info_.node_ID)
            MPI_Send(&flag_finish, 1, MPI_INT, i, FLAG_FINISH, MPI_COMM_WORLD);
    int sum_flag_finish = 1;
    while(sum_flag_finish != node_info_.node_num)
    {
        int flag = 0;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &mpi_status);
        if(flag == 1)
        {
            int tag = problem_info_.function_ID * 100 +  problem_info_.run_ID;

            if(mpi_status.MPI_TAG == tag)
            {
                int count = 0;
                MPI_Get_count(&mpi_status, MPI_DOUBLE, &count);
                int message_length = count;
                double * msg_recv = new double[message_length];
                int source = mpi_status.MPI_SOURCE;
                MPI_Recv(msg_recv, message_length, MPI_DOUBLE, source, mpi_status.MPI_TAG, MPI_COMM_WORLD, &mpi_status);

                delete [] msg_recv;
            }
            else if(mpi_status.MPI_TAG == FLAG_FINISH)
            {
                int msg_recv = 0;
                MPI_Recv(&msg_recv, 1, MPI_INT, MPI_ANY_SOURCE, FLAG_FINISH, MPI_COMM_WORLD, &mpi_status);
                if(msg_recv == 1)
                    sum_flag_finish++;
            }
//            else
//            {
//                printf("Err: receive tag = %d\n", mpi_status.MPI_TAG);
//            }
        }
    }
    return 0;
}

Population * Migrate::recv_buffer()
{
    return &recv_buffer_;
}
