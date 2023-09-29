#pragma once
#ifdef YUNUD2DGRAPHICCORE_EXPORTS
#define YUNUD2DGRAPHICCORE_API __declspec(dllexport)
#else
#define YUNUD2DGRAPHICCORE_API __declspec(dllimport)
#endif

template <typename T>
class SingletonClass abstract
{
protected:
    SingletonClass()
    {
             
    }
    virtual ~SingletonClass()
    {
 
    }
 
public:
    static T* GetInstance()
    {
        if (m_pInstance == nullptr)
            m_pInstance = new T;
        return m_pInstance;
    };
 
    static void DestroyInstance()
    {
        if (m_pInstance)
        {
            delete m_pInstance;
            m_pInstance = nullptr;
        }
    };
private:
    static T * m_pInstance;
};
 
template <typename T>
T * SingletonClass<T>::m_pInstance = 0;
