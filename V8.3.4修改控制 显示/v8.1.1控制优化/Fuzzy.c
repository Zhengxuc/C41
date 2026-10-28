
#include "Device.h"



//精英保留率通常是指在遗传算法等优化算法中，采用精英保留策略时，
//保留到下一代的精英个体数量占种群个体总数的比例。
//遗传算法是一种受生物进化过程启发的启发式搜索算法。
//在遗传算法的进化过程中，会通过选择、交叉和变异等操作产生新一代种群。
//为了防止当前群体的最优个体在下一代发生丢失，导致算法不能收敛到全局最优解，会采用精英保留策略。
//该策略会把群体在进化过程中迄今出现的最好个体，不进行配对交叉而直接复制到下一代中。
//例如，若种群规模为 100，设置精英个体数量为 5，
//让这 5 个最优个体直接进入下一代，那么精英保留率就是 5%。较高的精英保留率可以加快算法的收敛速度，
//但可能会影响种群的多样性，增加陷入局部最优解的风险；
//较低的精英保留率则可能导致最优解不易被保留，影响算法的收敛性能。



// 定义系统参数
#define INPUT_DIM 2       // 输入维度：误差和误差变化率
#define OUTPUT_DIM 1      // 输出维度

#define RULE_NUM 9        // 规则数量（3x3矩阵）      //这里的数量越大应该越准
#define MEMBERSHIP_NUM 3  // 每个输入的隶属度函数数量
#define POPULATION_SIZE 50 // 遗传算法种群大小
#define GENERATIONS 100    // 遗传算法迭代代数
#define CROSSOVER_RATE 0.8 // 交叉概率
#define MUTATION_RATE 0.1  // 变异概率

#define ELITISM_RATE 0.1   // 精英保留率  只有十分之一能保留

#define TRAINING_SAMPLES 100 // 训练样本数

// 模糊系统参数结构
typedef struct {
    // 隶属度函数参数: [输入维度][隶属度函数数][中心,宽度]
    float membership_params[INPUT_DIM][MEMBERSHIP_NUM][2];
    // 规则权重: [规则数][输出维度]
    float rule_weights[RULE_NUM][OUTPUT_DIM];
} FuzzyParams;

// 训练样本结构
typedef struct {
    float inputs[INPUT_DIM];   //输入维度2 
    float target[OUTPUT_DIM];   //输出维度1
} TrainingSample;

// 遗传算法个体结构
typedef struct {
    FuzzyParams params;
    float fitness;
} Individual;

// 种群结构
typedef struct {
    Individual *individuals;
    int size;
} Population;

// 高斯隶属度函数
float gaussian_membership(float x, float center, float width) {
    return exp(-pow((x - center) / width, 2));
}

// 计算输入对各隶属度函数的隶属度值
void calculate_memberships(float inputs[], FuzzyParams *params, 
                          float membership_values[INPUT_DIM][MEMBERSHIP_NUM]) {
    for (int i = 0; i < INPUT_DIM; i++) {
        for (int m = 0; m < MEMBERSHIP_NUM; m++) {
            float center = params->membership_params[i][m][0];
            float width = params->membership_params[i][m][1];
            membership_values[i][m] = gaussian_membership(inputs[i], center, width);
        }
    }
}

// 评估所有规则的激发强度
void evaluate_rules(float membership_values[INPUT_DIM][MEMBERSHIP_NUM], 
                   FuzzyParams *params, float rule_firings[RULE_NUM]) {
    int rule_index = 0;
    for (int m1 = 0; m1 < MEMBERSHIP_NUM; m1++) {
        for (int m2 = 0; m2 < MEMBERSHIP_NUM; m2++) {
            // 使用MIN操作符计算规则激发强度
            rule_firings[rule_index] = fmin(membership_values[0][m1], membership_values[1][m2]);
            rule_index++;
        }
    }
}

// 去模糊化（加权平均）
float defuzzify(float rule_firings[RULE_NUM], FuzzyParams *params) {
    float numerator = 0.0;
    float denominator = 0.0;
    
    for (int r = 0; r < RULE_NUM; r++) {
        numerator += rule_firings[r] * params->rule_weights[r][0];
        denominator += rule_firings[r];
    }
    
    // 防止除零错误
    if (denominator < 1e-6) {
        return 0.0;
    }
    
    return numerator / denominator;
}

// 模糊系统前向传播
float fuzzy_inference(float inputs[], FuzzyParams *params) {
    float membership_values[INPUT_DIM][MEMBERSHIP_NUM];
    float rule_firings[RULE_NUM];
    
    calculate_memberships(inputs, params, membership_values);
    evaluate_rules(membership_values, params, rule_firings);
    
    return defuzzify(rule_firings, params);
}

