#include "GA.h"

GA::GA()
{

}

GA::~GA()
{

}

string GA::GetParameters()
{
    string str;
    ostringstream temp1, temp2, temp3;
    string parameters = "CR/MR/Pool=";

    double CR = GA_info_.CR;
    temp1<<CR;
    str=temp1.str();
    parameters.append(str);

    parameters.append("/");
    double MR = GA_info_.MR;
    temp2 << MR;
    str=temp2.str();
    parameters.append(str);

    parameters.append("/");
    double pool = GA_info_.pool_size;
    temp3<<pool;
    str=temp3.str();
    parameters.append(str);
    if(GA_info_.selection_strategy == 0)
        parameters.append("_tournament");
    else if(GA_info_.selection_strategy == 1)
        parameters.append("_roulette");

    return parameters;
}
double GA::CheckBound(real to_check_elements, real min_bound, real max_bound)
{
    while ((to_check_elements < min_bound) || (to_check_elements > max_bound))
    {
        if (to_check_elements < min_bound)
            to_check_elements = min_bound + (min_bound - to_check_elements);
        if (to_check_elements > max_bound)
            to_check_elements = max_bound - (to_check_elements - max_bound);
    }
    return to_check_elements;
}

Individual GA::FindBestIndividual(Population & population)
{
    int best_individual_ind = 0;
    double best_individual_fitness_value = population[0].fitness_value;
    for(int i = 1; i < pop_size_; i++)
    {
        if(population[i].fitness_value < best_individual_fitness_value)
        {
            best_individual_ind = i;
            best_individual_fitness_value = population[i].fitness_value;
        }
    }
    return population[best_individual_ind];
}

int GA::Initilize(ProblemInfo problem_info, int pop_size)
{
	problem_info_ = problem_info;
    pop_size_ = pop_size;
    cec2014_.Initilize(problem_info_.function_ID, problem_info_.dim);
    GA_info_.CR = 0.8;
	GA_info_.MR = 0.2;
	GA_info_.selection_strategy = 0;
	GA_info_.pool_size = 2;
}
int GA::InitilizePopulation(Population & population)
{
    for(int i = 0; i < pop_size_; i++)
    {
        Individual tmp_individual;

        for (int j = 0; j < problem_info_.dim; j++)
            tmp_individual.elements.push_back(random_.RandDoubleUnif(problem_info_.min_bound, problem_info_.max_bound));

        tmp_individual.fitness_value = cec2014_.EvaluateFitness(tmp_individual.elements);
        population.push_back(tmp_individual);
    }
    candidate_ = population;
    return 0;
}
int GA::Unitilize()
{
    cec2014_.Unitilize();

    return 0;
}

vector<int> GA::SelectParents(Population & population, vector<int> parents_index_pool)
{
    vector<int> parent_index;
    vector<real> parents_fitness_value_pool;
    for (int i = 0; i < GA_info_.pool_size; i++)
        parents_fitness_value_pool.push_back(population[parents_index_pool[i]].fitness_value);

    for(int parents_num = 0; parents_num < 2; parents_num++)
    {
        real best_fitness_value = parents_fitness_value_pool[0];
        int local_best_index = 0;

        for (int i = 1; i < GA_info_.pool_size; i++)
        {
            if (best_fitness_value > parents_fitness_value_pool[i])
            {
                best_fitness_value = parents_fitness_value_pool[i];
                local_best_index = i;
            }
        }
        parent_index.push_back(parents_index_pool[local_best_index]);
        parents_fitness_value_pool[local_best_index] = 1e20;
    }

    return parent_index;
}

