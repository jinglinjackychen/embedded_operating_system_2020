CC = arm-unknown-linux-gnu-gcc
CPP = arm-unknown-linux-gnu-g++
-L = /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/lib/
-I1 = /opt/arm-unknown-linux-gnu/arm-unknown-linux-gnu/include/
-I2 = /home/jacky/Downloads/pxa270/microtime/linux/include/

%: %.c
	rm -rf $@
	$(CC) -o $@ $< -lm -L $(-L) -I $(-I1) -I $(-I2)