// 计算个体适应度（误差的倒数）
float calculate_fitness(Individual *individual, TrainingSample samples[], int sample_count) {
    float error_sum = 0.0;
    
    for (int i = 0; i < sample_count; i++) {
        float output = fuzzy_inference(samples[i].inputs, &individual->params);
        float error = samples[i].target[0] - output;
        error_sum += error * error;
    }
    
    // 适应度与误差成反比
    return 1.0 / (1.0 + error_sum / sample_count);
}

// 初始化个体
void initialize_individual(Individual *individual) {
    // 随机初始化隶属度函数参数
    for (int i = 0; i < INPUT_DIM; i++) {
        for (int m = 0; m < MEMBERSHIP_NUM; m++) {
            // 中心值均匀分布在输入范围内
            individual->params.membership_params[i][m][0] = -10.0 + 20.0 * (float)m / (MEMBERSHIP_NUM - 1);
            // 宽度随机在1-3之间
            individual->params.membership_params[i][m][1] = 1.0 + (float)rand() / RAND_MAX * 2.0;
        }
    }
    
    // 随机初始化规则权重
    for (int r = 0; r < RULE_NUM; r++) {
        for (int o = 0; o < OUTPUT_DIM; o++) {
            individual->params.rule_weights[r][o] = -10.0 + (float)rand() / RAND_MAX * 20.0;
        }
    }
    
    // 初始适应度为0
    individual->fitness = 0.0;
}

// 初始化种群
void initialize_population(Population *population) {
    population->individuals = (Individual*)malloc(POPULATION_SIZE * sizeof(Individual));
    population->size = POPULATION_SIZE;
    
    for (int i = 0; i < POPULATION_SIZE; i++) {
        initialize_individual(&population->individuals[i]);
    }
}

// 计算种群中所有个体的适应度
void evaluate_population(Population *population, TrainingSample samples[], int sample_count) {
    for (int i = 0; i < population->size; i++) {
        population->individuals[i].fitness = calculate_fitness(&population->individuals[i], samples, sample_count);
    }
}

// 选择操作（锦标赛选择）
void tournament_selection(Population *population, Individual *parent1, Individual *parent2) {
    int tournament_size = 3;
    int best_index1 = rand() % population->size;
    float best_fitness1 = population->individuals[best_index1].fitness;
    
    // 第一轮锦标赛
    for (int i = 1; i < tournament_size; i++) {
        int index = rand() % population->size;
        if (population->individuals[index].fitness > best_fitness1) {
            best_index1 = index;
            best_fitness1 = population->individuals[index].fitness;
        }
    }
    
    // 第二轮锦标赛（确保不选择同一个父代）
    int best_index2;
    float best_fitness2;
    do {
        best_index2 = rand() % population->size;
        best_fitness2 = population->individuals[best_index2].fitness;
    } while (best_index2 == best_index1);
    
    for (int i = 1; i < tournament_size; i++) {
        int index;
        do {
            index = rand() % population->size;
        } while (index == best_index1);
        
        if (population->individuals[index].fitness > best_fitness2) {
            best_index2 = index;
            best_fitness2 = population->individuals[index].fitness;
        }
    }
    
    // 返回两个最优个体
    *parent1 = population->individuals[best_index1];
    *parent2 = population->individuals[best_index2];
}

// 交叉操作
void crossover(Individual *parent1, Individual *parent2, Individual *child1, Individual *child2) {
    if ((float)rand() / RAND_MAX > CROSSOVER_RATE) {
        // 不交叉，直接复制父代
        *child1 = *parent1;
        *child2 = *parent2;
        return;
    }
    
    // 对隶属度函数参数进行交叉
    for (int i = 0; i < INPUT_DIM; i++) {
        for (int m = 0; m < MEMBERSHIP_NUM; m++) {
            for (int p = 0; p < 2; p++) {
                if ((float)rand() / RAND_MAX > 0.5) {
                    child1->params.membership_params[i][m][p] = parent1->params.membership_params[i][m][p];
                    child2->params.membership_params[i][m][p] = parent2->params.membership_params[i][m][p];
                } else {
                    child1->params.membership_params[i][m][p] = parent2->params.membership_params[i][m][p];
                    child2->params.membership_params[i][m][p] = parent1->params.membership_params[i][m][p];
                }
            }
        }
    }
    
    // 对规则权重进行交叉
    for (int r = 0; r < RULE_NUM; r++) {
        for (int o = 0; o < OUTPUT_DIM; o++) {
            if ((float)rand() / RAND_MAX > 0.5) {
                child1->params.rule_weights[r][o] = parent1->params.rule_weights[r][o];
                child2->params.rule_weights[r][o] = parent2->params.rule_weights[r][o];
            } else {
                child1->params.rule_weights[r][o] = parent2->params.rule_weights[r][o];
                child2->params.rule_weights[r][o] = parent1->params.rule_weights[r][o];
            }
        }
    }
    
    // 初始适应度为0
    child1->fitness = 0.0;
    child2->fitness = 0.0;
}

