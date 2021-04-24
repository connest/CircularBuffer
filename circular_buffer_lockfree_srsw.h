#ifndef CIRCULAR_BUFFER_LOCKFREE_SRSW_H
#define CIRCULAR_BUFFER_LOCKFREE_SRSW_H

#include <vector>
#include <atomic>
#include <stdexcept>
#include <type_traits>

namespace connest {

/**
  @brief Кольцевой lockfree буфер single reader - single writter
  @details

                R
                |
                V
  -----------------------------------------------------
  |   |   |   |XXX|XXX|XXX|XXX|XXX|XXX|XXX|   |   |RES|
  -----------------------------------------------------
  |                                     ^         |
  0                                     |         Size
                                        W
  R - Позиция чтения
  W - Позиция записи

  R <= W (В кольцевом отсчете)

  RES - зарезервированный элемент, чтобы отличать
        состояния "пуст" и "заполнен"

  R     --> W     - полезные данные
  W + 1 --> R - 1 - свободное место
 */
template <typename T>
class CircularBuffer_srsw
{
    static_assert ( std::is_default_constructible<T>::value,
                    "Type T must be default constructible: empty buffer should "
                    "have initialized elements");

    std::vector<T> m_data;
    std::atomic<size_t> m_head;
    std::atomic<size_t> m_tail;

public:
    struct const_iterator;

    struct iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type   = size_t;
        using value_type        = T;
        using pointer           = value_type*;
        using reference         = value_type&;

        iterator(size_t index, CircularBuffer_srsw<T>& container);
        iterator(const iterator&) = default;
        iterator(iterator&&) = default;
        ~iterator() = default;
        iterator& operator++();
        iterator& operator--();
        reference operator*();
        difference_type operator-(const iterator& other) const;
        iterator operator-(size_t value) const;
        iterator operator+(size_t value) const;
        iterator& operator=(const iterator& other);

        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;
        bool operator<(const iterator& other) const;
        bool operator<=(const iterator& other) const;
        bool operator>(const iterator& other) const;
        bool operator>=(const iterator& other) const;

        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
        bool operator<(const const_iterator& other) const;
        bool operator<=(const const_iterator& other) const;
        bool operator>(const const_iterator& other) const;
        bool operator>=(const const_iterator& other) const;

        friend struct const_iterator;
    private:
        size_t m_index;
        CircularBuffer_srsw<T>& m_container;
    };

    struct const_iterator
    {
        using iterator_category = std::random_access_iterator_tag;
        using difference_type   = size_t;
        using value_type        = const T;
        using pointer           = const value_type*;
        using reference         = const value_type&;
        const_iterator(size_t index, const CircularBuffer_srsw<T>& container);
        const_iterator(const const_iterator&) = default;
        const_iterator(const_iterator&&) = default;
        ~const_iterator() = default;
        const_iterator& operator++();
        const_iterator& operator--();
        reference operator*() const;
        difference_type operator-(const const_iterator& other) const;
        const_iterator operator-(size_t value) const;
        const_iterator operator+(size_t value) const;
        const_iterator& operator=(const const_iterator& other);

        bool operator==(const const_iterator& other) const;
        bool operator!=(const const_iterator& other) const;
        bool operator<(const const_iterator& other) const;
        bool operator<=(const const_iterator& other) const;
        bool operator>(const const_iterator& other) const;
        bool operator>=(const const_iterator& other) const;

        bool operator==(const iterator& other) const;
        bool operator!=(const iterator& other) const;
        bool operator<(const iterator& other) const;
        bool operator<=(const iterator& other) const;
        bool operator>(const iterator& other) const;
        bool operator>=(const iterator& other) const;

        friend struct iterator;
    private:
        size_t m_index;
        const CircularBuffer_srsw<T>& m_container;
    };


    CircularBuffer_srsw(size_t size);
    ~CircularBuffer_srsw() = default;

