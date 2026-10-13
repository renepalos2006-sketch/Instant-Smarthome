# Findet SDL2 auf dem Mac automatisch (Homebrew), damit nichts von Hand
# in platformio.ini eingetragen werden muss. Auf Windows/Linux passiert hier nichts.
Import("env")
import subprocess
import sys

if sys.platform == "darwin":
    try:
        prefix = subprocess.check_output(["brew", "--prefix"], text=True).strip()
    except Exception:
        prefix = "/opt/homebrew"
    env.Append(CPPPATH=[prefix + "/include"], LIBPATH=[prefix + "/lib"])
