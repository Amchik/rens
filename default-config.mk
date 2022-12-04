cc      := clang
cflags  := -O2 -pipe -flto
ldflags := -fuse-ld=lld -Wl,-O2 -Wl,--as-needed -lcurl -pthread
strip   := -g
