def makeHeader(modScheme=3, fecScheme=63, payloadLen=119, golay=False):
    header = int(0x00)
    header = header | (modScheme & 0x07)
    header = header << 6
    header = header | (fecScheme & 0x3F)
    print("0b{:09b}".format(header))

def main():
    print("Hello World!")
    makeHeader(3,63,119,False)
    makeHeader(3,0,119,False)

if __name__ == "__main__":
    main()
