# CXX=clang++
# CXXFLAGS = -rdynamic $(shell llvm-config --cxxflags) -g -O0 -fpic
# LDFLAGS = $(shell llvm-config --ldflags --libs --system-libs)

# all: p34.so

# %.so: %.o
# 	$(CXX) $(CXXFLAGS) $(LDFLAGS) -dylib -shared  $^ -o $@
# clean:
# 	rm -f *.o *~ *.so

CXX=clang++
CXXFLAGS=-rdynamic `llvm-config --cxxflags` -g -Og -fPIC
LDFLAGS=`llvm-config --ldflags`
LDLIBS=`llvm-config --libs`

all: p34.so

%.so: %.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

# # Provided for macOS users, run `make p34.dylib` instead.
%.dylib: %.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -dynamiclib -o $@ $^ $(LDLIBS)

%.bc: %.c
	clang -c -fno-discard-value-names -emit-llvm -o $@ $^

%_def.bc: %.bc
	opt --enable-new-pm=0 -load ./p34.so -def-pass -o $@ $^
%_fix.bc: %.bc
	opt --enable-new-pm=0 -load ./p34.so -fix-pass -o $@ $^

%_npm_def.bc: %.bc
	opt  -load-pass-plugin ./p34_npm.so --passes=def-pass -o $@ $^
%_npm_fix.bc: %.bc
	opt  -load-pass-plugin ./p34_npm.so --passes=fix-pass -o $@ $^

TESTS=test1.c test2.c test3.c test4.c test5.c \
      test6.c test7.c test8.c test9.c test10.c
test: $(TESTS:.c=_def.bc) $(TESTS:.c=_fix.bc)
test_npm: $(TESTS:.c=_npm_def.bc) $(TESTS:.c=_npm_fix.bc)

clean:
	rm -f *.o *~ *.so *.bc a.out

# Don't implicitly remove intermediate unoptimized byte codes
.PRECIOUS: $(TESTS:.c=.bc)
