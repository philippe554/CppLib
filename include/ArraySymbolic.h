#include "ArrayBase.h"

template<class T>
class ArraySymbolic : ArrayBase<T>
{
public:
    ArraySymbolic(std::shared_ptr<ArrayBase<T>> left, std::shared_ptr<ArrayBase<T>> right, std::function<T(T,T)> op)
    {
        this->left = left;
        this->right = right;
        this->op = op;
    }

    

private:
    std::shared_ptr<ArrayBase<T>> left;
    std::shared_ptr<ArrayBase<T>> right;
    
    std::function<T(T,T)> op;

};
