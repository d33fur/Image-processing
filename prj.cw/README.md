# Курсовая работа

```bash
make && ./../bin/cw -p=./../prj.cw/music/sold_out.mp3 -a=-150,50 -w=7 -l=1 -g=1 -m=20 -f=2048 -n=2 -size=400,1100
```

```bash
make && ./../bin/cw -p=./../prj.cw/music/vortex.mp3 -a=-90,100 -w=9 -l=0 -g=1 -m=20 -f=32768 -n=20 -size=400,2100 -grad=1 -fill=2 -grad_coef=127
```

```bash
make && ./../bin/cw -p=./../prj.cw/music/abaddon.mp3 -a=-90,100 -w=9 -l=0 -g=1 -m=20 -f=65636 -n=20 -size=400,2100 -grad=1 -fill=2 -grad_coef=255
```

```bash
./bin/cw -p=./prj.cw/music/abaddon.mp3 -f=65536 -grad=19 -n=20 -grad=18
```


linux или macos
```bash
cmake -B build -S . && sudo cmake --build build --target install
```

windows powershell с правами администратора
```powershell
cmake -B build -S . && cmake --build build --target install
```