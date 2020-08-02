
template<class T>
class DataObject
{
public:
    DataObject(int size)
    {
        buffer = new T[size];
    }

    ~DataObject()
    {
        delete[] buffer;
    }

    T* buffer;
};
