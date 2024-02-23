mkdir -p ./out
g++ --std=c++17 ./main.cpp   -o ./out/draw.exe
cd ./out
./draw.exe
rm ./draw.exe 
ppmtojpeg ./xxx1.ppm > xxx1.jpg 
rm xxx1.ppm
ppmtojpeg ./xxx2.ppm > xxx2.jpg 
rm xxx2.ppm
ppmtojpeg ./xxx3.ppm > xxx3.jpg 
rm xxx3.ppm
ppmtojpeg ./xxx4.ppm > xxx4.jpg 
rm xxx4.ppm
ppmtojpeg ./xxx5.ppm > xxx5.jpg 
rm xxx5.ppm
ppmtojpeg ./xxx6.ppm > xxx6.jpg 
rm xxx6.ppm
ppmtojpeg ./xxx7.ppm > xxx7.jpg 
rm xxx7.ppm
ppmtojpeg ./xxx8.ppm > xxx8.jpg 
rm xxx8.ppm
# ppmtojpeg ./xxx9.ppm > xxx9.jpg 
# rm xxx9.ppm
cd ..