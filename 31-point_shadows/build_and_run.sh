mkdir -p ./out
g++ --std=c++17  -I. ./main.cpp   -o ./out/draw.exe 
cd ./out
./draw.exe
rm ./draw.exe
ppmtojpeg ./xxxx.ppm > xxx.jpg 

ppmtojpeg ./back_depth.ppm  > back_depth.jpg
ppmtojpeg ./front_depth.ppm > front_depth.jpg
ppmtojpeg ./top_depth.ppm > top_depth.jpg
ppmtojpeg ./bottom_depth.ppm  > bottom_depth.jpg
ppmtojpeg ./left_depth.ppm  > left_depth.jpg
ppmtojpeg ./right_depth.ppm  > right_depth.jpg

ppmtojpeg ./back.ppm  > back.jpg
ppmtojpeg ./front.ppm > front.jpg
ppmtojpeg ./top.ppm > top.jpg
ppmtojpeg ./bottom.ppm  > bottom.jpg
ppmtojpeg ./left.ppm  > left.jpg
ppmtojpeg ./right.ppm  > right.jpg

ppmtojpeg ./no_shadow.ppm > no_shadow.jpg

rm back_depth.ppm
rm front_depth.ppm
rm top_depth.ppm
rm bottom_depth.ppm
rm left_depth.ppm   
rm right_depth.ppm

rm back.ppm
rm front.ppm
rm top.ppm
rm bottom.ppm  
rm left.ppm
rm right.ppm 

rm no_shadow.ppm 
rm xxxx.ppm
cd ..

 
