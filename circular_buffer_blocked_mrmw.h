#ifndef CIRCULAR_BUFFER_BLOCKED_MRMW_H
#define CIRCULAR_BUFFER_BLOCKED_MRMW_H

#include <vector>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <stdexcept>

namespace connest {
/**
  @brief    Циклический буфер multiple reader - multiple writter
            основанный на блокировании мьютекса
 */
template <typename T>
class CircularBuffer_mrmw_blocked
{
    std::vector<T> m_data;

    mutable std::mutex m_mutex;
    std::condition_variable m_cv;

    size_t m_head;
    size_t m_tail;

    std::atomic_bool m_delete;

public:
    CircularBuffer_mrmw_blocked(size_t size);

    CircularBuffer_mrmw_blocked(const CircularBuffer_mrmw_blocked& other);

    CircularBuffer_mrmw_blocked(CircularBuffer_mrmw_blocked&& other);

    ~CircularBuffer_mrmw_blocked();

    /**
     * @brief Получить размер данных в буфере
     * @return количество элементов в буфере
     */
    size_t size() const noexcept;

    /**
     * @brief Изменить размер буфера
     * @param newSize Новый размер
     */
    void resize(size_t newSize) noexcept;

    /**
     * @brief Получить размер буфера
     * @return максимальное количество элементов в буфере
     */
    size_t max_size() const noexcept;

    /**
     * @brief Проверить буфер на пустоту
     * @return флаг пустоты
     */
    bool empty() const noexcept;

    /**
     * @brief Проверить буфер на заполненность
     * @return флаг полноты
     */
    bool full() const noexcept;

    /**
     * @brief Попытаться добавить элемент в буфер
     * @param value значение элемента
     * @return флаг успешности операции
     */
    template<typename Type>
    bool try_push_back(Type&& value);

    /**
     * @brief   Добавить элемент в буфер.
     *          Если буфер заполнен, приостановить выполнения потока
     * @param value значение элемента
     */
    template<typename Type>
    void push_back_wait(Type&& value);

    /**
     * @brief Попытаться получить очередной элемент.
     * @param result Ссылка на место помещения результата
     * @return успешность операции
     */
    bool try_pop(T& result);

    /**
     * @brief   Получить очередной элемент.
     *          Если буфер пуст, приостановить выполнения потока
     * @param result Ссылка на место помещения результата
     */
    void pop_wait(T& result);

    /**
     * @brief Очистить буфер
     */
    void clear();

private:
    /**
     * @brief Получить очередную позицию в кольцевом буфере
     * @param position  Текущая позиция
     * @param n         Количество пропускаемых элементов
     * @return новая позиция
     */
    size_t next(size_t position, size_t n = 1) const noexcept;

    /**
     * @brief   Получить размер данных в буфере.
     *          Без блокировки мьютекса
     * @return количество элементов в буфере
     */
    size_t size_unsafe() const noexcept;

    /**
     * @brief   Проверить буфер на заполненность.
     *          Без блокировки мьютекса
     * @return флаг полноты
     */
    bool full_unsafe() const noexcept;