    CircularBuffer_srsw(const CircularBuffer_srsw& other);

    CircularBuffer_srsw(CircularBuffer_srsw&& other);

    template<typename ForwardInputIterator>
    CircularBuffer_srsw(ForwardInputIterator begin, ForwardInputIterator end);

    /**
     * @brief Определить пуст ли буфер
     * @return флаг пустоты буфера
     */
    bool empty() const noexcept;

    /**
     * @brief Очистить буфер
     */
    void clear() noexcept;

    /**
     * @brief Определить полон ли буфер
     * @return флаг полноты буфера
     */
    bool full() const noexcept;

    /**
     * @brief Получить количество элементов в буфере
     * @return количетсво элементов
     */
    size_t size() const noexcept;

    /**
     * @brief Получить размер буфера
     * @return маскимальное количество элементов в буфере
     */
    size_t max_size() const noexcept;

    /**
     * @brief Получить доступ к элементу буфера без изменения его состояния
     * @param pos индекс элемента
     * @return ссылка на элемент
     */
    T& at(size_t pos);

    /**
     * @brief Получить доступ к элементу буфера без изменения его состояния
     * @param pos индекс элемента
     * @return ссылка на элемент
     */
    const T& at(size_t pos) const;

    /**
     * @brief Добавить элемент в конец буфера
     * @param value значение элемента
     * @return флаг успешности добавления (буфер может быть заполнен)
     */
    template<typename Type>
    bool try_push_back(Type&& value);

    /**
     * @brief Создать и добавить элемент в конец буфера
     * @param args параметры конструктора типа T
     * @return флаг успешности добавления (буфер может быть заполнен)
     */
    template<typename ... Args>
    bool try_emplace_back(Args&& ... args);

    /**
     * @brief Получить очередной элемент буфера
     * @param result ссылка, куда должко быть положено значение
     * @return флаг успешности добавления (буфер может быть пуст)
     */
    bool try_pop(T& result);

    /**
     * @brief Добавить элементы из диапазона контейнера в буфер
     * @param begin итератор начала диапазона контейнера
     * @param end   итератор конца диапазона контейнера
     * @return количество записанных элементов
     */
    template<typename ForwardInputIterator>
    size_t push_back_all(ForwardInputIterator begin, ForwardInputIterator end);

    /**
     * @brief Получить все доступные элементы в контейнер
     * @param container контейнер назначения
     * @return количество полученных элементов
     */
    template<typename ContainerType>
    size_t pop_all(ContainerType& container);

    /**
     * @brief Получить итератор на начало контейнера
     * @return итератор на первый элемент
     */
    iterator begin();

    /**
     * @brief Получить итератор на конец контейнера
     * @return итератор за последним элементом
     */
    iterator end();

    /**
     * @brief Получить константный итератор на начало контейнера
     * @return итератор на первый элемент
     */
    const_iterator cbegin() const;

    /**
     * @brief Получить константный итератор на конец контейнера
     * @return итератор за последним элементом
     */
    const_iterator cend() const;

private:
    /**
     * @brief Получить позицию в кольцевом буфере
     * @param position текущая позиция
     * @param n        количество сдвигов
     * @return новая позиция
     */
    size_t next(size_t position, size_t n = 1) const noexcept;
};


// Implementation

template<typename T>
CircularBuffer_srsw<T>::CircularBuffer_srsw(size_t size)
    : m_data(size + 1)  // +1 - резервный элемент, чтобы отличать состояния
                        // "пуст" и "полон"
    , m_head{0}
    , m_tail{0}
{}

template<typename T>
CircularBuffer_srsw<T>::CircularBuffer_srsw(const CircularBuffer_srsw<T> &other)
    : m_data{other.m_data}
    , m_head{other.m_head.load(std::memory_order_acquire)}
    , m_tail{other.m_tail.load(std::memory_order_acquire)}
{}

