#!/usr/bin/env python3
"""
Smart video converter for 32x32 LED displays
Uses advanced techniques to preserve recognizable content
"""
import cv2
import numpy as np
import sys

def convert_smart_edgedetect(input_file, output_file, fps=20):
    """Convert using edge detection (Bad Apple style)"""
    print(f"Converting with EDGE DETECTION style...")
    print(f"  Input: {input_file}")
    
    cap = cv2.VideoCapture(input_file)
    if not cap.isOpened():
        print(f"Error: Could not open {input_file}")
        return
    
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(output_file, fourcc, fps, (32, 32))
    
    frame_count = 0
    processed = 0
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        # Resize first
        frame = cv2.resize(frame, (32, 32), interpolation=cv2.INTER_AREA)
        
        # Convert to grayscale
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        
        # Heavy blur to reduce noise
        blurred = cv2.GaussianBlur(gray, (3, 3), 0)
        
        # Edge detection
        edges = cv2.Canny(blurred, 50, 150)
        
        # Invert (white edges on black background)
        edges = cv2.bitwise_not(edges)
        
        # Dilate edges to make them more visible
        kernel = np.ones((2, 2), np.uint8)
        edges = cv2.dilate(edges, kernel, iterations=1)
        
        # Convert back to BGR
        frame_out = cv2.cvtColor(edges, cv2.COLOR_GRAY2BGR)
        
        out.write(frame_out)
        processed += 1
        
        if processed % 100 == 0:
            print(f"  Progress: {int(100 * frame_count / total_frames)}%")
        
        frame_count += 1
    
    cap.release()
    out.release()
    print(f"✓ Created: {output_file} (Edge Detection)")

