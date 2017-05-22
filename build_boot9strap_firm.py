import binascii, hashlib, struct, sys, os.path

perfect_signature = 'B6724531C448657A2A2EE306457E350A10D544B42859B0E5B0BED27534CCCC2A4D47EDEA60A7DD99939950A6357B1E35DFC7FAC773B7E12E7C1481234AF141B31CF08E9F62293AA6BAAE246C15095F8B78402A684D852C680549FA5B3F14D9E838A2FB9C09A15ABB40DCA25E40A3DDC1F58E79CEC901974363A946E99B4346E8A372B6CD55A707E1EAB9BEC0200B5BA0B661236A8708D704517F43C6C38EE9560111E1405E5E8ED356C49C4FF6823D1219AFAEEB3DF3C36B62BBA88FC15BA8648F9333FD9FC092B8146C3D908F73155D48BE89D72612E18E4AA8EB9B7FD2A5F7328C4ECBFB0083833CBD5C983A25CEB8B941CC68EB017CE87F5D793ACA09ACF7'
dev_perfect_signature = '88697CDCA9D1EA318256FCD9CED42964C1E98ABC6486B2F128EC02E71C5AE35D63D3BF1246134081AF68754787FCB922571D7F61A30DE4FCFA8293A9DA512396F1319A364968464CA9806E0A52567486754CDDD4C3A62BDCE255E0DEEC230129C1BAE1AE95D786865637C1E65FAE83EDF8E7B07D17C0AADA8F055B640D45AB0BAC76FF7B3439F5A4BFE8F7E0E103BCE995FAD913FB729D3D030B2644EC48396424E0563A1B3E6A1F680B39FC1461886FA7A60B6B56C5A846554AE648FC46E30E24678FAF1DC3CEB10C2A950F4FFA2083234ED8DCC3587A6D751A7E9AFA06156955084FF2725B698EB17454D9B02B6B76BE47ABBE206294366987A4CAB42CBD0B'

def main(argc, argv):
    sections = ['build/code11.bin', 'build/code9.bin', 'build/NDMA.bin', 'build/dabrt.bin']
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
        # Write (zero (boot priority)), ARM11 Entrypoint, ARM9 Entrypoint
        b9s.write(struct.pack('<III', 0x00000000, 0x1FF80200, 0x08001000))
        b9s.write(b'\x00' * 0x2D)
        # Write boot9strap magic value
        b9s.write(b'B9S')
        ofs = 0x200
        for i,data in enumerate(section_datas):
            b9s.write(struct.pack('<IIII', ofs, load_addresses[i], len(data), 0x00000002))
            b9s.write(hashlib.sha256(data).digest())
            ofs += len(data)
        b9s.write(binascii.unhexlify(perfect_signature))
        for data in section_datas:
            b9s.write(data)
    with open('out/boot9strap.firm', 'rb') as f:
        b9s = f.read()
    b9s_dev = b9s[:0x100] + binascii.unhexlify(dev_perfect_signature) + b9s[0x200:]
    with open('out/boot9strap.firm.sha', 'wb') as f:
        f.write(hashlib.sha256(b9s).digest())
    print ('Successfully built out/boot9strap.firm!')
    with open('out/boot9strap_dev.firm', 'wb') as f:
        f.write(b9s_dev)
    with open('out/boot9strap_dev.firm.sha', 'wb') as f:
        f.write(hashlib.sha256(b9s_dev).digest())
    print ('Successfully built out/boot9strap_dev.firm!')

if __name__ == '__main__':
    main(len(sys.argv), sys.argv)
