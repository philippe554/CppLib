#include "ArrayBase.h"

/*std::vector<int> test(std::vector<int> index, std::vector<int> broadcastDimension)
{
    for(int i = 0; i < broadcastDimension.size(); i++)
    {
        index[broadcastDimension[i]] = 0;
    }

    return index;
}*/



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

    /*T get(std::vector<int> index)
    {

        return op(left->get(broadcast(index)), right.get(broadcast(index)))
    }*/

    template<class... Int>
    T get(Int... args)
    {
        return op(left->get(args...), right->get(args...));
    }

private:
    std::shared_ptr<ArrayBase<T>> left;
    std::shared_ptr<ArrayBase<T>> right;
    
    std::function<T(T,T)> op;

    std::vector<int> leftBroadcast;
    std::vector<int> rightBroadcast;

};
