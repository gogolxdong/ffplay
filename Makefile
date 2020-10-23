
objects = ffplay.o utils.o samplefmt.o profiles.o mpeg4audio.o log.o id3v2enc.o demuxer_list.o aviobuf.o adtsenc.o ac3dec_fixed.o
all: $(objects)
  cc -o ffplay $(objects)

$(object): ffplay.h

.PHONY: clean
clean: rm ffplay $(objects)

