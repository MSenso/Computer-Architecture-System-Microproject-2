#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <omp.h>

typedef struct threadData { // Структура для хранения информации о предметах потока
	int tobaccoCount;	// Количество табака
	int paperCount;		// Количество бумаги
	int matchCount;		// Количество спичек
};

const int timeLimit = 51;	// Лимит по времени
int currentTime = 1;		// Текущее время
const int participantsNumber = 4;	// Количество потоков
bool isEnd = false;		// Переменная, показывающая, произошло ли досрочное завершение
std::mutex mutex1;		// Мьютекс
int items[3] = {0};		// Массив предметов

threadData MakeParticipant(int index) {	// Мето для заполнения структуры-информации о предметах
	threadData threadData;	// Создаем структуру
	int random = rand();	// Генерируем число
	switch (index)
	{
	case 0: {	// Посредник, у него нет предметов
		threadData.tobaccoCount = 0;
		threadData.paperCount = 0;
		threadData.matchCount = 0;
		break;
	}
	case 1: {	// Курильщик 1, у него табака от 1 до 5
		threadData.tobaccoCount = random % 5 + 1;
		threadData.paperCount = 0;
		threadData.matchCount = 0;
		break;
	}
	case 2: {	// Курильщик 2, у него бумаги от 1 до 5
		threadData.tobaccoCount = 0;
		threadData.paperCount = random % 5 + 1;
		threadData.matchCount = 0;
		break;
	}
	case 3: {	// Курильщик 3, у него спичек от 1 до 5
		threadData.tobaccoCount = 0;
		threadData.paperCount = 0;
		threadData.matchCount = random % 5 + 1;
		break;
	}
	}
	return threadData;
}

void GiveItems() {	// Метод генерации предметов
	items[0] = rand() % 2;	// Генерируем количество табака от 0 до 1
	if (items[0] != 0) {	// Если табак есть
		items[1] = rand() % 2;	// Генерируем количество бумаги от 0 до 1
		if (items[1] == 0) items[2] = 1;	// Если бумаги нет, то есть спички, т.к. нужно два предмета
	}
	else
	{	// Если табака нет, то генерируются бумага и спички, т.к. нужно два предмета
		items[1] = 1;
		items[2] = 1;
	}
}

int FindSmoker(threadData *threadsData) {	// Поиск подходящего курильщика

	if (items[0] == 0 && threadsData[1].tobaccoCount > 0)	// Если сгенерированы бумага и спички, а у первого курильщика есть табак, он подходит
	{
		return 1;
	}
	if (items[1] == 0 && threadsData[2].paperCount > 0) // Если сгенерированы табак и спички, а у второго курильщика есть бумага, он подходит
	{
		return 2;
	}
	if (items[2] == 0 && threadsData[3].matchCount > 0) // Если сгенерированы табак и бумага, а у третьего курильщика есть спички, он подходит
	{
		return 3;
	}
	return -1; // Если у нужного курильщика закончился третий подходящий предмет, то возвращаем -1
}

void Smoking(threadData &smoker) {	// Процесс курения
	if (smoker.tobaccoCount > 0) smoker.tobaccoCount--;	// Если первый курильщик, то табак уменьшается на 1
	else if (smoker.paperCount > 0) smoker.paperCount--;	// Если второй курильщик, то бумага уменьшается на 1
	else smoker.matchCount--;	// Если третий курильщик, то спички уменьшаются на 1
	std::this_thread::sleep_for(std::chrono::milliseconds(5000));	// Задержка на 5 секунд
}

int main() {
	srand(time(NULL));	// Инициализация ДСЧ

	threadData participants[participantsNumber];	// Массив данных о предметах
	for (int i = 0; i < participantsNumber; i++) {
		participants[i] = MakeParticipant(i);	// Генерируем предметы для каждого потока
		if (i > 0) {	// Если курильщик, то выводим информацию о предметах
			std::cout << "Smoker " << i << " has these items: tobacco: " << participants[i].tobaccoCount << ", paper: " << participants[i].paperCount << ", matches: " << participants[i].matchCount;
			std::cout << "\n";
			std::this_thread::sleep_for(std::chrono::milliseconds(2000));	// Задержка на 2 секунды
		}
	}

	std::cout << "\nCurrent time: " << currentTime << " min.\n \n";	// Вывод текущего времени
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));	// Задержка на 2 секунды

	do
	{
		mutex1.lock();	// Блокируем мьютекс
		items[0] = 0;	// Обнуляем количество табака
		items[1] = 0;	// Обнуляем количество бумаги
		items[2] = 0;	// Обнуляем количество спичек

#pragma omp parallel num_threads(participantsNumber)	// Распараллеливание на 4 потока
		{
			auto i = omp_get_thread_num();	// Получаем номер потока
			if (i == 0) {	// Если посредник
				GiveItems();	// Генерируем предметы
				std::cout << "Mediator is adding these items: \n" << "tobacco: " << items[0] << ", paper: " << items[1] << ", matches: " << items[2];	// Выводим предметы
				std::cout << "\n \n";
				std::this_thread::sleep_for(std::chrono::milliseconds(5000));	// Задержка на 5 секунд
				mutex1.unlock();	// Разблокировка мьютекса
			}
			else
			{	// Если курильщик
				if (items[0] == 0 && items[1] == 0 && items[2] == 0)	// Если посредник еще не выполнил свою работу
				{
					mutex1.lock();	// Попытка блокировки заблокированного мьютекса
					mutex1.unlock();	// Разблокировка мьютекса для "пробуждения" следующих потоков
				}
				int index = FindSmoker(participants);	// Поиск подходящего курильщика
				if (index > 0) {	// Курильщик найден
					if (i == index)	// Текущий курильщик может курить
					{
						std::this_thread::sleep_for(std::chrono::milliseconds(3000));	// Задержка на 3 секунды
						std::cout << "Current time: " << currentTime << " min., smoker " << std::to_string(i) << " is smoking \n";	// Вывод информации о курящем курильщике
						std::cout << "Other smokers and the mediator are waiting\n";

						Smoking(std::ref(participants[index]));	// Процесс курения
						// Вывод информации об оставшихся предметов курильщика
						std::cout << "Smoker's items now: \ntobacco: " << participants[i].tobaccoCount << ", paper: " << participants[i].paperCount << ", matches: " << participants[i].matchCount << "\n \n";
						std::this_thread::sleep_for(std::chrono::milliseconds(3000));	// Задержка на 3 секунды
					}

				}
				else isEnd = true;	// Курильщик не найден, завершаем цикл
			}
#pragma omp barrier	// Барьер для синхронизации
		}
		currentTime += 5;	// Увеличиваем текущее время на 5
		if (isEnd == 0) std::cout << "Smoker has finished, current time: " << currentTime << " min.\n \n";	// Вывод информации о завершении курения и о текущем времени
		else std::cout << "Current time: " << currentTime <<", the needed smoker is ran out of items, this is the end\n";	// Вывод текущего времени и сообщения о завершении работы
		std::this_thread::sleep_for(std::chrono::milliseconds(3000));	// Задержка на 3 секунды
	} while (currentTime < timeLimit && !isEnd);	// Пока текущее время не превысило лимит и пока не произошло досрочное окончание из-за того, что у подходящего курильщика закончился нужный предмет
	
	if (isEnd == 0) std::cout << "Current time: " << currentTime << " min., we've reached the time limit, this is the end";	// Если цикл прекратил работу не из-за досрочного окончания, вывод сообщения о превышении лимита по времени
}