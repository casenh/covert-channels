#pragma once
#include <vector>
#include "../../dlib-18.2/dlib/clustering.h"

typedef dlib::matrix<double,1,1> sample_type;
extern void analyzeData(FILE* rawData, FILE* clusteredResults, std::vector<sample_type> &samples, long long int* data, int numSamples);
