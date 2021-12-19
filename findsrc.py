#!/usr/bin/env python3

import sys
import argparse
from pathlib import Path
from collections.abc import Generator, Sequence


class HeaderNotFoundError (Exception):
    """ Exception to warn about header not found """

    def __init__ (self, header: str, fname: Path, line: int) -> None:
        """ Constructs HeaderNotFound exception """
        super().__init__(f"Header {header} not found at {fname}@{line}.")

def fetch_headers (fname: Path) -> Generator[Path, None]:
    """ Fetch all headers from a file """

    # Gets file directory
    fdir = fname.parent
    lineno = 0

    with fname.open("rt") as file:
        # Iterates over trimmed lines
        for line in map(str.lstrip, file):
            lineno += 1

            if not line.startswith("#include"):
                continue

            # Removes #include
            line = line[ 8 : ].lstrip()

            # Removes comments
            # NOTE only single line comments suported
            comm = line.find("//")

            if comm != -1:
                line = line[ : comm ]

            line = line.rstrip()

            # Gets only headers between quotes (assumes that they are local)
            if line[0] == "\"" and line[-1] == "\"":
                # Removes quotes
                hname = line[ 1 : -1 ]

                # Appends file directory to header location
                header = fdir / hname

                if not header.is_file():
                    raise HeaderNotFoundError(hname, fname, lineno)

                yield header

# Build argument parser
argparser = argparse.ArgumentParser(prog=Path(__file__).name)

argparser.add_argument("files", type=Path, nargs="+", help="Starting files.")
argparser.add_argument(
    "-he", "--headers", default=None, nargs="+", help="Supported header extensions."
)
argparser.add_argument(
    "-se", "--sources", default=[ "cc", "cpp" ], nargs="+", help="Supported source extensions."
)
argparser.add_argument("-i", "--ignore", type=Path, default=[], nargs="+", help="Files to ignore.")
argparser.add_argument(
    "-b", "--basepath", type=Path, default=Path.cwd(), help="Base directory for files."
)
argparser.add_argument(
    "-p", "--print-headers", action="store_true", help="Also print headers alongsize results."
)

def main (argv: Sequence[str]) -> None:
    """ Main function """

    # Parse arguments
    args = argparser.parse_args(argv)
    headers: frozenset[str] = frozenset(args.headers) if args.headers else None
    sources: frozenset[str] = frozenset(ext.lstrip(".") for ext in args.sources)

    result: list[Path] = []
    files: list[Path] = list(args.files)
    found: set[Path] = { ign.relative_to(args.basepath) for ign in args.ignore }

    # Iterate over files
    while files:
        fname = files.pop().resolve().relative_to(args.basepath)

        # Ignore duplicate files
        if fname in found:
            continue

        # Test if file is valid
        found.add(fname)
        ext = fname.suffix.lstrip(".")

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
                sname = fname.with_suffix(f".{ext}")

                if sname.is_file():
                    files.append(sname)

        try:
            # Fetch headers from file
            files.extend(fetch_headers(fname))

        except HeaderNotFoundError as e:
            print(e, file=sys.stderr)
            exit(1)

    # Print results
    print(" ".join(map(str, result)))

if __name__ == "__main__":
    main(sys.argv[ 1 : ])
