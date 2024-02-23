mkdir -p ./out
g++ --std=c++17  -I. ./main.cpp   -o ./out/draw.exe
cd ./out
./draw.exe
rm ./draw.exe
ppmtojpeg ./xxxx.ppm > xxx.jpg 

ppmtojpeg ./bright_image.ppm  > bright_image.jpg
ppmtojpeg ./color_image.ppm > color_image.jpg
ppmtojpeg ./blur_image.ppm > blur_image.jpg

rm bright_image.ppm
rm color_image.ppm
rm blur_image.ppm

rm xxxx.ppm
cd ..

 