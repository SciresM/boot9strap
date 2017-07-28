import binascii, hashlib, struct, sys, os.path
from Crypto.Cipher import AES

# Signatures
perfect_signature = 'B6724531C448657A2A2EE306457E350A10D544B42859B0E5B0BED27534CCCC2A4D47EDEA60A7DD99939950A6357B1E35DFC7FAC773B7E12E7C1481234AF141B31CF08E9F62293AA6BAAE246C15095F8B78402A684D852C680549FA5B3F14D9E838A2FB9C09A15ABB40DCA25E40A3DDC1F58E79CEC901974363A946E99B4346E8A372B6CD55A707E1EAB9BEC0200B5BA0B661236A8708D704517F43C6C38EE9560111E1405E5E8ED356C49C4FF6823D1219AFAEEB3DF3C36B62BBA88FC15BA8648F9333FD9FC092B8146C3D908F73155D48BE89D72612E18E4AA8EB9B7FD2A5F7328C4ECBFB0083833CBD5C983A25CEB8B941CC68EB017CE87F5D793ACA09ACF7'
dev_perfect_signature = '88697CDCA9D1EA318256FCD9CED42964C1E98ABC6486B2F128EC02E71C5AE35D63D3BF1246134081AF68754787FCB922571D7F61A30DE4FCFA8293A9DA512396F1319A364968464CA9806E0A52567486754CDDD4C3A62BDCE255E0DEEC230129C1BAE1AE95D786865637C1E65FAE83EDF8E7B07D17C0AADA8F055B640D45AB0BAC76FF7B3439F5A4BFE8F7E0E103BCE995FAD913FB729D3D030B2644EC48396424E0563A1B3E6A1F680B39FC1461886FA7A60B6B56C5A846554AE648FC46E30E24678FAF1DC3CEB10C2A950F4FFA2083234ED8DCC3587A6D751A7E9AFA06156955084FF2725B698EB17454D9B02B6B76BE47ABBE206294366987A4CAB42CBD0B'
ntr_perfect_signature = '37E96B10BAF28C74A710EF35824C93F5FBB341CEE4FB446CE4D290ABFCEFACB063A9B55B3E8A65511D900C5A6E9403AAB5943CEF3A1E882B77D2347942B9E9EB0D7566370F0CB7310C38CB4AC940D1A6BB476BCC2C487D1C532120F1D2A37DDB3E36F8A2945BD8B16FB354980384998ECC380CD5CF8530F1DAD2FD74BA35ACB9C9DA2C131CB295736AE7EFA0D268EE01872EF033058ABA07B5C684EAD60D76EA84A18D866307AAAAB764786E396F2F8B630E60E30E3F1CD8A67D02F0A88152DE7A9E0DD5E64AB7593A3701E4846B6F338D22FD455D45DF212C5577266AA8C367AE6E4CE89DF41691BF1F7FE58F2261F5D251DF36DE9F5AF1F368E650D576810B'
dev_ntr_perfect_signature = '18722BC76DC3602E2C0171F3BCA12AB40EA6D112AEFBECF4BE7A2A58FF759058A93C95CDA9B3B676D09A4E4C9E842E5C68229A6A9D77FAC76445E78EB5B363F8C66B166BE65AFAE40A1485A364C2C13B855CEEDE3DFEACEC68DD6B8687DD6DF8B6D3213F72252E7C03C027EE6079F9C5E0290E5DB8CA0BBCF30FCAD72EB637A170C4A2F41D96BF7D517A2F4F335930DC5E9792D78EDFB51DC79AD9D7A4E7F1ED4D5A5C621B6245A7F1652256011DC32C49B955304A423009E2B78072CEBC12B385B72F926F19318D64075F09278FBA8448FD2484B82654A55D064542A8F5D9F9828CDA5E60D31A40CF8EF18D027310DA4F807988BC753C1EB3B3FC06207E84DE'

# Keys
ntr_key = '07550C970C3DBD9EDDA9FB5D4C7FB713'
dev_ntr_key = '4DAD2124C2D32973100FBFBD1604C6F1'

def encrypt_firm_section(section, iv, is_dev=False):
    key = dev_ntr_key if is_dev else ntr_key
    aes = AES.new(binascii.unhexlify(key), AES.MODE_CBC, iv)
    return aes.encrypt(section)

def build_b9s_firm(signature, is_dev=False, ntr_crypt=False):
    sections = ['build/code11.bin', 'build/code9.bin', 'build/NDMA.bin', 'build/dabrt.bin']
    section_datas = []
    load_addresses = [0x1FF80000, 0x08000200, 0x10002000, 0xC0000000]
    for section in sections:
        if not os.path.isfile(section):
            print ('Failed to build FIRM: missing file %s!' % section)
            return -1
        with open(section, 'rb') as f:
            section_datas.append(f.read())
    # Write FIRM header.
    b9s = b'FIRM'
    # Write (zero (boot priority)), ARM11 Entrypoint, ARM9 Entrypoint
    b9s += struct.pack('<III', 0x00000000, 0x1FF80200, 0x08001000)
    b9s += b'\x00' * 0x2C
    # Write version value
    b9s += b'\x02'
    # Write boot9strap magic value
    b9s += b'B9S'
    ofs = 0x200
    section_ofs_lst = []
    for i,data in enumerate(section_datas):
        section_ofs_lst.append(ofs)
        b9s += struct.pack('<IIII', ofs, load_addresses[i], len(data), 0x00000002)
        b9s += (hashlib.sha256(data).digest())
        ofs += len(data)
    b9s += binascii.unhexlify(signature)
    for i,data in enumerate(section_datas):
        if ntr_crypt:
            iv = struct.pack('<IIII', section_ofs_lst[i], load_addresses[i], len(data), len(data))
            b9s += encrypt_firm_section(data, iv, is_dev)
        else:
            b9s += data
    return b9s

def main(argc, argv):
    firms = ['out/boot9strap.firm', 'out/boot9strap_dev.firm', 'out/boot9strap_ntr.firm', 'out/boot9strap_ntr_dev.firm']
    sigs = [perfect_signature, dev_perfect_signature, ntr_perfect_signature, dev_ntr_perfect_signature]
    for firm, sig in zip(firms, sigs):
        b9s = build_b9s_firm(sig, is_dev='dev' in firm, ntr_crypt='ntr' in firm)
        if type(b9s) != bytes:
            return
        with open(firm, 'wb') as f:
            f.write(b9s)
        with open('%s.sha' % firm, 'wb') as f:
            f.write(hashlib.sha256(b9s).digest())
        print ('Successfully built %s!' % firm)

if __name__ == '__main__':
    main(len(sys.argv), sys.argv)
