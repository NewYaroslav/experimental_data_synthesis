#include <iostream>
#include "data_synthesis.hpp"
#include "banana_filesystem.hpp"

void open_file(std::string file_name, std::vector<double> &data);
bool save_file(std::string file_name, std::vector<double> &data);

int main()
{

    std::vector<std::vector<double>> data;
    std::vector<std::string> list_file;
    bf::get_list_files("..\\..\\train\\rama_1", list_file, true);

    for(size_t i = 0; i < list_file.size(); ++i) {
        std::vector<double> temp;
        open_file(list_file[i], temp);
        if(temp.size() > 0) {
            data.push_back(temp);
        }
    }

    DataSynthesis iDataSynthesis;
    std::cout << "data size: " << data.size()<< std::endl;
    for(size_t i = 0; i < data.size(); ++i) {
        iDataSynthesis.quantization(data[i],data[i],0.001);
    }

    std::vector<std::vector<DataSynthesis::UnitPatternData>> patterns;
    iDataSynthesis.find_patterns(data, patterns, true);

    std::cout << "patterns size: " << patterns.size() << std::endl;

    for(size_t n = 0; n < patterns.size(); ++n) {
        std::cout << "pattern " << n << " size " << patterns[n].size() << std::endl;
        for(size_t i = 0; i < patterns[n].size(); ++i) {
            std::cout << patterns[n][i].data << "|" << patterns[n][i].len << " ";
        }
        std::cout << std::endl;
    }
    // найдем вероятность первого узла
    DataSynthesis::DataNode iDataNode;
    iDataNode.step_data = 0.1;
    iDataSynthesis.train(patterns, iDataNode);

    std::cout << "stop train" << std::endl;

    for(size_t  n = 0; n < 1000; ++n) {
        std::vector<double> data_test;
        iDataSynthesis.generate_data(iDataNode, iDataNode, data_test, 30);
        save_file("test\\test_" + std::to_string(n) + ".dat", data_test);

        //std::cout << "data_test" << std::endl;
        //for(size_t  i = 0; i < data_test.size(); ++i) {
        //    std::cout << data_test[i] << std::endl;
        //}
    }

    return 0;
}

void open_file(std::string file_name, std::vector<double> &data) {
    std::ifstream file(file_name);
    if(!file.is_open()) {
        return;
    }
    while(!file.eof()) {
        std::string word;
        file >> word;
        if(word.size() > 0 && std::isdigit(word[0])) {
            double temp = std::atof(word.c_str());
            data.push_back(temp);
        }
    }
    file.close();
}

bool save_file(std::string file_name, std::vector<double> &data) {
    std::ofstream file(file_name);
    if(!file.is_open()) {
        std::cout << "error: can not open file: " << file_name << std::endl;
        return false;
    }
    if(data.size() == 0) {
        std::cout << "error: no data" << std::endl;
        return false;
    }
    for(size_t n = 0; n < data.size(); ++n) {
        file << std::to_string(data[n]) << std::endl;
    }
    file.close();
    return true;
}
