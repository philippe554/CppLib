#include <memory>
#include <vector>
#include <functional>

#include "ArrayBase.h"
#include "ArraySymbolic.h"

template<class T>
class Array;

template<class T>
Array<T> zeros(std::vector<int> shape);

template<class T>
Array<T> ones(std::vector<int> shape);

template<class T>
class Array
{
public:
    Array(std::vector<int> shape)
    {
        array = std::make_shared<ArrayBase<T>>(shape);
    }

    Array(Array<T>& other)
    {
        array = other.array;
    }

    Array(Array<T>& other, Slice& slice)
    {
        array = std::make_shared<ArrayBase<T>>(*other.array, slice);
    }

    void operator=(Array<T>&& other)
    {
        array->broadcast1(*other.array, [](T& t1, T& t2){t1 = t2;});
    }

    void operator+=(Array<T>&& other)
    {
        array->broadcast1(*other.array, [](T& t1, T& t2){t1 += t2;});
    } 

    T& operator[](std::vector<int> index)
    {
        return array->get(index);
    }

    Array<T> operator[](Slice& slice)
    {
        return Array(*this, slice);
    }

    template<class... Int>
    T get(Int... args)
    {
        return 0;
    }

    //friend Array<T> zeros(std::vector<int> shape);
    //friend Array<T> ones(std::vector<int> shape);

//private:
    std::shared_ptr<ArrayBase<T>> array;
};

template<class T>
Array<T> zeros(std::vector<int> shape)
{
    Array<T> array(shape);
    
    array.array->set(0);

    return array;
}

template<class T>
Array<T> ones(std::vector<int> shape)
{
    Array<T> array(shape);
    
    array.array->set(1);

    return array;
}

