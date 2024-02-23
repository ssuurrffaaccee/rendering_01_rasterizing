mkdir -p ./out
g++ --std=c++17  -I. ./main.cpp   -o ./out/draw.exe
cd ./out
./draw.exe
rm ./draw.exe
ppmtojpeg ./xxxx.ppm > xxx.jpg 
ppmtojpeg ./inversion.ppm > inversion.jpg 
ppmtojpeg ./grayscale.ppm > grayscale.jpg 
ppmtojpeg ./sharpen.ppm > sharpen.jpg
ppmtojpeg ./blur.ppm > blur.jpg 
ppmtojpeg ./edge_detection.ppm > edge_detection.jpg 
rm xxxx.ppm
rm grayscale.ppm
rm inversion.ppm
rm sharpen.ppm
rm blur.ppm
rm edge_detection.ppm
cd ..

 