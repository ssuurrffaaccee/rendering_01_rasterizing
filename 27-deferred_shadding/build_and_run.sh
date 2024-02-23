mkdir -p ./out
g++ --std=c++17  -I. ./main.cpp   -o ./out/draw.exe
cd ./out
./draw.exe
rm ./draw.exe
ppmtojpeg ./xxxx.ppm > xxx.jpg 
ppmtojpeg ./xxx_color.ppm > xxx_color.jpg 
ppmtojpeg ./xxx_norm.ppm > xxx_norm.jpg 
ppmtojpeg ./xxx_position.ppm > xxx_position.jpg 
rm xxxx.ppm
rm xxx_color.ppm
rm xxx_norm.ppm
rm xxx_position.ppm
cd ..