import socket
import struct 
import tkinter
import tkinter.filedialog
import argparse, sys
from pathlib import Path

def send_file(filename: str, host: str, port: int):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((host, port))

        fp = Path(filename)
        with open(fp, "rb") as f:
            data = f.read()

        fnameSize = len(fp.name.encode("utf-8"))
        s.sendall(struct.pack("<I", fnameSize))
        s.sendall(fp.name.encode("utf-8"))

        size = len(data)
        s.sendall(struct.pack("<I", size))
        s.sendall(data)
        print(f"Sent {size} bytes")

if __name__ == "__main__":
    host = ""
    if len(sys.argv) >= 2:
        parser = argparse.ArgumentParser(description = "Send scripts to LunaCore plugin")
        parser.add_argument("-a", "--address", help="The host address that LunaCore shows", required=False)
        parser.add_argument("filename")
        args = parser.parse_args()
        host = args.address
        filename = args.filename
    elif len(sys.argv) == 1:    
        tkinter.Tk().withdraw()
        filename = tkinter.filedialog.askopenfilename(filetypes=[("Lua scripts", "*.lua")])
        if len(filename) > 0:
            host = input("Enter the host: ")

    port = 5432
    if host != "":
        send_file(filename, host, port)