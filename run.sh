rm -rf a.out
g++ -std=c++11 -pthread -fpermissive os_hw_3.cpp

echo "STARTING INPUT 3A.TXT"
./a.out < input3a.txt 1>output3at.txt

echo "STARTING INPUT 3B.TXT"
./a.out < input3b.txt 1>output3bt.txt

# echo "STARTING INPUT 3C.TXT"
# ./a.out < input3c.txt 1>output3ct.txt

# echo "STARTING INPUT NASTY.TXT"
# ./a.out < nasty.txt 1>nastyOutputTest.txt