int GA::Reproduce(Population & population, Population * recv_buffer)
{
    for (int i = 0; i < pop_size_; i++)
    {
        vector<int> parents_index_pool = random_.Permutate(pop_size_, GA_info_.pool_size);
        vector<int> parent_index = SelectParents(population, parents_index_pool);

        Individual select_from_buffer;
        if(recv_buffer->size() > 0)
        {
            select_from_buffer = (*recv_buffer)[0];
            recv_buffer->erase(recv_buffer->begin());

            if(select_from_buffer.fitness_value < 0)
            {
              for (int j = 0; j < problem_info_.dim; j++)
                candidate_[i].elements[j] = select_from_buffer.elements[j];
            }
            else
            {
                double rand_value = random_.RandDoubleUnif(0, 1);
                Individual parent1, parent2;
                if(rand_value < 0.5)
                {
                    parent1 = population[parent_index[0]];
                    parent2 = select_from_buffer;
                }
                else
                {
                    parent2 = population[parent_index[1]];
                    parent1 = select_from_buffer;
                }

                for (int j = 0; j < problem_info_.dim; j++)
                {
                    if (random_.RandDoubleUnif(0, 1) < GA_info_.CR)
                    {
                        real lambda = random_.RandDoubleUnif(0, 1);
                        candidate_[i].elements[j] = lambda * parent1.elements[j] + (1 - lambda) * parent2.elements[j];
                    }
                    else
                    {
                        real lambda = random_.RandDoubleUnif(0, 1);
                        if(lambda > 0.5)
                            candidate_[i].elements[j] = parent1.elements[j];
                        else
                            candidate_[i].elements[j] = parent2.elements[j];
                    }

                    if (random_.RandDoubleUnif(0, 1) < GA_info_.MR)
                        candidate_[i].elements[j] = random_.RandDoubleUnif(problem_info_.min_bound, problem_info_.max_bound);
                    candidate_[i].elements[j] = CheckBound(candidate_[i].elements[j], problem_info_.min_bound, problem_info_.max_bound);
                }
            }
        }
        else
        {
            Individual parent1, parent2;
            parent1 = population[parent_index[0]];
            parent2 = population[parent_index[1]];
            for (int j = 0; j < problem_info_.dim; j++)
            {
                if (random_.RandDoubleUnif(0, 1) < GA_info_.CR)
                {
                    real lambda = random_.RandDoubleUnif(0, 1);
                    candidate_[i].elements[j] = lambda * parent1.elements[j] + (1 - lambda) * parent2.elements[j];
                }
                else
                {
                    real lambda = random_.RandDoubleUnif(0, 1);
                    if(lambda > 0.5)
                        candidate_[i].elements[j] = parent1.elements[j];
                    else
                        candidate_[i].elements[j] = parent2.elements[j];
                }

                if (random_.RandDoubleUnif(0, 1) < GA_info_.MR)
                    candidate_[i].elements[j] = random_.RandDoubleUnif(problem_info_.min_bound, problem_info_.max_bound);
                candidate_[i].elements[j] = CheckBound(candidate_[i].elements[j], problem_info_.min_bound, problem_info_.max_bound);
            }
        }
    }

    for (int i = 0; i < pop_size_; i++)
        candidate_[i].fitness_value = cec2014_.EvaluateFitness(candidate_[i].elements);
    return 0;
}

int GA::Reproduce(Population & population)
{
    for (int i = 0; i < pop_size_; i++)
    {
        vector<int> parents_index_pool = random_.Permutate(pop_size_, GA_info_.pool_size);
        vector<int> parent_index = SelectParents(population, parents_index_pool);

            Individual parent1, parent2;
            parent1 = population[parent_index[0]];
            parent2 = population[parent_index[1]];
            for (int j = 0; j < problem_info_.dim; j++)
            {
                if (random_.RandDoubleUnif(0, 1) < GA_info_.CR)
                {
                    real lambda = random_.RandDoubleUnif(0, 1);
                    candidate_[i].elements[j] = lambda * parent1.elements[j] + (1 - lambda) * parent2.elements[j];
                }
                else
                {
                    real lambda = random_.RandDoubleUnif(0, 1);
                    if(lambda > 0.5)
                        candidate_[i].elements[j] = parent1.elements[j];
                    else
                        candidate_[i].elements[j] = parent2.elements[j];
                }

                if (random_.RandDoubleUnif(0, 1) < GA_info_.MR)
                    candidate_[i].elements[j] = random_.RandDoubleUnif(problem_info_.min_bound, problem_info_.max_bound);
                candidate_[i].elements[j] = CheckBound(candidate_[i].elements[j], problem_info_.min_bound, problem_info_.max_bound);
            }
    }

    for (int i = 0; i < pop_size_; i++)
        candidate_[i].fitness_value = cec2014_.EvaluateFitness(candidate_[i].elements);
    return 0;
}

int GA::Run(Population & population, Population * recv_buffer)
{
    Reproduce(population, recv_buffer);
    population = candidate_;

    return 0;
}

int GA::Run(Population & population)
{
    Reproduce(population);

    population = candidate_;

    return 0;
}