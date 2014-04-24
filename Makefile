MONGOOSE_FLAGS=-DMONGOOSE_NO_AUTH -DMONGOOSE_NO_CGI -DMONGOOSE_NO_DAV -DMONGOOSE_NO_WEBSOCKET
CC=gcc

all: lego hello

lego: libjsonc
	$(CC) -o $@ \
		-g \
		-I. \
		-L./json-c/.libs \
		mongoose/mongoose.c json-c/libjson.c lego.c \
		-ldl -lpthread -ljson-c \
		$(MONGOOSE_FLAGS)
	-ls -l $@
	-upx -7 $@

hello: controllers/hello.ctrl

controllers/hello.ctrl: libjsonc
	-mkdir controllers
	$(CC) -o $@ \
		-g -nostartfiles -shared -fPIC \
		-I. \
		-L./json-c/.libs \
		json-c/libjson.c hello-world.c \
		-lpthread -ljson-c
	-ls -l $@

libjsonc: json-c/.libs/json_object.o

json-c/.libs/json_object.o:	
	cd json-c ; \
	./autogen.sh && \
	./configure && \
	make

clean:
	rm lego
	rm controllers/hello.ctrl
