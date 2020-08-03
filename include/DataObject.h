
template<class T>
class DataObject
{
public:
    DataObject(int size)
    {
        buffer = new T[size];
        this->size = size;
    }

    ~DataObject()
    {
        delete[] buffer;
    }

    T* buffer;
    int size;
};
