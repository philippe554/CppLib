#pragma once

#include <memory>
#include <vector>
#include <algorithm>
#include <functional>

#include "Common.h"
#include "DataObject.h"
#include "Slice.h"

std::vector<int> broadcastShape(std::vector<int> shape1, std::vector<int> shape2)
{
    int size = std::max(shape1.size(), shape2.size());

    std::vector<int> shape(size);

    for(int i = 0; i < size; i++)
    {
        if(i >= shape1.size())
        {
            shape[size - 1 - i] = shape2[size - 1 - i];
        }
        else if(i >= shape2.size())
        {
            shape[size - 1 - i] = shape1[size - 1 - i];
        }
        else
        {
            if(shape1[size - 1 - i] == shape2[size - 1 - i])
            {
                shape[size - 1 - i] = shape1[size - 1 - i];
            }
            else if(shape1[size - 1 - i] == 1)
            {
                shape[size - 1 - i] = shape2[size - 1 - i];
            }
            else if(shape2[size - 1 - i] == 1)
            {
                shape[size - 1 - i] = shape1[size - 1 - i];
            }
            else
            {
                throw std::runtime_error("Broadcast mismatch");
            }
            
        }   
    }

    return shape;
}

template<class T>
class ArrayBase
{
public:
    ArrayBase(std::vector<int> shape)
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

    ArrayBase(ArrayBase<T>& other)
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

    ArrayBase(ArrayBase<T>& other, Slice& slice)
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

    void set(T value)
    {
        for(int i = 0; i < data->size; i++)
        {
            data->buffer[i] = value;
        }
    }

    T& get(std::vector<int> index)
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

    void broadcast1(ArrayBase<T>& other, std::function<void(T&,T&)> op)
    {
        int length1 = slice.ranges.back().getLength();
        int length2 = other.slice.ranges.back().getLength();

        if(length1 == length2)
        {
            broadcast2(other, slice.ranges.back().start, 
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
            broadcast2(other, slice.ranges.back().start, 
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
            broadcast2(other, slice.ranges.back().start, 
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

    void broadcast2(ArrayBase<T>& other, int position1, int position2, int stride1, int stride2, int steps, int multiplier1, int multiplier2, int dim1, int dim2, std::function<void(T&,T&)> op)
    {
        if(dim1 == physicalShape.size() && dim2 == other.physicalShape.size())
        {
            for(int i = 0; i < steps; i++)
            {
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
                broadcast2(other, position1, 
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
                broadcast2(other, position1 + (slice.ranges[slice.ranges.size() - 1 - dim1].start + i) * multiplier1, 
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
                broadcast2(other, position1 + (slice.ranges[slice.ranges.size() - 1 - dim1].start + i) * multiplier1, 
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
                broadcast2(other, position1 + slice.ranges[slice.ranges.size() - 1 - dim1].start * multiplier1, 
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
                broadcast2(other, position1 + (slice.ranges[slice.ranges.size() - 1 - dim1].start + i) * multiplier1, 
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

private:
    
    std::shared_ptr<DataObject<T>> data;

    std::vector<int> physicalShape;
    int size;

    Slice slice;

    std::vector<int> shape;
};