template<typename T>
CircularBuffer_srsw<T>::CircularBuffer_srsw(CircularBuffer_srsw<T> &&other)
    : m_data{std::move(other.m_data)}
    , m_head{other.m_head.load(std::memory_order_acquire)}
    , m_tail{other.m_tail.load(std::memory_order_acquire)}
{}

template<typename T>
template<typename ForwardInputIterator>
CircularBuffer_srsw<T>::CircularBuffer_srsw(ForwardInputIterator begin,
                                            ForwardInputIterator end)
    : m_data(std::distance(begin, end) + 1)
    , m_head{0}
    , m_tail{0}
{
    push_back_all(begin, end);
}

template<typename T>
bool CircularBuffer_srsw<T>::empty() const noexcept
{
    size_t head = m_head.load(std::memory_order_acquire);
    size_t tail = m_tail.load(std::memory_order_acquire);
    return  head == tail;
}

template<typename T>
void CircularBuffer_srsw<T>::clear() noexcept
{
    size_t tail = m_tail.load(std::memory_order_acquire);
    m_head.store(tail, std::memory_order_release);
}

template<typename T>
bool CircularBuffer_srsw<T>::full() const noexcept
{
    size_t head = m_head.load(std::memory_order_acquire);
    size_t tail = m_tail.load(std::memory_order_acquire);
    return  head == next(tail);
}

template<typename T>
size_t CircularBuffer_srsw<T>::size() const noexcept
{
    size_t head = m_head.load(std::memory_order_acquire);
    size_t tail = m_tail.load(std::memory_order_acquire);

    if(tail < head)
        return  m_data.size() + tail - head;

    return tail - head;
}

template<typename T>
size_t CircularBuffer_srsw<T>::max_size() const noexcept
{
    // -1 так как не считается резервный элемент
    return m_data.size() - 1;
}

template<typename T>
T &CircularBuffer_srsw<T>::at(size_t pos)
{
    if(pos >= size())
        throw std::out_of_range("CircularBuffer::at: no such index");

    pos = next(m_head.load(std::memory_order_acquire), pos);

    return m_data.at(pos);
}

template<typename T>
const T &CircularBuffer_srsw<T>::at(size_t pos) const
{
    if(pos >= size())
        throw std::out_of_range("CircularBuffer::at: no such index");

    pos = next(m_head.load(std::memory_order_acquire), pos);

    return m_data.at(pos);
}

template<typename T>
template<typename Type>
bool CircularBuffer_srsw<T>::try_push_back(Type &&value)
{
    if(full())
        return false;

    size_t tail = m_tail.load(std::memory_order_acquire);

    m_data.at(tail) = std::forward<Type>(value);

    m_tail.store(next(tail), std::memory_order_release);

    return true;
}

template<typename T>
bool CircularBuffer_srsw<T>::try_pop(T &result)
{
    if(empty())
        return false;

    size_t head = m_head.load(std::memory_order_acquire);

    result = std::move_if_noexcept(m_data.at(head));

    m_head.store(next(head), std::memory_order_release);

    return true;
}

template<typename T>
typename CircularBuffer_srsw<T>::iterator CircularBuffer_srsw<T>::begin()
{
    return iterator(0, *this);
}

template<typename T>
typename CircularBuffer_srsw<T>::iterator CircularBuffer_srsw<T>::end()
{
    return iterator(size(), *this);
}

template<typename T>
typename CircularBuffer_srsw<T>::const_iterator CircularBuffer_srsw<T>::cbegin() const
{
    return const_iterator(0, *this);
}

template<typename T>
typename CircularBuffer_srsw<T>::const_iterator CircularBuffer_srsw<T>::cend() const
{
    return const_iterator(size(), *this);
}

template<typename T>
size_t CircularBuffer_srsw<T>::next(size_t position, size_t n) const noexcept
{
    return (position + n) % m_data.size();
}


