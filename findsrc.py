#!/usr/bin/python3

import os
import sys
import argparse
import itertools as it

class HeaderNotFound (Exception):
    """ Exception to warn about header not found """

    def __init__ (self, header, fname, line):
        """ Constructs HeaderNotFound exception """
        super().__init__(
            "Header {} not found at {}@{}".format(header, fname, line)
        )

def fetch_headers (fname):
    """ Fetch all headers from a file """

    # Gets file directory
    fdir = os.path.dirname(fname)
    lineno = 0

    with open(fname) as file:
        # Iterates over trimmed lines
        for line in map(str.strip, file):
            lineno += 1

            if not line.startswith("#include"):
                continue

            # Removes #include
            line = line[ 8 : ].lstrip()

            # Removes comments
            # NOTE only single line comments suported
            comm = line.find("//")

            if comm != -1:
                line = line[ : comm ].rstrip()

            # Gets only headers between quotes (assumes that they are local)
            if line[0] == "\"" and line[-1] == "\"":
                # Removes quotes
                hname = line[ 1 : -1 ]

                # Appends file directory to header location
                header = os.path.join(fdir, hname)

                if not os.path.isfile(header):
                    raise HeaderNotFound(hname, fname, lineno)

                yield header

# Build argument parser
argparser = argparse.ArgumentParser(prog = os.path.basename(__file__))

argparser.add_argument("files", nargs = "+", help = "Starting files.")
argparser.add_argument("-headers", default = None, nargs = "+", help = "Supported header extensions.")
argparser.add_argument("-sources", default = [ "cc", "cpp" ], nargs = "+", help = "Supported source extensions.")
argparser.add_argument("-ignore", default = [], nargs = "+", help = "Files to ignore.")
argparser.add_argument("-basepath", default = ".", help = "Base directory for files.")
argparser.add_argument("-print-headers", action = "store_true", help = "Also print headers alongsize results.")

def main (argv):
    """ Main function """

    # Parse arguments
    args = argparser.parse_args(argv)
    headers = set(args.headers) if args.headers else None
    sources = set(ext.lstrip(".") for ext in args.sources)

    result = []
    files = list(args.files)
    found = { os.path.relpath(ign, args.basepath) for ign in args.ignore }

    # Iterate over files
    while files:
        fname = os.path.relpath(files.pop(), args.basepath)

        # Ignore duplicate files
        if fname in found:
            continue

        # Test if file is valid
        found.add(fname)
        name, ext = os.path.splitext(fname)
        ext = ext.lstrip(".")

        # Add file to found sources
        if ext in sources:
            result.append(fname)

        # Skips unsupported headers
        elif headers is not None and ext not in headers:
            continue

        else:
            if args.print_headers:
                result.append(fname)

            # Fetch sources by name
            for ext in sources:
                sname = "{}.{}".format(name, ext)

                if os.path.isfile(sname):
                    files.append(sname)

        try:
            # Fetch headers from file
            files.extend(fetch_headers(fname))

        except HeaderNotFound as e:
            print(e, file = sys.stderr)
            exit(1)

    # Print results
    print(" ".join(result))

if __name__ == "__main__":
    main(sys.argv[ 1 : ])
