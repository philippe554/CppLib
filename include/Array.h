#include <memory>
#include <vector>
#include <functional>

#include "DataObject.h"
#include "Slice.h"

std::string str(std::vector<int> vec)
{
    std::string s = "(";

    for(int i = 0; i < vec.size(); i++)
    {
        s += std::to_string(vec[i]);
        if(i != vec.size() - 1)
        {
            s += ",";
        }
    }
    s += ")";

    return s;
}

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
    : slice(shape)
    {
        physicalShape = shape;

        size = 1;
        for(int i = 0; i < shape.size(); i++)
        {
            size *= shape[i];
        }

        data = std::make_shared<DataObject<T>>(size);

        for(int i = 0; i < slice.ranges.size(); i++)
        {
            if(!slice.ranges[i].hidden)
            {
                this->shape.push_back(slice.ranges[i].getLength());
            }
        }
    }

    Array(Array<T>& other)
    : slice(other.slice)
    {
        physicalShape = other.physicalShape;
        size = other.size;

        data = other.data;

        for(int i = 0; i < slice.ranges.size(); i++)
        {
            if(!slice.ranges[i].hidden)
            {
                shape.push_back(slice.ranges[i].getLength());
            }
        }
    }

    Array(Array<T>& other, Slice& slice)
    : slice(other.slice, slice)
    {
        physicalShape = other.physicalShape;
        size = other.size;

        data = other.data;

        for(int i = 0; i < this->slice.ranges.size(); i++)
        {
            if(!this->slice.ranges[i].hidden)
            {
                shape.push_back(this->slice.ranges[i].getLength());
            }
        }
    }

    T& operator[](std::vector<int> index)
    {
        if(index.size() != getShape().size())
        {
            throw std::runtime_error(std::string("Rank error: ") + str(index) + " in " + str(getShape()));
        }

        int position = 0;
        int multiplier = 1;
        int length = physicalShape.size();
        int i2 = index.size() - 1;
        for(int i = length - 1; i >= 0; i--)
        {
            if(slice.ranges[i].hidden)
            {
                position += slice.ranges[i].start * multiplier;
                multiplier *= physicalShape[i];
            }
            else
            {
                if(slice.ranges[i].start + index[i2] >= slice.ranges[i].end)
                {
                    throw std::runtime_error("Index error: dimension error");
                }

                position += (slice.ranges[i].start + index[i2]) * multiplier;
                multiplier *= physicalShape[i];

                i2--;
            }
        }

        return data->buffer[position];
    }

    Array<T> operator[](Slice& slice)
    {
        return Array(*this, slice);
    }

    void operator=(Array<T>&& other)
    {
        std::cout << str(shape) << " " << str(other.shape) << "\n";

        /*std::vector<int> index1(shape.size());
        std::vector<int> index2(other.shape.size());

        run(other, index1, index2, 0, 0, [](T& t1, T& t2){t1 = t2;});*/
        //run2(other, 0, 0, 1, 1, 0, 0, [](T& t1, T& t2){t1 = t2;});
        run31(other, [](T& t1, T& t2){t1 = t2;});
    }

    void operator+=(Array<T>&& other)
    {
        //std::cout << str(shape) << " " << str(other.shape) << "\n";

        /*std::vector<int> index1(shape.size());
        std::vector<int> index2(other.shape.size());

        run(other, index1, index2, 0, 0, [](T& t1, T& t2){t1 += t2;});*/
        //run2(other, 0, 0, 1, 1, 0, 0, [](T& t1, T& t2){t1 += t2;});
        run31(other, [](T& t1, T& t2){t1 += t2;});
    } 

    void run(Array<T>& other, std::vector<int>& index1, std::vector<int>& index2, int size1, int size2, std::function<void(T&,T&)> op)
    {
        if(size1 == getShape().size() && size2 == other.getShape().size())
        {
            //std::cout << str(index1) << " " << str(index2) << "\n";
            op((*this)[index1], other[index2]);
            return;
        }

        if(size1 > getShape().size() || size2 > other.getShape().size())
        {
            throw std::runtime_error("Implementation error");
        }

        int length1 = 0;
        if(size1 < getShape().size())
        {
            length1 = getShape()[getShape().size() - 1 - size1];
        }

        int length2 = 0;
        if(size2 < other.getShape().size())
        {
            length2 = other.getShape()[other.getShape().size() - 1 - size2];
        }

        if(length1 == 0 && length2 == 0)
        {
            throw std::runtime_error("Implementation error");
        }

        if(length1 == length2)
        {
            for(int i = 0; i < length1; i++)
            {
                index1[shape.size() - size1 - 1] = i;
                index2[other.shape.size() - size2 - 1] = i;
                run(other, index1, index2, size1 + 1, size2 + 1, op);
            }
        }
        else if(length1 == 0)
        {
            for(int i = 0; i < length2; i++)
            {
                index2[other.shape.size() - size2 - 1] = i;
                run(other, index1, index2, size1, size2 + 1, op);
            }
        }
        else if(length2 == 0)
        {
            for(int i = 0; i < length1; i++)
            {
                index1[shape.size() - size1 - 1] = i;
                run(other, index1, index2, size1 + 1, size2, op);
            }
        }
        else if(length1 == 1)
        {
            for(int i = 0; i < length2; i++)
            {
                index1[shape.size() - size1 - 1] = 0;
                index2[other.shape.size() - size2 - 1] = i;
                run(other, index1, index2, size1 + 1, size2 + 1, op);
            }
        }
        else if(length2 == 1)
        {
            for(int i = 0; i < length1; i++)
            {
                index1[shape.size() - size1 - 1] = i;
                index2[other.shape.size() - size2 - 1] = 0;
                run(other, index1, index2, size1 + 1, size2 + 1, op);
            }
        }
        else
        {
            throw std::runtime_error("Index error: broadcast error");
        }
    }

    void run2(Array<T>& other, int position1, int position2, int multiplier1, int multiplier2, int dim1, int dim2, std::function<void(T&,T&)> op)
    {
        if(dim1 == physicalShape.size() && dim2 == other.physicalShape.size())
        {
            std::cout << position1 << " ";
            op(data->buffer[position1], other.data->buffer[position2]);
            return;
        }

        if(dim1 > physicalShape.size() || dim2 > other.physicalShape.size())
        {
            throw std::runtime_error("Implementation error");
        }

        if(dim1 == physicalShape.size())
        {
            int length2 = other.slice.ranges[other.slice.ranges.size() - 1 - dim2].getLength();

            for(int i = 0; i < length2; i++)
            {
                run2(other, position1, 
                            position2 + (other.slice.ranges[other.slice.ranges.size() - 1 - dim2].start + i) * multiplier2, 
                            multiplier1,
                            multiplier2 * other.physicalShape[other.physicalShape.size() - 1 - dim2],
                            dim1,
                            dim2 + 1,
                            op);
            }
            return;
        }

        if(dim2 == other.physicalShape.size())
        {
            int length1 = slice.ranges[slice.ranges.size() - 1 - dim1].getLength();

            for(int i = 0; i < length1; i++)
            {
                run2(other, position1 + (slice.ranges[slice.ranges.size() - 1 - dim1].start + i) * multiplier1, 
                            position2, 
                            multiplier1 * physicalShape[physicalShape.size() - 1 - dim1],
                            multiplier2,
                            dim1 + 1,
                            dim2,
                            op);
            }
            return;
        }

        int length1 = slice.ranges[slice.ranges.size() - 1 - dim1].getLength();
        int length2 = other.slice.ranges[other.slice.ranges.size() - 1 - dim2].getLength();

        if(length1 == length2)
        {
            for(int i = 0; i < length1; i++)
            {
                run2(other, position1 + (slice.ranges[slice.ranges.size() - 1 - dim1].start + i) * multiplier1, 
                            position2 + (other.slice.ranges[other.slice.ranges.size() - 1 - dim2].start + i) * multiplier2, 
                            multiplier1 * physicalShape[physicalShape.size() - 1 - dim1],
                            multiplier2 * other.physicalShape[other.physicalShape.size() - 1 - dim2],
                            dim1 + 1,
                            dim2 + 1,
                            op);
            }
        }
        else if(length1 == 1 && length2 >= 1)
        {
            for(int i = 0; i < length2; i++)
            {
                run2(other, position1 + slice.ranges[slice.ranges.size() - 1 - dim1].start * multiplier1, 
                            position2 + (other.slice.ranges[other.slice.ranges.size() - 1 - dim2].start + i) * multiplier2, 
                            multiplier1 * physicalShape[physicalShape.size() - 1 - dim1],
                            multiplier2 * other.physicalShape[other.physicalShape.size() - 1 - dim2],
                            dim1 + 1,
                            dim2 + 1,
                            op);
            }
        }
        else if(length2 == 1 && length1 >= 1)
        {
            for(int i = 0; i < length1; i++)
            {
                run2(other, position1 + (slice.ranges[slice.ranges.size() - 1 - dim1].start + i) * multiplier1, 
                            position2 + other.slice.ranges[other.slice.ranges.size() - 1 - dim2].start * multiplier2, 
                            multiplier1 * physicalShape[physicalShape.size() - 1 - dim1],
                            multiplier2 * other.physicalShape[other.physicalShape.size() - 1 - dim2],
                            dim1 + 1,
                            dim2 + 1,
                            op);
            }
        }
        else
        {
            std::cout << length1 << " " << length2 << " " << dim1 << " " << dim2 << " " << physicalShape.size() << " " << other.physicalShape.size() << "\n"; 
            throw std::runtime_error("Not yet implemented");
        }
    }

    void run31(Array<T>& other, std::function<void(T&,T&)> op)
    {
        int length1 = slice.ranges.back().getLength();
        int length2 = other.slice.ranges.back().getLength();

        if(length1 == length2)
        {
            run32(other, slice.ranges.back().start, 
                        other.slice.ranges.back().start, 
                        1, 1, length1,
                        physicalShape.back(),
                        other.physicalShape.back(),
                        1,
                        1,
                        op);
        }
        else if(length1 == 1 && length2 >= 1)
        {
            run32(other, slice.ranges.back().start, 
                        other.slice.ranges.back().start,
                        0, 1, length2,
                        physicalShape.back(),
                        other.physicalShape.back(),
                        1,
                        1,
                        op);
        }
        else if(length2 == 1 && length1 >= 1)
        {
            run32(other, slice.ranges.back().start, 
                        other.slice.ranges.back().start,
                        1, 0, length1,
                        physicalShape.back(),
                        other.physicalShape.back(),
                        1,
                        1,
                        op);
        }
        else
        {
            throw std::runtime_error("Dimension mismatch");
        }
    }

    void run32(Array<T>& other, int position1, int position2, int stride1, int stride2, int steps, int multiplier1, int multiplier2, int dim1, int dim2, std::function<void(T&,T&)> op)
    {
        if(dim1 == physicalShape.size() && dim2 == other.physicalShape.size())
        {
            for(int i = 0; i < steps; i++)
            {
                //std::cout << position1 + i * stride1 << " ";
                op(data->buffer[position1 + i * stride1], other.data->buffer[position2 + i * stride2]);
            }
            return;
        }

        if(dim1 > physicalShape.size() || dim2 > other.physicalShape.size())
        {
            throw std::runtime_error("Implementation error");
        }

        if(dim1 == physicalShape.size())
        {
            int length2 = other.slice.ranges[other.slice.ranges.size() - 1 - dim2].getLength();

            for(int i = 0; i < length2; i++)
            {
                run32(other, position1, 
                            position2 + (other.slice.ranges[other.slice.ranges.size() - 1 - dim2].start + i) * multiplier2,
                            stride1, stride2, steps, 
                            multiplier1,
                            multiplier2 * other.physicalShape[other.physicalShape.size() - 1 - dim2],
                            dim1,
                            dim2 + 1,
                            op);
            }
            return;
        }

        if(dim2 == other.physicalShape.size())
        {
            int length1 = slice.ranges[slice.ranges.size() - 1 - dim1].getLength();

            for(int i = 0; i < length1; i++)
            {
                run32(other, position1 + (slice.ranges[slice.ranges.size() - 1 - dim1].start + i) * multiplier1, 
                            position2, 
                            stride1, stride2, steps,
                            multiplier1 * physicalShape[physicalShape.size() - 1 - dim1],
                            multiplier2,
                            dim1 + 1,
                            dim2,
                            op);
            }
            return;
        }

        int length1 = slice.ranges[slice.ranges.size() - 1 - dim1].getLength();
        int length2 = other.slice.ranges[other.slice.ranges.size() - 1 - dim2].getLength();

        if(length1 == length2)
        {
            for(int i = 0; i < length1; i++)
            {
                run32(other, position1 + (slice.ranges[slice.ranges.size() - 1 - dim1].start + i) * multiplier1, 
                            position2 + (other.slice.ranges[other.slice.ranges.size() - 1 - dim2].start + i) * multiplier2,
                            stride1, stride2, steps, 
                            multiplier1 * physicalShape[physicalShape.size() - 1 - dim1],
                            multiplier2 * other.physicalShape[other.physicalShape.size() - 1 - dim2],
                            dim1 + 1,
                            dim2 + 1,
                            op);
            }
        }
        else if(length1 == 1 && length2 >= 1)
        {
            for(int i = 0; i < length2; i++)
            {
                run32(other, position1 + slice.ranges[slice.ranges.size() - 1 - dim1].start * multiplier1, 
                            position2 + (other.slice.ranges[other.slice.ranges.size() - 1 - dim2].start + i) * multiplier2,
                            stride1, stride2, steps, 
                            multiplier1 * physicalShape[physicalShape.size() - 1 - dim1],
                            multiplier2 * other.physicalShape[other.physicalShape.size() - 1 - dim2],
                            dim1 + 1,
                            dim2 + 1,
                            op);
            }
        }
        else if(length2 == 1 && length1 >= 1)
        {
            for(int i = 0; i < length1; i++)
            {
                run32(other, position1 + (slice.ranges[slice.ranges.size() - 1 - dim1].start + i) * multiplier1, 
                            position2 + other.slice.ranges[other.slice.ranges.size() - 1 - dim2].start * multiplier2,
                            stride1, stride2, steps, 
                            multiplier1 * physicalShape[physicalShape.size() - 1 - dim1],
                            multiplier2 * other.physicalShape[other.physicalShape.size() - 1 - dim2],
                            dim1 + 1,
                            dim2 + 1,
                            op);
            }
        }
        else
        {
            std::cout << length1 << " " << length2 << " " << dim1 << " " << dim2 << " " << physicalShape.size() << " " << other.physicalShape.size() << "\n"; 
            throw std::runtime_error("Not yet implemented");
        }
    }

    const std::vector<int>& getShape()
    {
        return shape;
    }

    friend Array<T> zeros<T>(std::vector<int> shape);

    friend Array<T> ones<T>(std::vector<int> shape);

private:
    
    std::shared_ptr<DataObject<T>> data;

    std::vector<int> physicalShape;
    int size;

    Slice slice;

    std::vector<int> shape;
};

template<class T>
Array<T> zeros(std::vector<int> shape)
{
    Array<T> array(shape);
    
    for(int i = 0; i < array.size; i++)
    {
        array.data->buffer[i] = 0;
    }

    return array;
}

template<class T>
Array<T> ones(std::vector<int> shape)
{
    Array<T> array(shape);
    
    for(int i = 0; i < array.size; i++)
    {
        array.data->buffer[i] = 1;
    }

    return array;
}

