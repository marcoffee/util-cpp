#!/usr/bin/python3

import os
import sys
import argparse
import itertools as it

def fetch_headers (fname):
    with open(fname) as file:
        for line in map(str.strip, file):
            if not line.startswith("#include"):
                continue

            line = line[ 8 : ].lstrip()
            comm = line.find("//")

            if comm != -1:
                line = line[ : comm ].rstrip()

            if line[0] == "\"" and line[-1] == "\"":
                yield line[ 1 : -1 ]

argparser = argparse.ArgumentParser()

argparser.add_argument("files", type = str, nargs = "+")
argparser.add_argument("-headers", type = str, default = None, nargs = "+")
argparser.add_argument("-sources", type = str, default = [ "cc", "cpp" ], nargs = "+")
argparser.add_argument("-ignore", type = str, default = [], nargs = "+")
argparser.add_argument("-basepath", type = str, default = ".")

def main (argv):
    args = argparser.parse_args(argv)
    headers = set(args.headers) if args.headers else None
    sources = set(ext.lstrip(".") for ext in args.sources)

    result = []
    files = list(args.files)
    found = { os.path.relpath(ign, args.basepath) for ign in args.ignore }

    while files:
        file = os.path.relpath(files.pop(), args.basepath)

        if file in found:
            continue

        found.add(file)
        name, ext = os.path.splitext(file)
        ext = ext.lstrip(".")

        if ext in sources:
            result.append(file)

        elif headers is not None and ext not in headers:
            continue

        else:
            for ext in sources:
                fname = "{}.{}".format(name, ext)

                if os.path.isfile(fname):
                    files.append(fname)

        fpath = os.path.dirname(file)
        files.extend(os.path.join(fpath, f) for f in fetch_headers(file))

    print(" ".join(result))

if __name__ == "__main__":
    main(sys.argv[ 1 : ])
