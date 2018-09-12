#include "island_EA.h"
IslandEA::IslandEA(const NodeInfo node_info):migrate_(node_info)
{
    node_info_ = node_info;

    CheckAndCreatRecordFile();
}

IslandEA::~IslandEA()
{

}

int IslandEA::CheckAndCreatRecordFile()
{
    string file_name;
    file_name = "./Results/GAP_CPU.csv";
    GA_ = new GA();
    ifstream exist_file;
    exist_file.open(file_name.c_str());
    ofstream file;
    if(!exist_file)
    {
        file.open(file_name.c_str());
        file<< "function_ID,run_ID,dim,best_fitness,time_period,communication_percentage,total_FEs,node_num,total_pop_size,interval,migration_size,buffer_capacity,migration_topology,buffer_manage,EA_parameters"<<endl;
        file.close();
    }
    else
        exist_file.close();

    return 0;
}

int IslandEA::Initilize(IslandInfo island_info, ProblemInfo problem_info)
{
    problem_info_ = problem_info;
    island_info_ = island_info;

    GA_->Initilize(problem_info_, island_info_.island_size);

	GA_->InitilizePopulation(sub_population_);

    migrate_.Initilize(island_info_, problem_info_);

}

int IslandEA::Unitilize()
{
    sub_population_.clear();

    v_display_unit_.clear();

	GA_->Unitilize();

	migrate_.Unitilize();

    return 0;
}

int IslandEA::Finish()
{
    migrate_.Finish();
    MPI_Barrier(MPI_COMM_WORLD);
    if(node_info_.node_ID != 0)
    {
        SendResultToIsland0();
    }
    else
    {
        vector < vector<DisplayUnit> > total_display_unit;
        for(int i = 0; i < island_info_.island_num; i++)
        {
            vector<DisplayUnit> tmp_display_unit;
            total_display_unit.push_back(tmp_display_unit);
        }
        RecvResultFromOtherIsland(total_display_unit);
        MergeResults(total_display_unit);
        PrintResult();
    }


    return 0;
}

int IslandEA::RunEA()
{
    GA_->Run(sub_population_, migrate_.recv_buffer());
   //GA_->Run(sub_population_);

    return 0;
}

int IslandEA::RunMigration(int generation)
{
    if (generation % island_info_.interval == 0)
        migrate_.ExportAndImport(sub_population_);
    
    return 0;
}



int IslandEA::Execute()
{
    int generation = 0;
    int current_FEs = island_info_.island_size;
    double time_period = 0;
    double current_time = 0;
    start_time_ = MPI_Wtime();

    long int total_FEs = problem_info_.max_base_FEs * problem_info_.dim / island_info_.island_num;

    while(current_FEs < total_FEs)
    {
        RunEA();
        RunMigration(generation);
        generation++;
        current_FEs += island_info_.island_size;
    }
    current_time = MPI_Wtime();

    RecordDisplayUnit(current_FEs, current_time, 0);
	Individual tmp_individual = GA_->FindBestIndividual(sub_population_);

	Finish();

	return 0;

}

int IslandEA::RecordDisplayUnit(int current_FEs, double current_time, double communication_time)
{
    DisplayUnit tmp_display_unit;
    tmp_display_unit.FEs = current_FEs;
    tmp_display_unit.time = current_time - start_time_;
    tmp_display_unit.communication_percentage = communication_time / tmp_display_unit.time;
	Individual tmp_individual = GA_->FindBestIndividual(sub_population_);
	tmp_display_unit.fitness_value = tmp_individual.fitness_value;
    v_display_unit_.push_back(tmp_display_unit);
    return 0;
}

int IslandEA::MergeResults(vector < vector<DisplayUnit> > &total_display_unit)
{
    int total_num_display_unit_of_each_island = v_display_unit_.size();

    for(int i = 0; i < total_num_display_unit_of_each_island; i++)
    {
        double best_fitness = v_display_unit_[i].fitness_value;
        for(int j = 1; j < node_info_.node_num; j++)
        {
            if(best_fitness > total_display_unit[j][i].fitness_value)
                best_fitness = total_display_unit[j][i].fitness_value;
        }
        v_display_unit_[i].fitness_value = best_fitness;

        double time_period = v_display_unit_[i].time;
        int current_FEs = v_display_unit_[i].FEs;
        double communication_percentage = v_display_unit_[i].communication_percentage;

        for(int j = 1; j < node_info_.node_num; j++)
        {
            current_FEs += total_display_unit[j][i].FEs;
            time_period += total_display_unit[j][i].time;
            communication_percentage += total_display_unit[j][i].communication_percentage;
        }
        v_display_unit_[i].fitness_value = best_fitness;
        v_display_unit_[i].FEs = current_FEs;
        v_display_unit_[i].time = time_period / (node_info_.node_num + 0.0);
        v_display_unit_[i].communication_percentage = communication_percentage / (node_info_.node_num + 0.0);
    }
    printf("Run: %d, Time: %.3lf s, Best fitness: %.9lf\n", problem_info_.run_ID, v_display_unit_[total_num_display_unit_of_each_island - 1].time, v_display_unit_[total_num_display_unit_of_each_island - 1].fitness_value);

}

