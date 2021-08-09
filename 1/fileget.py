import sys
import argparse
import mylib.myfunctions as mf

# Spracovanie argumentov
args = mf.parse_arguments()

# Vrati mi IP adresu a port file serveru
file_server = mf.get_fs_info(args)

# Zisk suborov
files = mf.get_files(args.f, file_server)