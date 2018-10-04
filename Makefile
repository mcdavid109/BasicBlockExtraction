all: BBdetect

BBdetect:
	g++ -std=c++11 -Wall -g BBdetect.cpp -o BBdetect
clean:
	rm -f BBdetect

