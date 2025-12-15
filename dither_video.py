#!/usr/bin/env python3
"""
Convert videos to high-contrast dithered versions for LED matrix
"""
import cv2
import numpy as np
import sys

def floyd_steinberg_dither(img):
    """Apply Floyd-Steinberg dithering to an image"""
    h, w = img.shape
    output = img.copy().astype(float)
    
    for y in range(h - 1):
        for x in range(1, w - 1):
            old_pixel = output[y, x]
            new_pixel = 255 if old_pixel > 127 else 0
            output[y, x] = new_pixel
            error = old_pixel - new_pixel
            
            output[y, x + 1] += error * 7 / 16
            output[y + 1, x - 1] += error * 3 / 16
            output[y + 1, x] += error * 5 / 16
            output[y + 1, x + 1] += error * 1 / 16
    
    return np.clip(output, 0, 255).astype(np.uint8)

def convert_to_dithered(input_file, output_file, target_fps=20, boost_contrast=True):
    """Convert video to high-contrast dithered version"""
    print(f"Converting {input_file} to dithered format...")
    
    cap = cv2.VideoCapture(input_file)
    if not cap.isOpened():
        print(f"Error: Could not open {input_file}")
        return
    
    # Get properties
    fps = cap.get(cv2.CAP_PROP_FPS)
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    
    # Output video
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(output_file, fourcc, target_fps, (32, 32))
    
    frame_count = 0
    processed = 0
    
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        # Sample frames based on FPS conversion
        if fps > target_fps:
            skip_ratio = fps / target_fps
            if frame_count % int(skip_ratio) != 0:
                frame_count += 1
                continue
        
        # Resize to 32x32
        frame = cv2.resize(frame, (32, 32), interpolation=cv2.INTER_LANCZOS4)
        
        # Convert to grayscale
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        
        # Boost contrast
        if boost_contrast:
            gray = cv2.equalizeHist(gray)
            # Additional contrast boost
            gray = np.clip(gray * 1.5 - 50, 0, 255).astype(np.uint8)
        
        # Apply dithering
        dithered = floyd_steinberg_dither(gray)
        
        # Convert back to BGR for video
        dithered_bgr = cv2.cvtColor(dithered, cv2.COLOR_GRAY2BGR)
        
        out.write(dithered_bgr)
        processed += 1
        
        if processed % 100 == 0:
            progress = int(100 * frame_count / total_frames)
            print(f"  Progress: {progress}% ({processed} frames)")
        
        frame_count += 1
    
    cap.release()
    out.release()
    
    print(f"✓ Created: {output_file} ({processed} frames)")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 dither_video.py <input_video> [output_video]")
        print("\nExamples:")
        print("  python3 dither_video.py big_buck_bunny.mp4")
        print("  python3 dither_video.py chaplin.mp4 chaplin_dithered.mp4")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else input_file.replace('.mp4', '_dithered.mp4')
    
    convert_to_dithered(input_file, output_file)
    
    print(f"\nStream it with:")
    print(f"  python3 video_streamer.py {output_file} --brightness 0.3 --loop")
