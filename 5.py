
import time
import sys
import random
import shutil

OVERLAY_TEXT = "overlaylinux"
LINUX_TEXT = " ...counting ..bytes? "

COLORS_GLITCH = [
    "\033[38;5;39m",
    "\033[38;5;51m",
    "\033[38;5;13m",
    "\033[38;5;208m",
    "\033[38;5;196m",
    "\033[38;5;46m",
]

GRADIENT = [
    "\033[38;5;93m",
    "\033[38;5;135m",
    "\033[38;5;141m",
    "\033[38;5;255m",
    "\033[38;5;250m",
    "\033[38;5;244m",
]

RESET = "\033[0m"

def clear():
    sys.stdout.write("\033[2J\033[H")

def glitch_chars(line):
    if not line: return line
    if random.random() < 0.15:
        chars = list(line)
        for _ in range(random.randint(1, max(1, len(chars)//3))):
            idx = random.randint(0, len(chars)-1)
            if chars[idx] != " ":
                chars[idx] = random.choice(["█", "▓", "▒", "░", "╬", "§", "?", "#", "@", "%", "&", "X"])
        return "".join(chars)
    return line

def render(seconds_left, total_seconds, chars_revealed):
    cols, rows = shutil.get_terminal_size((80, 24))
    
    sys.stdout.write("\033[H")
    
    is_ending = seconds_left < 1.5
    
    is_glitching_frame = (random.random() < 0.8) if is_ending else (random.random() < 0.05)
    
    if is_glitching_frame and random.random() < (0.9 if is_ending else 0.3):
        sys.stdout.write("\n" * random.randint(1, 4 if is_ending else 2))
    else:
        sys.stdout.write("\n")
        
    overlay_len = len(OVERLAY_TEXT)
    linux_len = len(LINUX_TEXT)
    
    current_overlay = OVERLAY_TEXT[:min(overlay_len, chars_revealed)]
    current_linux = ""
    if chars_revealed > overlay_len:
        current_linux = LINUX_TEXT[:min(linux_len, chars_revealed - overlay_len)]
    
    if current_overlay:
        glitched = glitch_chars(current_overlay) if is_glitching_frame else current_overlay
        colored_line = ""
        for i, char in enumerate(glitched):
            grad_idx = int((i / max(1, overlay_len)) * len(GRADIENT))
            grad_idx = min(max(grad_idx, 0), len(GRADIENT)-1)
            
            color = GRADIENT[grad_idx]
            if is_glitching_frame and random.random() < (0.6 if is_ending else 0.1):
                color = random.choice(COLORS_GLITCH)
                
            colored_line += f"{color}{char}"
            
        offset = " " * random.randint(0, 5 if is_ending else 2) if (is_glitching_frame and random.random() < 0.4) else ""
        sys.stdout.write(f"{offset}{colored_line}{RESET}\n")
    else:
        sys.stdout.write("\n")
        
    if current_linux:
        glitched = glitch_chars(current_linux) if is_glitching_frame else current_linux
        c = "\033[38;5;244m"
        if is_glitching_frame and random.random() < (0.6 if is_ending else 0.2):
            c = random.choice(COLORS_GLITCH)
            
        offset = " " * random.randint(0, 4 if is_ending else 2) if (is_glitching_frame and random.random() < 0.3) else ""
        sys.stdout.write(f"{offset}{c}{glitched}{RESET}\n")
    else:
        sys.stdout.write("\n")

    sys.stdout.write("\033[J")
    sys.stdout.flush()

def main():
    seconds = 100.0
    if len(sys.argv) > 1:
        try:
            seconds = float(sys.argv[1])
        except ValueError:
            pass

    clear()
    total_seconds = seconds
    end_time = time.time() + total_seconds
    
    total_chars = len(OVERLAY_TEXT) + len(LINUX_TEXT)
    typing_speed = 0.15
    start_time = time.time()
    
    try:
        while True:
            now = time.time()
            if now >= end_time:
                clear()
                sys.stdout.write("\nchopchop.\n")
                sys.stdout.flush()
                break
                
            left = end_time - now
            elapsed = now - start_time
            chars_revealed = int(elapsed / typing_speed) + 1
            if chars_revealed > total_chars:
                chars_revealed = total_chars
                
            render(left, total_seconds, chars_revealed)
            time.sleep(random.uniform(0.02, 0.08))
    except KeyboardInterrupt:
        pass
    finally:
        sys.stdout.write(f"{RESET}\n")

if __name__ == "__main__":
    main()
