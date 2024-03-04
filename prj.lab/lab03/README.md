# Лабораторная 03 (автоконтраст)

## флаги
- path |  | path to image
- q1 | 0.0 | left quantile
- q2 | 1.0 | right quantile

## демонтрация примеров
Изображения выводятся вместе
в случае чб изображения:
1)оригинал
2)с автоконтрастом

в случае ргб изображения:
1)оригинал
2)с автоконстрастом совместно
3)с автоконстрастом поканально


все команды я запускал из ```build```

```bash
../bin/lab03 -path=../prj.lab/lab03/2.jpg -q1=0.99 -q2=1
```
![1 example Image](prj.lab/lab03/1.jpg)


```bash
../bin/lab03 -path=../prj.lab/lab03/2.jpg -q1=0.1 -q2=0.9
```
![2 example Image](prj.lab/lab03/2.jpg)

```bash
../bin/lab03 -path=../prj.lab/lab03/1.jpg -q1=0.4 -q2=0.6
```
![3 example Image](prj.lab/lab03/3.jpg)

```bash
../bin/lab03 -path=../prj.lab/lab03/1.jpg -q2=0.05
```
![4 example Image](prj.lab/lab03/4.jpg)