template<typename T>
template<typename ... Args>
bool CircularBuffer_srsw<T>::try_emplace_back(Args&& ... args)
{
    if(full())
        return false;

    size_t tail = m_tail.load(std::memory_order_acquire);

    m_data.at(tail) = T{std::forward<T>(args) ... };

    m_tail.store(next(tail), std::memory_order_release);

    return true;
}


// iterator

template<typename T>
CircularBuffer_srsw<T>::iterator::iterator(size_t index,
                                           CircularBuffer_srsw<T> &container)
    : m_index{index}
    , m_container{container}
{}

template<typename T>
typename CircularBuffer_srsw<T>::iterator&
CircularBuffer_srsw<T>::iterator::operator++()
{
    ++m_index;
    return *this;
}

template<typename T>
typename CircularBuffer_srsw<T>::iterator&
CircularBuffer_srsw<T>::iterator::operator--()
{
    --m_index;
    return *this;
}

template<typename T>
typename CircularBuffer_srsw<T>::iterator::reference
CircularBuffer_srsw<T>::iterator::operator*()
{
    return m_container.at(m_index);
}

template<typename T>
typename CircularBuffer_srsw<T>::iterator::difference_type
CircularBuffer_srsw<T>::iterator::operator-(
            const CircularBuffer_srsw<T>::iterator& other
        ) const
{
    return m_index - other.m_index;
}


template<typename T>
typename CircularBuffer_srsw<T>::iterator
CircularBuffer_srsw<T>::iterator::operator-(size_t value) const
{
    return iterator(m_index - value, m_container);
}


template<typename T>
typename CircularBuffer_srsw<T>::iterator
CircularBuffer_srsw<T>::iterator::operator+(size_t value) const
{
    return iterator(m_index + value, m_container);
}

template<typename T>
typename CircularBuffer_srsw<T>::iterator&
CircularBuffer_srsw<T>::iterator::operator=(
            const CircularBuffer_srsw<T>::iterator& other
        )
{
    m_index = other.m_index;
    return *this;
}

