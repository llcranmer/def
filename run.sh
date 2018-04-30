echo "Removing previous build.."
rm -rf a.out
echo "Building.."
g++ -std=c++11 -pthread -fpermissive assignment3.cpp
# clear all
echo "Starting Experiments"
echo "STARTING INPUT 3A.TXT"
./a.out < input3a.txt 1>output3at.txt

echo "STARTING INPUT 3A.TXT"
./a.out < input3b.txt 1>output3bt.txt

echo "STARTING INPUT 3C.TXT"
./a.out < input3c.txt 1>output3ct.txt

echo "STARTING INPUT NASTY.TXT"
./a.out < nasty.txt 1>nastyOutputTest.txt

echo "End of Experimets"
