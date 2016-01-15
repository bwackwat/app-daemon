clang++ bcast.cpp conn.cpp server.cpp main.cpp -std=c++11 -Wall -lboost_system -lboost_timer -lboost_chrono -lrt -lpthread -o appd

echo "================ RUNNING ==========>>>>>>>>>>"

xterm -e ./appd 10000 &
xterm -e ./appd 10001 &
xterm -e ./appd 10002 &
xterm -e ./appd 10003 &

read -rsp $'Press any key to continue...\n' -n1 key

killall xterm
