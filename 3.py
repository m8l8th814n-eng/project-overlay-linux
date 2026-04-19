
import time
import sys
import random
import shutil
import string

OVERLAY_TEXT = "overlay"
LINUX_TEXT = " overlaylinux "

COLORS_GLITCH = [
    "\033[38;5;250m",
    "\033[38;5;244m",
    "\033[38;5;238m",
    "\033[38;5;39m",
]

GRADIENT = [
    "\033[38;5;255m",
    "\033[38;5;250m",
    "\033[38;5;245m",
    "\033[38;5;240m",
    "\033[38;5;238m",
]

RESET = "\033[0m"
NOODLE_CHARS = string.ascii_letters + string.digits + "@#$%&*!?"

def clear():
    sys.stdout.write("\033[2J\033[H")

def glitch_chars(line):
    if not line: return line
    if random.random() < 0.1:
        chars = list(line)
        for _ in range(random.randint(1, max(1, len(chars)//3))):
            idx = random.randint(0, len(chars)-1)
            if chars[idx] != " ":
                chars[idx] = random.choice(["█", "▓", "▒", "░", "╬", "§", "?", "#", "@", "%", "&", "X"])
        return "".join(chars)
    return line

def render(chars_revealed_overlay, chars_revealed_linux):
    sys.stdout.write("\033[H")
    
    is_glitching_frame = random.random() < 0.05
    
    if is_glitching_frame and random.random() < 0.3:
        sys.stdout.write("\n\n")
    else:
        sys.stdout.write("\n")
        
    overlay_len = len(OVERLAY_TEXT)
    linux_len = len(LINUX_TEXT)
    
    colored_overlay = ""
    for i, char in enumerate(OVERLAY_TEXT):
        grad_idx = int((i / max(1, overlay_len)) * len(GRADIENT))
        grad_idx = min(max(grad_idx, 0), len(GRADIENT)-1)
        color = GRADIENT[grad_idx]

        if i < chars_revealed_overlay:
            display_char = char
            if is_glitching_frame and random.random() < 0.1:
                display_char = random.choice(["█", "▓", "▒", "░", "╬", "§", "?", "#"])
                color = random.choice(COLORS_GLITCH)
        else:
            display_char = random.choice(NOODLE_CHARS)
            color = random.choice(COLORS_GLITCH)

        colored_overlay += f"{color}{display_char}"

    offset = " " * random.randint(0, 2) if (is_glitching_frame and random.random() < 0.3) else ""
    sys.stdout.write(f"{offset}{colored_overlay}{RESET}\n")

    colored_linux = ""
    for i, char in enumerate(LINUX_TEXT):
        color = "\033[38;5;244m"
        
        if i < chars_revealed_linux:
            display_char = char
            if is_glitching_frame and random.random() < 0.2:
                display_char = random.choice(["█", "▓", "▒", "░", "╬", "§", "?", "#"])
                color = random.choice(COLORS_GLITCH)
        else:
            display_char = random.choice(NOODLE_CHARS) if char != " " else " "
            if display_char != " ":
                color = random.choice(COLORS_GLITCH)

        colored_linux += f"{color}{display_char}"

    offset = " " * random.randint(0, 1) if (is_glitching_frame and random.random() < 0.2) else ""
    sys.stdout.write(f"{offset}{colored_linux}{RESET}\n")

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
    end_time = time.time() + seconds
    start_time = time.time()
    
    overlay_len = len(OVERLAY_TEXT)
    linux_len = len(LINUX_TEXT)
    
    reveal_duration = min(1.5, seconds * 0.8)
    
    try:
        while True:
            now = time.time()
            if now >= end_time:
                clear()
                sys.stdout.write("\nwe live.\n")
                sys.stdout.flush()
                break
                
            elapsed = now - start_time
            progress = elapsed / reveal_duration if reveal_duration > 0 else 1.0
            
            chars_revealed_overlay = int(progress * overlay_len * 1.2)
            chars_revealed_linux = int((progress - 0.2) * linux_len * 1.5)
            
            chars_revealed_overlay = max(0, min(overlay_len, chars_revealed_overlay))
            chars_revealed_linux = max(0, min(linux_len, chars_revealed_linux))
            
            render(chars_revealed_overlay, chars_revealed_linux)
            time.sleep(random.uniform(0.03, 0.08))
    except KeyboardInterrupt:
        pass
    finally:
        sys.stdout.write(f"{RESET}\n")

if __name__ == "__main__":
    main()
