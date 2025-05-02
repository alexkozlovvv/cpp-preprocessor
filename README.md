# Preprocessor

Учебный проект для закрепления навыков использования регулярных выражений. Представляет собой реализацию базового функционала стандартного препроцессора С++. А именно: поиск и inline раскрытие содрежания глобальных и локальных заголовочных файлов.

## Структура проекта

```mermaid
    classDiagram
    class Preprocessor {
        - ifstream input_file_
        - ofstream output_file_
        - const path& path_to_in_file_
        - const vector~path~& include_directories_

        + Preprocessor(const path& path_to_in_file, const path& path_to_out_file, const vector~path~& include_directories)
        + void Preprocess()
        - void RecursivePreprocess(const path& path_to_in_file, ifstream& input_file)
    }
```
<br>

### Основные структурные элементы:
- 
- 

## Download

Скачать репозиторий можно с помощью команды:

```
git clone git@github.com:alexkozlovvv/cpp-preprocessor.git
```

## Usage

На данном этапе проект не предполагает интерактивного взаимодействия с пользователем поскольку отсутствует обработка входных команд. Сейчас они задаются статически в теле программы. 

Алгоритм работы программы:
- Создается объект поискового сервера;
- С помощью контруктора передаются стоп слова; 
- Далее создается объект очереди запросов.

## Дальнейшее развитие

Необходимо добавить возможность интерактивного взаимодействия с пользователем с помощью терминала.
Добавить тесты для модульного тестирования компонентов архитектуры.


