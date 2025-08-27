# Consuming ACX via pkg-config
```
CXXFLAGS=$(pkg-config --cflags acx)
LDFLAGS=$(pkg-config --libs acx)
g++ your.cpp -o your_app $CXXFLAGS $LDFLAGS
```
See `examples/integration/pkg-config`.