def convert_smart_posterize(input_file, output_file, fps=20, colors=4):
    """Convert using posterization (reduce to key colors)"""
    print(f"Converting with POSTERIZATION ({colors} colors)...")
    print(f"  Input: {input_file}")
    
    cap = cv2.VideoCapture(input_file)
    if not cap.isOpened():
        print(f"Error: Could not open {input_file}")
        return
    
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(output_file, fourcc, fps, (32, 32))
    
    frame_count = 0
    processed = 0
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        # Resize
        frame = cv2.resize(frame, (32, 32), interpolation=cv2.INTER_AREA)
        
        # Boost saturation
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        hsv[:, :, 1] = cv2.multiply(hsv[:, :, 1], 1.5)
        frame = cv2.cvtColor(hsv, cv2.COLOR_HSV2BGR)
        
        # Posterize (reduce color depth)
        div = 256 // colors
        frame = (frame // div) * div + div // 2
        
        # Boost contrast
        lab = cv2.cvtColor(frame, cv2.COLOR_BGR2LAB)
        l, a, b = cv2.split(lab)
        clahe = cv2.createCLAHE(clipLimit=3.0, tileGridSize=(2, 2))
        l = clahe.apply(l)
        frame = cv2.merge([l, a, b])
        frame = cv2.cvtColor(frame, cv2.COLOR_LAB2BGR)
        
        out.write(frame)
        processed += 1
        
        if processed % 100 == 0:
            print(f"  Progress: {int(100 * frame_count / total_frames)}%")
        
        frame_count += 1
    
    cap.release()
    out.release()
    print(f"✓ Created: {output_file} (Posterized)")

def convert_smart_silhouette(input_file, output_file, fps=20):
    """Convert to high-contrast silhouettes"""
    print(f"Converting with SILHOUETTE style...")
    print(f"  Input: {input_file}")
    
    cap = cv2.VideoCapture(input_file)
    if not cap.isOpened():
        print(f"Error: Could not open {input_file}")
        return
    
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(output_file, fourcc, fps, (32, 32))
    
    frame_count = 0
    processed = 0
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        # Resize
        frame = cv2.resize(frame, (32, 32), interpolation=cv2.INTER_AREA)
        
        # Convert to grayscale
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        
        # Adaptive threshold for silhouette
        binary = cv2.adaptiveThreshold(
            gray, 255, cv2.ADAPTIVE_THRESH_GAUSSIAN_C,
            cv2.THRESH_BINARY, 11, 2
        )
        
        # Optional: invert if most content is dark
        if np.mean(binary) < 127:
            binary = cv2.bitwise_not(binary)
        
        # Convert to color
        frame_out = cv2.cvtColor(binary, cv2.COLOR_GRAY2BGR)
        
        out.write(frame_out)
        processed += 1
        
        if processed % 100 == 0:
            print(f"  Progress: {int(100 * frame_count / total_frames)}%")
        
        frame_count += 1
    
    cap.release()
    out.release()
    print(f"✓ Created: {output_file} (Silhouette)")

def convert_smart_cartoon(input_file, output_file, fps=20):
    """Convert to cartoon/cel-shaded style"""
    print(f"Converting with CARTOON style...")
    print(f"  Input: {input_file}")
    
    cap = cv2.VideoCapture(input_file)
    if not cap.isOpened():
        print(f"Error: Could not open {input_file}")
        return
    
    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
    out = cv2.VideoWriter(output_file, fourcc, fps, (32, 32))
    
    frame_count = 0
    processed = 0
    total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
    
    while True:
        ret, frame = cap.read()
        if not ret:
            break
        
        # Resize
        frame = cv2.resize(frame, (32, 32), interpolation=cv2.INTER_AREA)
        
        # Bilateral filter for cartoon effect
        cartoon = cv2.bilateralFilter(frame, 9, 75, 75)
        
        # Edge detection
        gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
        edges = cv2.adaptiveThreshold(
            gray, 255, cv2.ADAPTIVE_THRESH_MEAN_C,
            cv2.THRESH_BINARY, 9, 2
        )
        edges = cv2.cvtColor(edges, cv2.COLOR_GRAY2BGR)
        
        # Combine
        frame_out = cv2.bitwise_and(cartoon, edges)
        
        out.write(frame_out)
        processed += 1
        
        if processed % 100 == 0:
            print(f"  Progress: {int(100 * frame_count / total_frames)}%")
        
        frame_count += 1
    
    cap.release()
    out.release()
    print(f"✓ Created: {output_file} (Cartoon)")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("=" * 60)
        print("Smart Video Converter for 32x32 LED Displays")
        print("=" * 60)
        print("\nUsage:")
        print("  python3 smart_convert.py <input.mp4> <style>")
        print("\nStyles:")
        print("  edge       - Edge detection (Bad Apple style)")
        print("  posterize  - Reduce to key colors (4 colors)")
        print("  silhouette - High-contrast silhouettes")
        print("  cartoon    - Cartoon/cel-shaded effect")
        print("  all        - Generate all 4 versions")
        print("\nExamples:")
        print("  python3 smart_convert.py video.mp4 edge")
        print("  python3 smart_convert.py video.mp4 all")
        print("=" * 60)
        sys.exit(1)
    
    input_file = sys.argv[1]
    style = sys.argv[2].lower()
    
    base_name = input_file.rsplit('.', 1)[0]
    
    if style == 'edge':
        convert_smart_edgedetect(input_file, f"{base_name}_edge.mp4")
    elif style == 'posterize':
        convert_smart_posterize(input_file, f"{base_name}_posterize.mp4")
    elif style == 'silhouette':
        convert_smart_silhouette(input_file, f"{base_name}_silhouette.mp4")
    elif style == 'cartoon':
        convert_smart_cartoon(input_file, f"{base_name}_cartoon.mp4")
    elif style == 'all':
        convert_smart_edgedetect(input_file, f"{base_name}_edge.mp4")
        convert_smart_posterize(input_file, f"{base_name}_posterize.mp4")
        convert_smart_silhouette(input_file, f"{base_name}_silhouette.mp4")
        convert_smart_cartoon(input_file, f"{base_name}_cartoon.mp4")
    else:
        print(f"Unknown style: {style}")
        print("Use: edge, posterize, silhouette, cartoon, or all")
        sys.exit(1)
    
    print("\n" + "=" * 60)
    print("✨ Conversion complete!")
    print("=" * 60)
    print("\nStream with:")
    print(f"  python3 video_streamer.py {base_name}_<style>.mp4 --brightness 0.3 --loop")
