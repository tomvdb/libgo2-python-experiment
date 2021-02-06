gcc -o go2wrapper.o -c go2wrapper.c
gcc -shared -o go2wrapper.so  go2wrapper.o  -lgo2
sudo cp go2wrapper.so /usr/lib/