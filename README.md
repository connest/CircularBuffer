# Набор циклических буферов

## CircularBuffer_mrmw

Lockfree реализация циклического буфера multiple reader - multiple writter.

В отличие от похожей [реализации](https://github.com/kmdreko/wilt-ring/blob/master/wilt-ring/ring.cpp) (нашел её после создания собственной реализации):

- требуется 4 атомарные операции, вместо 6.
- хранятся не POD-типы, приведенные к char*, а полноценные объекты с сохранением типа, что потенциально быстрее, так как может быть вызвать перемещающий конструктор.

### Замечание
На состояние 17.04.2021 г.: не проведено достаточное количество тестов на различных архитектурах, но алгоритм формально линеаризован.

Поэтому, вероятно, следует использовать проверенные реализации очереди, [например](https://github.com/cameron314/concurrentqueue)

### Детали реализации

```
        R'          R
        |           |
        V           V
  -----------------------------------------------------
  |   |RRR|RRR|RRR|RRR|   |   |WWW|WWW|WWW|WWW|   |RES|
  -----------------------------------------------------
  |                             ^           ^     |
  0                             |           |     Size
                                W'          W

  
R' -  R  => ждет прочтения, т.е. "захвачено на чтение" (RRR)
W' -  W  => ждет записи,    т.е. "захвачено на запись" (WWW)
W  -  R' => свободное место

RES - зарезервированный элемент, чтобы отличать состояния "пуст" и "заполнен"

Тождества (кольцевой отсчет):
    R' <=  R
    W' <=  W
    R  <=  W
    W  <=  R'
```

### TODO

- [X] Добавить проверки на хранимый тип: если конструктор или оператор присваивания вызовет исключение, хвост никогда не будет "подобран" => контейнер зависнет.
- [ ] Увеличить покрытие кода тестами.

## CircularBuffer_srsw

Lockfree реализация циклического буфера single reader - single writter.

Частный случай алгоритма описанного выше, но с упором на гарантию от вызывающего кода, что с контейнером будут взаимодейстовать только **2** потока выполнения: читатель и писатель. Эта гарантия дает следующие преимущества:

- уменьшение количества атомарных операций на 2 шт. (не требуются R' и W').
- возможность использования random access итераторов читающим потоком.
- возможность "заглядывания" читающим потоком в данные, при этом не удаляя элемент из буфера.

### Детали реализации

```
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

  R --> W - полезные данные
  W --> R - свободное место
```

## CircularBuffer_mrmw_blocked

Блокирующая реализация циклического буфера multiple reader - multiple writter.

Следует использовать, с случаях, когда ожидать очередные данные (или пока они прочитаются, если буфер заполнен) слишком дорого, и потоку выполнения выгоднее в таких случаях "засыпать" в ожидании.

В отличие от описанных выше алгоритмов, здесь используется блокирование на основе std::mutex. Это позволяет _не_ использовать атомарные типы данных, что увеличивает лаяльность кеша (потенциально может дать более эффективную работу).

# Пример использования

Пример: 4 потока-писателя записывают в буфер целые числа от 0 до 1000, а также инкрементирует amount_written

Одновременно с ними 4 потока-читателя достают из буфера эти данные. Каждый их них добавляет полученное значение в глобальную переменную, а также инкрементирует amount_readed.

Ожидается, что сумма полученных значений будет равна сумме записанных, как и их количество.


```cpp
#define READER_COUNT 4
#define WRITER_COUNT 4
#define ELEMENTS_COUNT 1000u

// Собственно кольцевой буфер
//                   хранимый тип данных
//                          vvvvvvv
connest::CircularBuffer_mrmw<size_t> queue{16};
//                                        ^^^^
//                               размер буфера в элементах

// сумма значений считанных элементов
std::atomic<size_t> read_counter{0};
// сумма значений записанных элементов
std::atomic<size_t> write_counter{0};

// количество считанных элементов
std::atomic<size_t> amount_readed{0};
// количество записанных элементов
std::atomic<size_t> amount_written{0};


void reader()
{
    size_t value{0};
    while(true) {
        if(queue.try_pop(value)) {
            ++amount_readed;
            read_counter += value;
        }
    }

}

void writter()
{
    for(size_t i = 1; i < ELEMENTS_COUNT;) {
        if(queue.try_push_back(i)) {
            write_counter += i;
            ++i;
            ++amount_written;
        }
    }

    std::stringstream s;
    s << "writter end with: " << write_counter;
    std::cout << s.str() << std::endl;
}

int main()
{
    std::vector<std::thread> writters;
    std::vector<std::thread> readers;

    writters.reserve(WRITER_COUNT);
    for(int i = 0; i < WRITER_COUNT; ++i)
        writters.emplace_back(writter);

    readers.reserve(READER_COUNT);
    for(int i = 0; i < READER_COUNT; ++i)
        readers.emplace_back(reader);

	// не ждем читателей
    for(auto& r : readers)
        r.detach();

    // ждем пока допишут писатели
    for(auto& w : writters)
        w.join();

    // чтобы читатели успели дочитать (но, и прочитать слишком много, если в алгоритме ошибка)
    std::this_thread::sleep_for(std::chrono::seconds(3));

    assert(read_counter  == write_counter);
    assert(amount_readed == amount_written);

    return 0;
}
```

# Прочее

- Существует заголовочный файл circular_buffer_fwd.h - список forward declaration для перечисленных классов для ускорения компиляции.









