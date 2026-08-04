#pragma once

class RefCounted
{
public:
    virtual ~RefCounted() = default;

    U32 GetRefCount() const { return m_RefCount; }

private:
    void IncRefCount() { m_RefCount++; }
    void DecRefCount() { m_RefCount--; }

    U32 m_RefCount = 0;

    template<typename T>
    friend class Ref;
};

template<typename T>
class Ref final
{
public:
    Ref() = default;
    explicit Ref(std::nullptr_t) {}
    ~Ref()
    {
        if (m_Data)
        {
            m_Data->DecRefCount();
            if (m_Data->GetRefCount() == 0)
                delete m_Data;
        }
    }

    Ref(Ref&& rhs) noexcept
    {
        m_Data = rhs.m_Data;
        rhs.m_Data = nullptr;
    }

    Ref& operator=(Ref&& rhs) noexcept
    {
        if (&rhs == this)
            return *this;

        if (m_Data)
        {
            m_Data->DecRefCount();
            if (m_Data->GetRefCount() == 0)
                delete m_Data;
        }

        m_Data = rhs.m_Data;
        rhs.m_Data = nullptr;

        return *this;
    }

    Ref(const Ref& other)
    {
        m_Data = other.m_Data;

        if (m_Data)
            m_Data->IncRefCount();
    }

    Ref& operator=(const Ref& other)
    {
        if (&other == this)
            return *this;

        if (m_Data)
        {
            m_Data->DecRefCount();
            if (m_Data->GetRefCount() == 0)
                delete m_Data;
        }

        m_Data = other.m_Data;

        if (m_Data)
            m_Data->IncRefCount();

        return *this;
    }

    template<typename U>
        requires std::is_convertible_v<U*, T*>
    explicit Ref(Ref<U>&& rhs)
    {
        m_Data = rhs.m_Data;
        rhs.m_Data = nullptr;
    }

    template<typename U>
        requires std::is_convertible_v<U*, T*>
    Ref& operator=(Ref<U>&& rhs)
    {
        if (&rhs == this)
            return *this;

        if (m_Data)
        {
            m_Data->DecRefCount();
            if (m_Data->GetRefCount() == 0)
                delete m_Data;
        }

        m_Data = rhs.m_Data;
        rhs.m_Data = nullptr;

        return *this;
    }

    template<typename U>
        requires std::is_convertible_v<U*, T*>
    explicit Ref(const Ref<U>& other)
    {
        m_Data = other.m_Data;
        if (m_Data)
            m_Data->IncRefCount();
    }

    template<typename U>
        requires std::is_convertible_v<U*, T*>
    Ref& operator=(const Ref<U>& other)
    {
        if (&other == this)
            return *this;

        if (m_Data)
        {
            m_Data->DecRefCount();
            if (m_Data->GetRefCount() == 0)
                delete m_Data;
        }

        m_Data = other.m_Data;

        if (m_Data)
            m_Data->IncRefCount();

        return *this;
    }

    template<typename... Args>
    static Ref Create(Args&&... args)
    {
        return Ref(new T(std::forward<Args>(args)...));
    }

    T* Raw() { return m_Data; }

    const T* operator->() const { return m_Data; }
    T* operator->() { return m_Data; }
    const T& operator*() const { return *m_Data; }
    T& operator*() { return *m_Data; }

private:
    explicit Ref(T* data)
    {
        m_Data = data;
        if (m_Data)
            m_Data->IncRefCount();
    }

    T* m_Data = nullptr;

    template<typename U>
    friend class Ref;
};
