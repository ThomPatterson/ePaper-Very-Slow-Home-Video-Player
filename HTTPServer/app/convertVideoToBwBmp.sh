#!/bin/sh

#input is any video in landscape orientation
#script will create a directory of dithered black and white frames from the video as bitmap

# Input video file
input_video="$1"
directory=$(dirname "$input_video")
filename=$(basename "$input_video")
extension="${filename##*.}"
filename="${filename%.*}"

# Output directory for frames
temp_dir="/usr/data/videos/temp"
colored_output_dir="$temp_dir/${filename}_color_frames"
dithered_output_dir="$temp_dir/${filename}_dithered_frames"
bitmap_output_dir="$temp_dir/${filename}_bitmap_frames"

# Create output directories if they don't exist
mkdir -p "$temp_dir"
mkdir -p "$colored_output_dir"
mkdir -p "$dithered_output_dir"
mkdir -p "$bitmap_output_dir"

# Extract frames using FFmpeg with a complex filtergraph
ffmpeg -i "$input_video" -vf "scale=853:480,crop=800:480" -r 15 "$colored_output_dir/frame_%04d.png"

# Dither each frame using didder
for frame in "$colored_output_dir"/*.png; do
    base_name=$(basename "$frame" .png)
    didder --palette "black white" -i "$frame" -o "$dithered_output_dir/$base_name.png" --brightness 0.2 --contrast 0.05 edm FloydSteinberg

    # Convert dithered frame to 1-bit BMP
    convert "$dithered_output_dir/$base_name.png" -monochrome "$bitmap_output_dir/$base_name.bmp"
done

echo "Colored frames extracted and saved in $colored_output_dir"
echo "Dithered frames saved in $dithered_output_dir"
echo "Bitmap frames saved in $bitmap_output_dir"

# Move the bitmap_output_dir to the directory of the input file
mv "$bitmap_output_dir" "$directory"

# Delete the temporary directory
rm -rf "$temp_dir"