template<typename T>
bool CircularBuffer_srsw<T>::iterator::operator==(
            const CircularBuffer_srsw<T>::iterator& other
        ) const
{
    return m_index == other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::iterator::operator!=(
            const CircularBuffer_srsw<T>::iterator& other
        ) const
{
    return m_index != other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::iterator::operator<(
            const CircularBuffer_srsw<T>::iterator& other
        ) const
{
    return m_index < other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::iterator::operator<=(
        const CircularBuffer_srsw<T>::iterator& other
        ) const
{
    return m_index <= other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::iterator::operator>(
        const CircularBuffer_srsw<T>::iterator& other
        ) const
{
    return m_index > other.m_index;
}
template<typename T>
bool CircularBuffer_srsw<T>::iterator::operator>=(
        const CircularBuffer_srsw<T>::iterator& other
        ) const
{
    return m_index >= other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::iterator::operator!=(
        const CircularBuffer_srsw<T>::const_iterator& other
        ) const
{
    return m_index != other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::iterator::operator<(
        const CircularBuffer_srsw<T>::const_iterator& other
        ) const
{
    return m_index < other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::iterator::operator<=(
        const CircularBuffer_srsw<T>::const_iterator& other
        ) const
{
    return m_index <= other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::iterator::operator>(
        const CircularBuffer_srsw<T>::const_iterator& other
        ) const
{
    return m_index > other.m_index;
}
template<typename T>
bool CircularBuffer_srsw<T>::iterator::operator>=(
        const CircularBuffer_srsw<T>::const_iterator& other
        ) const
{
    return m_index >= other.m_index;
}

// const iterator

template<typename T>
CircularBuffer_srsw<T>::const_iterator::const_iterator(size_t index, const CircularBuffer_srsw<T>& container)
    : m_index{index}
    , m_container{container}
{}

template<typename T>
typename CircularBuffer_srsw<T>::const_iterator&
CircularBuffer_srsw<T>::const_iterator::operator++()
{
    ++m_index;
    return *this;
}

template<typename T>
typename CircularBuffer_srsw<T>::const_iterator&
CircularBuffer_srsw<T>::const_iterator::operator--()
{
    --m_index;
    return *this;
}

template<typename T>
typename CircularBuffer_srsw<T>::const_iterator::reference
CircularBuffer_srsw<T>::const_iterator::operator*() const
{
    return m_container.at(m_index);
}

template<typename T>
typename CircularBuffer_srsw<T>::const_iterator::difference_type
CircularBuffer_srsw<T>::const_iterator::operator-(
            const CircularBuffer_srsw<T>::const_iterator& other
        ) const
{
    return m_index - other.m_index;
}


template<typename T>
typename CircularBuffer_srsw<T>::const_iterator
CircularBuffer_srsw<T>::const_iterator::operator-(size_t value) const
{
    return const_iterator(m_index - value, m_container);
}


template<typename T>
typename CircularBuffer_srsw<T>::const_iterator
CircularBuffer_srsw<T>::const_iterator::operator+(size_t value) const
{
    return const_iterator(m_index + value, m_container);
}

template<typename T>
bool CircularBuffer_srsw<T>::const_iterator::operator==(
            const CircularBuffer_srsw<T>::const_iterator& other
        ) const
{
    return m_index == other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::const_iterator::operator!=(
            const CircularBuffer_srsw<T>::const_iterator& other
        ) const
{
    return m_index != other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::const_iterator::operator<(
            const CircularBuffer_srsw<T>::const_iterator& other
        ) const
{
    return m_index < other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::const_iterator::operator<=(
            const CircularBuffer_srsw<T>::const_iterator& other
        ) const
{
    return m_index <= other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::const_iterator::operator>(
            const CircularBuffer_srsw<T>::const_iterator& other
        ) const
{
    return m_index > other.m_index;
}
template<typename T>
bool CircularBuffer_srsw<T>::const_iterator::operator>=(
            const CircularBuffer_srsw<T>::const_iterator& other
        ) const
{
    return m_index >= other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::const_iterator::operator!=(
        const CircularBuffer_srsw<T>::iterator& other
        ) const
{
    return m_index != other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::const_iterator::operator<(
            const CircularBuffer_srsw<T>::iterator& other
        ) const
{
    return m_index < other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::const_iterator::operator<=(
            const CircularBuffer_srsw<T>::iterator& other
        ) const
{
    return m_index <= other.m_index;
}

template<typename T>
bool CircularBuffer_srsw<T>::const_iterator::operator>(
            const CircularBuffer_srsw<T>::iterator& other
        ) const
{
    return m_index > other.m_index;
}
template<typename T>
bool CircularBuffer_srsw<T>::const_iterator::operator>=(
            const CircularBuffer_srsw<T>::iterator& other
        ) const
{
    return m_index >= other.m_index;
}

template<typename T>
template<typename ForwardInputIterator>
size_t CircularBuffer_srsw<T>::push_back_all(
            ForwardInputIterator begin,
            ForwardInputIterator end
        )
{
    size_t counter{0};
    bool push_back_success;

    while(begin != end) {
         push_back_success = try_push_back(*begin);
         if(! push_back_success)
             break;

        ++counter;

        begin = std::next(begin);
    }

    return counter;
}

template<typename T>
template<typename ContainerType>
size_t CircularBuffer_srsw<T>::pop_all(ContainerType &container)
{
    container.reserve(container.size() + size());

    size_t counter{0};

    T value{};
    bool pop_success;


    while(pop_success) {
        pop_success = try_pop(value);
        if(! pop_success)
            break;

        container.push_back(value);
        ++counter;
    }
    return counter;
}





}
#endif // CIRCULAR_BUFFER_LOCKFREE_SRSW_H