// 变异操作
void mutate(Individual *individual) {
    // 对隶属度函数参数进行变异
    for (int i = 0; i < INPUT_DIM; i++) {
        for (int m = 0; m < MEMBERSHIP_NUM; m++) {
            for (int p = 0; p < 2; p++) {
                if ((float)rand() / RAND_MAX < MUTATION_RATE) {
                    // 添加随机扰动
                    float perturbation = (float)rand() / RAND_MAX * 2.0 - 1.0;
                    individual->params.membership_params[i][m][p] += perturbation;
                    
                    // 确保参数在合理范围内
                    if (p == 0) {  // 中心值
                        individual->params.membership_params[i][m][p] = 
                            fmax(-15.0, fmin(15.0, individual->params.membership_params[i][m][p]));
                    } else {  // 宽度
                        individual->params.membership_params[i][m][p] = 
                            fmax(0.5, fmin(5.0, individual->params.membership_params[i][m][p]));
                    }
                }
            }
        }
    }
    
    // 对规则权重进行变异
    for (int r = 0; r < RULE_NUM; r++) {
        for (int o = 0; o < OUTPUT_DIM; o++) {
            if ((float)rand() / RAND_MAX < MUTATION_RATE) {
                float perturbation = (float)rand() / RAND_MAX * 4.0 - 2.0;
                individual->params.rule_weights[r][o] += perturbation;
                
                // 确保权重在合理范围内
                individual->params.rule_weights[r][o] = 
                    fmax(-15.0, fmin(15.0, individual->params.rule_weights[r][o]));
            }
        }
    }
}

// 精英保留
void elitism(Population *old_pop, Population *new_pop) {
    int elite_count = (int)(POPULATION_SIZE * ELITISM_RATE);
    
    // 对旧种群按适应度排序
    for (int i = 0; i < old_pop->size - 1; i++) {
        for (int j = i + 1; j < old_pop->size; j++) {
            if (old_pop->individuals[i].fitness < old_pop->individuals[j].fitness) {
                Individual temp = old_pop->individuals[i];
                old_pop->individuals[i] = old_pop->individuals[j];
                old_pop->individuals[j] = temp;
            }
        }
    }
    
    // 复制精英个体到新种群
    for (int i = 0; i < elite_count; i++) {
        new_pop->individuals[i] = old_pop->individuals[i];
    }
}

// 替换旧种群
void replace_population(Population *old_pop, Population *new_pop) {
    elitism(old_pop, new_pop);
    
    int elite_count = (int)(POPULATION_SIZE * ELITISM_RATE);
    
    // 生成剩余个体
    for (int i = elite_count; i < POPULATION_SIZE; i += 2) {
        Individual parent1, parent2, child1, child2;
        
        // 选择父代
        tournament_selection(old_pop, &parent1, &parent2);
        
        // 交叉
        crossover(&parent1, &parent2, &child1, &child2);
        
        // 变异
        mutate(&child1);
        mutate(&child2);
        
        // 添加到新种群
        new_pop->individuals[i] = child1;
        if (i + 1 < POPULATION_SIZE) {
            new_pop->individuals[i + 1] = child2;
        }
    }
    
    // 释放旧种群内存
    free(old_pop->individuals);
}

// 找到最优个体
Individual* find_best_individual(Population *population) {
    float best_fitness = -1.0;
    Individual *best_individual = NULL;
    
    for (int i = 0; i < population->size; i++) {
        if (population->individuals[i].fitness > best_fitness) {
            best_fitness = population->individuals[i].fitness;
            best_individual = &population->individuals[i];
        }
    }
    
    return best_individual;
}

