#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <semaphore>
#include <chrono>
#include <functional>

// Класс для представления общего ресурса
class SharedResource {
public:
    static constexpr int MAX_READERS = 10;

    SharedResource() : rsemaphore(std::make_shared<std::counting_semaphore<MAX_READERS>>(MAX_READERS)) {}

    std::shared_ptr<std::counting_semaphore<MAX_READERS>> rsemaphore;
    std::mutex wlock;
};

// Класс для представления читателя
class Reader {
public:
    Reader(const std::string& name, std::shared_ptr<SharedResource> resource) :
        name(name), resource(resource) {}

    void read() {
        std::unique_lock<std::mutex> lock(resource->wlock);
        // Ждем, пока мьютекс записи не будет разблокирован
        while (resource->wlock.try_lock()) {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            lock.lock();
        }
        resource->rsemaphore->acquire(); // Захватываем разрешение на чтение
        std::cout << "Reader " << name << " reading data" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Reader " << name << " reading done" << std::endl;
        resource->rsemaphore->release(); // Освобождаем разрешение на чтение
    }

private:
    std::string name;
    std::shared_ptr<SharedResource> resource;
};

// Класс для представления писателя
class Writer {
public:
    Writer(const std::string& name, std::shared_ptr<SharedResource> resource) :
        name(name), resource(resource) {}

    void write() {
        // Ждем, пока все разрешения на чтение не будут освобождены
        while (resource->rsemaphore->try_acquire() == false) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::lock_guard<std::mutex> lock(resource->wlock);
        std::cout << "Writer " << name << " writing data" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Writer " << name << " writing done" << std::endl;
    }

private:
    std::string name;
    std::shared_ptr<SharedResource> resource;
};

int main() {
    // Создаем общий ресурс
    auto sr = std::make_shared<SharedResource>();

    // Создаем объекты читателей и писателей
    Reader reader1("reader1", sr);
    Reader reader2("reader2", sr);
    Reader reader3("reader3", sr);
    Writer writer1("writer1", sr);
    Writer writer2("writer2", sr);

    // Запускаем потоки
    std::thread t1(std::bind(&Reader::read, &reader1));
    std::thread t2(std::bind(&Writer::write, &writer1));
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::thread t3(std::bind(&Reader::read, &reader2));
    std::thread t4(std::bind(&Writer::write, &writer2));
    std::thread t5(std::bind(&Reader::read, &reader3));

    // Ожидаем завершения потоков
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();

    std::cout << "All threads finished!" << std::endl;

    return 0;
}