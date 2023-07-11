#!/usr/bin/python

# Initial python code used to generate data for the graphs but is far too slow for large wordlists

import sys
import math

data = [
    "`1234567890-= ",
    " qwertyuiop[]\\",
    " asdfghjkl;   ",
    " zxcvbnm,./   ",
]

data_caps = [
    "~!@#$%^&*()_+ ",
    " QWERTYUIOP{}|",
    ' ASDFGHJKL:"  ',
    " ZXCVBNM<>?   ",
]

def get_position(c):
    global data, data_caps

    for i in range(len(data)):
        for j in range(len(data[i])):
            if c == data[i][j] or c == data_caps[i][j]:
                return i, j
    return -1, -1

def find_sequences_old(s, word):
    d = {}

    for l in range(2, len(word) // 2):
        for i in range(len(s)):
            sub = word[i:i+l]
            d[sub] = word.count(sub)
        if d.get(sub, 0) == 0:
            break
    return d


def find_longest_sequence(s, word):
    d = {}

    # find substrings 2 to half the size
    for l in range(math.floor(len(s) / 2), 2, -1):
        sub = s[0:l]
        k = 0
        match = True
        for i in range(l, len(s) - l):
            if sub[k] != s[i]:
                match = False
                break
        if match:
            return l

    return 1

def find_sequences(s, word):
    d = {}

    substr = ''
    for x in s:
        substr += '%d,' % (x)

    # find substrings 2 to half the size
    #for l in range(2, math.ceil(len(s) / 2) + 1):
    for l in range(math.floor(len(s) / 2), 1, -1):
        for i in range(0, len(s) - l):
            sub = s[i:i+l]
            substr = repr(sub)
            if d.get(substr):
                continue
            j = i + l
            while j < len(s):
                if sub == s[j:j+l]:
                    d[substr] = d.get(substr, 1) + 1
                    #print('%s l=%d j=%d i=%d %d %d' % (substr, l, j, i, j, j+l))
                    j += l
                else:
                    j += 1

    return d

def check_word(word):
    if word.find('|') != -1:
        return
    if len(word) == 0:
        return 
    data = ''
    data = '%s|' % word
    total = 0

    lx = 0
    ly = 0
    s = []
    for i in range(len(word)):
        y, x = get_position(word[i])
        if x == -1 or y == -1:
            continue

        if i > 0:
            xd = abs(x - lx)
            yd = abs(y - ly)
            distance = xd + yd
            total += distance
            if i > 1:
                data += ',%d' % (distance)
            else:
                data += '%d' % (distance)
            s.append(distance)
            
        lx = x
        ly = y

    #data += ') len=%d' % (len(word))
    #data += ' total=%d' % (total)
    if len(s) > 0:
        data += '|%.1f' % (total / len(s))

        ones = s.count(1)
        data += '|%.1f' % (ones / len(s))

        i = 0
        current = 0
        maxrun = 0
        for i in range(len(s)):
            if s[i] <= 1:
                current += 1
                if current > maxrun:
                    maxrun = current
            else:
                current = 0

        data += '|%d' % (maxrun)
        #data += ' mean=%d' % (total / len(s))
    else:
        data += '|NA|NA|NA'

    d = find_sequences(s, word)
    keys = d.keys()
    sorted(keys, key=lambda x: d[x])
    maxscore = 0
    for k in keys:
        z = len(k.split(','))
        #print('%s %d x %d = %d' % (k, z, d[k], z * d[k]))
        score = len(k.split(',')) * d[k]
        if score > maxscore:
            maxscore = score
        # used for unique sequences
        # col = k.split()
        #data += ' seq(%d)=%d' % (k, d[k])
        #score += d[k] * k
        #print('\t', k, d[k])
    #data += ' score=%.2f' % (score / len(word))
    longest = find_longest_sequence(s, word)
    data += '|%d|%d|%d|%.2f' % (longest, len(word), total, maxscore / len(word))
    print(data)


def main():
    words = []

    buffer = ''
    with open(sys.argv[1], 'rb') as f:
        buffer = f.read()
    for line in buffer.decode('utf-8', errors='ignore').split('\n'):
        words.append(line)

    print('password|path|mean|ones|maxrun|longest|len|total|score')
    for word in words:
        check_word(word)


if __name__ == '__main__':
    main()

