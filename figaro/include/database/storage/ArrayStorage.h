#ifndef _FIGARO_ARRAY_STORAGE_H_
#define _FIGARO_ARRAY_STORAGE_H_

namespace Figaro 
{
    template <typename T>
    class ArrayStorage
    {
        uint32_t m_size = 0; 
        T* m_data = nullptr;
        void destroyData(void)
        {
            if (nullptr != m_data)
            {
                delete [] m_data;
                m_data = nullptr;
            }
        }
    public:

        ArrayStorage(uint32_t size): m_size(size) 
        {
            if (size > 0)
            {
                m_data = new T[size];
            }
        } 

        T& operator[](uint32_t idx) 
        {
            FIGARO_LOG_ASSERT(idx < m_size);
            return m_data[idx];
        }

        const T& operator[](uint32_t idx) const
        {
            FIGARO_LOG_ASSERT(idx < m_size);
            return m_data[idx];
        }

        ~ArrayStorage()
        {
            destroyData();
        }

        uint32_t getSize(void) const
        {
            return m_size;
        }

        T* getData(void)
        {
            return m_data;
        }

        void resize(uint32_t newSize)
        {       
            T* newData = nullptr;
            if (newSize > 0)
            {
                newData = new T[newSize];
            }

            if ((m_size > 0) && (newSize > 0))
            {
                memcpy(newData, m_data, sizeof(T) * std::min(newSize, m_size));
            }

            destroyData();
            m_size = newSize;
            m_data = newData;
        }
    };
}

#endif