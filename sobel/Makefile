all: sobel

sobel: obj/convolution.o obj/rgb_image.o obj/sobel.o
	g++ obj/convolution.o obj/rgb_image.o obj/sobel.o -lboost_regex -o bin/sobel

obj/convolution.o: src/convolution.cpp src/convolution.hpp
	g++ -Wall -O3 -lboost_regex -std=c++11 -c src/convolution.cpp -o obj/convolution.o
obj/rgb_image.o: src/rgb_image.cpp src/rgb_image.hpp
	g++ -Wall -O3 -lboost_regex -std=c++11 -c src/rgb_image.cpp -o obj/rgb_image.o
obj/sobel.o: src/sobel.cpp
	g++ -Wall -O3 -lboost_regex -std=c++11 -c src/sobel.cpp -o obj/sobel.o

clean:
	rm -f obj/*.o bin/sobel
