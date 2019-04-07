#ifndef DATA_SYNTHESIS_HPP_INCLUDED
#define DATA_SYNTHESIS_HPP_INCLUDED

#include <vector>
#include <numeric>
#include <algorithm>
#include <cmath>
#include <limits>
#include "normal_rand.hpp"

/** \brief Класс для синтеза случайных данных основываясь на базовой выборке
 */
class DataSynthesis {
public:


    /** \brief Класс одного элемента данных
     */
    class DataNode {
        public:
        std::vector<int> p_data;                                    /**< Распределение вероятностей */
        std::vector<std::vector<int>> p_len;                        /**< Распределение вероятностей длины сигнала*/
        std::vector<DataNode> p_transition;                         /**< Переход на следующую метку */

        double min_data = std::numeric_limits<double>::max();       /**< Минимальное значение данных */
        double max_data = std::numeric_limits<double>::lowest();    /**< Максимальное значение данных */
        double step_data;                                           /**< Шаг данных */

        DataNode() {}

        /** \brief Обновить минимумы и максимумы
         * \param data данные
         */
        void updata_min_max(double data) {
            if(data > max_data) {
                max_data = data;
            }
            if(data < min_data) {
                min_data = data;
            }
        }

        /** \brief Обновить распределение вероятности
         * \param data значение сэмпла данных
         * \param len длительность сэмпла данных
         */
        void updata_probability(double data, int len) {
            int indx = (data - min_data) / step_data;
            if(indx >= p_data.size()) {
                p_data.resize(indx + 1);
                p_len.resize(indx + 1);
                p_transition.resize(indx + 1);
            }
            p_data[indx]++; // прибавим баллы к вероятности данного значения сигнала

            if(len > p_len[indx].size()) {
                p_len[indx].resize(len);
            }
            p_len[indx][len - 1]++; // прибавим баллы к неизменчивости сигнала
        }

        /** \brief Сгладить статистику
         */
        void smooth_probability() {
            for(size_t i = 0; i < p_data.size(); ++i) {
                // если элемент статистики равен 0
                if(p_data[i] == 0) {
                    // если элемент статистики не скраю
                    if(i > 0 && i < p_data.size() - 1) {
                        // если элепент статистик окружен значениями
                        if(p_data[i - 1] != 0 && p_data[i + 1] != 0) {
                            p_data[i] = (p_data[i - 1] + p_data[i + 1]) / 2;
                        } // if
                    } // if
                } // if
            } // if
        }

        /** \brief Получить индекс перехода на следующий узел или для других целей
         * \param data данные для перехода
         */
        int get_transition_index(double data) {
            return (data - min_data) / step_data;
        }

        int get_max_transition() {
            return p_data.size();
        }
    };



    /** \brief Получить индекс случайно выбранного элемента
     * \param probability распределение вероятнстей
     * \return индекс элемента
     */
    int get_index(std::vector<int> &probability) {
        int sum_probability = std::accumulate(probability.begin(), probability.end(), double(0));
        int p = NormalRand::random_number(0, sum_probability);
        int level_up = 0;
        for(size_t i = 0; i < probability.size(); ++i) {
            if(probability[i] == 0) continue; // пропускаем с 0 вероятностью
            int level_dn = level_up; // нижняя граница
            level_up += probability[i];
            if(p < level_up && p >= level_dn) {
                return i;
            }
        }
        return probability.size() - 1;
    }

    /** \brief Получить индекс случайно выбранного элемента
     * \param probability распределение вероятнстей
     * \return индекс элемента
     */
    int get_index_whith_check(DataNode &node) {
        int sum_probability = 0;
        for(size_t i = 0; i < node.p_data.size(); ++i) {
            if(node.p_data[i] == 0 || node.p_transition[i].p_data.size() == 0) continue;
            sum_probability += node.p_data[i];
        }

        int p = NormalRand::random_number(0, sum_probability);
        int level_up = 0;
        for(size_t i = 0; i < node.p_data.size(); ++i) {
            if(node.p_data[i] == 0 || node.p_transition[i].p_data.size() == 0) continue;
            int level_dn = level_up; // нижняя граница
            level_up += node.p_data[i];
            if(p < level_up && p >= level_dn) {
                return i;
            }
        }
        return node.p_data.size() - 1;
    }

