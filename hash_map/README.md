#Robin Hood HashMap

Хэшмапа открытой адресации, с использоваанием алгоритма Робин Гуда для разрешения коллизий.

HashMap<class KeyType, class ValueType, class Hash>

1. KeyType - тип ключа
2. ValueType - тип значений
3. Класс c реализованным оператором() принимающим значение типа KeyType, и выдающее size_t значение вычисленного хэша. Дефолтное значение - std::hash<KeyType>
