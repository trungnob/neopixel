#!/usr/bin/env python3
"""
Generate retro game-style GIFs for 32x32 LED matrix
"""
from PIL import Image, ImageDraw
import random

WIDTH = 32
HEIGHT = 32

def save_gif(frames, filename, duration=100):
    """Save frames as animated GIF"""
    frames[0].save(
        filename,
        save_all=True,
        append_images=frames[1:],
        duration=duration,
        loop=0
    )
    print(f"✓ Generated: {filename} ({len(frames)} frames)")

def create_mario_run():
    """Super Mario running and jumping"""
    frames = []
    
    # Mario colors
    RED = (255, 0, 0)
    BLUE = (0, 0, 255)
    SKIN = (255, 200, 150)
    BROWN = (100, 50, 0)
    SKY = (100, 150, 255)
    GROUND = (0, 200, 0)
    BRICK = (200, 100, 50)
    
    mario_x = 0
    jump_cycle = 0
    
    for frame in range(60):
        img = Image.new('RGB', (WIDTH, HEIGHT), SKY)
        draw = ImageDraw.Draw(img)
        
        # Ground
        draw.rectangle([0, HEIGHT-6, WIDTH, HEIGHT], fill=GROUND)
        
        # Moving blocks
        for i in range(4):
            block_x = (WIDTH - 20 - frame + i * 10) % WIDTH
            draw.rectangle([block_x, HEIGHT-12, block_x+6, HEIGHT-7], fill=BRICK)
        
        # Mario position
        mario_x = (mario_x + 1) % (WIDTH + 10)
        
        # Jump animation
        jump_height = 0
        if 15 < frame % 30 < 25:
            jump_height = 8 - abs((frame % 30) - 20)
        
        mario_y = HEIGHT - 12 - jump_height
        
        # Mario sprite (simplified 8x8)
        # Hat
        draw.rectangle([mario_x, mario_y, mario_x+7, mario_y+2], fill=RED)
        # Face
        draw.rectangle([mario_x+1, mario_y+2, mario_x+6, mario_y+5], fill=SKIN)
        # Eyes
        draw.point((mario_x+2, mario_y+3), fill=(0, 0, 0))
        draw.point((mario_x+5, mario_y+3), fill=(0, 0, 0))
        # Overalls
        draw.rectangle([mario_x+1, mario_y+5, mario_x+6, mario_y+7], fill=BLUE)
        # Legs (animated)
        if frame % 4 < 2:
            draw.rectangle([mario_x+1, mario_y+7, mario_x+3, mario_y+9], fill=BLUE)
            draw.rectangle([mario_x+4, mario_y+7, mario_x+6, mario_y+9], fill=BLUE)
        else:
            draw.rectangle([mario_x+2, mario_y+7, mario_x+4, mario_y+9], fill=BLUE)
            draw.rectangle([mario_x+4, mario_y+7, mario_x+6, mario_y+9], fill=BLUE)
        
        frames.append(img)
    
    save_gif(frames, "mario_run.gif", 50)

