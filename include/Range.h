
class Range
{
public:
    Range(int start, int end)
    {
        this->start = start;
        this->end = end;
        this->stride = stride;

        hidden = false;
    }

    int getLength()
    {
        return end - start;
    }

    int start;
    int end;
    int stride;

    bool hidden;
};
