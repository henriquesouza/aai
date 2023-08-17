bp:
	"${HOME}/Downloads/android-ndk-r25c-linux/android-ndk-r25c/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android29-clang" -fPIC -O3 -Wall -shared src/aai.c -I includes -o aai.so
	adb push aai.so /data/local/tmp

dev:
	gcc -fPIC -O3 -Wall -shared src/aai.c -I includes -o aai.so

test: dev
	# rm /tmp/file.csv
	LD_PRELOAD=./aai.so mpv ~/Vídeos
	head -n 50 /tmp/file.csv

