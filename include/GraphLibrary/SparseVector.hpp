#ifndef GRAPHLIBRARY_SPARSEVECTOR_HPP
#define GRAPHLIBRARY_SPARSEVECTOR_HPP

#include <cstddef>
#include <vector>

namespace GraphLibrary
{
    template<typename T>
    class SparseVector
    {
    public:
        SparseVector() = default;
        ~SparseVector() = default;
        
        /// @brief Adds an element to the sparse vector.
        /// @param element The element to add.
        /// @return The index at which the element was added.
        size_t Add(const T& element)
        {
            if (freeIndices_.size() > 0)
            {
                size_t index = freeIndices_.back();
                freeIndices_.pop_back();
                data_[index] = element;
                return index;
            }
            else
            {
                data_.push_back(element);
                return data_.size() - 1;
            }
        }

        /// @brief Removes an element from the sparse vector at the given index.
        /// @param index The index of the element to remove.
        void Remove(size_t index)
        {
            data_[index] = nullptr;
            freeIndices_.push_back(index);
        }

        /// @brief Retrieves a reference to the element at the given index.
        /// @param index The index of the element to retrieve.
        /// @return A reference to the element at the specified index.
        T& Get(size_t index)
        {
            return data_[index];
        }
        
        /// @brief Removes and returns the element at the given index.
        /// @param index The index of the element to remove and return.
        /// @return The element that was removed.
        T Pop(size_t index)
        {
            T element = data_[index];
            Remove(index);
            return element;
        }

        /// @brief Clears the sparse vector.
        void Clear()
        {
            data_.clear();
            freeIndices_.clear();
        }

        /// @brief Converts the sparse vector to a standard array.
        /// @return A standard array containing all elements in the sparse vector.
        std::vector<T> ToArray() const
        {
            return data_;
        }

    private:
        std::vector<T> data_;
        std::vector<size_t> freeIndices_;
    };
} // namespace GraphLibrary


#endif // GRAPHLIBRARY_SPARSEVECTOR_HPP