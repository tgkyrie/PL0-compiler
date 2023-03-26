#pragma once
#include<vector>
class ArrayInf
{
private:
    /* data */
	// int dim_;
	// int* array;
    std::vector<int> dims_;
public:
    ArrayInf(/* args */){}
    ~ArrayInf(){}
    void push_back(int dim){
        dims_.push_back(dim);
    }
    int dim()const {return dims_.size();}
    int limit(int ndim){
        int ret=1;
        for(int i=ndim+1;i<dims_.size();i++){
            ret*=dims_[i];
        }
        return ret;
    }
};