    /** \brief Генерируем новые данные
     * \param
     * \return
     */
    void generate_data(DataNode &mom, DataNode &node, std::vector<double> &data, int max_data_len, int max_sublen = 4) {
        if(node.p_data.size() > 0) {
            int next_indx = get_index_whith_check(node);
            int rnd_len = 1 + get_index(node.p_len[next_indx]);
            int max_len = node.p_len[next_indx].size();

            double temp = node.step_data + (node.step_data/2.0) * NormalRand::standard_variate() + node.min_data;
            if(temp > node.max_data) {
                temp = node.max_data;
            }
            for(int l = 0; l < rnd_len && data.size() < max_data_len; ++l) {
                data.push_back(temp);
            }
            if(data.size() < max_data_len) {
                if(node.p_transition[next_indx].p_data.size() > 0) {
                    generate_data(mom, node.p_transition[next_indx], data, max_data_len);
                } else
                if(data.size() + (max_len - rnd_len) >= max_data_len) {
                    while(data.size() < max_data_len) {
                        data.push_back(temp);
                    }
                } else
                if(rnd_len < max_sublen && data.size() + (max_sublen - rnd_len) >= max_data_len) {
                    while(data.size() < max_data_len) {
                        data.push_back(temp);
                    }
                } else {
                    // все с начала начинаем
                    data.clear();
                    generate_data(mom, mom, data, max_data_len);
                }
            }
        } else {
             //std::cout << "NULL: " << data.size() << std::endl;
            data.clear();
            generate_data(mom, mom, data, max_data_len);
        }
    }


    double threshold_data = 0.1;

    /** \brief Квантование данных
     * \param in данные на входе
     * \param out данные на выходе
     * \param level шаг квантования данных
     */
    void quantization(std::vector<double> &in, std::vector<double> &out, double step) {
        out.resize(in.size());
        for(size_t i = 0; i < in.size(); ++i) {
            //int temp = in[i] / step;
            out[i] = ((int)(in[i] / step)) * step;
        }
    }

    /** \brief Класс для хранения данных шаблонов
     */
    class UnitPatternData {
        public:
        double data;            /**< Значение узла патерна данных */
        int len;                /**< Длина узла паттерна данных */

        UnitPatternData() {};

        UnitPatternData(double _data, int _len) : data(_data), len(_len) {};
    };

    /** \brief Ищем щаблоны
     * \param in Массив исходных данных
     * \param out Массив обощенных данных
     */
    void find_patterns(std::vector<std::vector<double>> &in, std::vector<std::vector<UnitPatternData>> &out, bool is_use_repeat = false) {
        for(size_t n = 0; n < in.size(); ++n) {
            double x0 = in[n][0];
            std::vector<UnitPatternData> pattern;
            int len = 1;
            for(size_t i = 1; i < in[n].size(); ++i) {
                double x1 = in[n][i];
                if(x1 != x0) {
                    pattern.push_back(UnitPatternData(x0, len));
                    len = 1;
                    x0 = x1;
                } else
                if(i == in[n].size() - 1) {
                    pattern.push_back(UnitPatternData(x0, len));
                } else {
                    len++;
                }
            }

            double diff = 0.0;
            for(size_t j = 0; j < out.size(); ++j) {
                if(out[j].size() == pattern.size()) {
                    for(size_t i = 0; i < pattern.size(); ++i) {
                        diff += std::abs(pattern[i].data - out[j][i].data) + std::abs(pattern[i].len - out[j][i].len);
                    }
                    if(diff == 0) break;
                }
            }
            if(diff == 0.0 && out.size() > 0 && !is_use_repeat) continue;
            out.push_back(pattern);
        }
    }

    int step_train = 0;
    int num_node = 0;
    int last_index = 0;
    void train(std::vector<std::vector<UnitPatternData>> &in, DataNode &pre_item, DataNode &new_item, int index, int item_index) {
        step_train++;
        new_item.step_data = pre_item.step_data; // скопируем шаг
        // обходим все данные
        for(size_t n = 0; n < in.size(); ++n) {
            if(in[n].size() > index) { // если данные доступны
                if(pre_item.get_transition_index(in[n][index - 1].data) == item_index) { // если данные принадлежат нашему узлу
                    // собираем статистику
                    new_item.updata_min_max(in[n][index].data);
                    new_item.updata_probability(in[n][index].data, in[n][index].len);
                }
            }
        }

        if(new_item.p_transition.size() > 0) {
            num_node++;
            // заполним пробелы в статистике
            new_item.smooth_probability();
        } else {
            return;
        }
        //std::cout << "transition " << new_item.p_transition.size() << std::endl;
        // обучим все остальное
        for(int i = 0; i < new_item.p_transition.size(); ++i) {
            train(in, new_item, new_item.p_transition[i], index + 1, i);
        }

    }

    void train(std::vector<std::vector<UnitPatternData>> &in, DataNode &item) {
        //std::cout << "step train (mom) " << step_train++ << std::endl;
        //item.step_data = 1.0; // скопируем шаг
        int index = 0;
        // обходим все данные
        for(size_t n = 0; n < in.size(); ++n) {
            if(in[n].size() > index) { // если данные доступны
                // собираем статистику
                item.updata_min_max(in[n][index].data);
                item.updata_probability(in[n][index].data, in[n][index].len);
            }
        }
        // заполним пробелы в статистике
        item.smooth_probability();
        //std::cout << "transition " << item.p_transition.size() << std::endl;
        // обучим все остальное
        for(int i = 0; i < item.p_transition.size(); ++i) {
            train(in, item, item.p_transition[i], index + 1, i);
        }
    }

    DataSynthesis() {};

};

#endif // DATA_SYNTHESIS_HPP_INCLUDED