// 生成示例训练数据
void generate_training_data(TrainingSample samples[], int count) {   //输入数组
  
    // 模拟阀门控制专家的决策规则
    for (int i = 0; i < count; i++) {
      
        // 生成随机输入（误差和误差变化率）
        samples[i].inputs[0] = (float)rand() / RAND_MAX * 20.0 - 10.0;  // 误差: [-10, 10]
        samples[i].inputs[1] = (float)rand() / RAND_MAX * 10.0 - 5.0;   // 误差变化率: [-5, 5]
        
        float error = samples[i].inputs[0];
        float error_rate = samples[i].inputs[1];
        
        // 基于专家知识的目标输出（模拟阀门控制策略）
        float target = 0.0;
        
        if (error > 5) {
            if (error_rate > 2) {
                target = 8.0;  // 正大
            } else if (error_rate < -2) {
                target = 6.0;  // 正中
            } else {
                target = 7.0;  // 正较大
            }
        } else if (error > 1) {
            if (error_rate > 2) {
                target = 6.0;  // 正中
            } else if (error_rate < -2) {
                target = 4.0;  // 正小
            } else {
                target = 5.0;  // 正中小
            }
        } else if (error > -1) {
            if (error_rate > 2) {
                target = 2.0;  // 负小
            } else if (error_rate < -2) {
                target = 4.0;  // 正小
            } else {
                target = 3.0;  // 零
            }
        } else if (error > -5) {
            if (error_rate > 2) {
                target = -4.0;  // 负小
            } else if (error_rate < -2) {
                target = -6.0;  // 负中
            } else {
                target = -5.0;  // 负中小
            }
        } else {
            if (error_rate > 2) {
                target = -6.0;  // 负中
            } else if (error_rate < -2) {
                target = -8.0;  // 负大
            } else {
                target = -7.0;  // 负较大
            }
        }
        
        samples[i].target[0] = target;
        
    }
}

// 打印模糊规则
void print_fuzzy_rules(FuzzyParams *params) {
    const char *error_labels[MEMBERSHIP_NUM] = {"负", "零", "正"};
    const char *error_rate_labels[MEMBERSHIP_NUM] = {"负", "零", "正"};
    
//    printf("\n优化后的模糊规则表:\n");
    int rule_index = 0;
    
    for (int m1 = 0; m1 < MEMBERSHIP_NUM; m1++) {
        for (int m2 = 0; m2 < MEMBERSHIP_NUM; m2++) {
            printf("规则 %d: 如果 误差是%s且误差变化率是%s, 则 输出 = %.2f\n",
                   rule_index + 1,
                   error_labels[m1],
                   error_rate_labels[m2],
                   params->rule_weights[rule_index][0]);
            rule_index++;
        }
    }
    
    printf("\n隶属度函数参数:\n");
    for (int i = 0; i < INPUT_DIM; i++) {
        const char *input_names[INPUT_DIM] = {"误差", "误差变化率"};
//        printf("输入 %s 的隶属度函数参数:\n", input_names[i]);
        
        for (int m = 0; m < MEMBERSHIP_NUM; m++) {
//            printf("  %s%d: 中心=%.2f, 宽度=%.2f\n",
//                   error_labels[m], m,
//                   params->membership_params[i][m][0],
//                   params->membership_params[i][m][1]);
        }
    }
}


void Fuzzy_Control(void ){
  
       // 初始化随机数种子
    srand(time(NULL));
    
    // 创建训练数据
    TrainingSample samples[TRAINING_SAMPLES];
    
    generate_training_data(samples, TRAINING_SAMPLES);    //输入样本，采样数1oo
    
    // 初始化种群
    Population population;
    initialize_population(&population);
    
    // 评估初始种群
    evaluate_population(&population, samples, TRAINING_SAMPLES);
    
    printf("遗传算法优化模糊规则表...\n"); 
    // 遗传算法主循环
    for (int gen = 0; gen < GENERATIONS; gen++) {
        // 创建新种群
        Population new_pop;
        new_pop.individuals = (Individual*)malloc(POPULATION_SIZE * sizeof(Individual));
        new_pop.size = POPULATION_SIZE;
        
        // 替换种群
        replace_population(&population, &new_pop);
        
        // 评估新种群
        evaluate_population(&new_pop, samples, TRAINING_SAMPLES);
        
        // 更新当前种群
        population = new_pop;
        
        // 打印进度
        if (gen % 10 == 0) {
            Individual *best = find_best_individual(&population);
//            printf("代数 %d, 最佳适应度 = %.4f, 误差 = %.4f\n", 
//                   gen, best->fitness, 1.0/best->fitness - 1.0);
        }
    }
    
    // 找到最优个体
    Individual *best = find_best_individual(&population);
    
    // 打印优化后的模糊规则
    print_fuzzy_rules(&best->params);
    
    // 测试优化后的模糊系统
//    printf("\n测试优化后的模糊系统:\n");
//    printf("误差\t误差变化率\t目标输出\t实际输出\t误差\n");
    
    for (int i = 0; i < 10; i++) {
        int index = rand() % TRAINING_SAMPLES;
        float output = fuzzy_inference(samples[index].inputs, &best->params);
        float error = samples[index].target[0] - output;
        
//        printf("%.2f\t%.2f\t\t%.2f\t\t%.2f\t\t%.2f\n",
//               samples[index].inputs[0],
//               samples[index].inputs[1],
//               samples[index].target[0],
//               output,
//               error);
    }
    
    // 释放内存
//    free(population.individuals);  
  
  
}




  
  
  
