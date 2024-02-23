mkdir -p ./out
g++ --std=c++17  -I. ./main.cpp   -o ./out/draw.exe 
cd ./out
./draw.exe
rm ./draw.exe
ppmtojpeg ./xxxx.ppm > xxx.jpg 
ppmtojpeg ./xxxx_depth.ppm > xxx_depth.jpg 
ppmtojpeg ./xxxx_depth_color.ppm > xxxx_depth_color.jpg 
ppmtojpeg ./debug0.ppm > debug0.jpg 
ppmtojpeg ./debug1.ppm > debug1.jpg 
rm xxxx.ppm
rm xxxx_depth.ppm
rm xxxx_depth_color.ppm
rm debug0.ppm
rm debug1.ppm
cd ..