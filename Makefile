IMAGES_INPUT=images/1_raw
IMAGES_BNW=images/2_bnw
IMAGES_VORONOI=images/3_voronoi
IMAGES_OUTPUT=images/4_output
VIDEO_IN=video_in.mp4
VIDEO_OUT=video_out.mp4

PYTHON:=python3
CFLAGS=-Wall -Wextra -Werror -Wvla -std=c99 -pedantic -O1
CPPFLAGS=-Isrc

TARGET_FIT=bad_apple
OBJ_MAIN=src/main.o
OBJ=src/logger/logger.o

.PHONY: all fit check dev clean

# pipeline steps

all: extract_frames convert_images1 fit convert_images2 group_frames

extract_frames:
	[ $(IMAGES_INPUT)/frame0001.png -ot $(VIDEO_IN) ] && ffmpeg -i $(VIDEO_IN) \
		$(IMAGES_INPUT)/frame%04d.png || echo "Skipping frames extraction"

convert_images1:
	$(PYTHON) py/img2bnw.py $(IMAGES_INPUT) $(IMAGES_BNW)

fit: $(TARGET)

convert_images2:
	$(PYTHON) py/bnw2img.py $(IMAGES_VORONOI) $(IMAGES_OUTPUT)

group_frames:
	[ $(IMAGES_OUTPUT)/frame0001.png -ot $(VIDEO_IN) ] && ffmpeg -framerate 60 \
		-i $(IMAGES_OUTPUT)/frame%04d.png $(VIDEO_OUT) || \
		echo "Skipping frames grouping"

# main pipeline part with C code

$(TARGET): $(OBJ_MAIN) $(OBJ)
	$(CC) -o $@ $^ $(LDLIBS) $(LDFLAGS)

dev: CFLAGS+=-fsanitize=address -g
dev: LDLIBS+=-fsanitize=address
dev: all

clean:
	$(RM) $(TARGET) $(OBJ_MAIN) $(OBJ)

clean-cache:
	$(RM) $(IMAGES_INPUT)/* $(IMAGES_VORONOI)/* $(IMAGES_OUTPUT)/* $(OUTPUT)
