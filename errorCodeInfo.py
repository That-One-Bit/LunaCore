import sys

def main(errCode: int):
    # u32 errorCode = (1 << core_state | 1 << (8 + plg_state) | 1 << (12 + game_state) | possibleOOM << 15 | luaEnvBusy << 16);
    core_state = -1
    for i in range(8):
        if errCode & (1 << i):
            core_state = i
    print(f"Core state: {core_state}")

if __name__ == "__main__":
    code = 0
    if len(sys.argv) == 1:
        hexStr = input("Enter the error code: ")
        code = int(hexStr, 16)
    else:
        code = int(sys.argv[1], 16)
    main(code)