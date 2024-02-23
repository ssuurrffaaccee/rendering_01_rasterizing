mkdir -p ./out
g++ --std=c++17  -I. ./main.cpp   -o ./out/draw.exe
cd ./out
./draw.exe
rm ./draw.exe
ppmtojpeg ./xxxx.ppm > xxx.jpg 
ppmtojpeg back_cube.ppm > back_cube.jpg
ppmtojpeg bottom_cube.ppm > bottom_cube.jpg
ppmtojpeg front_cube.ppm > front_cube.jpg
ppmtojpeg left_cube.ppm > left_cube.jpg
ppmtojpeg right_cube.ppm > right_cube.jpg
ppmtojpeg top_cube.ppm > top_cube.jpg
rm xxxx.ppm
rm back_cube.ppm
rm bottom_cube.ppm
rm front_cube.ppm
rm left_cube.ppm
rm right_cube.ppm
rm top_cube.ppm
cd ..