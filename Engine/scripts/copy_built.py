import os
import shutil
import glob
import sys

source_dir = sys.argv[1]
dest_dir = sys.argv[2]
ext = sys.argv[3]

os.makedirs(dest_dir, exist_ok=True)

for filepath in glob.glob(os.path.join(source_dir, f'*.{ext}')):
    shutil.copy2(filepath, dest_dir)
    
print("Post process copying done!")