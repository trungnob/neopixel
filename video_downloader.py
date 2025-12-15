#!/usr/bin/env python3
"""
Download and convert videos for 32x32 LED matrix
Supports downloading from YouTube and converting any video
"""
import subprocess
import sys
import os

def check_dependencies():
    """Check if required tools are installed"""
    try:
        subprocess.run(['yt-dlp', '--version'], capture_output=True, check=True)
        return True
    except (subprocess.CalledProcessError, FileNotFoundError):
        return False

def install_ytdlp():
    """Install yt-dlp"""
    print("Installing yt-dlp...")
    subprocess.run([sys.executable, '-m', 'pip', 'install', '--user', 'yt-dlp'], check=True)
    print("✓ yt-dlp installed!")

def download_video(url, output_name="downloaded_video"):
    """Download video from URL"""
    print(f"\nDownloading video from: {url}")
    
    cmd = [
        'yt-dlp',
        '-f', 'worst',  # Get lowest quality for faster download
        '-o', f'{output_name}.%(ext)s',
        url
    ]
    
    subprocess.run(cmd, check=True)
    
    # Find the downloaded file
    for ext in ['mp4', 'webm', 'mkv', 'avi']:
        if os.path.exists(f'{output_name}.{ext}'):
            return f'{output_name}.{ext}'
    
    return None

def convert_to_32x32(input_file, output_file='led_video.mp4', fps=20):
    """Convert any video to 32x32 optimized format using ffmpeg"""
    print(f"\nConverting {input_file} to 32x32...")
    
    cmd = [
        'ffmpeg', '-i', input_file,
        '-vf', f'scale=32:32:force_original_aspect_ratio=decrease,pad=32:32:(ow-iw)/2:(oh-ih)/2,fps={fps}',
        '-c:v', 'libx264',
        '-preset', 'fast',
        '-crf', '23',
        '-an',  # Remove audio
        '-y',  # Overwrite output
        output_file
    ]
    
    subprocess.run(cmd, check=True)
    print(f"✓ Converted to: {output_file}")
    return output_file

def main():
    print("=" * 60)
    print("Video Downloader for 32x32 LED Matrix")
    print("=" * 60)
    
    # Check dependencies
    if not check_dependencies():
        print("\nyt-dlp not found. Installing...")
        install_ytdlp()
    
    print("\n--- Recommended Videos for LED Matrices ---")
    print("\n1. Bad Apple (Classic)")
    print("   Search: 'Bad Apple Touhou'")
    print("   URL example: https://www.youtube.com/watch?v=FtutLA63Cp8")
    
    print("\n2. Big Buck Bunny (Test video)")
    print("   URL: https://www.youtube.com/watch?v=aqz-KE-bpKQ")
    
    print("\n3. Nyan Cat")
    print("   Search: 'Nyan Cat 10 hours'")
    
    print("\n4. Rick Roll")
    print("   Search: 'Rick Astley Never Gonna Give You Up'")
    
    print("\n5. Any music video or animation")
    print("   Tip: High-contrast videos work best!")
    
    print("\n" + "=" * 60)
    print("\nUsage examples:")
    print("  1. Download a video:")
    print("     python3 video_downloader.py <youtube-url>")
    print("\n  2. Convert existing video:")
    print("     python3 video_downloader.py --convert <input.mp4> <output.mp4>")
    print("=" * 60)
    
    if len(sys.argv) < 2:
        print("\nNo arguments provided. Showing help only.")
        return
    
    if sys.argv[1] == '--convert' and len(sys.argv) >= 3:
        # Convert existing video
        input_file = sys.argv[2]
        output_file = sys.argv[3] if len(sys.argv) > 3 else 'led_video.mp4'
        convert_to_32x32(input_file, output_file)
        print(f"\n✨ Ready to stream!")
        print(f"Run: python3 video_streamer.py {output_file} --brightness 0.3 --loop")
    else:
        # Download from URL
        url = sys.argv[1]
        downloaded = download_video(url, "downloaded_video")
        
        if downloaded:
            print(f"\n✓ Downloaded: {downloaded}")
            
            # Convert to 32x32
            final = convert_to_32x32(downloaded, 'led_video.mp4')
            
            print(f"\n✨ Ready to stream!")
            print(f"Run: python3 video_streamer.py {final} --brightness 0.3 --loop")
        else:
            print("❌ Download failed!")

if __name__ == "__main__":
    main()
