#ifndef CircularBufferLockfree_H
#define CircularBufferLockfree_H

#include <vector>
#include <atomic>
#include <type_traits>

namespace connest {

/**
  @brief Кольцевой lockfree буфер multiple reader - multiple writer
  @details
        R'          R
        |           |
        V           V
  -----------------------------------------------------
  |   |RRR|RRR|RRR|RRR|   |   |WWW|WWW|WWW|WWW|   |RES|
  -----------------------------------------------------
  |                             ^           ^     |
  0                             |           |     Size
                                W'          W

  R' - R  => ждет прочтения (RRR)
  W' - W  => ждет записи    (WWW)
  W  - R' => свободное место1

  RES - зарезервированный элемент, чтобы отличать
        состояния "пуст" и "заполнен"

  Тождества (кольцевой отсчет):
    R' <= R
    W' <= W
    R  <= W
    W  <= R'
 */

template<typename T>
class CircularBuffer_mrmw
{
    static_assert (     std::is_nothrow_move_assignable<T>::value
                    &&  std::is_nothrow_copy_assignable<T>::value
                    &&  std::is_nothrow_constructible<T>::value
                    &&  std::is_nothrow_default_constructible<T>::value,
    "Type T must not throw exceptions in ctor and in assign operator");

    std::vector<T> m_data;

    std::atomic<size_t> m_R;
    std::atomic<size_t> m_R_complite; // R'

    std::atomic<size_t> m_W;
    std::atomic<size_t> m_W_complite; // W'


public:
    CircularBuffer_mrmw(size_t size);

    /**
     * @brief Получить максимальную вместимость буфера
     * @return максимальная вместимость буфера
     */
    size_t max_size() const noexcept;

    /**
     * @brief Текущая заполненость буфера
     * @return текущая заполненость буфера (в элементах)
     */
    size_t size() const noexcept;

    /**
     * @brief Проверить заполнен ли буфер
     * @return флаг заполнености
     */
    bool full() const noexcept;

    /**
     * @brief Проверить пуст ли буфер
     * @return флаг пустоты
     */
    bool empty() const noexcept;

    /**
     * @brief Добавить элемент в конец буфера
     * @param value записиваемое значение
     * @return флаг успешности операции (буфер может быть переполнен)
     */
    template<typename Type>
    bool try_push_back(Type&& value);

    /**
     * @brief Получить очередное значение из буфера
     * @param result место, куда будет записано значение
     * @return флаг успешности операции (буфер может быть пуст)
     */
    bool try_pop(T& result);
private:

    /**
     * @brief Получить индекс в кольцевом буфере через step элементов
     * @param position  текущая позиция
     * @param step      количество пропускаемых элементов
     * @return индекс в кольцевом буфере
     */
    constexpr size_t next(size_t position, size_t step = 1) const noexcept;
};


// Implementation


template<typename T>
CircularBuffer_mrmw<T>::CircularBuffer_mrmw(size_t size)
    : m_data(size + 1) // +1 так как нужно создать резервный элемент
    , m_R{0}
    , m_R_complite{0}
    , m_W{0}
    , m_W_complite{0}
{}

template<typename T>
size_t CircularBuffer_mrmw<T>::max_size() const noexcept
{
    // -1 так как есть резервный элемент
    // (для определения "пуст" или "полон")
    return m_data.size() - 1;
}

template<typename T>
size_t CircularBuffer_mrmw<T>::size() const noexcept
{
    size_t writePos = m_W_complite.load(std::memory_order_acquire);
    size_t readPos  = m_R         .load(std::memory_order_acquire);

    if(writePos < readPos)
        return m_data.size() + writePos - readPos;

    return writePos - readPos;

}

template<typename T>
bool CircularBuffer_mrmw<T>::full() const noexcept
{
    size_t writePos = m_W         .load(std::memory_order_acquire);
    size_t readPos  = m_R_complite.load(std::memory_order_acquire);

    if(writePos < readPos)
        return writePos + 1 == readPos;

    return writePos - readPos == max_size();
}

template<typename T>
bool CircularBuffer_mrmw<T>::empty() const noexcept
{
    size_t R = m_R         .load(std::memory_order_acquire);
    size_t W = m_W_complite.load(std::memory_order_acquire);

    return R == W;
}

template<typename T>
bool CircularBuffer_mrmw<T>::try_pop(T& result)
{
    size_t currentR, newR, currentR_complite_copy;

    while(true) {
        currentR = m_R.load(std::memory_order_acquire);
        newR     = next(currentR);

        if(currentR == m_W_complite.load(std::memory_order_acquire))
            return false;


        if(! std::atomic_compare_exchange_weak_explicit(
                    &m_R,
                    &currentR,
                    newR,
                    std::memory_order_release,
                    std::memory_order_relaxed
                    ))
            continue;

        result = m_data.at(currentR);

        do {
            // currentW_complite_copy изменится (примет текущее значение),
            // если CAS не удастся => нужно его перезаписать заново
            currentR_complite_copy = currentR;

        } while(! std::atomic_compare_exchange_weak_explicit(
                    &m_R_complite,
                    &currentR_complite_copy,
                    newR,
                    std::memory_order_release,
                    std::memory_order_relaxed
                    ));

        return true;
    }
}

template<typename T>
constexpr size_t CircularBuffer_mrmw<T>::next(size_t position, size_t step) const noexcept
{
    return (position + step) % m_data.size();
}

template<typename T>
template<typename Type>
bool CircularBuffer_mrmw<T>::try_push_back(Type&& value)
{
    size_t currentW, newW, currentW_complite_copy;

    while(true) {
        currentW = m_W.load(std::memory_order_acquire);
        newW     = next(currentW);

        if(newW == m_R_complite.load(std::memory_order_acquire))
            return false;

        if(! std::atomic_compare_exchange_weak_explicit(
                    &m_W,
                    &currentW,
                    newW,
                    std::memory_order_release,
                    std::memory_order_relaxed
                    ))
            continue;

        m_data.at(currentW) = std::forward<Type>(value);

        do {
            // currentW_complite_copy изменится (примет текущее значение),
            // если CAS не удастся => нужно его перезаписать заново
            currentW_complite_copy = currentW;

        } while(! std::atomic_compare_exchange_weak_explicit(
                    &m_W_complite,
                    &currentW_complite_copy,
                    newW,
                    std::memory_order_release,
                    std::memory_order_relaxed
                    ));

        return true;
    }
}

}
#endif // CircularBufferLockfree_H