int IslandEA::PrintResult()
{
    string file_name;
    file_name = "./Results/GAP_CPU.csv";


    ofstream file;
    file.open(file_name.c_str(), ios::app);

    long int total_FEs = problem_info_.max_base_FEs * problem_info_.dim;
    int total_pop_size = island_info_.island_num * island_info_.island_size;

    string EA_parameters = GA_->GetParameters();
    int total_num_display_unit_of_each_island = v_display_unit_.size();
    double best_fitness = v_display_unit_[total_num_display_unit_of_each_island - 1].fitness_value;
    double time_period = v_display_unit_[total_num_display_unit_of_each_island - 1].time;
    double communication_percentage = v_display_unit_[total_num_display_unit_of_each_island - 1].communication_percentage;

    file<<problem_info_.function_ID<<','<<problem_info_.run_ID<<','<<problem_info_.dim<<','<<best_fitness<<','<<time_period<<','<<communication_percentage<<','<<total_FEs<<','<<node_info_.node_num\
    <<','<<total_pop_size<<','<<island_info_.interval<<','<<island_info_.migration_size<<','<<island_info_.buffer_capacity<<','<<island_info_.migration_topology<<','<<island_info_.buffer_manage<<','<<EA_parameters<<endl;
    file.close();
    return 0;
}

int IslandEA::SendResultToIsland0()
{
#ifdef DISPLAY
    int length_display_unit = v_display_unit_.size();
    int message_length = 4 * length_display_unit;
    double *msg = new double[message_length];

    for(int i = 0; i < length_display_unit; i++)
    {
        msg[i] = v_display_unit_[i].time;
        msg[i + length_display_unit] = v_display_unit_[i].communication_percentage;
        msg[i + 2 * length_display_unit] = v_display_unit_[i].FEs;
        msg[i + 3 * length_display_unit] = v_display_unit_[i].fitness_value;

    }
#else
    int message_length = 4;
    double *msg = new double[message_length];
    msg[0] = v_display_unit_[0].time;
    msg[1] = v_display_unit_[0].communication_percentage;

    msg[2] = v_display_unit_[0].FEs;
    msg[3] = v_display_unit_[0].fitness_value;
#endif
    MPI_Send(msg, message_length, MPI_DOUBLE, 0, FLAG_DISPLAY_UNIT, MPI_COMM_WORLD);
    delete [] msg;

    return 0;
}

int IslandEA::RecvResultFromOtherIsland(vector < vector<DisplayUnit> > &total_display_unit)
{
    MPI_Status mpi_status;
    for(int i = 1; i < node_info_.node_num; i++)
    {
        int message_length = 0;
        int any_island_id = -1;
        MPI_Probe(MPI_ANY_SOURCE, FLAG_DISPLAY_UNIT, MPI_COMM_WORLD, &mpi_status);
        MPI_Get_count(&mpi_status, MPI_DOUBLE, &message_length);
        if(message_length > 0)
        {
            double *serial_msg = new double [message_length];
            MPI_Recv(serial_msg, message_length, MPI_DOUBLE, mpi_status.MPI_SOURCE, mpi_status.MPI_TAG, MPI_COMM_WORLD, &mpi_status);
            any_island_id = mpi_status.MPI_SOURCE;

            for (int j = 0; j < message_length / 4; j++)
            {
                DisplayUnit tmp_display_unit;
                tmp_display_unit.time = serial_msg[j];
                tmp_display_unit.communication_percentage = serial_msg[j + 1 * message_length / 4];
                tmp_display_unit.FEs = serial_msg[j + 2 * message_length / 4];
                tmp_display_unit.fitness_value = serial_msg[j + 3 * message_length / 4];
                total_display_unit[any_island_id].push_back(tmp_display_unit);
            }
            delete []serial_msg;
        }
        else
        {
           printf("Error: Record\n");
        }
    }
    return 0;
}

