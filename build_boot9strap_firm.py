import binascii, hashlib, struct, sys, os.path

perfect_signature = 'B6724531C448657A2A2EE306457E350A10D544B42859B0E5B0BED27534CCCC2A4D47EDEA60A7DD99939950A6357B1E35DFC7FAC773B7E12E7C1481234AF141B31CF08E9F62293AA6BAAE246C15095F8B78402A684D852C680549FA5B3F14D9E838A2FB9C09A15ABB40DCA25E40A3DDC1F58E79CEC901974363A946E99B4346E8A372B6CD55A707E1EAB9BEC0200B5BA0B661236A8708D704517F43C6C38EE9560111E1405E5E8ED356C49C4FF6823D1219AFAEEB3DF3C36B62BBA88FC15BA8648F9333FD9FC092B8146C3D908F73155D48BE89D72612E18E4AA8EB9B7FD2A5F7328C4ECBFB0083833CBD5C983A25CEB8B941CC68EB017CE87F5D793ACA09ACF7'

def main(argc, argv):
    sections = ['out/code11.bin', 'out/code9.bin', 'out/NDMA.bin', 'out/dabrt.bin']
    section_datas = []
    load_addresses = [0x1FF80000, 0x08000200, 0x10002000, 0xC0000000]
    for section in sections:
        if not os.path.isfile(section):
            print ('Failed to build FIRM: missing file %s!' % section)
            return -1
        with open(section, 'rb') as f:
            section_datas.append(f.read())
    with open('out/boot9strap.firm', 'wb') as b9s:
        # Write FIRM header.
        b9s.write(b'FIRM')
        # Write (zero), ARM11 Entrypoint, ARM9 Entrypoint
        b9s.write(struct.pack('<III', 0x00000000, 0x1FF80200, 0x08010000))
        b9s.write(b'\x00' * 0x30)
        ofs = 0x200
        for i,data in enumerate(section_datas):
            b9s.write(struct.pack('<IIII', ofs, load_addresses[i], len(data), 0x00000002))
            b9s.write(binascii.unhexlify(hashlib.sha256(data).hexdigest()))
            ofs += len(data)
        b9s.write(binascii.unhexlify(perfect_signature))
        for data in section_datas:
            b9s.write(data)
    with open('out/boot9strap.firm', 'rb') as f:
        b9s = f.read()
    with open('out/boot9strap.firm.sha', 'wb') as f:
        f.write(binascii.unhexlify(hashlib.sha256(b9s).hexdigest()))
    print ('Successfully built out/boot9strap.firm!')


if __name__ == '__main__':
    main(len(sys.argv), sys.argv)