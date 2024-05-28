# Лабораторная 1 (простые операции, гамма-коррекция)

1. написать консольное приложение для генерации одноканального 8bpp изображения с "градиентной заливкой" (от 0 до 255, из прямоугольников s-ширина, h-высота) и гамма-скорректированной заливкой

2. полоски располагаются друг над другом

3. гамма-коррекция оформляется в виде функции

4. имя выходного файла указывается в виде опционального параметра (без ключа), если этот параметр не задан, то просто показать на экране результат и закрыть приложение по нажатию любой клавиши

5. параметры s, h, gamma получать из параметров коммандной строки, если ключи не заданы, то использовать умолчания (s=3, h=30, gamma=2.4)

# Ход работы

1) Используем ```cv::CommandLineParser``` для парсинга аргументов.

```cpp
int main(int argc, char* argv[]) {
  cv::CommandLineParser parser(argc, argv,
    "{ s | 3 | width }"
    "{ h | 30 | height }"
    "{ gamma | 2.4 | gamma correction }"
    "{ output | | output filename }");

  auto s = parser.get<int>("s");
  auto h = parser.get<int>("h");
  auto g = parser.get<double>("gamma");
  auto filename = parser.get<std::string>("output");
  

```

2) Создаем одноканальный ```cv::Mat``` для градиента, проходимся по столбцам, считаем отношение текущего столбца ко всему количеству и красим весь столбец в этот в цвет 255 * соотношение.

```cpp
  cv::Mat grad(h, s * 256, CV_8UC1);
  for (int col = 0; col < grad.cols; ++col) {
    auto intensity = static_cast<float>(col) / grad.cols;
    grad.col(col).setTo(cv::Scalar(255 * intensity, 255 * intensity, 255 * intensity));
  }


```

3) Создаем одноканальный ```cv::Mat``` для откорректированного градиента, вызываем функцию ```generateGammaCorrectedGradient```, передаем ```cv::Mat``` и гамму.

```cpp
  cv::Mat gradGammaCorrected(h, s * 256, CV_8UC1);
  generateGammaCorrectedGradient(gradGammaCorrected, g);

  
```

4) Функция ```generateGammaCorrectedGradient```:
Проходимся по столбцам, считаем интенсивность с помощью гаммы и перекрашиваем столбец.

```cpp
void generateGammaCorrectedGradient(cv::Mat& grad, double& gamma) {
  for (auto col = 0.0; col < grad.cols; ++col) {
    auto intensity = pow(col / grad.cols, 1.0 / gamma);
    grad.col(col).setTo(cv::Scalar(255 * intensity, 255 * intensity, 255 * intensity));
  }
}

  
```

5) Создаем новый ```cv::Mat```, конкатенируем два предыдущих в нем и записываем его в файл, название которого указали в аргументе к программе.

```cpp
  cv::Mat combined;
  cv::vconcat(grad, gradGammaCorrected, combined);

  if (!filename.empty()) {
    filename += ".jpg";
    cv::imwrite(filename, combined);
    std::cout << "Image saved" << std::endl;
  } else {
    cv::imshow("lab01", combined);
    cv::waitKey(0);
  }

  return 0;
}

  
```
### Получившиеся изображения - сверху оригинальный градиент, а снизу с гамма коррекцией.

гамма 2.4

![1 example Image](examples/1.jpg)

гамма 1

![1 example Image](examples/2.jpg)

гамма 0.1

![1 example Image](examples/3.jpg)

гамма 5

![1 example Image](examples/4.jpg)

# Пример использования
```bash
./../bin/lab01 -output=1 -gamma=5
```