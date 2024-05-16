#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <random>

using namespace std;

const int QUEUE_SIZE = 10; // Размер очереди

queue<int> requests; // Очередь заявок
mutex m; // Мьютекс для синхронизации доступа к очереди
condition_variable cv; // Условная переменная для уведомления о состоянии очереди

// Функция для генерации случайной задержки
int randomDelay(int min, int max) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}

// Функция производителя
void producer(int min_delay, int max_delay) {
    int request_id = 1;
    while (true) {
        {
            unique_lock<mutex> lock(m);

            // Ждем, пока в очереди появится место
            cv.wait(lock, [] { return requests.size() < QUEUE_SIZE; });

            // Добавляем заявку в очередь
            requests.push(request_id++);
            cout << "Producer: task added " << request_id - 1 << endl;

            // Уведомляем потребителя о появлении заявки
            cv.notify_one();
        }

        // Засыпаем на случайное время
        this_thread::sleep_for(chrono::milliseconds(randomDelay(min_delay, max_delay)));
    }
}

// Функция потребителя
void consumer(int min_delay, int max_delay) {
    while (true) {
        {
            unique_lock<mutex> lock(m);

            // Проверяем наличие заявок без ожидания
            if (!requests.empty()) {
                // Извлекаем заявку из очереди
                int request_id = requests.front();
                requests.pop();
                cout << "Consumer: task processed " << request_id << endl;

                // Уведомляем производителя о появлении свободного места
                cv.notify_one();
            }
        }

        // Засыпаем на случайное время
        this_thread::sleep_for(chrono::milliseconds(randomDelay(min_delay, max_delay)));
    }
}

int main() {
    // Параметры задержек
    int producer_min_delay = 10; // Минимальная задержка производителя (ms)
    int producer_max_delay = 20; // Максимальная задержка производителя (ms)
    int consumer_min_delay = 200; // Минимальная задержка потребителя (ms)
    int consumer_max_delay = 800; // Максимальная задержка потребителя (ms)

    // Создаем потоки производителя и потребителя
    thread producer_thread(producer, producer_min_delay, producer_max_delay);
    thread consumer_thread(consumer, consumer_min_delay, consumer_max_delay);

    // Ожидаем завершения потоков
    producer_thread.join();
    consumer_thread.join();

    return 0;
}