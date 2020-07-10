OBJS     = main.o adpcm.o fileloader.o adpcm_decode.o converter.o
OUT      = TXM-ADPCM-DECODE
CC       = gcc
CFLAGS   = -Ofast -std=c18 -Wall -Wextra
CXX      = g++
CXXFLAGS = -Ofast -std=c++17 -Wall -Wextra

all: $(OUT)

$(OUT): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(OUT) $^

clean:
	rm -f $(OBJS) $(OUT)

rebuild: clean all

clean-win:
	del /F $(OBJS) $(OUT).exe

rebuild-win: clean-win all