    /**
     * @brief   Проверить буфер на пустоту.
     *          Без блокировки мьютекса
     * @return флаг пустоты
     */
    bool empty_unsafe() const noexcept;
};



// Implementation



template<typename T>
CircularBuffer_mrmw_blocked<T>::CircularBuffer_mrmw_blocked(size_t size)
    : m_data(size + 1) // +1 для определения "полон" / "пуст"
    , m_mutex{}
    , m_cv{}
    , m_head{}
    , m_tail{}
    , m_delete{false}
{}

template<typename T>
CircularBuffer_mrmw_blocked<T>::CircularBuffer_mrmw_blocked(const CircularBuffer_mrmw_blocked &other)
    : m_data{other.m_data} // +1 для определения "полон" / "пуст"
    , m_mutex{}
    , m_cv{}
    , m_head{other.m_head}
    , m_tail{other.m_tail}
    , m_delete{false}
{}

template<typename T>
CircularBuffer_mrmw_blocked<T>::CircularBuffer_mrmw_blocked(CircularBuffer_mrmw_blocked &&other)
    : m_data{std::move(other.m_data)} // +1 для определения "полон" / "пуст"
    , m_mutex{}
    , m_cv{}
    , m_head{other.m_head}
    , m_tail{other.m_tail}
    , m_delete{false}
{}

template<typename T>
CircularBuffer_mrmw_blocked<T>::~CircularBuffer_mrmw_blocked()
{
    m_delete = true;
    m_cv.notify_all();
}

template<typename T>
size_t CircularBuffer_mrmw_blocked<T>::size() const noexcept
{
    std::lock_guard<std::mutex> locker(m_mutex);

    return size_unsafe();
}

template<typename T>
void CircularBuffer_mrmw_blocked<T>::resize(size_t newSize) noexcept
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_data.resize(newSize + 1); // + 1 так как резерв
}

template<typename T>
size_t CircularBuffer_mrmw_blocked<T>::max_size() const noexcept
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_data.size() - 1;
}

template<typename T>
bool CircularBuffer_mrmw_blocked<T>::empty() const noexcept
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return m_head == m_tail;
}

template<typename T>
bool CircularBuffer_mrmw_blocked<T>::full() const noexcept
{
    std::lock_guard<std::mutex> locker(m_mutex);
    return full_unsafe();
}

template<typename T>
bool CircularBuffer_mrmw_blocked<T>::try_pop(T &result)
{
    std::lock_guard<std::mutex> locker(m_mutex);

    if(empty_unsafe())
        return false;


    result = m_data.at(m_head);
    m_head = next(m_head);

    m_cv.notify_one();

    return true;
}

template<typename T>
void CircularBuffer_mrmw_blocked<T>::pop_wait(T &result)
{
    std::unique_lock<std::mutex> locker(m_mutex);

    if(empty_unsafe()) {
        m_cv.wait(locker, [&]() {
            return !empty_unsafe() || m_delete;
        });
    }

    if(m_delete)
        return;

    result = m_data.at(m_head);
    m_head = next(m_head);

    m_cv.notify_one();
}

template<typename T>
void CircularBuffer_mrmw_blocked<T>::clear()
{
    std::lock_guard<std::mutex> locker(m_mutex);
    m_head = m_tail;
}

template<typename T>
size_t CircularBuffer_mrmw_blocked<T>::next(size_t position, size_t n) const noexcept
{
    return (position + n) % m_data.size();
}

template<typename T>
size_t CircularBuffer_mrmw_blocked<T>::size_unsafe() const noexcept
{
    if(m_tail < m_head)
        return m_data.size() + m_tail - m_head;

    return m_tail - m_head;
}

template<typename T>
bool CircularBuffer_mrmw_blocked<T>::full_unsafe() const noexcept
{
    return m_head == next(m_tail);
}

template<typename T>
bool CircularBuffer_mrmw_blocked<T>::empty_unsafe() const noexcept
{
    return m_head == m_tail;
}

template<typename T>
template<typename Type>
bool CircularBuffer_mrmw_blocked<T>::try_push_back(Type&& value)
{
    std::lock_guard<std::mutex> locker(m_mutex);

    if(full_unsafe())
        return false;


    m_data.at(m_tail) = std::forward<Type>(value);
    m_tail = next(m_tail);

    m_cv.notify_one();

    return true;
}

template<typename T>
template<typename Type>
void CircularBuffer_mrmw_blocked<T>::push_back_wait(Type&& value)
{
    std::unique_lock<std::mutex> locker(m_mutex);

    if(full_unsafe()) {
        m_cv.wait(locker, [&]() {
            return !full_unsafe() || m_delete;
        });
    }

    if(m_delete)
        return;

    m_data.at(m_tail) = std::forward<Type>(value);
    m_tail = next(m_tail);

    m_cv.notify_one();
}

}
#endif // CIRCULAR_BUFFER_BLOCKED_MRMW_H
