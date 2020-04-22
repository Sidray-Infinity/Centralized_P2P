
# Lets write DEFLATE and INFLATE
# gzip : 1015 -> 601 ; 1k
# huffman : 1015 -> 970

from heapq import heappush, heappop, heapify
from bitstring import BitStream, BitArray, ConstBitStream
import sys
import struct

def calculate_frequencies(f):
    """
    Calculates the frequency of each character in the file f
    """
    res = {'0': 0, '1': 0, '2': 0, '3': 0, '4': 0,
           '5': 0, '6': 0, '7': 0, '8': 0, '9': 0,
           'a': 0, 'b': 0, 'c': 0, 'd': 0, 'e': 0, 'f': 0}

    int count = 0

    while True:
        print(count, end='\r', flush = True)
        count += 1
        c = f.read(1)
        if not c:
            print()
            break
        if c != '\n':
            res[c] += 1
    return res


if __name__ == "__main__":
    filename = "test-data.hex"
    test_file = "1G.data"

    with open(test_file, 'r+') as f:
        import time

        s = time.time()
        freq = calculate_frequencies(f)
        # print("TIME TAKEN: ", time.time() - s)
        # print(freq)

        # print(sorted(freq.items(), key=lambda x: x[1]))
        final_freq = sorted(freq.items(), key=lambda x: x[1])

        # Building the Huffman prefix tree

        heap = [[x[1], [x[0], ""]] for x in final_freq]
        heapify(heap)
        while len(heap) > 1:
            lo = heappop(heap)
            hi = heappop(heap)
            for pair in lo[1:]:
                pair[1] = '0' + pair[1]
            for pair in hi[1:]:
                pair[1] = '1' + pair[1]
            heappush(heap, [lo[0] + hi[0]] + lo[1:] + hi[1:])

        heap = sorted(heappop(heap)[1:], key=lambda p: (len(p[-1]), p))
        mapper = dict(heap)

        # Writing the encoded data to a file

        print(mapper)

        
        f.seek(0, 0)


        with open("1G-encoded.data", "wb") as fe:
            fe.seek(0)
            prev = ""
            char_count = 0
            write_count = 0
            while True:

                c = f.read(1)
                if not c: 
                    break
                
                if c == '\n':
                    continue            

                char_count += 1    
                bit_str = mapper[c] + prev

                if len(bit_str) == 8:
                    prev = ""
                    write_count += 1
                    final = struct.pack('!H', int(bit_str, 2))
                    print(final)
                    fe.write(final)
                elif len(bit_str) > 8:
                    write_count += 1
                    final = struct.pack('!H', int(bit_str[:8], 2))
                    print(final)
                    fe.write(final)
                    prev = bit_str[8:]
                else:
                    prev = mapper[c]

                print(write_count, char_count)
                
                # to_write = mapper[c[0]] + mapper[c[1]]
                # to_write = list(map(int, list(to_write)))
                # #print(to_write)
                # print(bytes(to_write))
                # to_write = list(map(int, list(mapper[c])))
                # fe.write(bytes(to_write))

            fe.close()
            f.close()


        
