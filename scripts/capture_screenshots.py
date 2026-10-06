"""Re-create the README screenshots from real pseudo-terminal runs.

Typed input is echoed by the terminal, exactly as a user sees it.
Usage (repo root, Linux/macOS):  make all original && python3 scripts/capture_screenshots.py
Requires Pillow and the DejaVu Sans Mono font.
"""
import os, pty, time, select, re, sys
from PIL import Image, ImageDraw, ImageFont

def capture(inputs, binary="./build/states"):
    pid, fd = pty.fork()
    if pid == 0:
        os.environ["TERM"] = "xterm-256color"
        os.execv(binary, [binary])
    buf = b""
    def read_until(pat, timeout=3):
        nonlocal buf
        end = time.time() + timeout
        while time.time() < end:
            r, _, _ = select.select([fd], [], [], 0.05)
            if r:
                try: chunk = os.read(fd, 4096)
                except OSError: return
                if not chunk: return
                buf += chunk
                if re.search(pat, buf.decode(errors="replace").split("\n")[-1]): return
    prompt = r"(Enter your choice: |Enter state to (insert|delete|search): )\x1b\[0m$"
    for line in inputs:
        read_until(prompt)
        os.write(fd, (line + "\n").encode())
    read_until(r"Goodbye", 2)
    time.sleep(0.2)
    try:
        while True:
            r, _, _ = select.select([fd], [], [], 0.2)
            if not r: break
            c = os.read(fd, 4096)
            if not c: break
            buf += c
    except OSError: pass
    os.waitpid(pid, 0)
    return buf.decode(errors="replace").replace("\r\n", "\n").replace("\r", "")

font = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 16)
bold = ImageFont.truetype("/usr/share/fonts/truetype/dejavu/DejaVuSansMono-Bold.ttf", 16)
COL = {"0": (204,204,204), "1;31": (240,90,90), "1;32": (80,210,130),
       "1;33": (240,220,80), "1;34": (90,150,250), "1;36": (80,200,230)}

def render(lines, path, title):
    lh = 22; top = 42
    img = Image.new("RGB", (820, top + lh*len(lines) + 16), (30,30,30))
    d = ImageDraw.Draw(img)
    d.rectangle((0,0,820,30), fill=(55,55,55))
    for i,c in enumerate([(237,106,94),(245,191,79),(98,197,84)]):
        d.ellipse((12+i*20,9,24+i*20,21), fill=c)
    d.text((410 - font.getlength(title)/2, 6), title, font=font, fill=(200,200,200))
    y = top
    for ln in lines:
        x = 15; c = COL["0"]; f = font
        for part in re.split(r"(\x1b\[[0-9;]*m)", ln):
            m = re.match(r"\x1b\[([0-9;]*)m", part)
            if m:
                c = COL.get(m.group(1), COL["0"]); f = font if m.group(1) == "0" else bold; continue
            d.text((x,y), part, font=f, fill=c); x += f.getlength(part)
        y += lh
    img.save(path, optimize=True)

def coloured_lines(text):
    cur = "\x1b[0m"; out = []
    for ln in text.split("\n"):
        out.append(cur + ln)
        codes = re.findall(r"\x1b\[[0-9;]*m", ln)
        if codes: cur = codes[-1]
    return out

# Screenshot 1: menu + ascending sort
t = capture(["1", "7"])
ls = coloured_lines(t)
s = next(i for i,l in enumerate(ls) if "Malaysian States" in l)
e = next(i for i,l in enumerate(ls) if "16. " in l)
render(ls[s:e+1], "docs/screenshots/sort_ascending.png", "states - sort A-Z")

# Screenshot 2: insert / search / delete / validation
inp = ["4","Cyberjaya","4","perak","6","la","5","pe","5","Sabah","6","Atlantis","abc","7"]
t = capture(inp)
keep = ("Enter","inserted","deleted","Match","No matching","Invalid","already","More than","Goodbye")
ls = [l for l in coloured_lines(t) if any(k in l for k in keep)]
render(ls, "docs/screenshots/insert_delete_search.png", "states - insert, search, delete")

# Screenshot 3: the submitted version's delete bug (needs `make original`)
if os.path.exists("./build/original_submitted"):
    t = capture(["4", "", "4", "a", "6"], "./build/original_submitted")
    keep = ("Enter your choice","Enter state to delete","deleted","Goodbye")
    ls = [l for l in coloured_lines(t) if any(k in l for k in keep)]
    render(ls, "docs/screenshots/original_delete_bug.png", "submitted version - delete bug")
