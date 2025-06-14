import socket
import struct 
import tkinter
import tkinter.filedialog
from pathlib import Path

def send_file(filename: str, host: str, port: int):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.connect((host, port))

        with open(filename, "rb") as f:
            data = f.read()


        fnameSize = len(("tcp."+Path(filename).name).encode("utf-8"))
        s.sendall(struct.pack("<I", fnameSize))
        s.sendall(("tcp."+Path(filename).name).encode("utf-8"))

        size = len(data)
        s.sendall(struct.pack("<I", size))
        s.sendall(data)
        print(f"Sent {size} bytes")

if __name__ == "__main__":
    tkinter.Tk().withdraw()
    filename = tkinter.filedialog.askopenfilename(filetypes=[("Lua scripts", "*.lua")])

    host = input("Enter the host: ")
    port = 5432

    send_file(filename, host, port)