def create_space_invaders():
    """Space Invaders style animation"""
    frames = []
    
    GREEN = (0, 255, 0)
    WHITE = (255, 255, 255)
    RED = (255, 0, 0)
    BLACK = (0, 0, 0)
    
    # Invader positions
    invaders = []
    for row in range(3):
        for col in range(5):
            invaders.append({'x': col * 6 + 2, 'y': row * 5 + 2, 'alive': True})
    
    player_x = WIDTH // 2
    bullets = []
    direction = 1
    
    for frame in range(120):
        img = Image.new('RGB', (WIDTH, HEIGHT), BLACK)
        draw = ImageDraw.Draw(img)
        
        # Move invaders
        if frame % 10 == 0:
            for inv in invaders:
                inv['x'] += direction
            
            # Check if need to move down
            if any(inv['x'] >= WIDTH - 6 or inv['x'] <= 0 for inv in invaders if inv['alive']):
                direction *= -1
                for inv in invaders:
                    inv['y'] += 2
        
        # Draw invaders (animated)
        alien_frame = (frame // 10) % 2
        for inv in invaders:
            if inv['alive']:
                x, y = inv['x'], inv['y']
                # Simple alien sprite
                if alien_frame == 0:
                    draw.rectangle([x+1, y, x+4, y+1], fill=GREEN)
                    draw.rectangle([x, y+1, x+5, y+3], fill=GREEN)
                    draw.point((x+1, y+2), fill=BLACK)
                    draw.point((x+4, y+2), fill=BLACK)
                else:
                    draw.rectangle([x+1, y, x+4, y+1], fill=GREEN)
                    draw.rectangle([x, y+1, x+5, y+3], fill=GREEN)
                    draw.point((x+2, y+2), fill=BLACK)
                    draw.point((x+3, y+2), fill=BLACK)
        
        # Player
        player_x = 10 + (frame // 2) % 12
        draw.rectangle([player_x-2, HEIGHT-4, player_x+2, HEIGHT-2], fill=WHITE)
        draw.rectangle([player_x-1, HEIGHT-5, player_x+1, HEIGHT-4], fill=WHITE)
        
        # Bullets
        if frame % 15 == 0:
            bullets.append({'x': player_x, 'y': HEIGHT-6})
        
        for bullet in bullets[:]:
            bullet['y'] -= 2
            if bullet['y'] < 0:
                bullets.remove(bullet)
            else:
                draw.rectangle([bullet['x'], bullet['y'], bullet['x']+1, bullet['y']+2], fill=RED)
                
                # Check collision
                for inv in invaders:
                    if inv['alive'] and abs(inv['x'] - bullet['x']) < 4 and abs(inv['y'] - bullet['y']) < 3:
                        inv['alive'] = False
                        bullets.remove(bullet)
                        break
        
        frames.append(img)
    
    save_gif(frames, "space_invaders.gif", 60)

def create_pac_man_maze():
    """Pac-Man eating dots in maze"""
    frames = []
    
    YELLOW = (255, 255, 0)
    BLUE = (0, 0, 255)
    DOT = (255, 200, 100)
    BLACK = (0, 0, 0)
    WALL = (0, 0, 200)
    
    # Create simple maze
    maze = [[1, 1, 1, 1, 1, 1, 1, 1],
            [1, 0, 0, 0, 0, 0, 0, 1],
            [1, 0, 1, 1, 1, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 1],
            [1, 0, 1, 1, 1, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 1],
            [1, 1, 1, 1, 1, 1, 1, 1]]
    
    # Pac-Man path
    path = [(1, 1), (2, 1), (3, 1), (4, 1), (5, 1), (6, 1),
            (6, 2), (6, 3), (6, 4), (6, 5),
            (5, 5), (4, 5), (3, 5), (2, 5), (1, 5),
            (1, 4), (1, 3)]
    
    eaten_dots = set()
    
    for frame in range(80):
        img = Image.new('RGB', (WIDTH, HEIGHT), BLACK)
        draw = ImageDraw.Draw(img)
        
        # Draw maze
        for row in range(len(maze)):
            for col in range(len(maze[0])):
                x, y = col * 4, row * 4
                if maze[row][col] == 1:
                    draw.rectangle([x, y, x+3, y+3], fill=WALL)
                elif (col, row) not in eaten_dots:
                    draw.point((x+1, y+1), fill=DOT)
        
        # Pac-Man position
        pos_idx = (frame // 4) % len(path)
        pac_x, pac_y = path[pos_idx]
        pac_x *= 4
        pac_y *= 4
        
        # Mark dot as eaten
        eaten_dots.add(path[pos_idx])
        
        # Pac-Man with animated mouth
        mouth = (frame // 2) % 4
        if mouth < 2:
            # Open mouth
            draw.ellipse([pac_x-1, pac_y-1, pac_x+4, pac_y+4], fill=YELLOW)
            if frame % 8 < 4:
                draw.polygon([(pac_x+1, pac_y+1), (pac_x+4, pac_y+1), (pac_x+1, pac_y+3)], fill=BLACK)
        else:
            # Closed mouth
            draw.ellipse([pac_x-1, pac_y-1, pac_x+4, pac_y+4], fill=YELLOW)
        
        # Eye
        draw.point((pac_x+2, pac_y+1), fill=BLACK)
        
        frames.append(img)
    
    save_gif(frames, "pacman_maze.gif", 80)

def create_tetris():
    """Tetris blocks falling"""
    frames = []
    
    COLORS = [(255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 0), 
              (255, 0, 255), (0, 255, 255), (255, 128, 0)]
    BLACK = (0, 0, 0)
    GRID = (50, 50, 50)
    
    # Tetris pieces (I, O, T, L, etc.)
    pieces = [
        [[1, 1, 1, 1]],  # I
        [[1, 1], [1, 1]],  # O
        [[0, 1, 0], [1, 1, 1]],  # T
        [[1, 0], [1, 0], [1, 1]],  # L
    ]
    
    grid = [[0 for _ in range(8)] for _ in range(8)]
    current_piece = random.choice(pieces)
    current_color = random.choice(COLORS)
    piece_x, piece_y = 3, 0
    
    for frame in range(100):
        img = Image.new('RGB', (WIDTH, HEIGHT), BLACK)
        draw = ImageDraw.Draw(img)
        
        # Draw grid
        for i in range(9):
            draw.line([(i*4, 0), (i*4, 32)], fill=GRID)
            draw.line([(0, i*4), (32, i*4)], fill=GRID)
        
        # Draw landed pieces
        for row in range(8):
            for col in range(8):
                if grid[row][col]:
                    draw.rectangle([col*4, row*4, col*4+3, row*4+3], fill=grid[row][col])
        
        # Draw current piece
        for row in range(len(current_piece)):
            for col in range(len(current_piece[0])):
                if current_piece[row][col]:
                    x = (piece_x + col) * 4
                    y = (piece_y + row) * 4
                    draw.rectangle([x, y, x+3, y+3], fill=current_color)
        
        # Move piece down
        if frame % 8 == 0:
            piece_y += 1
            
            # Check if landed
            if piece_y + len(current_piece) >= 8:
                # Land the piece
                for row in range(len(current_piece)):
                    for col in range(len(current_piece[0])):
                        if current_piece[row][col] and piece_y + row < 8:
                            grid[piece_y + row][piece_x + col] = current_color
                
                # New piece
                current_piece = random.choice(pieces)
                current_color = random.choice(COLORS)
                piece_x, piece_y = 3, 0
        
        frames.append(img)
    
    save_gif(frames, "tetris.gif", 100)

def create_snake_game():
    """Classic Snake game"""
    frames = []
    
    GREEN = (0, 255, 0)
    RED = (255, 0, 0)
    BLACK = (0, 0, 0)
    
    snake = [(15, 15), (14, 15), (13, 15)]
    direction = (1, 0)
    apple = (20, 20)
    
    for frame in range(120):
        img = Image.new('RGB', (WIDTH, HEIGHT), BLACK)
        draw = ImageDraw.Draw(img)
        
        # Move snake
        if frame % 3 == 0:
            # Calculate new head position
            head_x, head_y = snake[0]
            new_head = (head_x + direction[0], head_y + direction[1])
            
            # Wrap around
            new_head = (new_head[0] % WIDTH, new_head[1] % HEIGHT)
            
            snake.insert(0, new_head)
            
            # Check if ate apple
            if new_head == apple:
                # Generate new apple
                apple = (random.randint(0, WIDTH-1), random.randint(0, HEIGHT-1))
            else:
                snake.pop()
            
            # Change direction occasionally
            if frame % 20 == 0:
                direction = random.choice([(1, 0), (-1, 0), (0, 1), (0, -1)])
        
        # Draw snake
        for segment in snake:
            draw.point(segment, fill=GREEN)
        
        # Draw apple
        draw.point(apple, fill=RED)
        draw.point((apple[0]+1, apple[1]), fill=RED)
        draw.point((apple[0], apple[1]+1), fill=RED)
        draw.point((apple[0]+1, apple[1]+1), fill=RED)
        
        frames.append(img)
    
    save_gif(frames, "snake_game.gif", 60)

if __name__ == "__main__":
    print("=" * 60)
    print("Creating Retro Game GIFs for 32x32 LED Matrix")
    print("=" * 60)
    print()
    
    create_mario_run()
    create_space_invaders()
    create_pac_man_maze()
    create_tetris()
    create_snake_game()
    
    print()
    print("=" * 60)
    print("✨ All retro game GIFs generated!")
    print("=" * 60)
    print("\nStream them with:")
    print("  python3 streamer.py mario_run.gif --brightness 0.3")
    print("  python3 streamer.py space_invaders.gif --brightness 0.3")
    print("  python3 streamer.py pacman_maze.gif --brightness 0.3")
    print("  python3 streamer.py tetris.gif --brightness 0.3")
    print("  python3 streamer.py snake_game.gif --brightness 0.3")
    print("=" * 60)
