mkdir -p ./out
g++ --std=c++17  -I. ./main.cpp   -o ./out/draw.exe
cd ./out
./draw.exe
rm ./draw.exe
ppmtojpeg ./xxxx.ppm > xxx.jpg 
rm xxxx.ppm
cd ..