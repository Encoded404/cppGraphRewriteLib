#ifndef GRAPHLIBRARY_SPARSEVECTOR_HPP
#define GRAPHLIBRARY_SPARSEVECTOR_HPP

#include <vector>

namespace GraphLibrary
{
    template<typename T>
    class SparseVector
    {
    public:
        SparseVector() = default;
        ~SparseVector() = default;

        size_t Add(const T& element)
        {
            if (freeIndices.size() > 0)
            {
                size_t index = freeIndices.back();
                freeIndices.pop_back();
                data[index] = element;
                return index;
            }
            else
            {
                data.push_back(element);
                return data.size() - 1;
            }
        }

        void Remove(size_t index)
        {
            data[index] = nullptr;
            freeIndices.push_back(index);
        }

        T& Get(size_t index)
        {
            return data[index];
        }

        T& Pop(size_t index)
        {
            T element = data[index];
            Remove(index);
            return element;
        }

        void Clear()
        {
            data.clear();
            freeIndices.clear();
        }

    private:
        std::vector<T> data;
        std::vector<size_t> freeIndices;
    };
} // namespace GraphLibrary


#endif // GRAPHLIBRARY_SPARSEVECTOR_HPP